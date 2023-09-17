#ifndef SHOPPINGPAGE_H
#define SHOPPINGPAGE_H

#include "clientmodel.h"
#include "main.h"
#include "wallet.h"
#include "base58.h"
#include <QWidget>

#include <QDir>
#include <QFile>
#include <QProcess>
#include <QTime>
#include <QTimer>
#include <QStringList>
#include <QMap>
#include <QSettings>
#include <QSlider>

#include <QDialog>

double getBlockHardness(int);
double getTxTotalValue(std::string);
double convertCoins(int64_t);
double getTxFees(std::string);
int getBlockTime(int);
int getBlocknBits(int);
int getBlockNonce(int);
int blocksInPastHours(int);
int getBlockHashrate(int);
std::string getInputs(std::string);
std::string getOutputs(std::string);
std::string getBlockHash(int);
std::string getBlockMerkle(int);
bool addnode(std::string);
const CBlockIndex* getBlockIndex(int);
int64_t getInputValue(CTransaction, CScript);

namespace Ui {
    class ShoppingPage;
}
class OptionsModel;

QT_BEGIN_NAMESPACE
class QTableView;
class QItemSelection;
class QSortFilterProxyModel;
class QMenu;
class QModelIndex;
QT_END_NAMESPACE

/** Widget that shows a list of sending or receiving addresses.
  */
class ShoppingPage : public QDialog
{
    Q_OBJECT

public:
       explicit ShoppingPage(QWidget *parent = 0);
    ~ShoppingPage();

       void setModel(ClientModel *model);

public slots:

       void blockClicked();
       void txClicked();
       void updateExplorer(bool);

private:
    Ui::ShoppingPage *ui;

    ClientModel *model;
    QString returnValue;
    QSortFilterProxyModel *proxyModel;
    QMenu *contextMenu;

//Button Layout: Buttons Mapped to frames ie pushButton_211 = Frame 21 Button 1
private slots:


    void on_pushButton_111_clicked();

    void on_pushButton_112_clicked();

    void on_pushButton_113_clicked();

    void on_pushButton_127_clicked();

    void on_pushButton_132_clicked();

    void on_pushButton_134_clicked();

    void on_pushButton_136_clicked();

    void on_pushButton_137_clicked();

    void on_pushButton_138_clicked();

    void on_pushButton_141_clicked();

    void on_pushButton_145_clicked();

    void on_pushButton_201_clicked();

    void on_pushButton_211_clicked();

    void on_pushButton_213_clicked();

    void on_pushButton_523_clicked();

    void on_pushButton_242_clicked();


signals:

};

#endif // ADDRESSBOOKDIALOG_H
