#include "networkpage.h"
#include "ui_networkpage.h"

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


NetworkPage::NetworkPage(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NetworkPage)

{
    ui->setupUi(this);

    if (GetBoolArg("-staking", true))
    {
        QTimer *timerMyWeight = new QTimer();
        connect(timerMyWeight, SIGNAL(timeout()), this, SLOT(updateMyWeight()));
        timerMyWeight->start(30 * 1000);
        updateMyWeight();
    }

}
void NetworkPage::updateMyWeight()
{
    uint64_t nMinWeight = 0, nMaxWeight = 0, nWeight = 0;
    if (nLastCoinStakeSearchInterval && pwalletMain && !IsInitialBlockDownload()) //flapx GetStakeWeight requires mutex lock on wallet which tends to freeze initial block downloads
        pwalletMain->GetStakeWeight(*pwalletMain, nMinWeight, nMaxWeight, nWeight);

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

NetworkPage::~NetworkPage()
{
    delete ui;
}
