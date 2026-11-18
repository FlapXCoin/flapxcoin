// Copyright (c) 2014 Remy5 (original stake report)
// Copyright (c) 2026 FlapX Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "stakereportdialog.h"
#include "ui_stakereportdialog.h"

#include "walletmodel.h"
#include "optionsmodel.h"
#include "bitcoinunits.h"
#include "stakereport.h"
#include "wallet.h"
#include "main.h"
#include "init.h"

#include <QApplication>
#include <QClipboard>
#include <QDateTime>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QTimer>

// Refresh at most this often while new blocks come in (the report scans the
// whole wallet); Refresh and reopening the window always update at once
static const int STAKE_REPORT_MIN_INTERVAL = 60;

StakeReportDialog::StakeReportDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::StakeReportDialog),
    model(0),
    nLastHeight(-1),
    nLastRefresh(0),
    fRetry(false)
{
    ui->setupUi(this);

    QTableWidget *table = ui->tableDays;
    table->setRowCount(STAKE_REPORT_DAYS);
#if QT_VERSION < 0x050000
    table->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
#else
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
#endif
    for (int row = 0; row < STAKE_REPORT_DAYS; row++)
        for (int col = 0; col < 3; col++)
        {
            QTableWidgetItem *item = new QTableWidgetItem();
            item->setTextAlignment(col == 0 ? Qt::AlignLeft | Qt::AlignVCenter : Qt::AlignRight | Qt::AlignVCenter);
            table->setItem(row, col, item);
        }

    connect(ui->buttonRefresh, SIGNAL(clicked()), this, SLOT(refresh()));
    connect(ui->buttonCopy, SIGNAL(clicked()), this, SLOT(copyToClipboard()));

    // Checks every few seconds whether a refresh is due (only while shown)
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(tick()));
    timer->start(3000);
}

StakeReportDialog::~StakeReportDialog()
{
    delete ui;
}

void StakeReportDialog::setModel(WalletModel *model)
{
    this->model = model;
    if (model && model->getOptionsModel())
        connect(model->getOptionsModel(), SIGNAL(displayUnitChanged(int)), this, SLOT(refresh()));
}

void StakeReportDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    refresh();
}

void StakeReportDialog::tick()
{
    if (!isVisible())
        return;
    // The wallet was busy last time, or new blocks came in (at most once a minute)
    if (fRetry || (nBestHeight != nLastHeight && QDateTime::currentDateTime().toTime_t() - nLastRefresh >= STAKE_REPORT_MIN_INTERVAL))
        refresh();
}

void StakeReportDialog::refresh()
{
    if (!pwalletMain)
        return;

    StakeReport report;
    // Never wait for the wallet: if it's busy (staking, a new block), say so
    // and try again in a few seconds
    if (!GetStakeReport(pwalletMain, report, true))
    {
        fRetry = true;
        ui->labelStatus->setText(tr("Wallet busy, trying again..."));
        return;
    }
    fRetry = false;
    nLastHeight = nBestHeight;
    nLastRefresh = QDateTime::currentDateTime().toTime_t();
    showReport(report);
}

void StakeReportDialog::showReport(const StakeReport &report)
{
    int unit = (model && model->getOptionsModel()) ? model->getOptionsModel()->getDisplayUnit() : BitcoinUnits::BTC;

    // vTotals: last 24 hours, 7, 30, 365 days
    QLabel *amounts[] = { ui->amount24h, ui->amount7d, ui->amount30d, ui->amount365d };
    QLabel *counts[] = { ui->count24h, ui->count7d, ui->count30d, ui->count365d };
    for (int i = 0; i < 4 && i < (int)report.vTotals.size(); i++)
    {
        amounts[i]->setText(BitcoinUnits::formatWithUnit(unit, report.vTotals[i].nAmount));
        counts[i]->setText(QString::number(report.vTotals[i].nCount));
    }

    if (report.nLatestTime)
        ui->labelLatest->setText(tr("Latest stake: %1, %2")
            .arg(QDateTime::fromTime_t((uint)report.nLatestTime).toString("yyyy-MM-dd hh:mm"))
            .arg(BitcoinUnits::formatWithUnit(unit, report.nLatestAmount, true)));
    else
        ui->labelLatest->setText(tr("Latest stake: none yet"));

    if (report.nMaturingCount)
        ui->labelMaturing->setText(tr("Still maturing (included above): %1 in %n stake(s)", "", report.nMaturingCount)
            .arg(BitcoinUnits::formatWithUnit(unit, report.nMaturingAmount)));
    else
        ui->labelMaturing->setText(tr("Still maturing: none"));

    QTableWidget *table = ui->tableDays;
    for (int row = 0; row < STAKE_REPORT_DAYS && row < (int)report.vDays.size(); row++)
    {
        const StakePeriod &day = report.vDays[row];
        QString date = QString::fromStdString(day.strName);
        if (row == 0)
            date += " " + tr("(today)");
        table->item(row, 0)->setText(date);
        table->item(row, 1)->setText(day.nCount ? BitcoinUnits::formatWithUnit(unit, day.nAmount) : QString("-"));
        table->item(row, 2)->setText(QString::number(day.nCount));
    }

    ui->labelStatus->setText(tr("Updated %1 at block %2, %3 stakes counted")
        .arg(QDateTime::currentDateTime().toString("hh:mm"))
        .arg(nLastHeight)
        .arg(report.nStakesCounted));
}

void StakeReportDialog::copyToClipboard()
{
    QString text;
    text += tr("FlapX stake report") + "\n";
    text += tr("Last 24 hours") + ": " + ui->amount24h->text() + " (" + ui->count24h->text() + ")\n";
    text += tr("Last 7 days") + ": " + ui->amount7d->text() + " (" + ui->count7d->text() + ")\n";
    text += tr("Last 30 days") + ": " + ui->amount30d->text() + " (" + ui->count30d->text() + ")\n";
    text += tr("Last 365 days") + ": " + ui->amount365d->text() + " (" + ui->count365d->text() + ")\n";
    text += ui->labelLatest->text() + "\n" + ui->labelMaturing->text() + "\n\n";

    QTableWidget *table = ui->tableDays;
    text += tr("Date") + "\t" + tr("Earned") + "\t" + tr("Stakes") + "\n";
    for (int row = 0; row < table->rowCount(); row++)
        text += table->item(row, 0)->text() + "\t" + table->item(row, 1)->text() + "\t" + table->item(row, 2)->text() + "\n";
    text += "\n" + ui->labelStatus->text() + "\n";

    QApplication::clipboard()->setText(text);
    ui->labelStatus->setText(tr("Report copied to the clipboard"));
}
