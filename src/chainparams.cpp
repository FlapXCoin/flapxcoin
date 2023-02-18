// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2023 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "assert.h"

#include "chainparams.h"
#include "main.h"
#include "util.h"

#include <boost/assign/list_of.hpp>

using namespace boost::assign;

struct SeedSpec6 {
    uint8_t addr[16];
    uint16_t port;
};

#include "chainparamsseeds.h"

//
// Main network
//

// Convert the pnSeeds6 array into usable address objects.
static void convertSeed6(std::vector<CAddress> &vSeedsOut, const SeedSpec6 *data, unsigned int count)
{
    // It'll only connect to one or two seed nodes because once it connects,
    // it'll get a pile of addresses with newer timestamps.
    // Seed nodes are given a random 'last seen time' of between one and two
    // weeks ago.
    const int64_t nOneWeek = 7*24*60*60;
    for (unsigned int i = 0; i < count; i++)
    {
        struct in6_addr ip;
        memcpy(&ip, data[i].addr, sizeof(ip));
        CAddress addr(CService(ip, data[i].port));
        addr.nTime = GetTime() - GetRand(nOneWeek) - nOneWeek;
        vSeedsOut.push_back(addr);
    }
}

class CMainParams : public CChainParams {
public:
    CMainParams() {
        // The message start string is designed to be unlikely to occur in normal data.
        // The characters are rarely used upper ASCII, not valid as UTF-8, and produce
        // a large 4-byte int at any alignment.
        pchMessageStart[0] = 0xfd;
        pchMessageStart[1] = 0xb6;
        pchMessageStart[2] = 0xa5;
        pchMessageStart[3] = 0xdb;
        vAlertPubKey = ParseHex("04ef014b36647e8433a2cedf76f1d6ea0bc5914ba936fadceda90d7472da3cf442469d3a1ab5ee416e7428726761dd3188bda3d0ae163db491f8ca0bdad92a0506");
        nDefaultPort = 60042;
        nRPCPort = 60142;
        bnProofOfWorkLimit = CBigNum(~uint256(0) >> 20);

        // Build the genesis block. Note that the output of the genesis coinbase cannot
        // be spent as it did not originally exist in the database.
        //

        const char* pszTimestamp = "Sat Apr 14 20:10:41 PDT 2018 - FlapX arises from the ashes of Flappycoin";
        CTransaction txNew;
        txNew.vin.resize(1);
        txNew.vout.resize(1);
        txNew.vin[0].scriptSig = CScript() << 486604799 << CBigNum(4) << vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
        txNew.vout[0].nValue = 0 * COIN;
        txNew.vout[0].scriptPubKey = CScript() << ParseHex("0457575678901234567890000222222333444555666777888999000000aaaaabbbbbcccccdddddeeeeeff00ff00ff00ff001234567890abcdef0022446688abc89") << OP_CHECKSIG;
        txNew.nVersion = 2;
        genesis.vtx.push_back(txNew);
        genesis.hashPrevBlock = 0;
        genesis.hashMerkleRoot = genesis.BuildMerkleTree();
        genesis.nVersion = 1;
        genesis.nTime    = 1523761963;
        genesis.nBits    = 0x1e0ffff0;
        genesis.nNonce   = 151917;

        hashGenesisBlock = genesis.GetHash();
        assert(hashGenesisBlock == uint256("0xf30d6d37c276519974c14eae52dac5538b2bdc358f2e4a3b48a14b201eb48fed"));
        assert(genesis.hashMerkleRoot == uint256("0x9e17be6aefbffc74cf4deab73f93bd050e696008fe28c56de80520a22b05fafb"));
///removed seednodes as un reachable left in place incase they are/become live again
/*
        N/A
*/
///new seednodes should improve initial connectivity and sync	    
        vSeeds.push_back(CDNSSeedData("flapx", "flapx000.flapxcoin.com"));
        vSeeds.push_back(CDNSSeedData("flapx", "flapx001.flapxcoin.com"));
        vSeeds.push_back(CDNSSeedData("flapx", "flapx002.flapxcoin.com"));
        vSeeds.push_back(CDNSSeedData("flapx", "flapx003.flapxcoin.com"));
        vSeeds.push_back(CDNSSeedData("flapx", "flapx004.flapxcoin.com"));
        vSeeds.push_back(CDNSSeedData("flapx", "flapx005.flapxcoin.com"));
        vSeeds.push_back(CDNSSeedData("flapx", "flapx006.flapxcoin.com"));
        vSeeds.push_back(CDNSSeedData("flapx", "flapx007.flapxcoin.com"));
        vSeeds.push_back(CDNSSeedData("flapx", "flapx008.flapxcoin.com"));
        vSeeds.push_back(CDNSSeedData("flapx", "flapx009.flapxcoin.com"));
        vSeeds.push_back(CDNSSeedData("flapx", "flapx010.flapxcoin.com"));
        vSeeds.push_back(CDNSSeedData("nlabs", "flapx000.nerdLabs001.com"));
        vSeeds.push_back(CDNSSeedData("nlabs", "flapx001.nerdLabs001.com"));
        vSeeds.push_back(CDNSSeedData("nlabs", "flapx002.nerdLabs001.com"));
        vSeeds.push_back(CDNSSeedData("nlabs", "flapx003.nerdLabs001.com"));
        vSeeds.push_back(CDNSSeedData("nlabs", "flapx004.nerdLabs001.com"));
        vSeeds.push_back(CDNSSeedData("nlabs", "flapx005.nerdLabs001.com"));
        vSeeds.push_back(CDNSSeedData("nlabs", "flapx007.nerdLabs001.com"));
        vSeeds.push_back(CDNSSeedData("nlabs", "flapx008.nerdLabs001.com"));
        vSeeds.push_back(CDNSSeedData("nlabs", "flapx009.nerdLabs001.com"));
        vSeeds.push_back(CDNSSeedData("nlabs", "flapx010.nerdLabs001.com"));




        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1, 75); //X
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1, 80); //Z
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1, 240);
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x88)(0xB2)(0x1E).convert_to_container<std::vector<unsigned char> >();
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x88)(0xAD)(0xE4).convert_to_container<std::vector<unsigned char> >();

        convertSeed6(vFixedSeeds, pnSeed6_main, ARRAYLEN(pnSeed6_main));

        // nLastPOWBlock = 10000;
    }

    virtual const CBlock& GenesisBlock() const { return genesis; }
    virtual Network NetworkID() const { return CChainParams::MAIN; }

    virtual const vector<CAddress>& FixedSeeds() const {
        return vFixedSeeds;
    }
protected:
    CBlock genesis;
    vector<CAddress> vFixedSeeds;
};
static CMainParams mainParams;


//
// Testnet
//

class CTestNetParams : public CMainParams {
public:
    CTestNetParams() {
        // The message start string is designed to be unlikely to occur in normal data.
        // The characters are rarely used upper ASCII, not valid as UTF-8, and produce
        // a large 4-byte int at any alignment.
        pchMessageStart[0] = 0xfd;
        pchMessageStart[1] = 0xc0;
        pchMessageStart[2] = 0xb6;
        pchMessageStart[3] = 0xf1;
        bnProofOfWorkLimit = CBigNum(~uint256(0) >> 16);
        vAlertPubKey = ParseHex("0471dc165db490094d35cde15b1f5d755fa6ad6f2b5ed0f340e3f17f57389c3c2af113a8cbcc885bde73305a553b5640c83021128008ddf882e856336269080496");
        nDefaultPort = 21410;
        nRPCPort = 21311;
        strDataDir = "testnet";

        genesis.nTime    = 1300000000;
        // genesis.nBits  = bnProofOfWorkLimit.GetCompact();
        genesis.nNonce = 0;
        hashGenesisBlock = genesis.GetHash();
	// genesis.print();
        assert(hashGenesisBlock == uint256("0x9ff8786e790c41e6f44a926b8b2c70183eb655e7ef188a4630408164dd13bc6b"));
        assert(genesis.hashMerkleRoot == uint256("0x9e17be6aefbffc74cf4deab73f93bd050e696008fe28c56de80520a22b05fafb"));

        vFixedSeeds.clear();
        vSeeds.clear();

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1, 137); //x
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1, 142); //z
        base58Prefixes[SECRET_KEY]     = std::vector<unsigned char>(1, 239);
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x35)(0x87)(0xCF).convert_to_container<std::vector<unsigned char> >();
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x35)(0x83)(0x94).convert_to_container<std::vector<unsigned char> >();

        convertSeed6(vFixedSeeds, pnSeed6_test, ARRAYLEN(pnSeed6_test));

        nLastPOWBlock = 0x7fffffff;
    }
    virtual Network NetworkID() const { return CChainParams::TESTNET; }
};
static CTestNetParams testNetParams;


//
// Regression test
//
class CRegTestParams : public CTestNetParams {
public:
    CRegTestParams() {
        pchMessageStart[0] = 0xfa;
        pchMessageStart[1] = 0xbf;
        pchMessageStart[2] = 0xb5;
        pchMessageStart[3] = 0xda;
        bnProofOfWorkLimit = CBigNum(~uint256(0) >> 1);
        genesis.nTime = 1411111111;
        genesis.nBits  = bnProofOfWorkLimit.GetCompact();
        genesis.nNonce = 2;
        hashGenesisBlock = genesis.GetHash();
        nDefaultPort = 18444;
        strDataDir = "regtest";
        // assert(hashGenesisBlock == uint256("0x523dda6d336047722cbaf1c5dce622298af791bac21b33bf6e2d5048b2a13e3d"));

        vSeeds.clear();  // Regtest mode doesn't have any DNS seeds.
    }

    virtual bool RequireRPCPassword() const { return false; }
    virtual Network NetworkID() const { return CChainParams::REGTEST; }
};
static CRegTestParams regTestParams;

static CChainParams *pCurrentParams = &mainParams;

const CChainParams &Params() {
    return *pCurrentParams;
}

void SelectParams(CChainParams::Network network) {
    switch (network) {
        case CChainParams::MAIN:
            pCurrentParams = &mainParams;
            break;
        case CChainParams::TESTNET:
            pCurrentParams = &testNetParams;
            break;
        case CChainParams::REGTEST:
            pCurrentParams = &regTestParams;
            break;
        default:
            assert(false && "Unimplemented network");
            return;
    }
}

bool SelectParamsFromCommandLine() {
    bool fRegTest = GetBoolArg("-regtest", false);
    bool fTestNet = GetBoolArg("-testnet", false);

    if (fTestNet && fRegTest) {
        return false;
    }

    if (fRegTest) {
        SelectParams(CChainParams::REGTEST);
    } else if (fTestNet) {
        SelectParams(CChainParams::TESTNET);
    } else {
        SelectParams(CChainParams::MAIN);
    }
    return true;
}
