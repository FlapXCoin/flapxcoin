#include "explorerpage.h"
#include "ui_explorerpage.h"

#include "main.h"
#include "init.h"
#include "wallet.h"
#include "base58.h"
#include "bitcoinrpc.h"
#include "guiutil.h"

#include <QDateTime>
#include <QRegExp>
#include <QScrollBar>
#include <QUrl>

// Don't build huge pages for huge blocks/transactions
static const int MAX_LIST_ROWS = 500;
// Inputs whose amount and address are looked up (one disk read each)
static const int MAX_INPUT_LOOKUPS = 200;

ExplorerPage::ExplorerPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ExplorerPage)
{
    ui->setupUi(this);

    connect(ui->searchButton, SIGNAL(clicked()), this, SLOT(search()));
    connect(ui->searchBox, SIGNAL(returnPressed()), this, SLOT(search()));
    connect(ui->latestButton, SIGNAL(clicked()), this, SLOT(showLatest()));
    connect(ui->backButton, SIGNAL(clicked()), this, SLOT(back()));
    connect(ui->resultView, SIGNAL(anchorClicked(QUrl)), this, SLOT(linkClicked(QUrl)));
    ui->backButton->setEnabled(false);

    ui->resultView->setHtml("<p>" + tr("Type a <b>block height</b>, a <b>block hash</b> or a <b>transaction ID</b> "
                                       "and press Search. Blocks and transactions in the results can be clicked.") + "</p>"
                            "<p style='color:" + GUIUtil::mutedColor() + "'>" +
                            tr("Everything is read from this wallet's own copy of the blockchain, so a wallet that is "
                               "still syncing only knows the blocks it has downloaded so far.") + "</p>");
}

ExplorerPage::~ExplorerPage()
{
    delete ui;
}

void ExplorerPage::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    ui->searchBox->setFocus();
}

void ExplorerPage::search()
{
    QString query = ui->searchBox->text().trimmed();
    if (!query.isEmpty())
        show(query);
}

void ExplorerPage::showLatest()
{
    show(QString::number(nBestHeight));
}

void ExplorerPage::back()
{
    if (history.isEmpty())
        return;
    QString previous = history.takeLast();
    show(previous, false);
}

void ExplorerPage::linkClicked(const QUrl &url)
{
    // Links are "block:<hash>" or "tx:<hash>"; the hash alone is enough to find either
    QString target = url.toString().section(':', 1);
    if (!target.isEmpty())
        show(target);
}

// ---- formatting helpers ----

static QString Amount(int64_t n)
{
    return QString::fromStdString(FormatMoney(n)) + " FLAPX";
}

static QString Time(int64_t t)
{
    return QDateTime::fromMSecsSinceEpoch(t * 1000).toString("yyyy-MM-dd hh:mm:ss") +
           QString(" &nbsp;<span style='color:%1'>(%2)</span>").arg(GUIUtil::mutedColor()).arg(t);
}

static QString Row(const QString &label, const QString &value)
{
    return QString("<tr><td style='color:%1; padding-right:16px'>%2</td><td>%3</td></tr>")
        .arg(GUIUtil::mutedColor(), label, value);
}

static QString BlockLink(const CBlockIndex *pindex)
{
    return QString("<a href='block:%1'>%2</a>").arg(QString::fromStdString(pindex->GetBlockHash().GetHex())).arg(pindex->nHeight);
}

static QString TxLink(const uint256 &hash)
{
    QString hex = QString::fromStdString(hash.GetHex());
    return QString("<a href='tx:%1'><tt>%1</tt></a>").arg(hex);
}

static QString Destination(const CScript &script, bool &fMine)
{
    fMine = false;
    if (script.empty())
        return QObject::tr("(empty: coinstake marker)");
    CTxDestination dest;
    if (!ExtractDestination(script, dest))
        return QObject::tr("(non-standard script)");
    if (pwalletMain)
    {
        LOCK(pwalletMain->cs_wallet);
        fMine = IsMine(*pwalletMain, script);
    }
    return QString::fromStdString(CBitcoinAddress(dest).ToString());
}

static QString Heading(const QString &title, const QString &badge = QString())
{
    QString html = "<h2>" + title;
    if (!badge.isEmpty())
        html += QString(" &nbsp;<span style='font-size:small; color:%1'>%2</span>").arg(GUIUtil::themeColor(GUIUtil::TC_ACCENT).name(), badge);
    return html + "</h2>";
}

// ---- lookups ----

void ExplorerPage::show(const QString &queryIn, bool fRecord)
{
    QString query = queryIn.trimmed();
    if (query.startsWith("0x", Qt::CaseInsensitive))
        query = query.mid(2);

    QString html;
    bool fFound = false;

    if (QRegExp("\\d{1,9}").exactMatch(query))
    {
        int nHeight = query.toInt();
        LOCK(cs_main);
        if (pindexBest && nHeight >= 0 && nHeight <= nBestHeight)
        {
            CBlockIndex *pindex = FindBlockByHeight(nHeight);
            if (pindex)
                html = blockHtml(QString::fromStdString(pindex->GetBlockHash().GetHex()), fFound);
        }
        if (!fFound)
            html = "<p>" + tr("There is no block %1 yet: the newest block this wallet has is %2.").arg(nHeight).arg(nBestHeight) + "</p>";
    }
    else if (QRegExp("[0-9a-fA-F]{64}").exactMatch(query))
    {
        html = blockHtml(query, fFound);
        if (!fFound)
            html = txHtml(query, fFound);
        if (!fFound)
            html = "<p>" + tr("No block or transaction with this hash was found in this wallet's copy of the blockchain.") + "</p>";
    }
    else if (CBitcoinAddress(query.toStdString()).IsValid())
    {
        html = "<p>" + tr("That is a FlapX address. The wallet keeps no index of addresses, so it can't list an "
                          "address's transactions; the web explorer can. Transactions of your own addresses are on "
                          "the Transactions page.") + "</p>";
    }
    else
    {
        html = "<p>" + tr("Enter a block height (a number), a block hash or a transaction ID (64 hexadecimal characters).") + "</p>";
    }

    if (fFound && fRecord && !current.isEmpty() && current != query)
        history.append(current);
    if (fFound)
    {
        current = query;
        ui->searchBox->setText(query);
    }
    ui->backButton->setEnabled(!history.isEmpty());

    ui->resultView->setHtml(html);
    ui->resultView->verticalScrollBar()->setValue(0);
}

QString ExplorerPage::blockHtml(const QString &hashHex, bool &fFound)
{
    fFound = false;
    LOCK(cs_main);

    uint256 hash(hashHex.toStdString());
    std::map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(hash);
    if (mi == mapBlockIndex.end() || !mi->second)
        return QString();
    CBlockIndex *pindex = mi->second;

    CBlock block;
    if (!block.ReadFromDisk(pindex, true))
        return "<p>" + tr("Block %1 is in the index but could not be read from disk.").arg(pindex->nHeight) + "</p>";
    fFound = true;

    bool fMain = pindex->IsInMainChain();
    QString html = Heading(tr("Block %1").arg(pindex->nHeight),
                           pindex->IsProofOfStake() ? tr("Proof of stake") : tr("Proof of work"));
    html += "<table cellspacing='3'>";
    html += Row(tr("Hash"), "<tt>" + QString::fromStdString(pindex->GetBlockHash().GetHex()) + "</tt>");
    if (fMain)
        html += Row(tr("Confirmations"), QString::number(nBestHeight - pindex->nHeight + 1));
    else
        html += Row(tr("Status"), "<b>" + tr("Not in the main chain (orphaned)") + "</b>");
    html += Row(tr("Time"), Time(pindex->GetBlockTime()));
    html += Row(tr("Previous block"), pindex->pprev ? BlockLink(pindex->pprev) : tr("none (genesis block)"));
    html += Row(tr("Next block"), (fMain && pindex->pnext) ? BlockLink(pindex->pnext) : tr("none yet"));
    html += Row(tr("Difficulty"), QString::number(GetDifficulty(pindex), 'f', 8));
    html += Row(tr("Bits"), QString::fromStdString(strprintf("%08x", pindex->nBits)));
    html += Row(tr("Nonce"), QString::number(pindex->nNonce));
    html += Row(tr("Merkle root"), "<tt>" + QString::fromStdString(pindex->hashMerkleRoot.GetHex()) + "</tt>");
    html += Row(tr("Minted"), Amount(pindex->nMint));
    html += Row(tr("Money supply"), Amount(pindex->nMoneySupply));
    html += Row(tr("Size"), tr("%1 bytes").arg(::GetSerializeSize(block, SER_NETWORK, PROTOCOL_VERSION)));
    html += Row(tr("Transactions"), QString::number(block.vtx.size()));
    html += "</table>";

    html += "<h3>" + tr("Transactions") + "</h3><table cellspacing='3'>";
    html += QString("<tr><th align='left'>%1</th><th align='left' style='padding-left:12px'>%2</th><th align='right' style='padding-left:12px'>%3</th></tr>")
                .arg(tr("Transaction ID"), tr("Type"), tr("Total out"));
    int nRows = 0;
    BOOST_FOREACH(const CTransaction &tx, block.vtx)
    {
        if (++nRows > MAX_LIST_ROWS)
        {
            html += "<tr><td colspan='3'>" + tr("... and %1 more").arg(block.vtx.size() - MAX_LIST_ROWS) + "</td></tr>";
            break;
        }
        QString type = tx.IsCoinBase() ? tr("Coinbase") : tx.IsCoinStake() ? tr("Coinstake") : tr("Transfer");
        html += QString("<tr><td>%1</td><td style='padding-left:12px'>%2</td><td align='right' style='padding-left:12px'>%3</td></tr>")
                    .arg(TxLink(tx.GetHash()), type, Amount(tx.GetValueOut()));
    }
    html += "</table>";
    return html;
}

QString ExplorerPage::txHtml(const QString &hashHex, bool &fFound)
{
    fFound = false;
    LOCK(cs_main);

    uint256 hash(hashHex.toStdString());
    CTransaction tx;
    uint256 hashBlock = 0;
    if (!GetTransaction(hash, tx, hashBlock))
    {
        // The genesis block's coinbase is never added to the transaction index
        CBlock genesis;
        if (pindexGenesisBlock && genesis.ReadFromDisk(pindexGenesisBlock, true) &&
            !genesis.vtx.empty() && genesis.vtx[0].GetHash() == hash)
        {
            tx = genesis.vtx[0];
            hashBlock = genesis.GetHash();
        }
        else
            return QString();
    }
    fFound = true;

    CBlockIndex *pindex = NULL;
    if (hashBlock != 0)
    {
        std::map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(hashBlock);
        if (mi != mapBlockIndex.end())
            pindex = mi->second;
    }

    // Inputs: amounts and addresses come from the transactions they spend
    int64_t nValueIn = 0;
    bool fAllInputsKnown = true;
    QString inputs = "<table cellspacing='3'>";
    inputs += QString("<tr><th align='left'>#</th><th align='left' style='padding-left:12px'>%1</th><th align='right' style='padding-left:12px'>%2</th><th align='left' style='padding-left:12px'>%3</th></tr>")
                  .arg(tr("From"), tr("Amount"), tr("Spends"));
    for (unsigned int i = 0; i < tx.vin.size(); i++)
    {
        if ((int)i >= MAX_LIST_ROWS)
        {
            inputs += "<tr><td colspan='4'>" + tr("... and %1 more").arg(tx.vin.size() - MAX_LIST_ROWS) + "</td></tr>";
            fAllInputsKnown = false;
            break;
        }
        const CTxIn &txin = tx.vin[i];
        if (tx.IsCoinBase())
        {
            inputs += QString("<tr><td>%1</td><td colspan='3' style='padding-left:12px'>%2</td></tr>")
                          .arg(i).arg(tr("Newly generated coins (coinbase)"));
            continue;
        }
        QString from = tr("unknown"), amount = "-";
        CTransaction prev;
        uint256 hashPrevBlock = 0;
        if ((int)i < MAX_INPUT_LOOKUPS && GetTransaction(txin.prevout.hash, prev, hashPrevBlock) &&
            txin.prevout.n < prev.vout.size())
        {
            const CTxOut &out = prev.vout[txin.prevout.n];
            bool fMine;
            from = Destination(out.scriptPubKey, fMine);
            if (fMine)
                from += QString(" <b style='color:%1'>%2</b>").arg(GUIUtil::positiveColor(), tr("(yours)"));
            amount = Amount(out.nValue);
            nValueIn += out.nValue;
        }
        else
            fAllInputsKnown = false;
        inputs += QString("<tr><td>%1</td><td style='padding-left:12px'>%2</td><td align='right' style='padding-left:12px'>%3</td><td style='padding-left:12px'>%4 : %5</td></tr>")
                      .arg(i).arg(from, amount, TxLink(txin.prevout.hash)).arg(txin.prevout.n);
    }
    inputs += "</table>";

    QString outputs = "<table cellspacing='3'>";
    outputs += QString("<tr><th align='left'>#</th><th align='left' style='padding-left:12px'>%1</th><th align='right' style='padding-left:12px'>%2</th></tr>")
                   .arg(tr("To"), tr("Amount"));
    for (unsigned int i = 0; i < tx.vout.size(); i++)
    {
        if ((int)i >= MAX_LIST_ROWS)
        {
            outputs += "<tr><td colspan='3'>" + tr("... and %1 more").arg(tx.vout.size() - MAX_LIST_ROWS) + "</td></tr>";
            break;
        }
        const CTxOut &out = tx.vout[i];
        bool fMine;
        QString to = Destination(out.scriptPubKey, fMine);
        if (fMine)
            to += QString(" <b style='color:%1'>%2</b>").arg(GUIUtil::positiveColor(), tr("(yours)"));
        outputs += QString("<tr><td>%1</td><td style='padding-left:12px'>%2</td><td align='right' style='padding-left:12px'>%3</td></tr>")
                       .arg(i).arg(to, Amount(out.nValue));
    }
    outputs += "</table>";

    QString type = tx.IsCoinBase() ? tr("Coinbase") : tx.IsCoinStake() ? tr("Coinstake") : tr("Transfer");
    QString html = Heading(tr("Transaction"), type);
    html += "<table cellspacing='3'>";
    html += Row(tr("Transaction ID"), "<tt>" + QString::fromStdString(tx.GetHash().GetHex()) + "</tt>");
    if (pindex)
    {
        html += Row(tr("Block"), BlockLink(pindex));
        if (pindex->IsInMainChain())
            html += Row(tr("Confirmations"), QString::number(nBestHeight - pindex->nHeight + 1));
        else
            html += Row(tr("Status"), "<b>" + tr("In a block that is not in the main chain") + "</b>");
        html += Row(tr("Block time"), Time(pindex->GetBlockTime()));
    }
    else
        html += Row(tr("Status"), "<b>" + tr("Unconfirmed (waiting in the memory pool)") + "</b>");
    if (!tx.strTxComment.empty())
        html += Row(tr("Comment"), QString::fromStdString(tx.strTxComment).toHtmlEscaped());
    html += Row(tr("Size"), tr("%1 bytes").arg(::GetSerializeSize(tx, SER_NETWORK, PROTOCOL_VERSION)));
    if (!tx.IsCoinBase() && fAllInputsKnown)
        html += Row(tr("Total in"), Amount(nValueIn));
    html += Row(tr("Total out"), Amount(tx.GetValueOut()));
    if (!tx.IsCoinBase() && fAllInputsKnown)
    {
        if (tx.IsCoinStake())
            html += Row(tr("Stake reward"), Amount(tx.GetValueOut() - nValueIn));
        else
            html += Row(tr("Fee"), Amount(nValueIn - tx.GetValueOut()));
    }
    if (pwalletMain)
    {
        LOCK(pwalletMain->cs_wallet);
        if (pwalletMain->mapWallet.count(tx.GetHash()))
            html += Row(tr("Your wallet"), "<b style='color:" + GUIUtil::positiveColor() + "'>" + tr("This transaction is in your wallet") + "</b>");
    }
    html += "</table>";

    html += "<h3>" + tr("Inputs (%1)").arg(tx.vin.size()) + "</h3>" + inputs;
    html += "<h3>" + tr("Outputs (%1)").arg(tx.vout.size()) + "</h3>" + outputs;
    return html;
}
