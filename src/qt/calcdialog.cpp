#include "calcdialog.h"
#include "ui_calcdialog.h"

#include "clientmodel.h"
#include "notificator.h"
#include "version.h"
#include "bitcoinunits.h"
#include "optionsmodel.h"

#include "walletmodel.h"

#include "wallet.h"
#include "walletdb.h"
#include "main.h"
#include "init.h"
#include "net.h"

#include <algorithm>
#include <iomanip>
#include <QString>

calcDialog::calcDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::calcDialog)
{
    ui->setupUi(this);
	
	connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(pushButtonClicked()));
}

calcDialog::~calcDialog()
{
    delete ui;
}

void calcDialog::setModel(ClientModel *model)
{

}

// Uses the same reward rule the network enforces (GetPIRRewardCoinYear and
// GetProofOfStakeReward in main.cpp), so the estimate always matches what a
// stake of this size actually earns at the current block height.
void calcDialog::pushButtonClicked()
{
    bool fOk = false;
    double dAmount = ui->blockSizeEdit->text().trimmed().remove(',').toDouble(&fOk);
    if (!fOk || dAmount < 0 || dAmount > (double)MAX_MONEY / COIN)
    {
        ui->yearlyAPR->setText("-");
        ui->dayResult->setText(tr("Enter an amount of FLAPX"));
        return;
    }

    // Height decides the reward phase. While syncing our own height is old,
    // so use the tallest peer's if that is higher.
    int nHeight = std::max((int)nBestHeightUI, GetBestPeerHeight());
    int64_t nValue = (int64_t)(dAmount * COIN + 0.5);
    double dRate = (double)GetPIRRewardCoinYear(nValue, nHeight) / CENT;   // percent per year

    // Reward = coin-days x yearly rate / 365.24 (GetProofOfStakeReward)
    double dDaily = dAmount * dRate / 100.0 / 365.2424;
    ui->yearlyAPR->setText(QString::number(dRate, 'f', 2));
    ui->dayResult->setText(tr("%1 FLAPX a day, about %2 a month and %3 a year")
                           .arg(QString::number(dDaily, 'f', 4))
                           .arg(QString::number(dDaily * 30.44, 'f', 2))
                           .arg(QString::number(dDaily * 365.2424, 'f', 2)));
}

void calcDialog::on_buttonBox_accepted()
{
	close();
}
