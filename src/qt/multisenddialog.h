#ifndef MULTISENDDIALOG_H
#define MULTISENDDIALOG_H

#include <QWidget>

namespace Ui {
    class MultiSendDialog;
}
class WalletModel;

QT_BEGIN_NAMESPACE
class QLineEdit;
QT_END_NAMESPACE

class MultiSendDialog : public QWidget
{
    Q_OBJECT

public:
    explicit MultiSendDialog(QWidget *parent = 0);
    ~MultiSendDialog();

    void setModel(WalletModel *model);
    void setAddress(const QString &address);
    void setAddress(const QString &address, QLineEdit *addrEdit);
private slots:
    void on_enableButton_clicked();
    void on_disableButton_clicked();
    void on_addressBookButton_clicked();
    void on_changeAddressBookButton_clicked();


private:
    Ui::MultiSendDialog *ui;
    WalletModel *model;
};

#endif // MULTISENDDIALOG_H
