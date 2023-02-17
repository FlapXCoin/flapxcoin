// Copyright (c) 2011-2023 The Bitcoin Core developers
// Copyright (c) 2011-2023 The Flappycoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "clientmodel.h"
#include "guiconstants.h"
#include "guiutil.h"
#include "optionsmodel.h"
#include "addresstablemodel.h"
#include "transactiontablemodel.h"

#include "chainparams.h"
#include "checkpoints.h"
#include "main.h"
#include "net.h"
#include "ui_interface.h"
#include "util.h"

#include <QDateTime>
#include <QTimer>

#include <boost/bind/bind.hpp>

class CBlockIndex;

static const int64_t nClientStartupTime = GetTime();

ClientModel::ClientModel(OptionsModel *optionsModel, QObject *parent) :
    QObject(parent), 
    optionsModel(optionsModel),
    cachedNumBlocks(0),
    numBlocksAtStartup(-1), 
    pollTimer(0)
{
    // numBlocksAtStartup = -1;

    pollTimer = new QTimer(this);
    pollTimer->start(MODEL_UPDATE_DELAY);  
    connect(pollTimer, SIGNAL(timeout()), this, SLOT(updateTimer()));

    subscribeToCoreSignals();
}

ClientModel::~ClientModel()
{
    unsubscribeFromCoreSignals();
}

int ClientModel::getNumConnections() const
{
    return vNodes.size();
}

int ClientModel::getNumBlocks() const
{
    LOCK(cs_main);
    return nBestHeight;
}

int ClientModel::getNumBlocksAtStartup()
{
    if (numBlocksAtStartup == -1) numBlocksAtStartup = getNumBlocks();
    return numBlocksAtStartup;
}

QDateTime ClientModel::getLastBlockDate() const
{
    LOCK(cs_main);
    
    if (pindexBest)
        return QDateTime::fromTime_t(pindexBest->GetBlockTime());
    else
        return QDateTime::fromTime_t(1393221600); // Genesis block's time
}

void ClientModel::updateTimer()
{
    // Get required lock upfront. This avoids the GUI from getting stuck on
    // periodical polls if the core is holding the locks for a longer time -
    // for example, during a wallet rescan.
    TRY_LOCK(cs_main, lockMain);
    if(!lockMain)
        return;
    // Some quantities (such as number of blocks) change so fast that we don't want to be notified for each change.
    // Periodically check and update with a timer.
    int newNumBlocks = getNumBlocks();
    // int newNumBlocksOfPeers = getNumBlocksOfPeers();

    if(cachedNumBlocks != newNumBlocks)
    {
        cachedNumBlocks = newNumBlocks;
        // cachedNumBlocksOfPeers = newNumBlocksOfPeers;

        // emit numBlocksChanged(newNumBlocks, newNumBlocksOfPeers);
        // ensure we return the maximum of newNumBlocksOfPeers and newNumBlocks to not create weird displays in the GUI
        emit numBlocksChanged(newNumBlocks);
    }
}

void ClientModel::updateNumConnections(int numConnections)
{
    Q_EMIT numConnectionsChanged(numConnections);
}

void ClientModel::updateAlert()
{
    
    // Emit a numBlocksChanged when the status message changes,
    // so that the view recomputes and updates the status bar.
    // emit numBlocksChanged(getNumBlocks(), getNumBlocksOfPeers());
     Q_EMIT alertsChanged(getStatusBarWarnings());
}

bool ClientModel::isTestNet() const
{
    return TestNet();
}

bool ClientModel::inInitialBlockDownload() const
{
    return IsInitialBlockDownload();
}

/*
int ClientModel::getNumBlocksOfPeers() const
{
    return GetNumBlocksOfPeers();
}
*/

QString ClientModel::getStatusBarWarnings() const
{
    return QString::fromStdString(GetWarnings("statusbar"));
}

OptionsModel *ClientModel::getOptionsModel()
{
    return optionsModel;
}

QString ClientModel::formatFullVersion() const
{
    return QString::fromStdString(FormatFullVersion());
}

QString ClientModel::formatBuildDate() const
{
    return QString::fromStdString(CLIENT_DATE);
}

bool ClientModel::isReleaseVersion() const
{
    return CLIENT_VERSION_IS_RELEASE;
}

QString ClientModel::clientName() const
{
    return QString::fromStdString(CLIENT_NAME);
}

QString ClientModel::formatClientStartupTime() const
{
    return QDateTime::fromTime_t(nClientStartupTime).toString();
}

static void NotifyBlocksChanged(ClientModel *clientmodel)
{
    // This notification is too frequent. Don't trigger a signal.
    // Don't remove it, though, as it might be useful later.
}

static void NotifyNumConnectionsChanged(ClientModel *clientmodel, int newNumConnections)
{
    // Too noisy: qDebug() << "NotifyNumConnectionsChanged: " + QString::number(newNumConnections);
    QMetaObject::invokeMethod(clientmodel, "updateNumConnections", Qt::QueuedConnection,
                              Q_ARG(int, newNumConnections));
}

static void NotifyAlertChanged(ClientModel *clientmodel, const uint256 &hash, ChangeType status)
{
    qDebug() << "NotifyAlertChanged";
    QMetaObject::invokeMethod(clientmodel, "updateAlert", Qt::QueuedConnection);
}

void ClientModel::subscribeToCoreSignals()
{
    // Connect signals to client
    uiInterface.NotifyBlocksChanged.connect(boost::bind(NotifyBlocksChanged, this));
    uiInterface.NotifyNumConnectionsChanged.connect(boost::bind(NotifyNumConnectionsChanged, this, _1));
    uiInterface.NotifyAlertChanged.connect(boost::bind(NotifyAlertChanged, this));
}

void ClientModel::unsubscribeFromCoreSignals()
{
    // Disconnect signals from client
    uiInterface.NotifyBlocksChanged.disconnect(boost::bind(NotifyBlocksChanged, this));
    uiInterface.NotifyNumConnectionsChanged.disconnect(boost::bind(NotifyNumConnectionsChanged, this, _1));
    uiInterface.NotifyAlertChanged.disconnect(boost::bind(NotifyAlertChanged, this));
}
