#include "multisenddialog.h"
#include "guiutil.h"
#include "ui_multisenddialog.h"

#include "walletmodel.h"
#include "addressbookpage.h"

#include "init.h"
#include "base58.h"

#include <QLineEdit>

MultiSendDialog::MultiSendDialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MultiSendDialog),
    model(0)
{
    ui->setupUi(this);

#if (QT_VERSION >= 0x040700)
    /* Do not move this to the XML file, Qt before 4.7 will choke on it */
    ui->multiSendPercentEdit->setPlaceholderText(tr("Enter % to Give (1-50)"));
    ui->multiSendAddressEdit->setPlaceholderText(tr("Enter the recipient address"));
    ui->multiSendMinEdit->setPlaceholderText(tr("Enter Min Amount (optional)"));
    ui->multiSendMaxEdit->setPlaceholderText(tr("Enter Max Amount (optional)"));
    ui->multiSendChangeAddressEdit->setPlaceholderText(tr("Enter Change Address (optional)"));
#endif

    ui->label_2->setFocus();
}

MultiSendDialog::~MultiSendDialog()
{
    delete ui;
}

void MultiSendDialog::setModel(WalletModel *model)
{
    this->model = model;

    CBitcoinAddress strAddress;
    CBitcoinAddress strChangeAddress;
    int nPer;
    qint64 nMin;
    qint64 nMax;

    model->getMultiSend(nPer, strAddress, strChangeAddress, nMin, nMax);

    if (strAddress.IsValid() && nPer > 0 )
    {
        ui->multiSendAddressEdit->setText(strAddress.ToString().c_str());
        ui->multiSendPercentEdit->setText(QString::number(nPer));
        if (strChangeAddress.IsValid())
            ui->multiSendChangeAddressEdit->setText(strChangeAddress.ToString().c_str());
        if (nMin > 0  && nMin != MIN_TX_FEE)
            ui->multiSendMinEdit->setText(QString::number(nMin/COIN));
        if (nMax > 0 && nMax != MAX_MONEY)
            ui->multiSendMaxEdit->setText(QString::number(nMax/COIN));
        ui->message->setStyleSheet("QLabel { color: " + GUIUtil::positiveColor() + "; }");
        ui->message->setText(tr("Multisend is on, sending to\n") + strAddress.ToString().c_str() + tr("."));
    }
}

void MultiSendDialog::setAddress(const QString &address)
{
    setAddress(address, ui->multiSendAddressEdit);
}

void MultiSendDialog::setAddress(const QString &address, QLineEdit *addrEdit)
{
    addrEdit->setText(address);
    addrEdit->setFocus();
}

void MultiSendDialog::on_addressBookButton_clicked()
{
    if (model && model->getAddressTableModel())
    {
        AddressBookPage dlg(AddressBookPage::ForSending, AddressBookPage::SendingTab, this);
        dlg.setModel(model->getAddressTableModel());
        if (dlg.exec())
            setAddress(dlg.getReturnValue(), ui->multiSendAddressEdit);
    }
}

void MultiSendDialog::on_changeAddressBookButton_clicked()
{
    if (model && model->getAddressTableModel())
    {
        AddressBookPage dlg(AddressBookPage::ForSending, AddressBookPage::ReceivingTab, this);
        dlg.setModel(model->getAddressTableModel());
        if (dlg.exec())
            setAddress(dlg.getReturnValue(), ui->multiSendChangeAddressEdit);
    }
}

void MultiSendDialog::on_enableButton_clicked()
{
    if(model->getEncryptionStatus() == WalletModel::Locked)
    {
        ui->message->setStyleSheet("");
        ui->message->setText(tr("Please unlock the wallet before starting Multisend."));
        return;
    }

    bool fValidConversion = false;
    qint64 nMinAmount = MIN_TXOUT_AMOUNT;
    qint64 nMaxAmount = MAX_MONEY;
    CBitcoinAddress changeAddress = "";

    CBitcoinAddress address = ui->multiSendAddressEdit->text().toStdString();
    if (!address.IsValid())
    {
        ui->message->setStyleSheet("QLabel { color: " + GUIUtil::negativeColor() + "; }");
        ui->message->setText(tr("The entered address:\n") + ui->multiSendAddressEdit->text() + tr(" is invalid.\nPlease check the address and try again."));
        ui->multiSendAddressEdit->setFocus();
        return;
    }

    int nMultiSendPercent = ui->multiSendPercentEdit->text().toInt(&fValidConversion, 10);
    if (!fValidConversion || nMultiSendPercent > 50 || nMultiSendPercent <= 0)
    {
        ui->message->setStyleSheet("QLabel { color: " + GUIUtil::negativeColor() + "; }");
        ui->message->setText(tr("Please enter a percentage from 1 to 50."));
        ui->multiSendPercentEdit->setFocus();
        return;
    }

    if (!ui->multiSendMinEdit->text().isEmpty())
    {
        nMinAmount = ui->multiSendMinEdit->text().toDouble(&fValidConversion) * COIN;
        if(!fValidConversion || nMinAmount <= MIN_TXOUT_AMOUNT || nMinAmount >= MAX_MONEY  )
        {
            ui->message->setStyleSheet("QLabel { color: " + GUIUtil::negativeColor() + "; }");
            ui->message->setText(tr("Min amount is out of range, please re-enter."));
            ui->multiSendMinEdit->setFocus();
            return;
        }
    }

    if (!ui->multiSendMaxEdit->text().isEmpty())
    {
        nMaxAmount = ui->multiSendMaxEdit->text().toDouble(&fValidConversion) * COIN;
        if(!fValidConversion || nMaxAmount <= MIN_TXOUT_AMOUNT || nMaxAmount >= MAX_MONEY  )
        {
            ui->message->setStyleSheet("QLabel { color: " + GUIUtil::negativeColor() + "; }");
            ui->message->setText(tr("Max amount is out of range, please re-enter."));
            ui->multiSendMaxEdit->setFocus();
            return;
        }
    }

    if (nMinAmount >= nMaxAmount)
    {
        ui->message->setStyleSheet("QLabel { color: " + GUIUtil::negativeColor() + "; }");
        ui->message->setText(tr("Min amount is larger than max amount, please re-enter."));
        ui->multiSendMinEdit->setFocus();
        return;
    }

    if (!ui->multiSendChangeAddressEdit->text().isEmpty())
    {
        changeAddress = ui->multiSendChangeAddressEdit->text().toStdString();
        if (!changeAddress.IsValid())
        {
            ui->message->setStyleSheet("QLabel { color: " + GUIUtil::negativeColor() + "; }");
            ui->message->setText(tr("The entered change address:\n") + ui->multiSendChangeAddressEdit->text() + tr(" is invalid.\nPlease check the address and try again."));
            ui->multiSendChangeAddressEdit->setFocus();
            return;
        }
        else if (!model->isMine(changeAddress))
        {
           ui->message->setStyleSheet("QLabel { color: " + GUIUtil::negativeColor() + "; }");
           ui->message->setText(tr("The entered change address:\n") + ui->multiSendChangeAddressEdit->text() + tr(" is not owned.\nPlease check the address and try again."));
           ui->multiSendChangeAddressEdit->setFocus();
           return;
        }
    }

    model->setMultiSend(true, nMultiSendPercent, address, changeAddress, nMinAmount, nMaxAmount);
    if(!fGlobalMultiSend)
         fGlobalMultiSend = true;
    ui->message->setStyleSheet("QLabel { color: " + GUIUtil::positiveColor() + "; }");
    ui->message->setText(tr("Multisend is on, sending to\n") + QString(address.ToString().c_str()) + tr("."));
    return;
}

void MultiSendDialog::on_disableButton_clicked()
{
    int nMultiSendPercent = 0;
    CBitcoinAddress address = "";
    CBitcoinAddress changeAddress = "";
    qint64 nMinAmount = MIN_TXOUT_AMOUNT;
    qint64 nMaxAmount = MAX_MONEY;

    model->setMultiSend(false, nMultiSendPercent, address, changeAddress, nMinAmount, nMaxAmount);
    ui->multiSendAddressEdit->clear();
    ui->multiSendChangeAddressEdit->clear();
    ui->multiSendMaxEdit->clear();
    ui->multiSendMinEdit->clear();
    ui->multiSendPercentEdit->clear();
    ui->message->setStyleSheet("");
    ui->message->setText(tr("Multisend is now off"));
    return;
}
