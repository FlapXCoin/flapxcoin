#include "networkpage.h"
#include "ui_networkpage.h"


NetworkPage::NetworkPage(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NetworkPage)

{
    ui->setupUi(this);
}

NetworkPage::~NetworkPage()
{
    delete ui;
}
