#include "overviewpage.h"
#include "ui_overviewpage.h"

#include "walletmodel.h"
#include "clientmodel.h"
#include "main.h"
#include "bitcoinunits.h"
#include "init.h"
#include "base58.h"
#include "bitcoingui.h"
#include "calcdialog.h"
#include "optionsmodel.h"
#include "transactiontablemodel.h"
#include "transactionfilterproxy.h"
#include "guiutil.h"
#include "guiconstants.h"
#include "wallet.h"
#include "bitcoinrpc.h"
#include "askpassphrasedialog.h"

#include <QAbstractItemDelegate>
#include <QPainter>
#include <QIcon>
#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QFrame>
#include <sstream>
#include <string>
#include <QMenu>

#define DECORATION_SIZE 85
#define NUM_ITEMS 5

using namespace json_spirit;
extern CWallet* pwalletMain;
extern int64_t nLastCoinStakeSearchInterval;
double GetPoSKernelPS();

class TxViewDelegate : public QAbstractItemDelegate
{
    Q_OBJECT
public:
    TxViewDelegate(): QAbstractItemDelegate(), unit(BitcoinUnits::BTC)
    {

    }

    inline void paint(QPainter *painter, const QStyleOptionViewItem &option,
                      const QModelIndex &index ) const
    {
        painter->save();

        QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
        QRect mainRect = option.rect;
        QRect decorationRect(mainRect.topLeft(), QSize(DECORATION_SIZE, DECORATION_SIZE));
        int xspace = DECORATION_SIZE + 8;
        int ypad = 6;
        int halfheight = (mainRect.height() - 2*ypad)/2;
        QRect amountRect(mainRect.left() + xspace, mainRect.top()+ypad, mainRect.width() - xspace, halfheight);
        QRect addressRect(mainRect.left() + xspace, mainRect.top()+ypad+halfheight, mainRect.width() - xspace, halfheight);
        icon.paint(painter, decorationRect);

        QDateTime date = index.data(TransactionTableModel::DateRole).toDateTime();
        QString address = index.data(Qt::DisplayRole).toString();
        qint64 amount = index.data(TransactionTableModel::AmountRole).toLongLong();
        bool confirmed = index.data(TransactionTableModel::ConfirmedRole).toBool();
        QVariant value = index.data(Qt::ForegroundRole);
        QColor foreground = option.palette.color(QPalette::Text);
        if(qVariantCanConvert<QColor>(value))
        {
            foreground = qvariant_cast<QColor>(value);
        }

        painter->setPen(foreground);
        painter->drawText(addressRect, Qt::AlignLeft|Qt::AlignVCenter, address);

        if(amount < 0)
        {
            foreground = COLOR_NEGATIVE;
        }
        else if(!confirmed)
        {
            foreground = COLOR_UNCONFIRMED;
        }
        else
        {
            foreground = option.palette.color(QPalette::Text);
        }
        painter->setPen(foreground);
        QString amountText = BitcoinUnits::formatWithUnit(unit, amount, true);
        if(!confirmed)
        {
            amountText = QString("[") + amountText + QString("]");
        }
        painter->drawText(amountRect, Qt::AlignRight|Qt::AlignVCenter, amountText);

        painter->setPen(option.palette.color(QPalette::Text));
        painter->drawText(amountRect, Qt::AlignLeft|Qt::AlignVCenter, GUIUtil::dateTimeStr(date));

        painter->restore();
    }

    inline QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        return QSize(DECORATION_SIZE, DECORATION_SIZE);
    }

    int unit;

};
#include "overviewpage.moc"

OverviewPage::OverviewPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OverviewPage),
    clientModel(0),
    walletModel(0),
    modelStatistics(0),
    currentBalance(-1),
    currentStake(0),
    currentUnconfirmedBalance(-1),
    currentImmatureBalance(-1),
    currentWeight(0),
    currentNetworkWeight(0),

    txdelegate(new TxViewDelegate()),
    filter(0)
{

    ui->setupUi(this);

    if (GetBoolArg("-staking", true))
    {
        QTimer *timerMyWeight = new QTimer();
        connect(timerMyWeight, SIGNAL(timeout()), this, SLOT(updateMyWeight()));
        timerMyWeight->start(30 * 1000);
        updateMyWeight();
    }
    QAction *stakeForCharityAction = new QAction(ui->startButton->text(), this);

    contextMenu = new QMenu();
    contextMenu->addAction(stakeForCharityAction);
    connect(stakeForCharityAction, SIGNAL(triggered()), this, SLOT(on_startButton_clicked()));

    // Recent transactions
    ui->listTransactions->setItemDelegate(txdelegate);
    ui->listTransactions->setIconSize(QSize(DECORATION_SIZE, DECORATION_SIZE));
    ui->listTransactions->setMinimumHeight(NUM_ITEMS * (DECORATION_SIZE + 2));
    ui->listTransactions->setAttribute(Qt::WA_MacShowFocusRect, false);

    connect(ui->listTransactions, SIGNAL(clicked(QModelIndex)), this, SLOT(handleTransactionClicked(QModelIndex)));

    // init "out of sync" warning labels
    ui->labelTransactionsStatus->setText("(" + tr("out of sync") + ")");

    // start with displaying the "out of sync" warnings
    showOutOfSyncWarning(true);
}



void OverviewPage::handleTransactionClicked(const QModelIndex &index)
{
    if(filter)
        emit transactionClicked(filter->mapToSource(index));
}



void OverviewPage::setBalance(qint64 balance, qint64 stake, qint64 unconfirmedBalance, qint64 immatureBalance)
{
    // int unit = model->getOptionsModel()->getDisplayUnit();
    int unit = walletModel->getOptionsModel()->getDisplayUnit();
    currentBalance = balance;
    currentStake = stake;
    currentUnconfirmedBalance = unconfirmedBalance;
    currentImmatureBalance = immatureBalance;
    ui->labelBalance->setText(BitcoinUnits::formatWithUnit(unit, balance));
    ui->labelStake->setText(BitcoinUnits::formatWithUnit(unit, stake));
    ui->labelUnconfirmed->setText(BitcoinUnits::formatWithUnit(unit, unconfirmedBalance));
    ui->labelImmature->setText(BitcoinUnits::formatWithUnit(unit, immatureBalance));
    ui->labelTotal->setText(BitcoinUnits::formatWithUnit(unit, balance + stake + unconfirmedBalance + immatureBalance));

    // only show immature (newly mined) balance if it's non-zero, so as not to complicate things
    // for the non-mining users
    bool showImmature = immatureBalance != 0;
    ui->labelImmature->setVisible(showImmature);
    ui->labelImmatureText->setVisible(showImmature);

    // qint64 interest = model->getTransactionTableModel()->getInterestGenerated();
    qint64 interest = walletModel->getTransactionTableModel()->getInterestGenerated();
     // Setup display values for 'interest summary' widget at bottom right
    ui->labelInterest->setText(BitcoinUnits::formatWithUnit(unit, interest));
}

void OverviewPage::setNumTransactions(int count)
{
    ui->labelNumTransactions->setText(QLocale::system().toString(count));
}


void OverviewPage::updateMyWeight()
{
    uint64_t nMinWeight = 0, nMaxWeight = 0, nWeight = 0;
    // Skipped while catching up (newest block over an hour old): GetStakeWeight
    // reads every coin's block from disk and would slow the block download
    if (nLastCoinStakeSearchInterval && pwalletMain && pindexBest && pindexBest->GetBlockTime() > GetTime() - 60 * 60)
    {
        // Skip this refresh if the core is busy rather than freezing the window
        TRY_LOCK(cs_main, lockMain);
        if (!lockMain)
            return;
        TRY_LOCK(pwalletMain->cs_wallet, lockWallet);
        if (!lockWallet)
            return;
        pwalletMain->GetStakeWeight(*pwalletMain, nMinWeight, nMaxWeight, nWeight);
    }

    if (nLastCoinStakeSearchInterval && nWeight)
    {
        uint64_t nNetworkWeight = GetPoSKernelPS();
        unsigned nEstimateTime = nTargetSpacing * 2 * nNetworkWeight / nWeight;

        QString text;
        if (nEstimateTime < 60)
        {
           // text = tr("%n second(s)", "", nEstimateTime);
        }
        else if (nEstimateTime < 60*60)
        {
           // text = tr("%n minute(s)", "", nEstimateTime/60);
        }
        else if (nEstimateTime < 24*60*60)
        {
           // text = tr("%n hour(s)", "", nEstimateTime/(60*60));
        }
        else
        {
           // text = tr("%n day(s)", "", nEstimateTime/(60*60*24));
        }

        ui->labelMyWeight->setText(tr("Staking.<br>Your weight is %1<br>Network weight is %2").arg(nWeight).arg(nNetworkWeight));
    }
    else
    {
        if (pwalletMain && pwalletMain->IsLocked())
            ui->labelMyWeight->setText(tr("Not staking because your wallet is locked,<br> please unlock for staking."));
        else if (vNodes.empty())
            ui->labelMyWeight->setText(tr("Not staking because your wallet is offline,<br> please wait for a connection..."));
        else if (IsInitialBlockDownload())
            ui->labelMyWeight->setText(tr("Not staking because your wallet is syncing,<br> please wait for this process to end..."));
        else if (!nWeight)
            ui->labelMyWeight->setText(tr("Not staking because you don't have mature coins..."));
        else
            ui->labelMyWeight->setText(tr("Not staking"));
    }
}

void OverviewPage::on_startButton_clicked()
{
    return stakeForCharitySignal();
}

// void OverviewPage::setModel(WalletModel *model)
void OverviewPage::setClientModel(ClientModel *model)
{
    // this->model = model;
    this->clientModel = model;
    // The Network box reads block count, nodes etc. from the client model
    this->modelStatistics = model;
    if(model)
    {
        updateStatistics();
        // Show warning if this is a prerelease version
        connect(model, SIGNAL(alertsChanged(QString)), this, SLOT(updateAlerts(QString)));
        updateAlerts(model->getStatusBarWarnings());
    }
}

void OverviewPage::setWalletModel(WalletModel *model)
{
    this->walletModel = model;
    if(model && model->getOptionsModel())
    {
        // Set up transaction list
        filter = new TransactionFilterProxy();
        filter->setSourceModel(model->getTransactionTableModel());
        filter->setLimit(NUM_ITEMS);
        filter->setDynamicSortFilter(true);
        filter->setSortRole(Qt::EditRole);
        filter->setShowInactive(false);
        filter->sort(TransactionTableModel::Date, Qt::DescendingOrder);

        ui->listTransactions->setModel(filter);
        ui->listTransactions->setModelColumn(TransactionTableModel::ToAddress);

        // Keep up to date with wallet
        setBalance(model->getBalance(), model->getStake(), model->getUnconfirmedBalance(), model->getImmatureBalance());
        connect(model, SIGNAL(balanceChanged(qint64, qint64, qint64, qint64)), this, SLOT(setBalance(qint64, qint64, qint64, qint64)));

        setNumTransactions(model->getNumTransactions());
        connect(model, SIGNAL(numTransactionsChanged(int)), this, SLOT(setNumTransactions(int)));

        connect(model->getOptionsModel(), SIGNAL(displayUnitChanged(int)), this, SLOT(updateDisplayUnit()));
    }

    // update the display unit, to not use the default ("BTC")
    updateDisplayUnit();

    // update statistics
    updateStatistics();

    //set up a timer to auto refresh every 30 seconds to update the statistics
    QTimer *timerNetworkStats = new QTimer();
    connect(timerNetworkStats, SIGNAL(timeout()), this, SLOT(updateStatistics()));
    timerNetworkStats->start(30 * 1000);

    // Unlock wallet button
    WalletModel::EncryptionStatus status = model->getEncryptionStatus();
    if(status == WalletModel::Unencrypted)
    {
        ui->unlockWalletActionNew->setDisabled(true);
    }
    connect(ui->unlockWalletActionNew, SIGNAL(clicked()), this, SLOT(lockWalletToggle()));
}

void OverviewPage::updateDisplayUnit()
{
    // if(model && model->getOptionsModel())
    if(walletModel && walletModel->getOptionsModel())
    {
        if(currentBalance != -1)
            // setBalance(currentBalance, model->getStake(), currentUnconfirmedBalance, currentImmatureBalance);
            setBalance(currentBalance, walletModel->getStake(), currentUnconfirmedBalance, currentImmatureBalance);

        // Update txdelegate->unit with the current unit
        // txdelegate->unit = model->getOptionsModel()->getDisplayUnit();
        txdelegate->unit = walletModel->getOptionsModel()->getDisplayUnit();

        ui->listTransactions->update();
    }
}

void OverviewPage::updateAlerts(const QString &warnings)
{
    this->ui->labelAlerts->setVisible(!warnings.isEmpty());
    this->ui->labelAlerts->setText(warnings);

}

void OverviewPage::showOutOfSyncWarning(bool fShow)
{
    ui->labelTransactionsStatus->setVisible(fShow);
}

void OverviewPage::lockWalletToggle()
{
    // if(model->getEncryptionStatus() == WalletModel::Locked)
    if(walletModel->getEncryptionStatus() == WalletModel::Locked)
    {
        AskPassphraseDialog dlg(AskPassphraseDialog::UnlockStaking, this);
        // dlg.setModel(model);
        dlg.setModel(walletModel);
        if(dlg.exec() == QDialog::Accepted)
        {
            ui->unlockWalletActionNew->setText("Lock FlapX");
        }
    }
    else
    {
        // model->setWalletLocked(true);
        walletModel->setWalletLocked(true);
        ui->unlockWalletActionNew->setText("Unlock FlapX");
    }
}

// While the wallet is catching up only the block count and node count are
// shown; everything else reads "Wallet syncing..."
void OverviewPage::showSyncing()
{
    const QString syncing = "<i>" + tr("Wallet syncing...") + "</i>";
    ui->heightBox->setText(QString::number(nBestHeight));
    ui->rewardBox->setText(syncing);
    ui->volumeBox->setText(syncing);
    ui->diffBox->setText(syncing);
    ui->diffBox2->setText(syncing);
    ui->pawrateBox->setText(syncing);
    ui->localBox->setText(syncing);
    if (modelStatistics)
        ui->connectionBox->setText(QString::number(modelStatistics->getNumConnections()));
}

void OverviewPage::updateStatistics()
{
    if (!modelStatistics || !pindexBest)
        return;

    // Until the wallet has caught up (newest block less than an hour old), only
    // show that it's syncing: working out these figures on every refresh slows
    // the wallet down while it's downloading blocks
    if (pindexBest->GetBlockTime() < GetTime() - 60 * 60)
    {
        showSyncing();
        return;
    }

    // Skip this refresh if the core is busy, rather than freezing the window
    // while waiting for it (the timer tries again in 30 seconds)
    TRY_LOCK(cs_main, lockMain);
    if (!lockMain)
        return;

    double pHardness = GetDifficulty();
    double pHardness2 = GetDifficulty(GetLastBlockIndex(pindexBest, true));
    int pPawrate = GetPoWMHashPS();
    double pPawrate2 = 0.000;
    int nHeight = pindexBest->nHeight;
    double nSubsidy = double (GetProofOfWorkReward(nHeight, 0, pindexBest->GetBlockHash())/ double (COIN));
    // Stake weight isn't shown in this box (the staking label has its own refresh)
    uint64_t nMinWeight = 0, nNetworkWeight = 0;
    int64_t volume = ((pindexBest->nMoneySupply)/100000000);
    int peers = this->modelStatistics->getNumConnections();
    pPawrate2 = (double)pPawrate;
    QString height = QString::number(nHeight);
    QString subsidy = QString::number(nSubsidy, 'f', 6);
    QString hardness = QString::number(pHardness, 'f', 6);
    QString hardness2 = QString::number(pHardness2, 'f', 6);
    QString pawrate = QString::number(pPawrate2, 'f', 3);
    QString Qlpawrate = modelStatistics->getLastBlockDate().toString();

    QString QPeers = QString::number(peers);
    QString qVolume = QLocale::system().toString((qlonglong)volume);

    if(nHeight > heightPrevious)
    {
        ui->heightBox->setText("<b><font color=\"" + GUIUtil::positiveColor() + "\">" + height + "</font></b>");
    } 
    else 
    {
        ui->heightBox->setText(height);
    }

    if(nSubsidy < rewardPrevious)
    {
        ui->rewardBox->setText("<b><font color=\"" + GUIUtil::negativeColor() + "\">" + subsidy + "</font></b>");
    } 
    else 
    {
        ui->rewardBox->setText(subsidy);
    }

    if(pHardness > hardnessPrevious)
    {
        ui->diffBox->setText("<b><font color=\"" + GUIUtil::positiveColor() + "\">" + hardness + "</font></b>");
    } 
    else if(pHardness < hardnessPrevious) 
    {
        ui->diffBox->setText("<b><font color=\"" + GUIUtil::negativeColor() + "\">" + hardness + "</font></b>");
    } 
    else 
    {
        ui->diffBox->setText(hardness);
    }

    if(pHardness2 > hardnessPrevious2)
    {
        ui->diffBox2->setText("<b><font color=\"" + GUIUtil::positiveColor() + "\">" + hardness2 + "</font></b>");
    }
    else if(pHardness2 < hardnessPrevious2)
    {
        ui->diffBox2->setText("<b><font color=\"" + GUIUtil::negativeColor() + "\">" + hardness2 + "</font></b>");
    }
    else
    {
        ui->diffBox2->setText(hardness2);
    }

    if(pPawrate2 > netPawratePrevious)
    {
        ui->pawrateBox->setText("<b><font color=\"" + GUIUtil::positiveColor() + "\">" + pawrate + " MH/s</font></b>");
    } 
    else if(pPawrate2 < netPawratePrevious) 
    {
        ui->pawrateBox->setText("<b><font color=\"" + GUIUtil::negativeColor() + "\">" + pawrate + " MH/s</font></b>");
    } 
    else 
    {
        ui->pawrateBox->setText(pawrate + " MH/s");
    }

    if(Qlpawrate != pawratePrevious)
    {
        ui->localBox->setText("<b><font color=\"" + GUIUtil::positiveColor() + "\">" + Qlpawrate + "</font></b>");
    } 
    else 
    {
    ui->localBox->setText(Qlpawrate);
    }

    if(peers > connectionPrevious)
    {
        ui->connectionBox->setText("<b><font color=\"" + GUIUtil::positiveColor() + "\">" + QPeers + "</font></b>");
    } 
    else if(peers < connectionPrevious) 
    {
        ui->connectionBox->setText("<b><font color=\"" + GUIUtil::negativeColor() + "\">" + QPeers + "</font></b>");
    } 
    else 
    {
        ui->connectionBox->setText(QPeers);
    }

    if(volume > volumePrevious)
    {
        ui->volumeBox->setText("<b>" + qVolume + " FLAPX</b>");
    } 
    else if(volume < volumePrevious) 
    {
        ui->volumeBox->setText("<b>" + qVolume + " FLAPX</b>");
    } 
    else 
    {
        ui->volumeBox->setText(qVolume + " FLAPX");
    }
    
    updatePrevious(nHeight, nMinWeight, nNetworkWeight, nSubsidy, pHardness, pHardness2, pPawrate2, Qlpawrate, peers, volume);
}

void OverviewPage::updatePrevious(int nHeight, int nMinWeight, int nNetworkWeight, double nSubsidy, double pHardness, double pHardness2, double pPawrate2, QString Qlpawrate, int peers, int volume)
{
    heightPrevious = nHeight;
    stakeminPrevious = nMinWeight;
    stakemaxPrevious = nNetworkWeight;
    rewardPrevious = nSubsidy;
    hardnessPrevious = pHardness;
    hardnessPrevious2 = pHardness2;
    netPawratePrevious = pPawrate2;
    pawratePrevious = Qlpawrate;
    connectionPrevious = peers;
    volumePrevious = volume;
}

void OverviewPage::setStatistics(ClientModel *modelStatistics)
{
    this->modelStatistics = modelStatistics;
    updateStatistics();

}

OverviewPage::~OverviewPage()
{
    delete ui;
}



void OverviewPage::on_unlockWalletActionNew_clicked()
{

}
