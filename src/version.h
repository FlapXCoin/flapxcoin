// Copyright (c) 2021 The Bitcoin developers
// Copyright (c) 2021 Litecoin Developers
// Copyright (c) 2021 NerdLabs001

// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef BITCOIN_VERSION_H
#define BITCOIN_VERSION_H

#include "clientversion.h"
#include <string>

// client versioning


// These need to be macro's, as version.cpp's voodoo requires it
// #define CLIENT_VERSION_MAJOR       2
// #define CLIENT_VERSION_MINOR       3
// #define CLIENT_VERSION_REVISION    0
// #define CLIENT_VERSION_BUILD       0

static const int CLIENT_VERSION =
                           1000000 * CLIENT_VERSION_MAJOR    //Manditory
                         +   10000 * CLIENT_VERSION_MINOR    //Checkpoint update
                         +     100 * CLIENT_VERSION_REVISION //GUI Update
                         +       1 * CLIENT_VERSION_BUILD;   //Development (Ignore)

extern const std::string CLIENT_NAME;
extern const std::string CLIENT_BUILD;
extern const std::string CLIENT_DATE;

//
// database format versioning
//
static const int DATABASE_VERSION = 70501;

//
// network protocol versioning
//

static const int PROTOCOL_VERSION = 210043;

// intial proto version, to be increased after version/verack negotiation
static const int INIT_PROTO_VERSION = 209;

// disconnect from peers older than this proto version
static const int MIN_PEER_PROTO_VERSION = 199999;

// nTime field added to CAddress, starting with this version;
// if possible, avoid requesting addresses nodes older than this
static const int CADDR_TIME_VERSION = 31402;

// only request blocks from nodes outside this range of versions
static const int NOBLKS_VERSION_START = 0;
static const int NOBLKS_VERSION_END = 199999;

// BIP 0031, pong message, is enabled for all versions AFTER this one
static const int BIP0031_VERSION = 60000;

// reject blocks with non-canonical signatures starting from this version
static const int CANONICAL_BLOCK_SIG_VERSION = 60016;
static const int CANONICAL_BLOCK_SIG_LOW_S_VERSION = 60018;

#endif
