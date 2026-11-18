// Copyright (c) 2014 Remy5 (original stake report)
// Copyright (c) 2026 FlapX Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
//
// APR Tracking > Stake Report: what this wallet earned from staking.
#ifndef STAKEREPORTDIALOG_H
#define STAKEREPORTDIALOG_H

#include <QDialog>

namespace Ui {
    class StakeReportDialog;
}
class WalletModel;
class QTimer;
struct StakeReport;

class StakeReportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit StakeReportDialog(QWidget *parent = 0);
    ~StakeReportDialog();

    void setModel(WalletModel *model);

protected:
    void showEvent(QShowEvent *event);

private:
    Ui::StakeReportDialog *ui;
    WalletModel *model;
    QTimer *timer;
    int nLastHeight;          // chain height at the last refresh
    qint64 nLastRefresh;      // when the report was last worked out (seconds)
    bool fRetry;              // the wallet was busy: try again soon

    void showReport(const StakeReport &report);

private slots:
    void refresh();
    void tick();
    void copyToClipboard();
};

#endif // STAKEREPORTDIALOG_H
