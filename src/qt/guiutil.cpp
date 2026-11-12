#include "guiutil.h"
#include "bitcoinaddressvalidator.h"
#include "walletmodel.h"
#include "bitcoinunits.h"
#include "util.h"
#include "init.h"

#include <QString>
#include <QDateTime>
#include <QDoubleValidator>
#include <QFont>
#include <QLineEdit>
#include <QUrl>
#include <QTextDocument> // For Qt::escape
#include <QAbstractItemView>
#include <QApplication>
#include <QClipboard>
#include <QFileDialog>
#include <QDesktopServices>
#include <QThread>
#include <QPalette>
#include <QStyle>
#include <QStyleFactory>
#include <QFile>
#include <QMap>
#include <QMapIterator>
#include <QPainter>
#include <QGraphicsDropShadowEffect>
#include <QPixmap>
#include <QBrush>

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#ifdef WIN32
#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
#define _WIN32_WINNT 0x0501
#ifdef _WIN32_IE
#undef _WIN32_IE
#endif
#define _WIN32_IE 0x0501
#define WIN32_LEAN_AND_MEAN 1
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "shlwapi.h"
#include "shlobj.h"
#include "shellapi.h"
#endif

namespace GUIUtil {

QString dateTimeStr(const QDateTime &date)
{
    return date.date().toString(Qt::SystemLocaleShortDate) + QString(" ") + date.toString("hh:mm");
}

QString dateTimeStr(qint64 nTime)
{
    return dateTimeStr(QDateTime::fromTime_t((qint32)nTime));
}

QFont bitcoinAddressFont()
{
    QFont font("Monospace");
    font.setStyleHint(QFont::TypeWriter);
    return font;
}

void setupAddressWidget(QLineEdit *widget, QWidget *parent)
{
    widget->setMaxLength(BitcoinAddressValidator::MaxAddressLength);
    widget->setValidator(new BitcoinAddressValidator(parent));
    widget->setFont(bitcoinAddressFont());
}

void setupAmountWidget(QLineEdit *widget, QWidget *parent)
{
    QDoubleValidator *amountValidator = new QDoubleValidator(parent);
    amountValidator->setDecimals(8);
    amountValidator->setBottom(0.0);
    widget->setValidator(amountValidator);
    widget->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
}

bool parseBitcoinURI(const QUrl &uri, SendCoinsRecipient *out)
{
    // NovaCoin: check prefix
    if(uri.scheme() != QString("flapx"))
        return false;

    SendCoinsRecipient rv;
    rv.address = uri.path();
    rv.amount = 0;
    QList<QPair<QString, QString> > items = uri.queryItems();
    for (QList<QPair<QString, QString> >::iterator i = items.begin(); i != items.end(); i++)
    {
        bool fShouldReturnFalse = false;
        if (i->first.startsWith("req-"))
        {
            i->first.remove(0, 4);
            fShouldReturnFalse = true;
        }

        if (i->first == "label")
        {
            rv.label = i->second;
            fShouldReturnFalse = false;
        }
        else if (i->first == "amount")
        {
            if(!i->second.isEmpty())
            {
                if(!BitcoinUnits::parse(BitcoinUnits::BTC, i->second, &rv.amount))
                {
                    return false;
                }
            }
            fShouldReturnFalse = false;
        }

        if (fShouldReturnFalse)
            return false;
    }
    if(out)
    {
        *out = rv;
    }
    return true;
}

bool parseBitcoinURI(QString uri, SendCoinsRecipient *out)
{
    // Convert flapx:// to flapx:
    //
    //    Cannot handle this later, because bitcoin:// will cause Qt to see the part after // as host,
    //    which will lower-case it (and thus invalidate the address).
    if(uri.startsWith("flapx://"))
    {
        uri.replace(0, 12, "flapx:");
    }
    QUrl uriInstance(uri);
    return parseBitcoinURI(uriInstance, out);
}

QString HtmlEscape(const QString& str, bool fMultiLine)
{
    QString escaped = Qt::escape(str);
    if(fMultiLine)
    {
        escaped = escaped.replace("\n", "<br>\n");
    }
    return escaped;
}

QString HtmlEscape(const std::string& str, bool fMultiLine)
{
    return HtmlEscape(QString::fromStdString(str), fMultiLine);
}

void copyEntryData(QAbstractItemView *view, int column, int role)
{
    if(!view || !view->selectionModel())
        return;
    QModelIndexList selection = view->selectionModel()->selectedRows(column);

    if(!selection.isEmpty())
    {
        // Copy first item
        QApplication::clipboard()->setText(selection.at(0).data(role).toString());
    }
}

QString getSaveFileName(QWidget *parent, const QString &caption,
                                 const QString &dir,
                                 const QString &filter,
                                 QString *selectedSuffixOut)
{
    QString selectedFilter;
    QString myDir;
    if(dir.isEmpty()) // Default to user documents location
    {
        myDir = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
    }
    else
    {
        myDir = dir;
    }
    QString result = QFileDialog::getSaveFileName(parent, caption, myDir, filter, &selectedFilter);

    /* Extract first suffix from filter pattern "Description (*.foo)" or "Description (*.foo *.bar ...) */
    QRegExp filter_re(".* \\(\\*\\.(.*)[ \\)]");
    QString selectedSuffix;
    if(filter_re.exactMatch(selectedFilter))
    {
        selectedSuffix = filter_re.cap(1);
    }

    /* Add suffix if needed */
    QFileInfo info(result);
    if(!result.isEmpty())
    {
        if(info.suffix().isEmpty() && !selectedSuffix.isEmpty())
        {
            /* No suffix specified, add selected suffix */
            if(!result.endsWith("."))
                result.append(".");
            result.append(selectedSuffix);
        }
    }

    /* Return selected suffix if asked to */
    if(selectedSuffixOut)
    {
        *selectedSuffixOut = selectedSuffix;
    }
    return result;
}

Qt::ConnectionType blockingGUIThreadConnection()
{
    if(QThread::currentThread() != QCoreApplication::instance()->thread())
    {
        return Qt::BlockingQueuedConnection;
    }
    else
    {
        return Qt::DirectConnection;
    }
}

bool checkPoint(const QPoint &p, const QWidget *w)
{
    QWidget *atW = qApp->widgetAt(w->mapToGlobal(p));
    if (!atW) return false;
    return atW->topLevelWidget() == w;
}

bool isObscured(QWidget *w)
{
    return !(checkPoint(QPoint(0, 0), w)
        && checkPoint(QPoint(w->width() - 1, 0), w)
        && checkPoint(QPoint(0, w->height() - 1), w)
        && checkPoint(QPoint(w->width() - 1, w->height() - 1), w)
        && checkPoint(QPoint(w->width() / 2, w->height() / 2), w));
}

void openDebugLogfile()
{
    boost::filesystem::path pathDebug = GetDataDir() / "debug.log";

    /* Open debug.log with the associated application */
    if (boost::filesystem::exists(pathDebug))
        QDesktopServices::openUrl(QUrl::fromLocalFile(QString::fromStdString(pathDebug.string())));
}

ToolTipToRichTextFilter::ToolTipToRichTextFilter(int size_threshold, QObject *parent) :
    QObject(parent), size_threshold(size_threshold)
{

}

bool ToolTipToRichTextFilter::eventFilter(QObject *obj, QEvent *evt)
{
    if(evt->type() == QEvent::ToolTipChange)
    {
        QWidget *widget = static_cast<QWidget*>(obj);
        QString tooltip = widget->toolTip();
        if(tooltip.size() > size_threshold && !tooltip.startsWith("<qt>") && !Qt::mightBeRichText(tooltip))
        {
            // Prefix <qt/> to make sure Qt detects this as rich text
            // Escape the current message as HTML and replace \n by <br>
            tooltip = "<qt>" + HtmlEscape(tooltip, true) + "<qt/>";
            widget->setToolTip(tooltip);
            return true;
        }
    }
    return QObject::eventFilter(obj, evt);
}

#ifdef WIN32
boost::filesystem::path static StartupShortcutPath()
{
    return GetSpecialFolderPath(CSIDL_STARTUP) / "FlapX.lnk";
}

bool GetStartOnSystemStartup()
{
    // check for Bitcoin.lnk
    return boost::filesystem::exists(StartupShortcutPath());
}

bool SetStartOnSystemStartup(bool fAutoStart)
{
    // If the shortcut exists already, remove it for updating
    boost::filesystem::remove(StartupShortcutPath());

    if (fAutoStart)
    {
        CoInitialize(NULL);

        // Get a pointer to the IShellLink interface.
        IShellLink* psl = NULL;
        HRESULT hres = CoCreateInstance(CLSID_ShellLink, NULL,
                                CLSCTX_INPROC_SERVER, IID_IShellLink,
                                reinterpret_cast<void**>(&psl));

        if (SUCCEEDED(hres))
        {
            // Get the current executable path
            TCHAR pszExePath[MAX_PATH];
            GetModuleFileName(NULL, pszExePath, sizeof(pszExePath));

            TCHAR pszArgs[5] = TEXT("-min");

            // Set the path to the shortcut target
            psl->SetPath(pszExePath);
            PathRemoveFileSpec(pszExePath);
            psl->SetWorkingDirectory(pszExePath);
            psl->SetShowCmd(SW_SHOWMINNOACTIVE);
            psl->SetArguments(pszArgs);

            // Query IShellLink for the IPersistFile interface for
            // saving the shortcut in persistent storage.
            IPersistFile* ppf = NULL;
            hres = psl->QueryInterface(IID_IPersistFile,
                                       reinterpret_cast<void**>(&ppf));
            if (SUCCEEDED(hres))
            {
                WCHAR pwsz[MAX_PATH];
                // Ensure that the string is ANSI.
                MultiByteToWideChar(CP_ACP, 0, StartupShortcutPath().string().c_str(), -1, pwsz, MAX_PATH);
                // Save the link by calling IPersistFile::Save.
                hres = ppf->Save(pwsz, TRUE);
                ppf->Release();
                psl->Release();
                CoUninitialize();
                return true;
            }
            psl->Release();
        }
        CoUninitialize();
        return false;
    }
    return true;
}

#elif defined(LINUX)

// Follow the Desktop Application Autostart Spec:
//  http://standards.freedesktop.org/autostart-spec/autostart-spec-latest.html

boost::filesystem::path static GetAutostartDir()
{
    namespace fs = boost::filesystem;

    char* pszConfigHome = getenv("XDG_CONFIG_HOME");
    if (pszConfigHome) return fs::path(pszConfigHome) / "autostart";
    char* pszHome = getenv("HOME");
    if (pszHome) return fs::path(pszHome) / ".config" / "autostart";
    return fs::path();
}

boost::filesystem::path static GetAutostartFilePath()
{
    return GetAutostartDir() / "flapx.desktop";
}

bool GetStartOnSystemStartup()
{
    boost::filesystem::ifstream optionFile(GetAutostartFilePath());
    if (!optionFile.good())
        return false;
    // Scan through file for "Hidden=true":
    std::string line;
    while (!optionFile.eof())
    {
        getline(optionFile, line);
        if (line.find("Hidden") != std::string::npos &&
            line.find("true") != std::string::npos)
            return false;
    }
    optionFile.close();

    return true;
}

bool SetStartOnSystemStartup(bool fAutoStart)
{
    if (!fAutoStart)
        boost::filesystem::remove(GetAutostartFilePath());
    else
    {
        char pszExePath[MAX_PATH+1];
        memset(pszExePath, 0, sizeof(pszExePath));
        if (readlink("/proc/self/exe", pszExePath, sizeof(pszExePath)-1) == -1)
            return false;

        boost::filesystem::create_directories(GetAutostartDir());

        boost::filesystem::ofstream optionFile(GetAutostartFilePath(), std::ios_base::out|std::ios_base::trunc);
        if (!optionFile.good())
            return false;
        // Write a bitcoin.desktop file to the autostart directory:
        optionFile << "[Desktop Entry]\n";
        optionFile << "Type=Application\n";
        optionFile << "Name=FlapX\n";
        optionFile << "Exec=" << pszExePath << " -min\n";
        optionFile << "Terminal=false\n";
        optionFile << "Hidden=false\n";
        optionFile.close();
    }
    return true;
}
#else

// TODO: OSX startup stuff; see:
// https://developer.apple.com/library/mac/#documentation/MacOSX/Conceptual/BPSystemStartup/Articles/CustomLogin.html

bool GetStartOnSystemStartup() { return false; }
bool SetStartOnSystemStartup(bool fAutoStart) { return false; }

#endif

HelpMessageBox::HelpMessageBox(QWidget *parent) :
    QMessageBox(parent)
{
    header = tr("FlapX-Qt") + " " + tr("version") + " " +
        QString::fromStdString(FormatFullVersion()) + "\n\n" +
        tr("Usage:") + "\n" +
        "  flapx-qt [" + tr("command-line options") + "]                     " + "\n";

    coreOptions = QString::fromStdString(HelpMessage());

    uiOptions = tr("UI options") + ":\n" +
        "  -lang=<lang>           " + tr("Set language, for example \"de_DE\" (default: system locale)") + "\n" +
        "  -min                   " + tr("Start minimized") + "\n" +
        "  -splash                " + tr("Show splash screen on startup (default: 1)") + "\n";

    setWindowTitle(tr("FlapX-Qt"));
    setTextFormat(Qt::PlainText);
    // setMinimumWidth is ignored for QMessageBox so put in non-breaking spaces to make it wider.
    setText(header + QString(QChar(0x2003)).repeated(50));
    setDetailedText(coreOptions + "\n" + uiOptions);
}

void HelpMessageBox::printToConsole()
{
    // On other operating systems, the expected action is to print the message to the console.
    QString strUsage = header + "\n" + coreOptions + "\n" + uiOptions;
    fprintf(stdout, "%s", strUsage.toStdString().c_str());
}

void HelpMessageBox::showOrPrint()
{
#if defined(WIN32)
        // On Windows, show a message box, as there is no stderr/stdout in windowed applications
        exec();
#else
        // On other operating systems, print help text to console
        printToConsole();
#endif
}

// The FlapX starfield, drawn in code (no image): the same stars as the block explorer's
// stylesheet, on a 420x420 tile that repeats across the window background.
static QPixmap flapxStarfieldTile(const QColor &space)
{
    struct Star { double x, y, size; int r, g, b; double alpha; };
    static const Star stars[] = {
        { 0.12, 0.18, 1.0, 255, 255, 255, 0.90 },
        { 0.68, 0.09, 1.0, 255, 255, 255, 0.80 },
        { 0.85, 0.42, 1.5, 200, 246, 251, 0.90 },   // pale cyan
        { 0.33, 0.64, 1.0, 255, 255, 255, 0.75 },
        { 0.52, 0.88, 1.0, 255, 255, 255, 0.70 },
        { 0.07, 0.79, 1.5, 200, 251, 222, 0.85 },   // pale mint
    };
    const int tile = 420;

    QPixmap pixmap(tile, tile);
    pixmap.fill(space);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);
    for (unsigned int i = 0; i < sizeof(stars) / sizeof(stars[0]); i++)
    {
        const Star &star = stars[i];
        QColor color(star.r, star.g, star.b);
        color.setAlphaF(star.alpha);
        painter.setBrush(color);
        const double radius = star.size / 2.0 + 0.25;
        painter.drawEllipse(QPointF(star.x * tile, star.y * tile), radius, radius);
    }
    painter.end();

    return pixmap;
}

void applyFlapXPalette()
{
    // Fusion draws every widget from the palette, so the whole wallet takes the FlapX colours
    if (QStyle *style = QStyleFactory::create("Fusion"))
        QApplication::setStyle(style);

    const QColor space(11, 18, 32), space2(17, 26, 46), panel(21, 32, 58), border(36, 52, 90);
    const QColor text(230, 237, 247), cyan(42, 216, 234), mint(34, 240, 122);
    const QColor disabled(91, 106, 134);

    QPalette p;
    p.setColor(QPalette::Window, space);
    // window backgrounds carry the starfield; panels and inputs stay solid
    p.setBrush(QPalette::Active, QPalette::Window, QBrush(flapxStarfieldTile(space)));
    p.setBrush(QPalette::Inactive, QPalette::Window, QBrush(flapxStarfieldTile(space)));
    p.setColor(QPalette::WindowText, text);
    p.setColor(QPalette::Base, space2);
    p.setColor(QPalette::AlternateBase, panel);
    p.setColor(QPalette::ToolTipBase, space2);
    p.setColor(QPalette::ToolTipText, text);
    p.setColor(QPalette::Text, text);
    p.setColor(QPalette::Button, panel);
    p.setColor(QPalette::ButtonText, text);
    p.setColor(QPalette::BrightText, QColor(255, 107, 129));
    p.setColor(QPalette::Link, cyan);
    p.setColor(QPalette::LinkVisited, mint);
    p.setColor(QPalette::Highlight, QColor(31, 111, 128));
    p.setColor(QPalette::HighlightedText, Qt::white);
    p.setColor(QPalette::Light, QColor(44, 62, 104));
    p.setColor(QPalette::Midlight, border);
    p.setColor(QPalette::Mid, QColor(27, 40, 72));
    p.setColor(QPalette::Dark, QColor(7, 12, 23));
    p.setColor(QPalette::Shadow, Qt::black);
    p.setColor(QPalette::Disabled, QPalette::WindowText, disabled);
    p.setColor(QPalette::Disabled, QPalette::Text, disabled);
    p.setColor(QPalette::Disabled, QPalette::ButtonText, disabled);
    p.setColor(QPalette::Disabled, QPalette::Highlight, border);
    QApplication::setPalette(p);
}

QString loadStyleSheet(const QString &path, const QString &themeDir)
{
    QFile qss(path);
    if (!qss.open(QFile::ReadOnly))
        return QString();

    QString styleSheet = QString::fromUtf8(qss.readAll());
    qss.close();

    // Variables are declared between "/** [VARS]" and "[/VARS] */", one "@name = value" per line
    QMap<QString, QString> variables;
    bool readingVariables = false;
    foreach (const QString &rawLine, styleSheet.split('\n'))
    {
        const QString line = rawLine.trimmed();
        if (line == "/** [VARS]") {
            readingVariables = true;
            continue;
        }
        if (line == "[/VARS] */")
            break;
        if (readingVariables && line.length() > 3 && line.contains('=')) {
            const int eq = line.indexOf('=');
            variables[line.left(eq).trimmed()] = line.mid(eq + 1).trimmed();
        }
    }

    // $theme-dir lets a theme point at its own files, e.g. url($theme-dir/image.png)
    styleSheet.replace("$theme-dir", themeDir);

    // Longest names first (reverse map order), so @space2 is replaced before @space
    QMapIterator<QString, QString> variable(variables);
    variable.toBack();
    while (variable.hasPrevious()) {
        variable.previous();
        styleSheet.replace(variable.key(), variable.value());
    }

    return styleSheet;
}

QString flapxStyleSheet()
{
    return loadStyleSheet(":/themes/flapxcoin.qss", ":/themes");
}

QString flappyStyleSheet()
{
    return loadStyleSheet(":/themes/flappycoin.qss", ":/themes");
}

static bool fFlappyTheme = false;

void setFlappyTheme(bool fFlappy)
{
    fFlappyTheme = fFlappy;
}

bool isFlappyTheme()
{
    return fFlappyTheme;
}

QColor themeColor(ThemeColor which)
{
    if (fFlappyTheme)
    {
        switch (which)
        {
        case TC_POSITIVE:        return QColor(47, 125, 42);    // bush green, darkened for text
        case TC_NEGATIVE:        return QColor(210, 60, 60);
        case TC_NEGATIVE_AMOUNT: return QColor(43, 58, 31);     // normal text
        case TC_MUTED:           return QColor(94, 115, 86);
        case TC_ACCENT:          return QColor(240, 130, 30);   // orange
        case TC_ACCENT2:         return QColor(47, 125, 42);
        case TC_GRAPH_BG:        return QColor(253, 255, 247);
        case TC_GRAPH_GRID:      return QColor(156, 201, 143);
        }
    }
    switch (which)
    {
    case TC_POSITIVE:        return QColor(34, 240, 122);   // mint
    case TC_NEGATIVE:        return QColor(255, 107, 129);
    case TC_NEGATIVE_AMOUNT: return QColor(201, 212, 227);  // silver
    case TC_MUTED:           return QColor(138, 155, 184);
    case TC_ACCENT:          return QColor(42, 216, 234);   // cyan
    case TC_ACCENT2:         return QColor(34, 240, 122);   // mint
    case TC_GRAPH_BG:        return QColor(11, 18, 32);
    case TC_GRAPH_GRID:      return QColor(36, 52, 90);
    }
    return QColor();
}

QString positiveColor()
{
    return themeColor(TC_POSITIVE).name();
}

QString negativeColor()
{
    return themeColor(TC_NEGATIVE).name();
}

QString mutedColor()
{
    return themeColor(TC_MUTED).name();
}

void applyTextOutline(QWidget *widget)
{
    if (!widget)
        return;
    if (fFlappyTheme)
    {
        if (!widget->graphicsEffect())
        {
            QGraphicsDropShadowEffect *outline = new QGraphicsDropShadowEffect(widget);
            outline->setBlurRadius(0);
            outline->setOffset(1.5, 1.5);
            outline->setColor(QColor(84, 56, 71));   // Flappy Bird's dark brown outline
            widget->setGraphicsEffect(outline);
        }
    }
    else if (widget->graphicsEffect())
    {
        widget->setGraphicsEffect(0);   // deletes the effect
    }
}

void applyFlappyPalette()
{
    if (QStyle *style = QStyleFactory::create("Fusion"))
        QApplication::setStyle(style);

    const QColor sky(112, 197, 206), cream(244, 251, 233), cream2(253, 255, 247);
    const QColor text(43, 58, 31), outline(81, 127, 75), border(156, 201, 143);
    const QColor disabled(140, 155, 132);

    QPalette p;
    p.setColor(QPalette::Window, sky);
    p.setColor(QPalette::WindowText, text);
    p.setColor(QPalette::Base, cream2);
    p.setColor(QPalette::AlternateBase, cream);
    p.setColor(QPalette::ToolTipBase, cream2);
    p.setColor(QPalette::ToolTipText, text);
    p.setColor(QPalette::Text, text);
    p.setColor(QPalette::Button, cream);
    p.setColor(QPalette::ButtonText, text);
    p.setColor(QPalette::BrightText, QColor(210, 60, 60));
    p.setColor(QPalette::Link, QColor(200, 97, 12));
    p.setColor(QPalette::LinkVisited, outline);
    p.setColor(QPalette::Highlight, QColor(249, 194, 127));
    p.setColor(QPalette::HighlightedText, text);
    p.setColor(QPalette::Light, Qt::white);
    p.setColor(QPalette::Midlight, cream);
    p.setColor(QPalette::Mid, border);
    p.setColor(QPalette::Dark, outline);
    p.setColor(QPalette::Shadow, QColor(43, 58, 31));
    p.setColor(QPalette::Disabled, QPalette::WindowText, disabled);
    p.setColor(QPalette::Disabled, QPalette::Text, disabled);
    p.setColor(QPalette::Disabled, QPalette::ButtonText, disabled);
    p.setColor(QPalette::Disabled, QPalette::Highlight, border);
    QApplication::setPalette(p);
}

// One 192 px wide strip of the FlappyCoin scenery: clouds, a pale city, and
// bushes, in the style of the original wallet's footer. Drawn once, then tiled.
static QPixmap flappySceneryTile()
{
    const int w = 192, h = 96;
    QPixmap pixmap(w, h);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    p.setPen(Qt::NoPen);

    // Clouds: cream puffs with a pale edge, along the top of the strip
    const QColor cloudEdge(205, 236, 214), cloud(233, 250, 217);
    static const int puffs[][3] = { {10,30,22}, {38,22,26}, {70,30,20}, {96,26,24}, {126,20,28},
                                    {158,28,22}, {182,24,24}, {-8,24,24}, {202,30,22} };
    for (unsigned int i = 0; i < sizeof(puffs) / sizeof(puffs[0]); i++)
    {
        p.setBrush(cloudEdge);
        p.drawEllipse(QPoint(puffs[i][0], puffs[i][1]), puffs[i][2] + 2, puffs[i][2] + 2);
    }
    for (unsigned int i = 0; i < sizeof(puffs) / sizeof(puffs[0]); i++)
    {
        p.setBrush(cloud);
        p.drawEllipse(QPoint(puffs[i][0], puffs[i][1]), puffs[i][2], puffs[i][2]);
    }
    p.fillRect(0, 34, w, h - 34, cloud);

    // City: pale green-blue buildings with rows of windows
    const QColor bldEdge(170, 221, 212), bld(214, 240, 222), win(170, 221, 212);
    static const int towers[][3] = { {4,34,22}, {28,24,16}, {46,40,24}, {72,18,18}, {92,32,20},
                                     {114,26,26}, {142,38,18}, {162,22,26} };   // x, top, width
    for (unsigned int i = 0; i < sizeof(towers) / sizeof(towers[0]); i++)
    {
        const int x = towers[i][0], top = towers[i][1], tw = towers[i][2];
        p.fillRect(x, top, tw, h - top, bldEdge);
        p.fillRect(x + 2, top + 2, tw - 4, h - top - 2, bld);
        for (int y = top + 6; y < h - 30; y += 7)
            for (int wx = x + 4; wx + 3 < x + tw - 2; wx += 6)
                p.fillRect(wx, y, 3, 3, win);
    }

    // Bushes: rounded green mounds with a darker outline, across the bottom
    const QColor bushEdge(111, 194, 150), bush(135, 226, 148), bushLight(160, 234, 168);
    static const int mounds[][3] = { {0,74,20}, {28,70,22}, {58,76,18}, {84,70,24}, {116,74,20},
                                     {144,68,24}, {176,74,20}, {200,72,20} };
    for (unsigned int i = 0; i < sizeof(mounds) / sizeof(mounds[0]); i++)
    {
        p.setBrush(bushEdge);
        p.drawEllipse(QPoint(mounds[i][0], mounds[i][1]), mounds[i][2] + 2, mounds[i][2] + 2);
    }
    for (unsigned int i = 0; i < sizeof(mounds) / sizeof(mounds[0]); i++)
    {
        p.setBrush(bush);
        p.drawEllipse(QPoint(mounds[i][0], mounds[i][1]), mounds[i][2], mounds[i][2]);
        p.setBrush(bushLight);
        p.drawEllipse(QPoint(mounds[i][0] - mounds[i][2] / 3, mounds[i][1] - mounds[i][2] / 3), 4, 3);
    }
    p.fillRect(0, 80, w, h - 80, bush);
    p.end();
    return pixmap;
}

void drawFlappyScenery(QPainter &painter, const QRect &rect)
{
    static QPixmap tile = flappySceneryTile();

    // Sky: a little lighter towards the top, like the game
    QLinearGradient sky(rect.topLeft(), rect.bottomLeft());
    sky.setColorAt(0.0, QColor(132, 207, 214));
    sky.setColorAt(1.0, QColor(112, 197, 206));
    painter.fillRect(rect, sky);

    // Scenery strip along the bottom
    const int y = rect.bottom() + 1 - tile.height();
    painter.drawTiledPixmap(QRect(rect.left(), y, rect.width(), tile.height()), tile);
}

QString formatBytes(quint64 bytes)
{
    if (bytes < 1024)
        return QObject::tr("%1 B").arg(bytes);
    if (bytes < 1024 * 1024)
        return QObject::tr("%1 kB").arg(bytes / 1024.0, 0, 'f', 1);
    if (bytes < 1024 * 1024 * 1024)
        return QObject::tr("%1 MB").arg(bytes / (1024.0 * 1024), 0, 'f', 2);
    return QObject::tr("%1 GB").arg(bytes / (1024.0 * 1024 * 1024), 0, 'f', 2);
}

QString formatBytesPerSecond(double bytes)
{
    return QObject::tr("%1/s").arg(formatBytes((quint64)(bytes + 0.5)));
}

QString formatDurationStr(qint64 secs)
{
    if (secs < 0)
        secs = 0;
    qint64 d = secs / 86400, h = (secs % 86400) / 3600, m = (secs % 3600) / 60, s = secs % 60;
    if (d > 0)
        return QObject::tr("%1 d %2 h").arg(d).arg(h);
    if (h > 0)
        return QObject::tr("%1 h %2 m").arg(h).arg(m);
    if (m > 0)
        return QObject::tr("%1 m %2 s").arg(m).arg(s);
    return QObject::tr("%1 s").arg(s);
}

} // namespace GUIUtil

