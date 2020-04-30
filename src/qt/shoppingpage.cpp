#include "shoppingpage.h"
#include "ui_shoppingpage.h"

#include "addresstablemodel.h"
#include "optionsmodel.h"
#include "bitcoingui.h"
#include "editaddressdialog.h"
#include "csvmodelwriter.h"
#include "guiutil.h"
#include "main.h"
#include "wallet.h"
#include "base58.h"
#include "clientmodel.h"
#include "bitcoinrpc.h"
#include "transactionrecord.h"

#include <sstream>
#include <string>

#include <QClipboard>
#include <QMessageBox>
#include <QScrollArea>
#include <QMenu>
#include <QUrl>
#include <QDesktopServices>

double getBlockHardness(int height)
{
    const CBlockIndex* blockindex = getBlockIndex(height);

    int nShift = (blockindex->nBits >> 24) & 0xff;

    double dDiff =
        (double)0x0000ffff / (double)(blockindex->nBits & 0x00ffffff);

    while (nShift < 29)
    {
        dDiff *= 256.0;
        nShift++;
    }
    while (nShift > 29)
    {
        dDiff /= 256.0;
        nShift--;
    }

    return dDiff;
}

int getBlockHashrate(int height)
{
    int lookup = height;

    double timeDiff = getBlockTime(height) - getBlockTime(1);
    double timePerBlock = timeDiff / lookup;

    return (boost::int64_t)(((double)getBlockHardness(height) * pow(2.0, 32)) / timePerBlock);
}

const CBlockIndex* getBlockIndex(int height)
{
    std::string hex = getBlockHash(height);
    uint256 hash(hex);
    return mapBlockIndex[hash];
}

std::string getBlockHash(int Height)
{
    if(Height > pindexBest->nHeight) { return "0x38624e3834cfdc4410a5acbc32f750171aadad9620e6ba6d5c73201c16f7c8d1"; }
    if(Height < 0) { return "0x38624e3834cfdc4410a5acbc32f750171aadad9620e6ba6d5c73201c16f7c8d1"; }
    int desiredheight;
    desiredheight = Height;
    if (desiredheight < 0 || desiredheight > nBestHeight)
        return 0;

    CBlock block;
    CBlockIndex* pblockindex = mapBlockIndex[hashBestChain];
    while (pblockindex->nHeight > desiredheight)
        pblockindex = pblockindex->pprev;
    return pblockindex->phashBlock->GetHex();
}

int getBlockTime(int Height)
{
    std::string strHash = getBlockHash(Height);
    uint256 hash(strHash);

    if (mapBlockIndex.count(hash) == 0)
        return 0;

    CBlock block;
    CBlockIndex* pblockindex = mapBlockIndex[hash];
    return pblockindex->nTime;
}

std::string getBlockMerkle(int Height)
{
    std::string strHash = getBlockHash(Height);
    uint256 hash(strHash);

    if (mapBlockIndex.count(hash) == 0)
        return 0;

    CBlock block;
    CBlockIndex* pblockindex = mapBlockIndex[hash];
    return pblockindex->hashMerkleRoot.ToString().substr(0,10).c_str();
}

int getBlocknBits(int Height)
{
    std::string strHash = getBlockHash(Height);
    uint256 hash(strHash);

    if (mapBlockIndex.count(hash) == 0)
        return 0;

    CBlock block;
    CBlockIndex* pblockindex = mapBlockIndex[hash];
    return pblockindex->nBits;
}

int getBlockNonce(int Height)
{
    std::string strHash = getBlockHash(Height);
    uint256 hash(strHash);

    if (mapBlockIndex.count(hash) == 0)
        return 0;

    CBlock block;
    CBlockIndex* pblockindex = mapBlockIndex[hash];
    return pblockindex->nNonce;
}

std::string getBlockDebug(int Height)
{
    std::string strHash = getBlockHash(Height);
    uint256 hash(strHash);

    if (mapBlockIndex.count(hash) == 0)
        return 0;

    CBlock block;
    CBlockIndex* pblockindex = mapBlockIndex[hash];
    return pblockindex->ToString();
}

int blocksInPastHours(int hours)
{
    int wayback = hours * 3600;
    bool check = true;
    int height = pindexBest->nHeight;
    int heightHour = pindexBest->nHeight;
    int utime = (int)time(NULL);
    int target = utime - wayback;

    while(check)
    {
        if(getBlockTime(heightHour) < target)
        {
            check = false;
            return height - heightHour;
        } else {
            heightHour = heightHour - 1;
        }
    }

    return 0;
}

double getTxTotalValue(std::string txid)
{
    uint256 hash;
    hash.SetHex(txid);

    CTransaction tx;
    uint256 hashBlock = 0;
    if (!GetTransaction(hash, tx, hashBlock))
        return 505;

    CDataStream ssTx(SER_NETWORK, PROTOCOL_VERSION);
    ssTx << tx;

    double value = 0;
    double buffer = 0;
    for (unsigned int i = 0; i < tx.vout.size(); i++)
    {
        const CTxOut& txout = tx.vout[i];

        buffer = value + convertCoins(txout.nValue);
        value = buffer;
    }

    return value;
}

double convertCoins(int64_t amount)
{
    return (double)amount / (double)COIN;
}

std::string getOutputs(std::string txid)
{
    uint256 hash;
    hash.SetHex(txid);

    CTransaction tx;
    uint256 hashBlock = 0;
    if (!GetTransaction(hash, tx, hashBlock))
        return "fail";

    CDataStream ssTx(SER_NETWORK, PROTOCOL_VERSION);
    ssTx << tx;

    std::string str = "";
    for (unsigned int i = 0; i < tx.vout.size(); i++)
    {
        const CTxOut& txout = tx.vout[i];
        CTxDestination source;
        ExtractDestination(txout.scriptPubKey, source);
        CBitcoinAddress addressSource(source);
        std::string lol7 = addressSource.ToString();
        double buffer = convertCoins(txout.nValue);
        std::string amount = boost::to_string(buffer);
        str.append(lol7);
        str.append(": ");
        str.append(amount);
        str.append(" FLAPX");
        str.append("\n");
    }

    return str;
}

std::string getInputs(std::string txid)
{
    uint256 hash;
    hash.SetHex(txid);

    CTransaction tx;
    uint256 hashBlock = 0;
    if (!GetTransaction(hash, tx, hashBlock))
        return "fail";

    CDataStream ssTx(SER_NETWORK, PROTOCOL_VERSION);
    ssTx << tx;

    std::string str = "";
    for (unsigned int i = 0; i < tx.vin.size(); i++)
    {
        uint256 hash;
        const CTxIn& vin = tx.vin[i];
        hash.SetHex(vin.prevout.hash.ToString());
        CTransaction wtxPrev;
        uint256 hashBlock = 0;
        if (!GetTransaction(hash, wtxPrev, hashBlock))
             return "fail";

        CDataStream ssTx(SER_NETWORK, PROTOCOL_VERSION);
        ssTx << wtxPrev;

        CTxDestination source;
        ExtractDestination(wtxPrev.vout[vin.prevout.n].scriptPubKey, source);
        CBitcoinAddress addressSource(source);
        std::string lol6 = addressSource.ToString();
        const CScript target = wtxPrev.vout[vin.prevout.n].scriptPubKey;
        double buffer = convertCoins(getInputValue(wtxPrev, target));
        std::string amount = boost::to_string(buffer);
        str.append(lol6);
        str.append(": ");
        str.append(amount);
        str.append(" FLAPX");
        str.append("\n");
    }

    return str;
}

int64_t getInputValue(CTransaction tx, CScript target)
{
    for (unsigned int i = 0; i < tx.vin.size(); i++)
    {
        const CTxOut& txout = tx.vout[i];
        if(txout.scriptPubKey == target)
        {
            return txout.nValue;
        }
    }
    return 0;
}

double getTxFees(std::string txid)
{
    uint256 hash;
    hash.SetHex(txid);


    CTransaction tx;
    uint256 hashBlock = 0;
    if (!GetTransaction(hash, tx, hashBlock))
        return 0.0001;

    CDataStream ssTx(SER_NETWORK, PROTOCOL_VERSION);
    ssTx << tx;

    double value = 0;
    double buffer = 0;
    for (unsigned int i = 0; i < tx.vout.size(); i++)
    {
        const CTxOut& txout = tx.vout[i];

        buffer = value + convertCoins(txout.nValue);
        value = buffer;
    }

    double value0 = 0;
    double buffer0 = 0;
    for (unsigned int i = 0; i < tx.vin.size(); i++)
    {
        uint256 hash0;
        const CTxIn& vin = tx.vin[i];
        hash0.SetHex(vin.prevout.hash.ToString());
        CTransaction wtxPrev;
        uint256 hashBlock0 = 0;
        if (!GetTransaction(hash0, wtxPrev, hashBlock0))
             return 0;
        CDataStream ssTx(SER_NETWORK, PROTOCOL_VERSION);
        ssTx << wtxPrev;
        const CScript target = wtxPrev.vout[vin.prevout.n].scriptPubKey;
        buffer0 = value0 + convertCoins(getInputValue(wtxPrev, target));
        value0 = buffer0;
    }

    return value0 - value;
}

ShoppingPage::ShoppingPage(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ShoppingPage)
{
    ui->setupUi(this);

    connect(ui->blockButton_2, SIGNAL(pressed()), this, SLOT(blockClicked()));
    connect(ui->txButton_2, SIGNAL(pressed()), this, SLOT(txClicked()));

}

void ShoppingPage::updateExplorer(bool block)
{
    if(block)
    {
        ui->heightLabel_3->show();
        ui->heightLabel_4->show();
        ui->hashLabel_2->show();
        ui->hashBox_2->show();
        ui->merkleLabel_2->show();
        ui->merkleBox_2->show();
        ui->nonceLabel_2->show();
        ui->nonceBox_2->show();
        ui->bitsLabel_2->show();
        ui->bitsBox_2->show();
        ui->timeLabel_2->show();
        ui->timeBox_2->show();
        ui->hardLabel_2->show();
        ui->hardBox_2->show();;
        ui->pawLabel_2->show();
        ui->pawBox_2->show();
        int height = ui->heightBox_2->value();
        if (height > pindexBest->nHeight)
        {
            ui->heightBox_2->setValue(pindexBest->nHeight);
            height = pindexBest->nHeight;
        }
        int Pawrate = getBlockHashrate(height);
        double Pawrate2 = 0.000;
        Pawrate2 = ((double)Pawrate / 1000000);
        std::string hash = getBlockHash(height);
        std::string merkle = getBlockMerkle(height);
        int nBits = getBlocknBits(height);
        int nNonce = getBlockNonce(height);
        int atime = getBlockTime(height);
        double hardness = getBlockHardness(height);
        QString QHeight = QString::number(height);
        QString QHash = QString::fromUtf8(hash.c_str());
        QString QMerkle = QString::fromUtf8(merkle.c_str());
        QString QBits = QString::number(nBits);
        QString QNonce = QString::number(nNonce);
        QString QTime = QString::number(atime);
        QString QHardness = QString::number(hardness, 'f', 6);
        QString QPawrate = QString::number(Pawrate2, 'f', 3);
        ui->heightLabel_4->setText(QHeight);
        ui->hashBox_2->setText(QHash);
        ui->merkleBox_2->setText(QMerkle);
        ui->bitsBox_2->setText(QBits);
        ui->nonceBox_2->setText(QNonce);
        ui->timeBox_2->setText(QTime);
        ui->hardBox_2->setText(QHardness);
        ui->pawBox_2->setText(QPawrate + " MH/s");
    }

    if(block == false) {
        ui->txID_2->show();
        ui->txLabel_2->show();
        ui->valueLabel_2->show();
        ui->valueBox_2->show();
        ui->inputLabel_2->show();
        ui->inputBox_2->show();
        ui->outputLabel_2->show();
        ui->outputBox_2->show();
        ui->feesLabel_2->show();
        ui->feesBox_2->show();
        std::string txid = ui->txBox_2->text().toUtf8().constData();
        double value = getTxTotalValue(txid);
        double fees = getTxFees(txid);
        std::string outputs = getOutputs(txid);
        std::string inputs = getInputs(txid);
        QString QValue = QString::number(value, 'f', 6);
        QString QID = QString::fromUtf8(txid.c_str());
        QString QOutputs = QString::fromUtf8(outputs.c_str());
        QString QInputs = QString::fromUtf8(inputs.c_str());
        QString QFees = QString::number(fees, 'f', 6);
        ui->valueBox_2->setText(QValue + " FLAPX");
        ui->txID_2->setText(QID);
        ui->outputBox_2->setText(QOutputs);
        ui->inputBox_2->setText(QInputs);
        ui->feesBox_2->setText(QFees + " FLAPX");
    }
}


void ShoppingPage::txClicked()
{
    updateExplorer(false);
}

void ShoppingPage::blockClicked()
{
    updateExplorer(true);
}

void ShoppingPage::setModel(ClientModel *model)
{
    this->model = model;
}

//FlapXcoin
void ShoppingPage::on_pushButton_111_clicked()
{
    QString link="https://sites.google.com/view/flapxcoin/landing-pad";
    QDesktopServices::openUrl(QUrl(link));
}
void ShoppingPage::on_pushButton_112_clicked()
{
    QString link="http://blocks.flapxcoin.com";
    QDesktopServices::openUrl(QUrl(link));
}
void ShoppingPage::on_pushButton_113_clicked()
{
    QString link="https://pool.nerdlabs001.com";
    QDesktopServices::openUrl(QUrl(link));
}
//void ShoppingPage::on_pushButton_153_clicked()
//{
//    QString link="https://www.crypto-city.com/index.php/crypto-city-academy/starthere/";
//    QDesktopServices::openUrl(QUrl(link));
//}
//void ShoppingPage::on_pushButton_126_clicked()
//{
//    QString link="https://sites.google.com/view/flapxcoin/bots/crawlie";
//    QDesktopServices::openUrl(QUrl(link));
//}
void ShoppingPage::on_pushButton_127_clicked()
{
    QString link="https://sites.google.com/view/flapxcoin/bots/tipbot";
    QDesktopServices::openUrl(QUrl(link));
}
//void ShoppingPage::on_pushButton_124_clicked()
//{
//    QString link="https://playforcrypto.com/index.php?action=register&referrer=ukmayhem";
//    QDesktopServices::openUrl(QUrl(link));
//}
void ShoppingPage::on_pushButton_131_clicked()
{
    QString link="https://www.crypto-city.com/index.php/flapx/wall";
    QDesktopServices::openUrl(QUrl(link));
}
void ShoppingPage::on_pushButton_132_clicked()
{
    QString link="https://discord.gg/qbZ6hxx";
    QDesktopServices::openUrl(QUrl(link));
}
//void ShoppingPage::on_pushButton_133_clicked()
//{
//    QString link="https://twitter.com/FlappyNewsWing";
//    QDesktopServices::openUrl(QUrl(link));
//}
void ShoppingPage::on_pushButton_134_clicked()
{
    QString link="https://bitcointalk.org/index.php?topic=3031911.0";
    QDesktopServices::openUrl(QUrl(link));
}
//void ShoppingPage::on_pushButton_135_clicked()
//{
//    QString link="https://youtu.be/EY-u_mjNGKY";
//    QDesktopServices::openUrl(QUrl(link));
//}
void ShoppingPage::on_pushButton_136_clicked()
{
    QString link="https://sites.google.com/view/flapxcoin/dont-panic";
    QDesktopServices::openUrl(QUrl(link));
}


//FlapXM
void ShoppingPage::on_pushButton_141_clicked()
{
    QString link="https://sites.google.com/view/nerdlabs001";
    QDesktopServices::openUrl(QUrl(link));
}
//void ShoppingPage::on_pushButton_142_clicked()
//{
//    QString link="https://sites.google.com/view/flapxcoin/flap-x-fm/krewshul-krosshairs";
//    QDesktopServices::openUrl(QUrl(link));
//}
//void ShoppingPage::on_pushButton_143_clicked()
//{
//    QString link="https://sites.google.com/view/flapxcoin/flap-x-fm/crypto-takeover";
//    QDesktopServices::openUrl(QUrl(link));
//}
//void ShoppingPage::on_pushButton_144_clicked()
//{
//    QString link="https://sites.google.com/view/flapxcoin/flap-x-fm/reggae-blockchain";
//    QDesktopServices::openUrl(QUrl(link));
//}
void ShoppingPage::on_pushButton_145_clicked()
{
    QString link="https://youtu.be/EY-u_mjNGKY";
    QDesktopServices::openUrl(QUrl(link));
}
//communities
//NET
void ShoppingPage::on_pushButton_201_clicked()
{
    QString link="http://netcoin.io/";
    QDesktopServices::openUrl(QUrl(link));
}
void ShoppingPage::on_pushButton_202_clicked()
{
    QString link="https://discordapp.com/invite/RCh3xsp";
    QDesktopServices::openUrl(QUrl(link));
}
void ShoppingPage::on_pushButton_203_clicked()
{
    QString link="https://www.crypto-city.com/index.php/net-netcoin/";
    QDesktopServices::openUrl(QUrl(link));
}
//FLAPX
void ShoppingPage::on_pushButton_211_clicked()
{
    QString link="https://sites.google.com/view/flapxcoin/landing-pad";
    QDesktopServices::openUrl(QUrl(link));
}
void ShoppingPage::on_pushButton_212_clicked()
{
    QString link="https://discord.gg/UZK4wVU";
    QDesktopServices::openUrl(QUrl(link));
}
void ShoppingPage::on_pushButton_213_clicked()
{
    QString link="https://www.crypto-city.com/index.php/flapx/";
    QDesktopServices::openUrl(QUrl(link));
}
//WW
void ShoppingPage::on_pushButton_221_clicked()
{
    QString link="http://wayawolfcoin.org/";
    QDesktopServices::openUrl(QUrl(link));
}
void ShoppingPage::on_pushButton_222_clicked()
{
    QString link="https://discordapp.com/invite/X43Z7qR";
    QDesktopServices::openUrl(QUrl(link));
}
void ShoppingPage::on_pushButton_223_clicked()
{
    QString link="https://www.crypto-city.com/index.php/wayawolfcoin/";
    QDesktopServices::openUrl(QUrl(link));
}
//Gsave
//void ShoppingPage::on_pushButton_231_clicked()
//{
//    QString link="https://bitcointalk.org/index.php?topic=2429913.0";
//    QDesktopServices::openUrl(QUrl(link));
//}
//void ShoppingPage::on_pushButton_232_clicked()
//{
//    QString link="https://discord.gg/PSb7GuP";
//    QDesktopServices::openUrl(QUrl(link));
//}
//void ShoppingPage::on_pushButton_233_clicked()
//{
//    QString link="https://bitcointalk.org/index.php?topic=2429913.0";
//   QDesktopServices::openUrl(QUrl(link));
//}
//HYP
void ShoppingPage::on_pushButton_241_clicked()
{
    QString link="http://hyperstake.io/";
    QDesktopServices::openUrl(QUrl(link));
}
void ShoppingPage::on_pushButton_242_clicked()
{
    QString link="https://discordapp.com/invite/dqVrxNV";
    QDesktopServices::openUrl(QUrl(link));
}
void ShoppingPage::on_pushButton_243_clicked()
{
    QString link="https://www.crypto-city.com/index.php/hyperstake-hyp-ann/";
    QDesktopServices::openUrl(QUrl(link));
}
//ELETE
void ShoppingPage::on_pushButton_251_clicked()
{
    QString link="https://www.elitecurrency.info/";
    QDesktopServices::openUrl(QUrl(link));
}
void ShoppingPage::on_pushButton_252_clicked()
{
    QString link="https://discord.gg/np7BmKV";
    QDesktopServices::openUrl(QUrl(link));
}
void ShoppingPage::on_pushButton_253_clicked()
{
    QString link="https://bitcointalk.org/index.php?topic=2249640";
    QDesktopServices::openUrl(QUrl(link));
}
//888
void ShoppingPage::on_pushButton_524_clicked()
{
    QString link="https://altexchanger.com/trade/index/market/flapx_doge/";
    QDesktopServices::openUrl(QUrl(link));
}
void ShoppingPage::on_pushButton_261_clicked()
{
    QString link="https://www.crypto-city.com/index.php/ssfcoin/";
    QDesktopServices::openUrl(QUrl(link));
}
void ShoppingPage::on_pushButton_523_clicked()
{
   QString link="https://freiexchange.com/market";
     QDesktopServices::openUrl(QUrl(link));
}
void ShoppingPage::on_pushButton_254_clicked()
{
   QString link="https://bitcoingarden.org/forum/index.php?topic=67923";
   QDesktopServices::openUrl(QUrl(link));
}
void ShoppingPage::on_pushButton_262_clicked()
{
   QString link="https://discord.gg/2bABBgE";
   QDesktopServices::openUrl(QUrl(link));
}
//SAT3
void ShoppingPage::on_pushButton_281_clicked()
{
    QString link="https://sites.google.com/view/sat3coin/home";
    QDesktopServices::openUrl(QUrl(link));
}
void ShoppingPage::on_pushButton_282_clicked()
{
    QString link="https://discord.gg/EGzXQrZ";
    QDesktopServices::openUrl(QUrl(link));
}
void ShoppingPage::on_pushButton_283_clicked()
{
    QString link="https://www.crypto-city.com/index.php/sat3/";
    QDesktopServices::openUrl(QUrl(link));
}
//RBLT
//void ShoppingPage::on_pushButton_291_clicked()
//{
//   QString link="https://bitcoingarden.org/forum/index.php?topic=26019.0";
//   QDesktopServices::openUrl(QUrl(link));
//}
//void ShoppingPage::on_pushButton_292_clicked()
//{
//    QString link="https://discord.gg/b6myWHv";
//    QDesktopServices::openUrl(QUrl(link));
//}
//void ShoppingPage::on_pushButton_293_clicked()
//{
//    QString link="https://bitcointalk.org/index.php?topic=2768822.0;all";
//    QDesktopServices::openUrl(QUrl(link));
//}
//GDC
void ShoppingPage::on_pushButton_331_clicked()
{
    QString link="https://www.crypto-city.com/index.php/grandcoin-gdc/info";
    QDesktopServices::openUrl(QUrl(link));
}
void ShoppingPage::on_pushButton_332_clicked()
{
    QString link="https://discordapp.com/invite/ZrdbSNZ";
    QDesktopServices::openUrl(QUrl(link));
}
void ShoppingPage::on_pushButton_333_clicked()
{
    QString link="https://www.crypto-city.com/index.php/grandcoin-gdc/";
    QDesktopServices::openUrl(QUrl(link));
}
//MAZA
void ShoppingPage::on_pushButton_311_clicked()
{
    QString link="https://www.mazacoin.org/";
    QDesktopServices::openUrl(QUrl(link));
}

void ShoppingPage::on_pushButton_312_clicked()
{
    QString link="https://discord.gg/x4qpdWD";
    QDesktopServices::openUrl(QUrl(link));
}

void ShoppingPage::on_pushButton_313_clicked()
{
    QString link="https://bitcointalk.org/index.php?topic=508849.0";
    QDesktopServices::openUrl(QUrl(link));
}
//cryptolinks
//void ShoppingPage::on_pushButton_501_clicked()
//{
//    QString link="https://cryptohub.online/";
//   QDesktopServices::openUrl(QUrl(link));
//}

//void ShoppingPage::on_pushButton_555_clicked()
//{
//    QString link="https://tradesatoshi.com/Exchange/?market=FLAPX_BTC";
//    QDesktopServices::openUrl(QUrl(link));
//}

//void ShoppingPage::on_pushButton_503_clicked()
//{
//    QString link="https://www.cryptopia.co.nz";
//    QDesktopServices::openUrl(QUrl(link));
//}
//void ShoppingPage::on_pushButton_504_clicked()
//{
//    QString link="http://fiatleak.com/";
//    QDesktopServices::openUrl(QUrl(link));
//}

void ShoppingPage::on_pushButton_505_clicked()
{
    QString link="https://www.crypto-city.com/index.php/";
    QDesktopServices::openUrl(QUrl(link));
}

//void ShoppingPage::on_pushButton_506_clicked()
//{
//    QString link="https://playforcrypto.com/index.php?action=register&referrer=ukmayhem";
//    QDesktopServices::openUrl(QUrl(link));
//}
//void ShoppingPage::on_pushButton_507_clicked()
//{
//    QString link="https://www.cryptocompare.com/";
//   QDesktopServices::openUrl(QUrl(link));
//}

//void ShoppingPage::on_pushButton_508_clicked()
//{
//    QString link="http://mapofcoins.com/";
//    QDesktopServices::openUrl(QUrl(link));
//}

void ShoppingPage::on_pushButton_509_clicked()
{
    QString link="https://www.livecoinwatch.com/";
    QDesktopServices::openUrl(QUrl(link));
}
void ShoppingPage::on_pushButton_510_clicked()
{
    QString link="https://www.miningrigrentals.com/register?ref=107473";
    QDesktopServices::openUrl(QUrl(link));
}
//void ShoppingPage::on_pushButton_511_clicked()
//{
//   QString link="https://altcoinsfoundation.com/";
//    QDesktopServices::openUrl(QUrl(link));
//}

//void ShoppingPage::on_pushButton_512_clicked()
//{
//    QString link="https://give-me-coins.com";
//   QDesktopServices::openUrl(QUrl(link));
//}

void ShoppingPage::on_pushButton_513_clicked()
{
    QString link="https://www.crypto-city.com/index.php/";
    QDesktopServices::openUrl(QUrl(link));
}
//void ShoppingPage::on_pushButton_514_clicked()
//{
//    QString link="https://coinmarketcap.com/";
//    QDesktopServices::openUrl(QUrl(link));
//}

//void ShoppingPage::on_pushButton_600_clicked()
//{
//    QString link="http://thecoinbuilders.com/";
//  QDesktopServices::openUrl(QUrl(link));
//}




ShoppingPage::~ShoppingPage()
{
    delete ui;
}
