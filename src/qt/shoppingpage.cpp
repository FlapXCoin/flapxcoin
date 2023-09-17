﻿#include "shoppingpage.h"
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
    QString link="https://pools.flapxcoin.com";
    QDesktopServices::openUrl(QUrl(link));
}

void ShoppingPage::on_pushButton_127_clicked()
{
    QString link="https://sites.google.com/view/flapxcoin/bots/tipbot";
    QDesktopServices::openUrl(QUrl(link));
}

void ShoppingPage::on_pushButton_134_clicked()
{
    QString link="https://bitcointalk.org/index.php?topic=3031911.0";
    QDesktopServices::openUrl(QUrl(link));
}

void ShoppingPage::on_pushButton_136_clicked()
{
    QString link="https://dont-panic.flapxcoin.com";
    QDesktopServices::openUrl(QUrl(link));
}

void ShoppingPage::on_pushButton_137_clicked()
{
    QString link="https://swag.flapxcoin.com";
    QDesktopServices::openUrl(QUrl(link));
}

void ShoppingPage::on_pushButton_138_clicked()
{
    QString link="https://www.teepublic.com/user/nerdlabs001";
    QDesktopServices::openUrl(QUrl(link));
}

void ShoppingPage::on_pushButton_145_clicked()
{
    QString link="https://sites.google.com/view/nerdlabs001/development/elitecraft/getting-started";
    QDesktopServices::openUrl(QUrl(link));
}

void ShoppingPage::on_pushButton_201_clicked()
{
    QString link="https://nerdcraft.dev";
    QDesktopServices::openUrl(QUrl(link));
}

void ShoppingPage::on_pushButton_211_clicked()
{
    QString link="https://www.elitecurrency.info/";
    QDesktopServices::openUrl(QUrl(link));
}

void ShoppingPage::on_pushButton_213_clicked()
{
    QString link="https://sites.google.com/view/nerdlabs001/development/elitecraft";
    QDesktopServices::openUrl(QUrl(link));
}

ShoppingPage::~ShoppingPage()
{
    delete ui;
}
