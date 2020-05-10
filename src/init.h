// Copyright (c) 2009-2021 Satoshi Nakamoto
// Copyright (c) 2009-2021 The Bitcoin developers
// Copyright (c) 2011-2021 Litecoin Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef BITCOIN_INIT_H
#define BITCOIN_INIT_H
#include <stdint.h>
#include "wallet.h"

namespace boost {
    class thread_group;
} // namespace boost

extern CWallet* pwalletMain;
extern std::string strWalletFileName;
void StartShutdown();
bool ShutdownRequested();
void Shutdown();
bool AppInit2(boost::thread_group &threadGroup);
std::string HelpMessage();

#endif
