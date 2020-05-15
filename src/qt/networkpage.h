#ifndef NETWORKPAGE_H
#define NETWORKPAGE_H

#include <QDialog>

namespace Ui {
    class NetworkPage;
}


class NetworkPage : public QDialog
{
    Q_OBJECT

public:
    explicit NetworkPage(QWidget *parent = 0);
    ~NetworkPage();

public slots:


private:
    Ui::NetworkPage *ui;


private slots:

signals:

};

#endif
