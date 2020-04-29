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

//    void on_pushButton_531_clicked();

//    void on_pushButton_126_clicked();

    void on_pushButton_127_clicked();

//    void on_pushButton_555_clicked();

    void on_pushButton_131_clicked();

    void on_pushButton_132_clicked();

//   void on_pushButton_133_clicked();

    void on_pushButton_134_clicked();

//    void on_pushButton_135_clicked();

    void on_pushButton_254_clicked();

    void on_pushButton_136_clicked();

    void on_pushButton_141_clicked();

 //   void on_pushButton_142_clicked();

//    void on_pushButton_143_clicked();

//    void on_pushButton_144_clicked();

    void on_pushButton_145_clicked();

    void on_pushButton_201_clicked();

    void on_pushButton_202_clicked();

    void on_pushButton_203_clicked();

    void on_pushButton_211_clicked();

    void on_pushButton_212_clicked();

    void on_pushButton_213_clicked();

    void on_pushButton_221_clicked();

    void on_pushButton_222_clicked();

    void on_pushButton_223_clicked();

    void on_pushButton_523_clicked();

    //void on_pushButton_232_clicked();

//    void on_pushButton_153_clicked();

    void on_pushButton_241_clicked();

    void on_pushButton_242_clicked();

    void on_pushButton_243_clicked();

    void on_pushButton_251_clicked();

    void on_pushButton_252_clicked();

    void on_pushButton_253_clicked();

    void on_pushButton_261_clicked();

    void on_pushButton_262_clicked();

    //void on_pushButton_263_clicked();

    //void on_pushButton_271_clicked();

    //void on_pushButton_272_clicked();

    //void on_pushButton_273_clicked();

    void on_pushButton_281_clicked();

    void on_pushButton_282_clicked();

    void on_pushButton_283_clicked();

    //void on_pushButton_291_clicked();

    //void on_pushButton_292_clicked();

    void on_pushButton_524_clicked();

    void on_pushButton_331_clicked();

    void on_pushButton_332_clicked();

    void on_pushButton_333_clicked();

    void on_pushButton_311_clicked();

    void on_pushButton_312_clicked();

    void on_pushButton_313_clicked();

//    void on_pushButton_501_clicked();

//    void on_pushButton_502_clicked();

//    void on_pushButton_503_clicked();

//    void on_pushButton_504_clicked();

    void on_pushButton_505_clicked();

//    void on_pushButton_506_clicked();

//    void on_pushButton_507_clicked();

//    void on_pushButton_508_clicked();

    void on_pushButton_509_clicked();

    void on_pushButton_510_clicked();

//    void on_pushButton_511_clicked();

//    void on_pushButton_512_clicked();

    void on_pushButton_513_clicked();

//    void on_pushButton_514_clicked();

//    void on_pushButton_515_clicked();

//    void on_pushButton_600_clicked();




signals:

};

#endif // ADDRESSBOOKDIALOG_H
