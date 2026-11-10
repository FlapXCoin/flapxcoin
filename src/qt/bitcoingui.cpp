/*
 * Qt4 bitcoin GUI.
 *
 * W.J. van der Laan 2011-2023
 * The Bitcoin Developers 2011-2023
 */
#include "bitcoingui.h"
#include "calcdialog.h"
#include "transactiontablemodel.h"
#include "addressbookpage.h"
#include "shoppingpage.h"
#include "explorerpage.h"
#include "sendcoinsdialog.h"
#include "signverifymessagedialog.h"
#include "optionsdialog.h"
#include "aboutdialog.h"
#include "charitydialog.h"
#include "clientmodel.h"
#include "walletmodel.h"
#include "editaddressdialog.h"
#include "optionsmodel.h"
#include "transactiondescdialog.h"
#include "addresstablemodel.h"
#include "transactionview.h"
#include "overviewpage.h"
#include "bitcoinunits.h"
#include "guiconstants.h"
#include "askpassphrasedialog.h"
#include "notificator.h"
#include "guiutil.h"
#include "clientversion.h"
#include "rpcconsole.h"
#include "wallet.h"
#include "bitcoinrpc.h"
#include "allocators.h"
#include "init.h"
#include "util.h"
#include "main.h"
#include "net.h"
#include <algorithm>

#ifdef Q_OS_MAC
#include "macdockiconhandler.h"
#endif

#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QIcon>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QToolBar>
#include <QStatusBar>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QLocale>
#include <QMessageBox>
#include <QMimeData>
#include <QProgressBar>
#include <QStackedWidget>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QDesktopServices>
#include <QTimer>
#include <QPainter>
#include <QDragEnterEvent>
#if QT_VERSION < 0x050000
#include <QUrl>
#endif
#include <QStyle>
#include <QMimeData>
#include <QSignalMapper>
#include <QActionGroup>
#include <QDir>
#include <QSettings>
#include <QPaintEvent>
#include <QRadialGradient>
#include <QString>

#include <iostream>
#include <cmath>

extern CWallet* pwalletMain;
extern int64_t nLastCoinStakeSearchInterval;
double GetPoSKernelPS();

BitcoinGUI::BitcoinGUI(QWidget *parent):
    QMainWindow(parent),
    clientModel(0),
    walletModel(0),
    encryptWalletAction(0),
    changePassphraseAction(0),
    unlockWalletAction(0),
    lockWalletAction(0),
    aboutQtAction(0),
    trayIcon(0),
    notificator(0),
    rpcConsole(0),
    prevBlocks(0),
    nWeight(0)
{

    // Wide enough for every toolbar page button to be visible
    resize(1120, 680);
    setWindowTitle(tr("FlapX-Starhopper") + " - " + QString("%1.%2.%3.%4")
                   .arg(CLIENT_VERSION_MAJOR).arg(CLIENT_VERSION_MINOR)
                   .arg(CLIENT_VERSION_REVISION).arg(CLIENT_VERSION_BUILD)
                   + " - " + tr("Pre Release"));
#ifndef Q_OS_MAC
    qApp->setWindowIcon(QIcon(":icons/bitcoin"));
    setWindowIcon(QIcon(":icons/bitcoin"));
#else
    setUnifiedTitleAndToolBarOnMac(false);
    QApplication::setAttribute(Qt::AA_DontShowIconsInMenus);
#endif
    // Accept D&D of URIs
    setAcceptDrops(true);

    /* zeewolf: Hot swappable wallet themes */
    // Discover themes and apply the saved one (FlapXCoin by default)
    themeActionGroup = 0;
    listThemes(themesList);
    /* /zeewolf: Hot swappable wallet themes */

    // Create actions for the toolbar, menu bar and tray/dock icon
    createActions();

    // Create application menu bar
    createMenuBar();

    // Create the toolbars
    createToolBars();

    // Create the tray icon (or setup the dock icon)
    createTrayIcon();

    // Create tabs

    overviewPage = new OverviewPage();

    transactionsPage = new QWidget(this);
    QVBoxLayout *vbox = new QVBoxLayout();
    transactionView = new TransactionView(this);
    vbox->addWidget(transactionView);
    vbox->setContentsMargins(0,0,0,0);
    transactionView->setContentsMargins(0,0,0,0);
    transactionsPage->setContentsMargins(0,0,0,0);
    transactionsPage->setLayout(vbox);


    addressBookPage = new AddressBookPage(AddressBookPage::ForEditing, AddressBookPage::SendingTab);

    shoppingPage = new ShoppingPage();

    explorerPage = new ExplorerPage(this);


    receiveCoinsPage = new AddressBookPage(AddressBookPage::ForEditing, AddressBookPage::ReceivingTab);

    sendCoinsPage = new SendCoinsDialog(this);

    signVerifyMessageDialog = new SignVerifyMessageDialog(this);
    stakeForCharityDialog = new StakeForCharityDialog(this);

    centralWidget = new QStackedWidget(this);
    centralWidget->addWidget(overviewPage);
    centralWidget->addWidget(transactionsPage);
    centralWidget->addWidget(addressBookPage);
    centralWidget->addWidget(shoppingPage);
    centralWidget->addWidget(explorerPage);
    centralWidget->addWidget(receiveCoinsPage);
    centralWidget->addWidget(sendCoinsPage);
    centralWidget->addWidget(stakeForCharityDialog);
    setCentralWidget(centralWidget);


    // Create status bar
    statusBar();

    // Status bar notification icons
    QFrame *frameBlocks = new QFrame();
    frameBlocks->setContentsMargins(0,0,0,0);
    frameBlocks->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    QHBoxLayout *frameBlocksLayout = new QHBoxLayout(frameBlocks);
    frameBlocksLayout->setContentsMargins(3,0,3,0);
    frameBlocksLayout->setSpacing(3);
    labelEncryptionIcon = new QLabel();
    labelStakingIcon = new QLabel();
    labelConnectionsIcon = new QLabel();
    labelBlocksIcon = new QLabel();
    frameBlocksLayout->addStretch();
    frameBlocksLayout->addWidget(labelEncryptionIcon);
    frameBlocksLayout->addStretch();
    frameBlocksLayout->addWidget(labelStakingIcon);
    frameBlocksLayout->addStretch();
    frameBlocksLayout->addWidget(labelConnectionsIcon);
    frameBlocksLayout->addStretch();
    frameBlocksLayout->addWidget(labelBlocksIcon);
    frameBlocksLayout->addStretch();

    // Staking animation: only runs while the wallet is staking
    nStakingAnimFrame = 0;
    stakingAnimTimer = new QTimer(this);
    stakingAnimTimer->setInterval(150);
    connect(stakingAnimTimer, SIGNAL(timeout()), this, SLOT(animateStakingIcon()));

    if (GetBoolArg("-staking", true))
    {
        QTimer *timerStakingIcon = new QTimer(labelStakingIcon);
        connect(timerStakingIcon, SIGNAL(timeout()), this, SLOT(updateStakingIcon()));
        timerStakingIcon->start(30 * 1000);
        updateStakingIcon();
    }

    // Progress bar and label for blocks download
    progressBarLabel = new QLabel();
    progressBarLabel->setVisible(false);

    progressBar = new QProgressBar();
    progressBar->setAlignment(Qt::AlignCenter);
    progressBar->setVisible(false);

    // (the progress bar is styled by the application style sheet, GUIUtil::flapxStyleSheet)
    statusBar()->addWidget(progressBarLabel);
    statusBar()->addWidget(progressBar);

    // Time-left estimate while syncing. Each tick reads two numbers without
    // waiting on block processing, so it costs next to nothing.
    syncTargetHeight = -1;
    syncEstimateTimer = new QTimer(this);
    connect(syncEstimateTimer, SIGNAL(timeout()), this, SLOT(updateSyncEstimate()));
    syncEstimateTimer->start(SYNC_ESTIMATE_DELAY);
    statusBar()->addPermanentWidget(frameBlocks);

    // Clicking on a transaction on the overview page simply sends you to transaction history page
    connect(overviewPage, SIGNAL(transactionClicked(QModelIndex)), this, SLOT(gotoHistoryPage()));
    connect(overviewPage, SIGNAL(transactionClicked(QModelIndex)), transactionView, SLOT(focusTransaction(QModelIndex)));

    // Double-clicking on a transaction on the transaction history page shows details
    connect(transactionView, SIGNAL(doubleClicked(QModelIndex)), transactionView, SLOT(showDetails()));

    rpcConsole = new RPCConsole(this);
    connect(openRPCConsoleAction, SIGNAL(triggered()), rpcConsole, SLOT(show()));

    // prevents an open debug window from becoming stuck/unusable on client shutdown
    connect(quitAction, SIGNAL(triggered()), rpcConsole, SLOT(hide()));

    // Clicking on "Verify Message" in the address book sends you to the verify message tab
    connect(addressBookPage, SIGNAL(verifyMessage(QString)), this, SLOT(gotoVerifyMessageTab(QString)));

    // Clicking on "Sign Message" in the receive coins page sends you to the sign message tab
    connect(receiveCoinsPage, SIGNAL(signMessage(QString)), this, SLOT(gotoSignMessageTab(QString)));

	// Clicking on stake for charity button in the address book sends you to the S4C page
    connect(addressBookPage, SIGNAL(stakeForCharitySignal(QString)), this, SLOT(charityClicked(QString)));

    // Clicking on stake for charity button in the Overview Page sends you to the S4C page
    connect(overviewPage, SIGNAL(stakeForCharitySignal()), this, SLOT(charityClicked()));
    connect(charityAction, SIGNAL(triggered()), this, SLOT(charityClicked()));
    gotoOverviewPage();
}

BitcoinGUI::~BitcoinGUI()
{
    if(trayIcon) // Hide tray icon, as deleting will let it linger until quit (on Ubuntu)
        trayIcon->hide();
#ifdef Q_OS_MAC
    delete appMenuBar;
#endif
}

void BitcoinGUI::createActions()
{
    QActionGroup *tabGroup = new QActionGroup(this);


    overviewAction = new QAction(QIcon(":/icons/overview"), tr("&Overview"), this);


    overviewAction->setCheckable(true);
    overviewAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_1));
    tabGroup->addAction(overviewAction);

    sendCoinsAction = new QAction(QIcon(":/icons/send"), tr("&Send coins"), this);
    sendCoinsAction->setCheckable(true);
    sendCoinsAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_2));
    tabGroup->addAction(sendCoinsAction);

    receiveCoinsAction = new QAction(QIcon(":/icons/receiving_addresses"), tr("&Receive coins"), this);
    receiveCoinsAction->setCheckable(true);
    receiveCoinsAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_3));
    tabGroup->addAction(receiveCoinsAction);

    historyAction = new QAction(QIcon(":/icons/history"), tr("&Transactions"), this);
    historyAction->setCheckable(true);
    historyAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_4));
    tabGroup->addAction(historyAction);

    addressBookAction = new QAction(QIcon(":/icons/address-book"), tr("&Address Book"), this);
    addressBookAction->setCheckable(true);
    addressBookAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_5));
    tabGroup->addAction(addressBookAction);

    shoppingAction = new QAction(QIcon(":/icons/res/icons/services.png"), tr("&Links"), this);
    shoppingAction->setCheckable(true);
    shoppingAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_6));
    tabGroup->addAction(shoppingAction);

    explorerAction = new QAction(QIcon(":/icons/res/icons/stakingTab.png"), tr("&Explorer"), this);
    explorerAction->setToolTip(tr("Look up blocks with the built-in block explorer"));
    explorerAction->setCheckable(true);
    explorerAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_7));
    tabGroup->addAction(explorerAction);

    charityAction = new QAction(QIcon(":/icons/send"), tr("&Multisend..."), this);
    charityAction->setToolTip(tr("Automatically send part of your stake rewards to another address"));
    charityAction->setCheckable(true);
    tabGroup->addAction(charityAction);

    calcAction = new QAction(QIcon(":/icons/bitcoin"), tr("&Interest Estimate..."), this);
    calcAction->setToolTip(tr("Estimate the staking interest on an amount of FLAPX"));

    connect(overviewAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(overviewAction, SIGNAL(triggered()), this, SLOT(gotoOverviewPage()));
    connect(sendCoinsAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(sendCoinsAction, SIGNAL(triggered()), this, SLOT(gotoSendCoinsPage()));
    connect(receiveCoinsAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(receiveCoinsAction, SIGNAL(triggered()), this, SLOT(gotoReceiveCoinsPage()));
    connect(historyAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(historyAction, SIGNAL(triggered()), this, SLOT(gotoHistoryPage()));
    connect(addressBookAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(addressBookAction, SIGNAL(triggered()), this, SLOT(gotoAddressBookPage()));
    connect(shoppingAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(shoppingAction, SIGNAL(triggered()), this, SLOT(gotoShoppingPage()));
    connect(explorerAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(explorerAction, SIGNAL(triggered()), this, SLOT(gotoExplorerPage()));
    connect(calcAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(calcAction, SIGNAL(triggered()), this, SLOT(calcClicked()));


    quitAction = new QAction(QIcon(":/icons/quit"), tr("E&xit"), this);
    quitAction->setToolTip(tr("Quit application"));
    quitAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q));
    quitAction->setMenuRole(QAction::QuitRole);
    aboutAction = new QAction(QIcon(":/icons/bitcoin"), tr("&About FlapX"), this);
    aboutAction->setToolTip(tr("Show information about FlapX"));
    aboutAction->setMenuRole(QAction::AboutRole);
    aboutQtAction = new QAction(QIcon(":/trolltech/qmessagebox/images/qtlogo-64.png"), tr("About &Qt"), this);
    aboutQtAction->setToolTip(tr("Show information about Qt"));
    aboutQtAction->setMenuRole(QAction::AboutQtRole);
    optionsAction = new QAction(QIcon(":/icons/options"), tr("&Options..."), this);
    optionsAction->setToolTip(tr("Modify configuration options for FlapX"));
    optionsAction->setMenuRole(QAction::PreferencesRole);
    toggleHideAction = new QAction(QIcon(":/icons/bitcoin"), tr("&Show / Hide"), this);
    encryptWalletAction = new QAction(QIcon(":/icons/lock_closed"), tr("&Encrypt Wallet..."), this);
    encryptWalletAction->setToolTip(tr("Encrypt or decrypt wallet"));
    encryptWalletAction->setCheckable(true);
    backupWalletAction = new QAction(QIcon(":/icons/filesave"), tr("&Backup Wallet..."), this);
    backupWalletAction->setToolTip(tr("Backup wallet to another location"));
    changePassphraseAction = new QAction(QIcon(":/icons/key"), tr("&Change Passphrase..."), this);
    changePassphraseAction->setToolTip(tr("Change the passphrase used for wallet encryption"));
    unlockWalletAction = new QAction(QIcon(":/icons/lock_open"), tr("&Unlock Wallet..."), this);
    unlockWalletAction->setToolTip(tr("Unlock wallet"));
    lockWalletAction = new QAction(QIcon(":/icons/lock_closed"), tr("&Lock Wallet"), this);
    lockWalletAction->setToolTip(tr("Lock wallet"));
    signMessageAction = new QAction(QIcon(":/icons/edit"), tr("Sign &message..."), this);
    verifyMessageAction = new QAction(QIcon(":/icons/transaction_0"), tr("&Verify message..."), this);

    exportAction = new QAction(QIcon(":/icons/export"), tr("&Export..."), this);
    //exportAction->setToolTip(tr("<html><head/><body><p><img src=:/toolTip/res/tooltips/exportTooltip.png/></p></body></html>"));

    openRPCConsoleAction = new QAction(QIcon(":/icons/debugwindow"), tr("&Debug window"), this);
    openRPCConsoleAction->setToolTip(tr("Open debugging and diagnostic console"));

    connect(quitAction,              SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(aboutAction,             SIGNAL(triggered()), this, SLOT(aboutClicked()));
    connect(aboutQtAction,           SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    connect(optionsAction,           SIGNAL(triggered()), this, SLOT(optionsClicked()));
    connect(toggleHideAction,        SIGNAL(triggered()), this, SLOT(toggleHidden()));
    connect(encryptWalletAction,     SIGNAL(triggered(bool)), this, SLOT(encryptWallet(bool)));
    connect(backupWalletAction,      SIGNAL(triggered()), this, SLOT(backupWallet()));
    connect(changePassphraseAction,  SIGNAL(triggered()), this, SLOT(changePassphrase()));
    connect(unlockWalletAction,      SIGNAL(triggered()), this, SLOT(unlockWallet()));
    connect(lockWalletAction,        SIGNAL(triggered()), this, SLOT(lockWallet()));
    connect(signMessageAction,       SIGNAL(triggered()), this, SLOT(gotoSignMessageTab()));
    connect(verifyMessageAction,     SIGNAL(triggered()), this, SLOT(gotoVerifyMessageTab()));
    /* zeewolf: Hot swappable wallet themes */
    // One checkable entry per theme; the active one is ticked
    themeActionGroup = new QActionGroup(this);
    themeActionGroup->setExclusive(true);
    foreach (const QString &theme, themesList)
    {
        QAction *themeAction = new QAction(theme, themeActionGroup);
        themeAction->setCheckable(true);
        themeAction->setData(theme);
        themeAction->setChecked(theme == selectedTheme);
        themeAction->setStatusTip(tr("Switch to the %1 theme").arg(theme));
    }
    connect(themeActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(themeActionTriggered(QAction*)));
    /* /zeewolf: Hot swappable wallet themes */

}

void BitcoinGUI::createMenuBar()
{
#ifdef Q_OS_MAC
    // Create a decoupled menu bar on Mac which stays even if the window is closed
    appMenuBar = new QMenuBar();
#else
    // Get the main window's menu bar on other platforms
    appMenuBar = menuBar();
#endif

    // Configure the menus
    QMenu *file = appMenuBar->addMenu(tr("&File"));
    file->addAction(backupWalletAction);
    file->addAction(exportAction);
    file->addAction(signMessageAction);
    file->addAction(verifyMessageAction);
    file->addSeparator();
    file->addAction(quitAction);

    QMenu *settings = appMenuBar->addMenu(tr("&Settings"));

    settings->addAction(encryptWalletAction);
    settings->addAction(changePassphraseAction);
    settings->addAction(lockWalletAction);
	settings->addAction(charityAction);
    settings->addSeparator();
    settings->addAction(optionsAction);
    settings->addSeparator();
    settings->addAction(calcAction);
    
    /* zeewolf: Hot swappable wallet themes */
    QMenu *themes = appMenuBar->addMenu(tr("T&hemes"));
    themes->addActions(themeActionGroup->actions());
    /* /zeewolf: Hot swappable wallet themes */

    QMenu *help = appMenuBar->addMenu(tr("&Help"));

    help->addAction(openRPCConsoleAction);
    help->addSeparator();
    help->addAction(aboutAction);
    help->addAction(aboutQtAction);
}

void BitcoinGUI::createToolBars()
{
    QToolBar *toolbar = addToolBar(tr("Tabs toolbar"));
    toolbar->setIconSize(QSize(32,32));

    toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    toolbar->addAction(overviewAction);
    toolbar->addAction(sendCoinsAction);
    toolbar->addAction(receiveCoinsAction);
    toolbar->addAction(historyAction);
    toolbar->addAction(addressBookAction);
    toolbar->addAction(shoppingAction);
    toolbar->addAction(explorerAction);

    QToolBar *toolbar2 = addToolBar(tr("Actions toolbar"));
    toolbar2->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    toolbar2->addAction(exportAction);

}

void BitcoinGUI::setClientModel(ClientModel *clientModel)
{
    this->clientModel = clientModel;
    if(clientModel)
    {
        // Replace some strings and icons, when using the testnet
        if(clientModel->isTestNet())
        {
            setWindowTitle(windowTitle() + QString(" ") + tr("[testnet]"));
#ifndef Q_OS_MAC
            qApp->setWindowIcon(QIcon(":icons/bitcoin_testnet"));
            setWindowIcon(QIcon(":icons/bitcoin_testnet"));
#else
            MacDockIconHandler::instance()->setIcon(QIcon(":icons/bitcoin_testnet"));
#endif
            if(trayIcon)
            {
                trayIcon->setToolTip(tr("FlapX client") + QString(" ") + tr("[testnet]"));
                trayIcon->setIcon(QIcon(":/icons/toolbar_testnet"));
                toggleHideAction->setIcon(QIcon(":/icons/toolbar_testnet"));
            }

            aboutAction->setIcon(QIcon(":/icons/toolbar_testnet"));
        }

        // Keep up to date with client
        setNumConnections(clientModel->getNumConnections());
        connect(clientModel, SIGNAL(numConnectionsChanged(int)), this, SLOT(setNumConnections(int)));

        setNumBlocks(clientModel->getNumBlocks());
        connect(clientModel, SIGNAL(numBlocksChanged(int)), this, SLOT(setNumBlocks(int)));

        // Report errors from network/worker thread
        connect(clientModel, SIGNAL(error(QString,QString,bool)), this, SLOT(error(QString,QString,bool)));

        overviewPage->setClientModel(clientModel);
        rpcConsole->setClientModel(clientModel);
        addressBookPage->setOptionsModel(clientModel->getOptionsModel());
        receiveCoinsPage->setOptionsModel(clientModel->getOptionsModel());
    }
}

void BitcoinGUI::setWalletModel(WalletModel *walletModel)
{
    this->walletModel = walletModel;
    if(walletModel)
    {
        // Report errors from wallet thread
        connect(walletModel, SIGNAL(error(QString,QString,bool)), this, SLOT(error(QString,QString,bool)));

        // Put transaction list in tabs
        transactionView->setModel(walletModel);

        overviewPage->setWalletModel(walletModel);
        addressBookPage->setModel(walletModel->getAddressTableModel());
        receiveCoinsPage->setModel(walletModel->getAddressTableModel());
        sendCoinsPage->setModel(walletModel);
        signVerifyMessageDialog->setModel(walletModel);
        stakeForCharityDialog->setModel(walletModel);

        setEncryptionStatus(walletModel->getEncryptionStatus());
        connect(walletModel, SIGNAL(encryptionStatusChanged(int)), this, SLOT(setEncryptionStatus(int)));

        // Balloon pop-up for new transaction
        connect(walletModel->getTransactionTableModel(), SIGNAL(rowsInserted(QModelIndex,int,int)),
                this, SLOT(incomingTransaction(QModelIndex,int,int)));

        // Ask for passphrase if needed
        connect(walletModel, SIGNAL(requireUnlock()), this, SLOT(unlockWallet()));
    }
}

void BitcoinGUI::createTrayIcon()
{
    QMenu *trayIconMenu;
#ifndef Q_OS_MAC
    trayIcon = new QSystemTrayIcon(this);
    trayIconMenu = new QMenu(this);
    trayIcon->setContextMenu(trayIconMenu);
    trayIcon->setToolTip(tr("FlapX client"));
    trayIcon->setIcon(QIcon(":/icons/toolbar"));
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(trayIconActivated(QSystemTrayIcon::ActivationReason)));
    trayIcon->show();
#else
    // Note: On Mac, the dock icon is used to provide the tray's functionality.
    MacDockIconHandler *dockIconHandler = MacDockIconHandler::instance();
    dockIconHandler->setMainWindow((QMainWindow *)this);
    trayIconMenu = dockIconHandler->dockMenu();
    dockIconHandler->setMainWindow((QMainWindow *)this);
#endif

    // Configuration of the tray icon (or dock icon) icon menu
    trayIconMenu->addAction(toggleHideAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(sendCoinsAction);
    trayIconMenu->addAction(receiveCoinsAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(signMessageAction);
    trayIconMenu->addAction(verifyMessageAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(optionsAction);
    trayIconMenu->addAction(openRPCConsoleAction);
#ifndef Q_OS_MAC // This is built-in on Mac
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);
#endif

    notificator = new Notificator(qApp->applicationName(), trayIcon);
}

#ifndef Q_OS_MAC
void BitcoinGUI::trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if(reason == QSystemTrayIcon::Trigger)
    {
        // Click on system tray icon triggers show/hide of the main window
        toggleHideAction->trigger();
    }
}
#endif

void BitcoinGUI::optionsClicked()
{
    if(!clientModel || !clientModel->getOptionsModel())
        return;
    OptionsDialog dlg;
    dlg.setModel(clientModel->getOptionsModel());
    dlg.exec();
}

void BitcoinGUI::aboutClicked()
{
    AboutDialog dlg;
    dlg.setModel(clientModel);
    dlg.exec();
}

void BitcoinGUI::setNumConnections(int count)
{
    QString icon;
    switch(count)
    {
    case 0: icon = ":/icons/connect_0"; break;
    case 1: case 2: case 3: icon = ":/icons/connect_1"; break;
    case 4: case 5: case 6: icon = ":/icons/connect_2"; break;
    case 7: case 8: case 9: icon = ":/icons/connect_3"; break;
    default: icon = ":/icons/connect_4"; break;
    }

    labelConnectionsIcon->setPixmap(QIcon(icon).pixmap(STATUSBAR_ICONSIZE,STATUSBAR_ICONSIZE));
    labelConnectionsIcon->setToolTip(tr("%n active connection(s) to FlapX network", "", count));
}

void BitcoinGUI::setNumBlocks(int count)
{
    // don't show / hide progress bar and its label if we have no connection to the network
    if (!clientModel || clientModel->getNumConnections() == 0)
    {
        progressBarLabel->setVisible(false);
        progressBar->setVisible(false);

        return;
    }

    QString tooltip;
    QDateTime lastBlockDate = clientModel->getLastBlockDate();
    QDateTime currentDate = QDateTime::currentDateTime();
    int totalSecs = GetTime() - 1393221600;
    int secs = lastBlockDate.secsTo(currentDate);
    tooltip = tr("Processed %1 blocks of transaction history.").arg(count);

    // Set icon state: red bird while catching up, green once up to date
    if(secs < 90*60)
    {
        tooltip = tr("Up to date") + QString(".<br>") + tooltip;
        labelBlocksIcon->setPixmap(QIcon(":/icons/sync_green").pixmap(STATUSBAR_ICONSIZE, STATUSBAR_ICONSIZE));

        overviewPage->showOutOfSyncWarning(false);

        progressBarLabel->setVisible(false);
        progressBar->setVisible(false);
    }
    else
    {
        // Represent time from last generated block in human readable text
        QString timeBehindText;
        // if(secs < 48*60*60)
        const int HOUR_IN_SECONDS = 60*60;
        const int DAY_IN_SECONDS = 24*60*60;
        const int WEEK_IN_SECONDS = 7*24*60*60;
        const int YEAR_IN_SECONDS = 31556952; // Average length of year in Gregorian calendar
        if(secs < 2*DAY_IN_SECONDS)
        {
            timeBehindText = tr("%n hour(s)","",secs/HOUR_IN_SECONDS);
        }
        else if(secs < 2*WEEK_IN_SECONDS)
        {
            timeBehindText = tr("%n day(s)","",secs/DAY_IN_SECONDS);
        }
        else if(secs < YEAR_IN_SECONDS)
        {
            timeBehindText = tr("%n week(s)","",secs/WEEK_IN_SECONDS);
        }
        else
        {
            int years = secs / YEAR_IN_SECONDS;
            int remainder = secs % YEAR_IN_SECONDS;
            timeBehindText = tr("%1 and %2").arg(tr("%n year(s)", "", years)).arg(tr("%n week(s)","", remainder/WEEK_IN_SECONDS));
        }

        progressBarLabel->setText(tr("Synchronizing with network..."));
        progressBarLabel->setVisible(true);
        if (syncTargetHeight > count && !syncEstimateText.isEmpty())
            applySyncEstimate(count);
        else
        {
            // No estimate yet (just started, or no peer is ahead): how far behind
            progressBar->setFormat(tr("%1 behind").arg(timeBehindText));
            progressBar->setMaximum(totalSecs);
            progressBar->setValue(totalSecs - secs);
        }
        progressBar->setVisible(true);

        tooltip = tr("Catching up...") + QString("<br>") + tooltip;
        labelBlocksIcon->setPixmap(QIcon(":/icons/sync_red").pixmap(STATUSBAR_ICONSIZE, STATUSBAR_ICONSIZE));
        prevBlocks = count;

        overviewPage->showOutOfSyncWarning(true);
        tooltip += QString("<br>");
        tooltip += tr("Last received block was generated %1 ago.").arg(timeBehindText);
        tooltip += QString("<br>");
        tooltip += tr("Transactions after this will not yet be visible.");
        if (!syncRateText.isEmpty())
            tooltip += QString("<br>") + syncRateText;
    }

    // Don't word-wrap this (fixed-width) tooltip
    tooltip = QString("<nobr>") + tooltip + QString("</nobr>");

    labelBlocksIcon->setToolTip(tooltip);
    progressBarLabel->setToolTip(tooltip);


    progressBar->setToolTip(tooltip);
}

void BitcoinGUI::error(const QString &title, const QString &message, bool modal)
{
    // Report errors from network/worker thread
    if(modal)
    {
        QMessageBox::critical(this, title, message, QMessageBox::Ok, QMessageBox::Ok);
    } else {
        notificator->notify(Notificator::Critical, title, message);
    }
}

void BitcoinGUI::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
#ifndef Q_OS_MAC // Ignored on Mac
    if(e->type() == QEvent::WindowStateChange)
    {
        if(clientModel && clientModel->getOptionsModel()->getMinimizeToTray())
        {
            QWindowStateChangeEvent *wsevt = static_cast<QWindowStateChangeEvent*>(e);
            if(!(wsevt->oldState() & Qt::WindowMinimized) && isMinimized())
            {
                QTimer::singleShot(0, this, SLOT(hide()));
                e->ignore();
            }
        }
    }
#endif
}

void BitcoinGUI::closeEvent(QCloseEvent *event)
{
    if(clientModel)
    {
#ifndef Q_OS_MAC // Ignored on Mac
        if(!clientModel->getOptionsModel()->getMinimizeToTray() &&
           !clientModel->getOptionsModel()->getMinimizeOnClose())
        {
            qApp->quit();
        }
#endif
    }
    QMainWindow::closeEvent(event);
}

void BitcoinGUI::askFee(qint64 nFeeRequired, bool *payFee)
{
     QString strMessage =

        tr("This transaction is over the size limit.  You can still send it for a fee of %1, "
          "which goes to the nodes that process your transaction and helps to support the network.  "
          "Do you want to pay the fee?").arg(
                 BitcoinUnits::formatWithUnit(BitcoinUnits::BTC, nFeeRequired));



    QMessageBox::StandardButton retval = QMessageBox::question(
          this, tr("Confirm transaction fee"), strMessage,
          QMessageBox::Yes|QMessageBox::Cancel, QMessageBox::Yes);
    *payFee = (retval == QMessageBox::Yes);
}

void BitcoinGUI::incomingTransaction(const QModelIndex & parent, int start, int end)
{
    if(!walletModel || !clientModel)
        return;
    TransactionTableModel *ttm = walletModel->getTransactionTableModel();
    qint64 amount = ttm->index(start, TransactionTableModel::Amount, parent)
                    .data(Qt::EditRole).toULongLong();
    if(!clientModel->inInitialBlockDownload())
    {
        // On new transaction, make an info balloon
        // Unless the initial block download is in progress, to prevent balloon-spam
        QString date = ttm->index(start, TransactionTableModel::Date, parent)
                        .data().toString();
        QString type = ttm->index(start, TransactionTableModel::Type, parent)
                        .data().toString();
        QString address = ttm->index(start, TransactionTableModel::ToAddress, parent)
                        .data().toString();
        QIcon icon = qvariant_cast<QIcon>(ttm->index(start,
                            TransactionTableModel::ToAddress, parent)
                        .data(Qt::DecorationRole));

        notificator->notify(Notificator::Information,
                            (amount)<0 ? (fS4CNotificator == true ? tr("Sent transaction") : tr("Sent transaction") ):
                                         tr("Incoming transaction"),
                              tr("Date: %1\n"
                                 "Amount: %2\n"
                                 "Type: %3\n"
                                 "Address: %4\n")
                              .arg(date)
                              .arg(BitcoinUnits::formatWithUnit(walletModel->getOptionsModel()->getDisplayUnit(), amount, true))
                              .arg(type)
                              .arg(address), icon);
    }
}

void BitcoinGUI::gotoOverviewPage()
{
    overviewAction->setChecked(true);
    centralWidget->setCurrentWidget(overviewPage);

    exportAction->setEnabled(false);
    disconnect(exportAction, SIGNAL(triggered()), 0, 0);
}

void BitcoinGUI::gotoHistoryPage()
{
    historyAction->setChecked(true);
    centralWidget->setCurrentWidget(transactionsPage);

    exportAction->setEnabled(true);
    disconnect(exportAction, SIGNAL(triggered()), 0, 0);
    connect(exportAction, SIGNAL(triggered()), transactionView, SLOT(exportClicked()));
}

void BitcoinGUI::gotoAddressBookPage()
{
    addressBookAction->setChecked(true);
    centralWidget->setCurrentWidget(addressBookPage);

    exportAction->setEnabled(true);
    disconnect(exportAction, SIGNAL(triggered()), 0, 0);
    connect(exportAction, SIGNAL(triggered()), addressBookPage, SLOT(exportClicked()));
}

void BitcoinGUI::gotoShoppingPage()
{
    shoppingAction->setChecked(true);
    centralWidget->setCurrentWidget(shoppingPage);
}

void BitcoinGUI::gotoExplorerPage()
{
    explorerAction->setChecked(true);
    centralWidget->setCurrentWidget(explorerPage);

    exportAction->setEnabled(false);
    disconnect(exportAction, SIGNAL(triggered()), 0, 0);
}

void BitcoinGUI::gotoReceiveCoinsPage()
{
    receiveCoinsAction->setChecked(true);
    centralWidget->setCurrentWidget(receiveCoinsPage);

    exportAction->setEnabled(true);
    disconnect(exportAction, SIGNAL(triggered()), 0, 0);
    connect(exportAction, SIGNAL(triggered()), receiveCoinsPage, SLOT(exportClicked()));
}

void BitcoinGUI::gotoSendCoinsPage()
{
    sendCoinsAction->setChecked(true);
    centralWidget->setCurrentWidget(sendCoinsPage);

    exportAction->setEnabled(false);
    disconnect(exportAction, SIGNAL(triggered()), 0, 0);
}

void BitcoinGUI::gotoSignMessageTab(QString addr)
{
    // call show() in showTab_SM()
    signVerifyMessageDialog->showTab_SM(true);

    if(!addr.isEmpty())
        signVerifyMessageDialog->setAddress_SM(addr);
}

void BitcoinGUI::gotoVerifyMessageTab(QString addr)
{
    // call show() in showTab_VM()
    signVerifyMessageDialog->showTab_VM(true);

    if(!addr.isEmpty())
        signVerifyMessageDialog->setAddress_VM(addr);
}

void BitcoinGUI::dragEnterEvent(QDragEnterEvent *event)
{
    // Accept only URIs
    if(event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void BitcoinGUI::dropEvent(QDropEvent *event)
{
    if(event->mimeData()->hasUrls())
    {
        int nValidUrisFound = 0;
        QList<QUrl> uris = event->mimeData()->urls();
        foreach(const QUrl &uri, uris)
        {
            if (sendCoinsPage->handleURI(uri.toString()))
                nValidUrisFound++;
        }

        // if valid URIs were found
        if (nValidUrisFound)
            gotoSendCoinsPage();
        else
            notificator->notify(Notificator::Warning, tr("URI handling"), tr("URI can not be parsed! This can be caused by an invalid FlapX address or malformed URI parameters."));
    }

    event->acceptProposedAction();
}

void BitcoinGUI::handleURI(QString strURI)
{
    // URI has to be valid
    if (sendCoinsPage->handleURI(strURI))
    {
        showNormalIfMinimized();
        gotoSendCoinsPage();
    }
    else
        notificator->notify(Notificator::Warning, tr("URI handling"), tr("URI can not be parsed! This can be caused by an invalid FlapX address or malformed URI parameters."));
}

void BitcoinGUI::setEncryptionStatus(int status)
{
    switch(status)
    {
    case WalletModel::Unencrypted:
        labelEncryptionIcon->hide();
        encryptWalletAction->setChecked(false);
        changePassphraseAction->setEnabled(false);
        unlockWalletAction->setVisible(false);
        lockWalletAction->setVisible(false);
        encryptWalletAction->setEnabled(true);
        break;
    case WalletModel::Unlocked:
        labelEncryptionIcon->show();

        labelEncryptionIcon->setPixmap(QIcon(":/icons/lock_open").pixmap(STATUSBAR_ICONSIZE,STATUSBAR_ICONSIZE));
        //labelEncryptionIcon->setToolTip(tr("<html><head/><body><p><img src=:/toolTip/res/tooltips/encryptedUnlockedTooltip.png/></p></body></html>"));
        encryptWalletAction->setChecked(true);
        changePassphraseAction->setEnabled(true);
        unlockWalletAction->setVisible(false);
        lockWalletAction->setVisible(true);
        encryptWalletAction->setEnabled(false); // TODO: decrypt currently not supported
        break;
    case WalletModel::Locked:
        labelEncryptionIcon->show();

        labelEncryptionIcon->setPixmap(QIcon(":/icons/lock_closed").pixmap(STATUSBAR_ICONSIZE,STATUSBAR_ICONSIZE));
        //labelEncryptionIcon->setToolTip(tr("<html><head/><body><p><img src=:/toolTip/res/tooltips/encryptedLockedTooltip.png/></p></body></html>"));
        encryptWalletAction->setChecked(true);
        changePassphraseAction->setEnabled(true);
        unlockWalletAction->setVisible(true);
        lockWalletAction->setVisible(false);
        encryptWalletAction->setEnabled(false); // TODO: decrypt currently not supported
        break;
    }
}

void BitcoinGUI::encryptWallet(bool status)
{
    if(!walletModel)
        return;
    AskPassphraseDialog dlg(status ? AskPassphraseDialog::Encrypt:
                                     AskPassphraseDialog::Decrypt, this);
    dlg.setModel(walletModel);
    dlg.exec();

    setEncryptionStatus(walletModel->getEncryptionStatus());
}

void BitcoinGUI::backupWallet()
{
    QString saveDir = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
    QString filename = QFileDialog::getSaveFileName(this, tr("Backup Wallet"), saveDir, tr("Wallet Data (*.dat)"));
    if(!filename.isEmpty()) {
        if(!walletModel->backupWallet(filename)) {
            QMessageBox::warning(this, tr("Backup Failed"), tr("There was an error trying to save the wallet data to the new location."));
        }
    }
}

void BitcoinGUI::changePassphrase()
{
    AskPassphraseDialog dlg(AskPassphraseDialog::ChangePass, this);
    dlg.setModel(walletModel);
    dlg.exec();
}

void BitcoinGUI::unlockWallet()
{
    if(!walletModel)
        return;
    // Unlock wallet when requested by wallet model
    if(walletModel->getEncryptionStatus() == WalletModel::Locked)
    {
        AskPassphraseDialog::Mode mode = sender() == unlockWalletAction ?
              AskPassphraseDialog::UnlockStaking : AskPassphraseDialog::Unlock;
        AskPassphraseDialog dlg(mode, this);
        dlg.setModel(walletModel);
        dlg.exec();
    }
}

void BitcoinGUI::lockWallet()
{
    if(!walletModel)
        return;

    walletModel->setWalletLocked(true);
}

void BitcoinGUI::showNormalIfMinimized(bool fToggleHidden)
{
    // activateWindow() (sometimes) helps with keyboard focus on Windows
    if (isHidden())
    {
        show();
        activateWindow();
    }
    else if (isMinimized())
    {
        showNormal();
        activateWindow();
    }
    else if (GUIUtil::isObscured(this))
    {
        raise();
        activateWindow();
    }
    else if(fToggleHidden)
        hide();
}

void BitcoinGUI::toggleHidden()
{
    showNormalIfMinimized(true);
}

void BitcoinGUI::updateWeight()
{
    if (!pwalletMain)
        return;

    TRY_LOCK(cs_main, lockMain);
    if (!lockMain)
        return;

    TRY_LOCK(pwalletMain->cs_wallet, lockWallet);
    if (!lockWallet)
        return;

    uint64_t nMinWeight = 0, nMaxWeight = 0;
    pwalletMain->GetStakeWeight(*pwalletMain, nMinWeight, nMaxWeight, nWeight);
}

void BitcoinGUI::updateStakingIcon()
{
    // While the wallet is catching up (newest block over an hour old) it can't
    // stake, so skip working out the stake weight: it reads every coin's block
    // from disk and would compete with the block download
    bool fSyncing = !pindexBest || pindexBest->GetBlockTime() < GetTime() - 60 * 60;
    uint64_t nNetworkWeight = 0;
    if (fSyncing)
        nWeight = 0;
    else
    {
        // Skip this refresh if the core is busy rather than freezing the window
        TRY_LOCK(cs_main, lockMain);
        if (!lockMain)
            return;
        updateWeight();
        if (nLastCoinStakeSearchInterval && nWeight)
            nNetworkWeight = GetPoSKernelPS();
    }

    if (nLastCoinStakeSearchInterval && nWeight)
    {
        unsigned nEstimateTime = nTargetSpacing * 2 * nNetworkWeight / nWeight;

        QString text;
        if (nEstimateTime < 60)
        {
            text = tr("%n second(s)", "", nEstimateTime);
        }
        else if (nEstimateTime < 60*60)
        {
            text = tr("%n minute(s)", "", nEstimateTime/60);
        }
        else if (nEstimateTime < 24*60*60)
        {
            text = tr("%n hour(s)", "", nEstimateTime/(60*60));
        }
        else
        {
            text = tr("%n day(s)", "", nEstimateTime/(60*60*24));
        }
        if (!stakingAnimTimer->isActive())
        {
            animateStakingIcon();
            stakingAnimTimer->start();
        }
        labelStakingIcon->setToolTip(tr("Staking.<br>Your weight: %1<br>Network weight: %2<br>Expected time to earn reward: %3").arg(nWeight).arg(nNetworkWeight).arg(text));

    }
    else
    {
        stakingAnimTimer->stop();
        labelStakingIcon->setPixmap(QIcon(":/icons/res/icons/staking_off.png").pixmap(STATUSBAR_ICONSIZE,STATUSBAR_ICONSIZE));
        if (pwalletMain && pwalletMain->IsLocked())
            labelStakingIcon->setToolTip(tr("Not staking because wallet is locked"));
        else if (vNodes.empty())
            labelStakingIcon->setToolTip(tr("Not staking because wallet is offline"));
        else if (fSyncing)
            labelStakingIcon->setToolTip(tr("Not staking because wallet is syncing"));
        else if (!nWeight)
            labelStakingIcon->setToolTip(tr("Not staking because you don't have mature coins"));
        else
            labelStakingIcon->setToolTip(tr("Not staking"));
    }
}

void BitcoinGUI::paintEvent(QPaintEvent *event)
{
    QMainWindow::paintEvent(event);

    // FlappyCoin theme: sky, with clouds, city and bushes just above the status bar
    if (GUIUtil::isFlappyTheme())
    {
        QRect area = rect();
        if (statusBar() && statusBar()->isVisible())
            area.setBottom(statusBar()->geometry().top() - 1);
        QPainter painter(this);
        GUIUtil::drawFlappyScenery(painter, area);
        return;
    }

    // Same as the explorer's background: a navy glow centred above the top edge
    // (ellipse at 50% -20%), fading into the background 60% of the way to the far corner.
    // "Lighten" only brightens, so the starfield under it stays visible.
    const QRectF area = rect();
    const QPointF centre(area.width() / 2.0, -0.2 * area.height());
    const double farCorner = std::sqrt(std::pow(area.width() / 2.0, 2) + std::pow(1.2 * area.height(), 2));

    QRadialGradient glow(centre, 0.6 * farCorner);
    glow.setColorAt(0.0, QColor(24, 40, 74));
    glow.setColorAt(1.0, QColor(11, 18, 32));

    QPainter painter(this);
    painter.setCompositionMode(QPainter::CompositionMode_Lighten);
    painter.fillRect(event->rect(), glow);
}

void BitcoinGUI::animateStakingIcon()
{
    // The staking bird bobs gently up and down (8 frames, 1.2 s per cycle)
    static const int bob[8] = { 0, -1, -2, -1, 0, 1, 2, 1 };
    QPixmap bird = QIcon(":/icons/res/icons/staking_on.png").pixmap(STATUSBAR_ICONSIZE, STATUSBAR_ICONSIZE);

    QPixmap frame(STATUSBAR_ICONSIZE, STATUSBAR_ICONSIZE);
    frame.fill(Qt::transparent);
    QPainter painter(&frame);
    painter.drawPixmap(0, bob[nStakingAnimFrame], bird);
    painter.end();
    labelStakingIcon->setPixmap(frame);

    nStakingAnimFrame = (nStakingAnimFrame + 1) % 8;
}

void BitcoinGUI::detectShutdown()
{
    if (ShutdownRequested())
        QMetaObject::invokeMethod(QCoreApplication::instance(), "quit", Qt::QueuedConnection);
}

void BitcoinGUI::charityClicked(QString addr)
{
    charityAction->setChecked(true);
    centralWidget->setCurrentWidget(stakeForCharityDialog);

    if(!addr.isEmpty())
        stakeForCharityDialog->setAddress(addr);

    exportAction->setEnabled(false);
    disconnect(exportAction, SIGNAL(triggered()), 0, 0);
}

// Text for a duration, rounded to what's useful for a rough estimate
static QString FormatTimeLeft(qint64 nSecs)
{
    if (nSecs < 60)
        return QObject::tr("under a minute");
    if (nSecs < 60 * 60)
        return QObject::tr("%n min", "", (int)((nSecs + 30) / 60));
    if (nSecs < 48 * 60 * 60)
    {
        int nMins = (int)((nSecs + 30) / 60);
        return QObject::tr("%1 h %2 min").arg(nMins / 60).arg(nMins % 60);
    }
    int nHours = (int)((nSecs + 1800) / 3600);
    return QObject::tr("%1 days %2 h").arg(nHours / 24).arg(nHours % 24);
}

void BitcoinGUI::updateSyncEstimate()
{
    int nHeight = nBestHeightUI;
    qint64 nNowMs = GetTimeMillis();

    // Only while catching up (tip more than 90 minutes old, as the bar uses)
    if (nHeight < 0 || nBestTimeUI >= GetTime() - 90 * 60)
    {
        syncSamples.clear();
        syncEstimateText.clear();
        syncRateText.clear();
        syncTargetHeight = -1;
        return;
    }

    syncTargetHeight = GetBestPeerHeight();

    // Average the download rate over the last few minutes so short stalls or
    // bursts don't make the estimate jump around
    syncSamples.push_back(std::make_pair(nNowMs, nHeight));
    while (syncSamples.size() > 1 && nNowMs - syncSamples.front().first > SYNC_ESTIMATE_WINDOW_MS)
        syncSamples.pop_front();

    qint64 nSpanMs = syncSamples.back().first - syncSamples.front().first;
    int nGained = syncSamples.back().second - syncSamples.front().second;
    int nLeft = syncTargetHeight - nHeight;

    if (syncTargetHeight <= nHeight)
    {
        syncEstimateText.clear();
        syncRateText.clear();
    }
    else if (nSpanMs < 30 * 1000)
    {
        syncEstimateText = tr("estimating time left...");
        syncRateText = tr("%n block(s) to go.", "", nLeft);
    }
    else if (nGained <= 0)
    {
        syncEstimateText = tr("waiting for blocks...");
        syncRateText = tr("No blocks received in the last %n minute(s); %1 to go.", "", (int)(nSpanMs / 60000)).arg(nLeft);
    }
    else
    {
        double dRate = nGained * 1000.0 / nSpanMs;   // blocks per second
        syncEstimateText = tr("about %1 left").arg(FormatTimeLeft((qint64)(nLeft / dRate)));
        syncRateText = tr("Downloading %1 blocks a second, %2 blocks to go (highest peer: %3).")
                       .arg(QString::number(dRate, 'f', dRate < 10 ? 1 : 0)).arg(nLeft).arg(syncTargetHeight);
    }

    if (progressBar->isVisible() && syncTargetHeight > nHeight && !syncEstimateText.isEmpty())
        applySyncEstimate(nHeight);
}

void BitcoinGUI::applySyncEstimate(int count)
{
    progressBar->setMaximum(syncTargetHeight);
    progressBar->setValue(std::min(count, syncTargetHeight));
    progressBar->setFormat(syncEstimateText);
}

void BitcoinGUI::calcClicked()
{
    calcDialog dlg;
    dlg.exec();
}

/* zeewolf: Hot swappable wallet themes */
// Names of the built-in themes (compiled into the wallet): the default look
// (res/flapxcoin.qss) and the original 2014 FlappyCoin wallet's look (res/flappycoin.qss)
static const QString FLAPXCOIN_THEME = "FlapXCoin";
static const QString FLAPPYCOIN_THEME = "FlappyCoin";

void BitcoinGUI::themeActionTriggered(QAction *action)
{
    changeTheme(action->data().toString());
}

void BitcoinGUI::changeTheme(QString theme)
{
    loadTheme(theme);
}

void BitcoinGUI::loadTheme(QString theme)
{
    QString styleSheet;
    bool fFlappy = (theme == FLAPPYCOIN_THEME);

    if (fFlappy)
        styleSheet = GUIUtil::flappyStyleSheet();
    // Extra themes live in <themes dir>/<name>/styles.qss and may use $theme-dir for their own files
    else if (theme != FLAPXCOIN_THEME && !themesDir.isEmpty())
    {
        QString themeDir = themesDir + "/" + theme;
        styleSheet = GUIUtil::loadStyleSheet(themeDir + "/styles.qss", themeDir);
    }

    // FlapXCoin, or a theme that is missing or unreadable: use the built-in look
    if (styleSheet.isEmpty())
    {
        theme = FLAPXCOIN_THEME;
        fFlappy = false;
        styleSheet = GUIUtil::flapxStyleSheet();
    }

    // FlappyCoin has its own light palette; every other theme sits on the FlapX one
    if (fFlappy)
        GUIUtil::applyFlappyPalette();
    else
        GUIUtil::applyFlapXPalette();
    GUIUtil::setFlappyTheme(fFlappy);
    qApp->setStyleSheet(styleSheet);
    update();

    // remember the choice and tick it in the Themes menu
    selectedTheme = theme;
    QSettings settings;
    settings.setValue("Template", selectedTheme);

    if (themeActionGroup)
    {
        foreach (QAction *action, themeActionGroup->actions())
            action->setChecked(action->data().toString() == selectedTheme);
    }
}

void BitcoinGUI::listThemes(QStringList& themes)
{
    // The built-in themes are always available and always first
    themes.clear();
    themes.append(FLAPXCOIN_THEME);
    themes.append(FLAPPYCOIN_THEME);

    // Extra themes: a "themes" folder in the data directory, next to the wallet,
    // or in the source tree when running a development build
    QStringList searchDirs;
    searchDirs << QString::fromStdString((GetDataDir(false) / "themes").string())
               << qApp->applicationDirPath() + "/themes"
               << qApp->applicationDirPath() + "/src/qt/res/themes"
               << qApp->applicationDirPath() + "/../src/qt/res/themes";

    themesDir.clear();
    foreach (const QString &dirPath, searchDirs)
    {
        QDir dir(dirPath);
        if (dir.exists())
        {
            themesDir = dir.absolutePath();
            break;
        }
    }

    if (!themesDir.isEmpty())
    {
        QDir dir(themesDir);
        foreach (const QString &entry, dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name))
        {
            // a theme folder needs a styles.qss, and can't reuse a built-in name
            if (entry.compare(FLAPXCOIN_THEME, Qt::CaseInsensitive) != 0 &&
                entry.compare(FLAPPYCOIN_THEME, Qt::CaseInsensitive) != 0 &&
                QFile::exists(dir.filePath(entry + "/styles.qss")))
                themes.append(entry);
        }
    }

    // Saved theme from settings; old "Default" and unknown names fall back to FlapXCoin
    QSettings settings;
    selectedTheme = settings.value("Template").toString();
    if (!themes.contains(selectedTheme))
        selectedTheme = FLAPXCOIN_THEME;

    // load it!
    loadTheme(selectedTheme);
}



void BitcoinGUI::keyPressEvent(QKeyEvent * e)
{
    switch (e->type())
     {
       case QEvent::KeyPress:
         // $ key
         if (e->key() == 36) {
             // dev feature: key reloads selected theme
             loadTheme(selectedTheme);
         }
         break;
       default:
         break;
     }

}

/* /zeewolf: Hot swappable wallet themes */
