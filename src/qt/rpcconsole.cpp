#include "rpcconsole.h"
#include "main.h"
#include "guiconstants.h"
#include <QTimer>
#include "ui_rpcconsole.h"

#include "clientmodel.h"
#include "bitcoinrpc.h"
#include "guiutil.h"

#include <QTime>
#include <QTimer>
#include <QThread>
#include <QTextEdit>
#include <QKeyEvent>
#include <QUrl>
#include <QScrollBar>
#include <QTableWidget>
#include <QTextBrowser>
#include <QHeaderView>
#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

#include "net.h"
#include "trafficgraphwidget.h"

#include <openssl/crypto.h>

// TODO: make it possible to filter out categories (esp debug messages when implemented)
// TODO: receive errors and debug messages through ClientModel

const int CONSOLE_SCROLLBACK = 50;
const int CONSOLE_HISTORY = 50;

const QSize ICON_SIZE(24, 24);

// Peers tab refresh and traffic sampling interval. Traffic is sampled even
// while the window is closed (two counter reads), so the graph has history.
static const int PEER_REFRESH_MS = 2000;

enum PeerColumn { PC_ADDRESS, PC_DIRECTION, PC_VERSION, PC_HEIGHT, PC_PING, PC_SENT, PC_RECV, PC_COUNT };

/** Table cell that sorts by a number stored in Qt::UserRole when there is one,
 * so "1.2 MB" sorts after "900 kB" */
class PeerItem : public QTableWidgetItem
{
public:
    explicit PeerItem(const QString &text) : QTableWidgetItem(text) {}
    PeerItem(const QString &text, double sortKey) : QTableWidgetItem(text) { setData(Qt::UserRole, sortKey); }
    bool operator<(const QTableWidgetItem &other) const
    {
        QVariant a = data(Qt::UserRole), b = other.data(Qt::UserRole);
        if (a.isValid() && b.isValid())
            return a.toDouble() < b.toDouble();
        return QTableWidgetItem::operator<(other);
    }
};

const struct {
    const char *url;
    const char *source;
} ICON_MAPPING[] = {
    {"cmd-request", ":/icons/tx_input"},
    {"cmd-reply", ":/icons/tx_output"},
    {"cmd-error", ":/icons/tx_output"},
    {"misc", ":/icons/tx_inout"},
    {NULL, NULL}
};

/* Object for executing console RPC commands in a separate thread.
*/
class RPCExecutor: public QObject
{
    Q_OBJECT
public slots:
    void start();
    void request(const QString &command);
signals:
    void reply(int category, const QString &command);
};

#include "rpcconsole.moc"

void RPCExecutor::start()
{
   // Nothing to do
}

/**
 * Split shell command line into a list of arguments. Aims to emulate \c bash and friends.
 *
 * - Arguments are delimited with whitespace
 * - Extra whitespace at the beginning and end and between arguments will be ignored
 * - Text can be "double" or 'single' quoted
 * - The backslash \c \ is used as escape character
 *   - Outside quotes, any character can be escaped
 *   - Within double quotes, only escape \c " and backslashes before a \c " or another backslash
 *   - Within single quotes, no escaping is possible and no special interpretation takes place
 *
 * @param[out]   args        Parsed arguments will be appended to this list
 * @param[in]    strCommand  Command line to split
 */
bool parseCommandLine(std::vector<std::string> &args, const std::string &strCommand)
{
    enum CmdParseState
    {
        STATE_EATING_SPACES,
        STATE_ARGUMENT,
        STATE_SINGLEQUOTED,
        STATE_DOUBLEQUOTED,
        STATE_ESCAPE_OUTER,
        STATE_ESCAPE_DOUBLEQUOTED
    } state = STATE_EATING_SPACES;
    std::string curarg;
    foreach(char ch, strCommand)
    {
        switch(state)
        {
        case STATE_ARGUMENT: // In or after argument
        case STATE_EATING_SPACES: // Handle runs of whitespace
            switch(ch)
            {
            case '"': state = STATE_DOUBLEQUOTED; break;
            case '\'': state = STATE_SINGLEQUOTED; break;
            case '\\': state = STATE_ESCAPE_OUTER; break;
            case ' ': case '\n': case '\t':
                if(state == STATE_ARGUMENT) // Space ends argument
                {
                    args.push_back(curarg);
                    curarg.clear();
                }
                state = STATE_EATING_SPACES;
                break;
            default: curarg += ch; state = STATE_ARGUMENT;
            }
            break;
        case STATE_SINGLEQUOTED: // Single-quoted string
            switch(ch)
            {
            case '\'': state = STATE_ARGUMENT; break;
            default: curarg += ch;
            }
            break;
        case STATE_DOUBLEQUOTED: // Double-quoted string
            switch(ch)
            {
            case '"': state = STATE_ARGUMENT; break;
            case '\\': state = STATE_ESCAPE_DOUBLEQUOTED; break;
            default: curarg += ch;
            }
            break;
        case STATE_ESCAPE_OUTER: // '\' outside quotes
            curarg += ch; state = STATE_ARGUMENT;
            break;
        case STATE_ESCAPE_DOUBLEQUOTED: // '\' in double-quoted text
            if(ch != '"' && ch != '\\') curarg += '\\'; // keep '\' for everything but the quote and '\' itself
            curarg += ch; state = STATE_DOUBLEQUOTED;
            break;
        }
    }
    switch(state) // final state
    {
    case STATE_EATING_SPACES:
        return true;
    case STATE_ARGUMENT:
        args.push_back(curarg);
        return true;
    default: // ERROR to end in one of the other states
        return false;
    }
}

void RPCExecutor::request(const QString &command)
{
    std::vector<std::string> args;
    if(!parseCommandLine(args, command.toStdString()))
    {
        emit reply(RPCConsole::CMD_ERROR, QString("Parse error: unbalanced ' or \""));
        return;
    }
    if(args.empty())
        return; // Nothing to do
    try
    {
        std::string strPrint;
        // Convert argument list to JSON objects in method-dependent way,
        // and pass it along with the method name to the dispatcher.
        json_spirit::Value result = tableRPC.execute(
            args[0],
            RPCConvertValues(args[0], std::vector<std::string>(args.begin() + 1, args.end())));

        // Format result reply
        if (result.type() == json_spirit::null_type)
            strPrint = "";
        else if (result.type() == json_spirit::str_type)
            strPrint = result.get_str();
        else
            strPrint = write_string(result, true);

        emit reply(RPCConsole::CMD_REPLY, QString::fromStdString(strPrint));
    }
    catch (json_spirit::Object& objError)
    {
        try // Nice formatting for standard-format error
        {
            int code = find_value(objError, "code").get_int();
            std::string message = find_value(objError, "message").get_str();
            emit reply(RPCConsole::CMD_ERROR, QString::fromStdString(message) + " (code " + QString::number(code) + ")");
        }
        catch(std::runtime_error &) // raised when converting to invalid type, i.e. missing code or message
        {   // Show raw JSON object
            emit reply(RPCConsole::CMD_ERROR, QString::fromStdString(write_string(json_spirit::Value(objError), false)));
        }
    }
    catch (std::exception& e)
    {
        emit reply(RPCConsole::CMD_ERROR, QString("Error: ") + QString::fromStdString(e.what()));
    }
}

RPCConsole::RPCConsole(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RPCConsole),
    clientModel(0),
    historyPtr(0),
    peerStats(new std::vector<CNodeStats>())
{
    ui->setupUi(this);

    setupPeersTab();
    setupTrafficTab();
    QTimer *peerTimer = new QTimer(this);
    connect(peerTimer, SIGNAL(timeout()), this, SLOT(refreshPeers()));
    connect(peerTimer, SIGNAL(timeout()), this, SLOT(sampleTraffic()));
    peerTimer->start(PEER_REFRESH_MS);
    sampleTraffic();

#ifndef Q_OS_MAC
    ui->openDebugLogfileButton->setIcon(QIcon(":/icons/export"));
    ui->showCLOptionsButton->setIcon(QIcon(":/icons/options"));
#endif

    // Install event filter for up and down arrow
    ui->lineEdit->installEventFilter(this);
    ui->messagesWidget->installEventFilter(this);

    connect(ui->clearButton, SIGNAL(clicked()), this, SLOT(clear()));

    // set OpenSSL version label

    ui->openSSLVersion->setText(SSLeay_version(SSLEAY_VERSION));
    // ui->totalBlocks->setStyleSheet("selection-background-color: rgba(42, 216, 234, 90); selection-color: white;");

    startExecutor();

    clear();
}

RPCConsole::~RPCConsole()
{
    emit stopExecutor();
    delete peerStats;
    delete ui;
}

bool RPCConsole::eventFilter(QObject* obj, QEvent *event)
{
    if(event->type() == QEvent::KeyPress) // Special key handling
    {
        QKeyEvent *keyevt = static_cast<QKeyEvent*>(event);
        int key = keyevt->key();
        Qt::KeyboardModifiers mod = keyevt->modifiers();
        switch(key)
        {
        case Qt::Key_Up: if(obj == ui->lineEdit) { browseHistory(-1); return true; } break;
        case Qt::Key_Down: if(obj == ui->lineEdit) { browseHistory(1); return true; } break;
        case Qt::Key_PageUp: /* pass paging keys to messages widget */
        case Qt::Key_PageDown:
            if(obj == ui->lineEdit)
            {
                QApplication::postEvent(ui->messagesWidget, new QKeyEvent(*keyevt));
                return true;
            }
            break;
        default:
            // Typing in messages widget brings focus to line edit, and redirects key there
            // Exclude most combinations and keys that emit no text, except paste shortcuts
            if(obj == ui->messagesWidget && (
                  (!mod && !keyevt->text().isEmpty() && key != Qt::Key_Tab) ||
                  ((mod & Qt::ControlModifier) && key == Qt::Key_V) ||
                  ((mod & Qt::ShiftModifier) && key == Qt::Key_Insert)))
            {
                ui->lineEdit->setFocus();
                QApplication::postEvent(ui->lineEdit, new QKeyEvent(*keyevt));
                return true;
            }
        }
    }
    return QDialog::eventFilter(obj, event);
}

void RPCConsole::setClientModel(ClientModel *model)
{
    this->clientModel = model;
    if(model)
    {
        // Subscribe to information, replies, messages, errors
        connect(model, SIGNAL(numConnectionsChanged(int)), this, SLOT(setNumConnections(int)));
        // connect(model, SIGNAL(numBlocksChanged(int,int)), this, SLOT(setNumBlocks(int,int)));

        setNumBlocks(model->getNumBlocks());
        connect(model, SIGNAL(numBlocksChanged(int)), this, SLOT(setNumBlocks(int)));
        // Block count 10 times a second, read without locking the core
        QTimer *liveHeightTimer = new QTimer(this);
        connect(liveHeightTimer, SIGNAL(timeout()), this, SLOT(updateLiveHeight()));
        liveHeightTimer->start(LIVE_HEIGHT_DELAY);

        // Provide initial values
        ui->clientVersion->setText(model->formatFullVersion());
        ui->clientName->setText(model->clientName());
        ui->buildDate->setText(model->formatBuildDate());
        ui->startupTime->setText(model->formatClientStartupTime());

        setNumConnections(model->getNumConnections());
        ui->isTestNet->setChecked(model->isTestNet());

        // setNumBlocks(model->getNumBlocks(), model->getNumBlocksOfPeers());
    }
}

static QString categoryClass(int category)
{
    switch(category)
    {
    case RPCConsole::CMD_REQUEST:  return "cmd-request"; break;
    case RPCConsole::CMD_REPLY:    return "cmd-reply"; break;
    case RPCConsole::CMD_ERROR:    return "cmd-error"; break;
    default:                       return "misc";
    }
}

void RPCConsole::clear()
{
    ui->messagesWidget->clear();
    ui->lineEdit->clear();
    ui->lineEdit->setFocus();

    // Add smoothly scaled icon images.
    // (when using width/height on an img, Qt uses nearest instead of linear interpolation)
    for(int i=0; ICON_MAPPING[i].url; ++i)
    {
        ui->messagesWidget->document()->addResource(
                    QTextDocument::ImageResource,
                    QUrl(ICON_MAPPING[i].url),
                    QImage(ICON_MAPPING[i].source).scaled(ICON_SIZE, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    }

    // Set default style sheet
    // (set again on clear, so it follows the active theme)
    const QString teal = GUIUtil::themeColor(GUIUtil::TC_ACCENT).name(); // FlapX cyan / Flappy orange
    ui->messagesWidget->document()->setDefaultStyleSheet(
                "table { }"
                "td.time { color: " + GUIUtil::mutedColor() + "; padding-top: 3px; } "
                "td.message { font-family: Monospace; font-size: 12px; } "
                "td.cmd-request { color: " + teal + "; } "
                "td.cmd-error { color: " + GUIUtil::negativeColor() + "; } "
                "b { color: " + teal + "; } "
                );

    message(CMD_REPLY, (tr("Welcome to the FlapX RPC console.") + "<br>" +
                        tr("Use up and down arrows to navigate history, and <b>Ctrl-L</b> to clear screen.") + "<br>" +
                        tr("Type <b>help</b> for an overview of available commands.")), true);
}

void RPCConsole::message(int category, const QString &message, bool html)
{
    QTime time = QTime::currentTime();
    QString timeString = time.toString();
    QString out;
    out += "<table><tr><td class=\"time\" width=\"65\">" + timeString + "</td>";
    out += "<td class=\"icon\" width=\"32\"><img src=\"" + categoryClass(category) + "\"></td>";
    out += "<td class=\"message " + categoryClass(category) + "\" valign=\"middle\">";
    if(html)
        out += message;
    else
        out += GUIUtil::HtmlEscape(message, true);
    out += "</td></tr></table>";
    ui->messagesWidget->append(out);
}

void RPCConsole::setNumConnections(int count)
{
    ui->numberOfConnections->setText(QString::number(count));
}

void RPCConsole::setNumBlocks(int count)
{
    ui->numberOfBlocks->setText(QString::number(count));
    // ui->totalBlocks->setText(QString::number(countOfPeers));
    // If there is no current countOfPeers available display N/A instead of 0, which can't ever be true
    // ui->totalBlocks->setText(countOfPeers == 0 ? tr("N/A") : QString::number(countOfPeers));
    if(clientModel)
    // {
        // If there is no current number available display N/A instead of 0, which can't ever be true
    //    ui->totalBlocks->setText(clientModel->getNumBlocksOfPeers() == 0 ? tr("N/A") : QString::number(clientModel->getNumBlocksOfPeers()));
        ui->lastBlockTime->setText(clientModel->getLastBlockDate().toString());
    // }
}

void RPCConsole::updateLiveHeight()
{
    int nHeight = nBestHeightUI;
    if (nHeight >= 0 && ui->numberOfBlocks->text() != QString::number(nHeight))
        ui->numberOfBlocks->setText(QString::number(nHeight));
}

void RPCConsole::on_lineEdit_returnPressed()
{
    QString cmd = ui->lineEdit->text();
    ui->lineEdit->clear();

    if(!cmd.isEmpty())
    {
        message(CMD_REQUEST, cmd);
        emit cmdRequest(cmd);
        // Truncate history from current position
        history.erase(history.begin() + historyPtr, history.end());
        // Append command to history
        history.append(cmd);
        // Enforce maximum history size
        while(history.size() > CONSOLE_HISTORY)
            history.removeFirst();
        // Set pointer to end of history
        historyPtr = history.size();
        // Scroll console view to end
        scrollToEnd();
    }
}

void RPCConsole::browseHistory(int offset)
{
    historyPtr += offset;
    if(historyPtr < 0)
        historyPtr = 0;
    if(historyPtr > history.size())
        historyPtr = history.size();
    QString cmd;
    if(historyPtr < history.size())
        cmd = history.at(historyPtr);
    ui->lineEdit->setText(cmd);
}

void RPCConsole::startExecutor()
{
    QThread* thread = new QThread;
    RPCExecutor *executor = new RPCExecutor();
    executor->moveToThread(thread);

    // Notify executor when thread started (in executor thread)
    connect(thread, SIGNAL(started()), executor, SLOT(start()));
    // Replies from executor object must go to this object
    connect(executor, SIGNAL(reply(int,QString)), this, SLOT(message(int,QString)));
    // Requests from this object must go to executor
    connect(this, SIGNAL(cmdRequest(QString)), executor, SLOT(request(QString)));
    // On stopExecutor signal
    // - queue executor for deletion (in execution thread)
    // - quit the Qt event loop in the execution thread
    connect(this, SIGNAL(stopExecutor()), executor, SLOT(deleteLater()));
    connect(this, SIGNAL(stopExecutor()), thread, SLOT(quit()));
    // Queue the thread for deletion (in this thread) when it is finished
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

    // Default implementation of QThread::run() simply spins up an event loop in the thread,
    // which is what we want.
    thread->start();
}

void RPCConsole::on_tabWidget_currentChanged(int index)
{
    if(ui->tabWidget->widget(index) == ui->tab_console)
    {
        ui->lineEdit->setFocus();
    }
    else if(ui->tabWidget->widget(index) == peersTab)
    {
        refreshPeers();
    }
}

void RPCConsole::setupPeersTab()
{
    peersTab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(peersTab);

    peerSummary = new QLabel();
    layout->addWidget(peerSummary);

    QSplitter *splitter = new QSplitter(Qt::Vertical);
    peerTable = new QTableWidget(0, PC_COUNT);
    peerTable->setHorizontalHeaderLabels(QStringList() << tr("Address") << tr("Direction") << tr("Version")
                                         << tr("Height") << tr("Ping") << tr("Sent") << tr("Received"));
    peerTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    peerTable->setSelectionMode(QAbstractItemView::SingleSelection);
    peerTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    peerTable->setAlternatingRowColors(true);
    peerTable->setShowGrid(false);
    peerTable->verticalHeader()->hide();
    peerTable->setSortingEnabled(true);
    peerTable->sortByColumn(PC_DIRECTION, Qt::AscendingOrder);
#if QT_VERSION >= 0x050000
    peerTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    peerTable->horizontalHeader()->setSectionResizeMode(PC_ADDRESS, QHeaderView::Stretch);
#else
    peerTable->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    peerTable->horizontalHeader()->setResizeMode(PC_ADDRESS, QHeaderView::Stretch);
#endif
    connect(peerTable, SIGNAL(itemSelectionChanged()), this, SLOT(showPeerDetails()));
    splitter->addWidget(peerTable);

    peerDetails = new QTextBrowser();
    peerDetails->setOpenLinks(false);
    peerDetails->setHtml("<p style='color:" + GUIUtil::mutedColor() + "'>" + tr("Select a peer to see its details.") + "</p>");
    splitter->addWidget(peerDetails);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 2);
    layout->addWidget(splitter);

    ui->tabWidget->addTab(peersTab, tr("&Peers"));
}

void RPCConsole::setupTrafficTab()
{
    trafficTab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(trafficTab);

    trafficGraph = new TrafficGraphWidget();
    layout->addWidget(trafficGraph, 1);

    QHBoxLayout *totals = new QHBoxLayout();
    trafficRecvLabel = new QLabel();
    trafficSentLabel = new QLabel();
    totals->addWidget(trafficRecvLabel);
    totals->addStretch();
    totals->addWidget(trafficSentLabel);
    layout->addLayout(totals);

    QLabel *note = new QLabel(tr("Totals since the wallet started. The graph shows the last 10 minutes."));
    note->setObjectName("trafficNote");
    layout->addWidget(note);

    ui->tabWidget->addTab(trafficTab, tr("&Network Traffic"));
}

void RPCConsole::sampleTraffic()
{
    quint64 nRecv = CNode::GetTotalBytesRecv();
    quint64 nSent = CNode::GetTotalBytesSent();
    trafficGraph->addSample(nRecv, nSent, GetTimeMillis());

    if (isVisible() && ui->tabWidget->currentWidget() == trafficTab)
    {
        trafficRecvLabel->setText(QString("<span style='color:%1'>&#9632;</span> ").arg(GUIUtil::themeColor(GUIUtil::TC_ACCENT).name()) +
            tr("Received: %1 (%2)").arg(GUIUtil::formatBytes(nRecv), GUIUtil::formatBytesPerSecond(trafficGraph->currentRecvRate())));
        trafficSentLabel->setText(QString("<span style='color:%1'>&#9632;</span> ").arg(GUIUtil::themeColor(GUIUtil::TC_ACCENT2).name()) +
            tr("Sent: %1 (%2)").arg(GUIUtil::formatBytes(nSent), GUIUtil::formatBytesPerSecond(trafficGraph->currentSentRate())));
    }
}

void RPCConsole::refreshPeers()
{
    if (!isVisible() || ui->tabWidget->currentWidget() != peersTab)
        return;

    std::vector<CNodeStats> vstats;
    {
        // Don't make the GUI wait for the network thread; try again next time
        TRY_LOCK(cs_vNodes, lockNodes);
        if (!lockNodes)
            return;
        vstats.reserve(vNodes.size());
        BOOST_FOREACH(CNode* pnode, vNodes)
        {
            CNodeStats stats;
            pnode->copyStats(stats);
            vstats.push_back(stats);
        }
    }
    peerStats->swap(vstats);

    int nInbound = 0;
    BOOST_FOREACH(const CNodeStats& stats, *peerStats)
        if (stats.fInbound) nInbound++;
    peerSummary->setText(tr("%n peer(s): ", "", (int)peerStats->size()) +
                         tr("%1 outbound, %2 inbound").arg(peerStats->size() - nInbound).arg(nInbound));

    QString keep = selectedPeer;
    peerTable->blockSignals(true);
    peerTable->setSortingEnabled(false);
    peerTable->clearContents();
    peerTable->setRowCount(peerStats->size());
    int nRow = 0;
    BOOST_FOREACH(const CNodeStats& stats, *peerStats)
    {
        QString subver = QString::fromStdString(stats.strSubVer);
        subver.remove('/');
        QString direction = stats.fInbound ? tr("Inbound") : tr("Outbound");
        if (stats.fSyncNode)
            direction += tr(" (sync)");
        double fPing = stats.dPingWait > stats.dPingTime ? stats.dPingWait : stats.dPingTime;

        peerTable->setItem(nRow, PC_ADDRESS, new PeerItem(QString::fromStdString(stats.addrName)));
        peerTable->setItem(nRow, PC_DIRECTION, new PeerItem(direction, stats.fInbound ? 1 : 0));
        peerTable->setItem(nRow, PC_VERSION, new PeerItem(subver.isEmpty() ? QString::number(stats.nVersion) : subver));
        peerTable->setItem(nRow, PC_HEIGHT, new PeerItem(stats.nKnownHeight >= 0 ? QString::number(stats.nKnownHeight) : tr("unknown"), stats.nKnownHeight));
        peerTable->setItem(nRow, PC_PING, new PeerItem(fPing > 0 ? tr("%1 ms").arg((int)(fPing * 1000)) : QString("-"), fPing > 0 ? fPing : 1e9));
        peerTable->setItem(nRow, PC_SENT, new PeerItem(GUIUtil::formatBytes(stats.nSendBytes), (double)stats.nSendBytes));
        peerTable->setItem(nRow, PC_RECV, new PeerItem(GUIUtil::formatBytes(stats.nRecvBytes), (double)stats.nRecvBytes));
        for (int c = PC_DIRECTION; c < PC_COUNT; c++)
            if (c != PC_VERSION && peerTable->item(nRow, c))
                peerTable->item(nRow, c)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        nRow++;
    }
    peerTable->setSortingEnabled(true);

    // Keep the same peer selected across refreshes
    for (int r = 0; r < peerTable->rowCount(); r++)
        if (peerTable->item(r, PC_ADDRESS) && peerTable->item(r, PC_ADDRESS)->text() == keep)
            peerTable->selectRow(r);
    peerTable->blockSignals(false);
    showPeerDetails();
}

void RPCConsole::showPeerDetails()
{
    QList<QTableWidgetItem*> sel = peerTable->selectedItems();
    QString addr;
    foreach (QTableWidgetItem *item, sel)
        if (item->column() == PC_ADDRESS)
            addr = item->text();
    if (addr.isEmpty())
    {
        selectedPeer.clear();
        peerDetails->setHtml("<p style='color:" + GUIUtil::mutedColor() + "'>" + tr("Select a peer to see its details.") + "</p>");
        return;
    }
    selectedPeer = addr;

    const CNodeStats *p = NULL;
    BOOST_FOREACH(const CNodeStats& stats, *peerStats)
        if (QString::fromStdString(stats.addrName) == addr)
            p = &stats;
    if (!p)
    {
        peerDetails->setHtml("<p style='color:" + GUIUtil::mutedColor() + "'>" + tr("This peer has disconnected.") + "</p>");
        return;
    }

    int64_t nNow = GetTime();
    QStringList rows;
    const QString muted = GUIUtil::mutedColor();
#define ROW(label, value) rows << QString("<tr><td style='color:%3; padding-right:18px'>%1</td><td>%2</td></tr>").arg(label, value, muted)
    ROW(tr("Address"), addr.toHtmlEscaped());
    ROW(tr("Direction"), p->fInbound ? tr("Inbound (they connected to us)") : tr("Outbound (we connected to them)"));
    if (p->fSyncNode)
        ROW(tr("Sync"), tr("Blocks are being downloaded from this peer"));
    ROW(tr("Version"), QString::fromStdString(p->strSubVer).toHtmlEscaped() + tr(" &nbsp;(protocol %1)").arg(p->nVersion));
    ROW(tr("Services"), QString::fromStdString(strprintf("%08" PRIx64, p->nServices)));
    ROW(tr("Starting height"), p->nStartingHeight >= 0 ? QString::number(p->nStartingHeight) : tr("unknown"));
    QString height = p->nKnownHeight >= 0 ? QString::number(p->nKnownHeight) : tr("unknown");
    if (p->nKnownHeight >= 0 && nBestHeight > p->nKnownHeight)
        height += tr(" &nbsp;(%1 blocks behind us)").arg(nBestHeight - p->nKnownHeight);
    ROW(tr("Current height"), height);
    ROW(tr("Connected for"), GUIUtil::formatDurationStr(nNow - p->nTimeConnected));
    ROW(tr("Last send"), p->nLastSend ? tr("%1 ago").arg(GUIUtil::formatDurationStr(nNow - p->nLastSend)) : tr("never"));
    ROW(tr("Last receive"), p->nLastRecv ? tr("%1 ago").arg(GUIUtil::formatDurationStr(nNow - p->nLastRecv)) : tr("never"));
    ROW(tr("Sent"), GUIUtil::formatBytes(p->nSendBytes));
    ROW(tr("Received"), GUIUtil::formatBytes(p->nRecvBytes));
    ROW(tr("Ping"), p->dPingTime > 0 ? tr("%1 ms").arg((int)(p->dPingTime * 1000)) : tr("not measured yet"));
    if (p->dPingWait > 0)
        ROW(tr("Ping waiting"), tr("%1 ms").arg((int)(p->dPingWait * 1000)));
    ROW(tr("Sees us at"), p->addrLocal.empty() ? tr("unknown") : QString::fromStdString(p->addrLocal).toHtmlEscaped());
    ROW(tr("Ban score"), QString::number(p->nMisbehavior));
#undef ROW
    peerDetails->setHtml("<table cellspacing='3'>" + rows.join("") + "</table>");
}

void RPCConsole::on_openDebugLogfileButton_clicked()
{
    GUIUtil::openDebugLogfile();
}

void RPCConsole::scrollToEnd()
{
    QScrollBar *scrollbar = ui->messagesWidget->verticalScrollBar();
    scrollbar->setValue(scrollbar->maximum());
}

void RPCConsole::on_showCLOptionsButton_clicked()
{
    GUIUtil::HelpMessageBox help;
    help.exec();
}
