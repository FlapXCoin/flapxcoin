// Copyright (c) 2009-2023 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/assign/list_of.hpp> // for 'map_list_of()'
#include <boost/foreach.hpp>

#include "checkpoints.h"

#include "txdb.h"
#include "main.h"
#include "uint256.h"


static const int nCheckpointSpan = 10;

namespace Checkpoints
{
    typedef std::map<int, uint256> MapCheckpoints;

    //
    // What makes a good checkpoint block?
    // + Is surrounded by blocks with reasonable timestamps
    //   (no blocks before with a timestamp after, none after with
    //    timestamp before)
    // + Contains no strange transactions
    //
static MapCheckpoints mapCheckpoints =
        boost::assign::map_list_of
        (      0, Params().HashGenesisBlock() )
        (      5, uint256("0xf87e49aa55da512d4d52d9845548095d79455da0f8ce04333239fa93a411fbaf"))
        ( 100000, uint256("0xbfbe09c7103b672ce64e99b8b02b1eb1a5d7d39ce211331bb9ad3ae6f7d381ed"))
        ( 125000, uint256("0x0c4a145b60b899bcaef36d2c26784b7550dc9f38c9d1ee6866e084e03b3b0372"))
        ( 150000, uint256("0x8f282b87a5ec547753bd54f8d98ddac174ac4fac3c85d852071a16f9e088a20b"))
        ( 175000, uint256("0x881b5e81a5367994a9604d48b5b3df8f1038f0f75b78e47ee88a867559e16ca7"))
        ( 200000, uint256("0xe66c728d7ef8986afe6009a292f76161658882ea7adc3eb6f9e59db4e7a76849"))
        ( 225000, uint256("0x76bbbb56a9aff06d5e21db2ee22984b78e91cdb2da41c3300f299bf549a6b6e8"))
        ( 250000, uint256("0x62dab06a1296eee06a51d2944f9ce1465e71ab15f3dd291505113341813f0d05"))
        ( 275000, uint256("0xa660fc539487844e36a5811bd63ca28eb71305b9ea9c5efabcd34e333d2cd6ce"))
        ( 300000, uint256("0x6786bc4f06982eeaa218b526aa181cf8188e8eb33b8418de8996001a5dca982a"))
        ( 325000, uint256("0x67269ff31c308d77802504fa0ba5c99126d34769a9f6461eae4644627813e5de"))
        ( 350000, uint256("0x3cb61c2848edf501334f20a2f87bb9d1d2b55e52c44c46ca05ce9c9a82b60a05"))
        ( 400000, uint256("0x1d7befbec411d8134306b621aa6758709f9a8209313a447869538f5fe4048b65"))
        ( 500000, uint256("0xdb03b4adda738d910b07def47870ce87e9b792cf024468b73beda8f07bbf6c39"))
        ( 600000, uint256("0x62458e42995e5c7b7f01443c94161921b2e55d874270c488bcb3bd82467ae06a"))
        ( 650000, uint256("0xcac0df1cb10a311d8b9bd53e803b661d7d44ac118a9f5ed5fbe97546b4572524"))
        ( 700000, uint256("0xed6761a21f69e5b3bd51cbdaf81cef02bd9a9c0d3a5aaad1e886831ffd8f9452"))
        ( 750000, uint256("0x0311b62721b49f695154db60553f709f224b59f4c94dd2befe9abb781e79c44e"))
        ( 800000, uint256("0xbcf9f4a5a1291a6dc35b8078b2e5bb94f265d877cbcf5a9678429a35579c0ae5"))
        ( 850000, uint256("0x82614434cc3e63839cdc3805c8c90689f5dcf87821e6a39820cd50914d2575b4"))
        ( 900000, uint256("0xda053bc86b0cda2d26a45be64bec2959b77d37a44d717debafec10c45770faac"))
        ( 950000, uint256("0xb33dd8c5f481970b0b12fab8920a4b35163dd5e89c4e6671fbd5664d87dee13b"))
        (1000000, uint256("0x55b8b3aa75a004b405e6c70ca924c25d95cb0ed0f95c5b3fdbb48a5708a54693"))
        (1100000, uint256("0x43449510f872065f532ed668e561caa3c00557775a6c3c0676e02f1f36fb50d8"))
        (1200000, uint256("0x31cb8e35ee44835f3dd387c42607c7921afce08c09dabd18d8b83d56df3ed3b9"))
        (1225000, uint256("0x28a95a10bd256f5562bc81897746adf9c7db0bb1f98aece93fdde1e7b2790e5c"))
        (1250000, uint256("0x577ec3afa7bd8011c495b16ca49305847c81c4602150ac77e5475befcc38ca59"))
        (1275000, uint256("0xa04017b12664d9ddde808faeccfae98406a03193546f1ad7311799b7da181d29"))
        (1284933, uint256("0x564c09f12ad9709034d2311a57b8c71c5309d88f1482e2b4399f44e92f788990"))
        (1285512, uint256("0x0842fce8c689f567872fc379895efff06e8489e472e67e1c810426046d992d7e"))
        (1300000, uint256("0xc18598869ee398ca7d2cb2146c74cff5853c3cc02093500cd79f7eac184efaf2"))
        (1325000, uint256("0xe6e454f922ec57b5281804794ad1dd7677eb20dc9f8dee03bac9222c6e6e7c09"))
        (1350000, uint256("0xe0894e502ca83f77dc7ac2b3b9cb2acbfe4893b411e7b07b9d895916e8e8141f"))
        (1400000, uint256("0x03e964fec5c36709598b3a731e9249c5d7e10184dd25e860a7748bff53de4f81"))
        (1500000, uint256("0x06f28a0fe6afd55ab0aa1c0bc7a78f2e1e398f6aafd3d842cd37abf97bc86c1f"))
        (1600000, uint256("0xd80365f6170441e838c5db9e746462c6079b220818d14389b03c7da907f7b616"))
        (1700000, uint256("0xe3daf0f87e3c4a77f1768c652e323a3a9dea18091d8793a6bb38adebc1584386"))
        (1750000, uint256("0x5b8ee6b7eab27882ee38cdab4739faf5830c21520aee39a131094bfe4d8306d0"))
        (1800000, uint256("0xb6913150349ab572c4563a74506e2b9e4c757229f51c973a68c6b128e44945f4"))
        (1850000, uint256("0x73d1fe2d41965c04551b82163cfb89de2c4e2621347c1a4bb5827f17320839a1"))
        (1900000, uint256("0x10a17638f5745ef5f6208af702d81f74674e5fc09157001cda66e3e7714f762b"))
        (1950000, uint256("0x62cdaf0c82704acb19ce9dbf659bff978a51fc25e8bd540ca4d552e1b9ea73df"))
        (2000000, uint256("0xf1a60d75a54bd6907eb42dca5e574d3410bbeb89b507768b376a79d408b37ca1"))
        (2100000, uint256("0x89c6ceec919059f9d9c8960ebe3c34adc195c96844d91b84dba1f51958cf5c03"))
        (2200000, uint256("0xd0b5357c5133bfd4a36cf828dcb3b1c1ec0413fcf6042eed730329b04ff94bed"))
        (2300000, uint256("0xc73bdc59ae0ab897c668dea0550f467e4521caebb4abed00e2ddda66e07da236"))
        (2400000, uint256("0x1377d727071219b9d07c164d4d47f315e00ec2c57682fc68c73268b11d6daf5b"))
        (2500000, uint256("0x53d621ec70ae61a97a9ede5f25d24d40f52cfda4a2c514617ed0e5dd651c473e"))
        (2600000, uint256("0xb2c140d171353fbcfc82b80f6c1ee041e7137e6d06c76310336d61a710de1c03"))
        (2650000, uint256("0x5b87ec66086de7697c8144fb080075ecb042d1d455245536d7b084c44612c99a"))




















        ;
        ;

    // TestNet has no checkpoints
    static MapCheckpoints mapCheckpointsTestnet =
        boost::assign::map_list_of
        ( 0, Params().HashGenesisBlock() )
        ;

    bool CheckHardened(int nHeight, const uint256& hash)
    {
        MapCheckpoints& checkpoints = (TestNet() ? mapCheckpointsTestnet : mapCheckpoints);

        MapCheckpoints::const_iterator i = checkpoints.find(nHeight);
        if (i == checkpoints.end()) return true;
        return hash == i->second;
    }

    int GetTotalBlocksEstimate()
    {
        MapCheckpoints& checkpoints = (TestNet() ? mapCheckpointsTestnet : mapCheckpoints);

        return checkpoints.rbegin()->first;
    }

    CBlockIndex* GetLastCheckpoint(const std::map<uint256, CBlockIndex*>& mapBlockIndex)
    {
        MapCheckpoints& checkpoints = (TestNet() ? mapCheckpointsTestnet : mapCheckpoints);

        BOOST_REVERSE_FOREACH(const MapCheckpoints::value_type& i, checkpoints)
        {
            const uint256& hash = i.second;
            std::map<uint256, CBlockIndex*>::const_iterator t = mapBlockIndex.find(hash);
            if (t != mapBlockIndex.end())
                return t->second;
        }
        return NULL;
    }

    // ppcoin: synchronized checkpoint (centrally broadcasted)
    uint256 hashSyncCheckpoint = 0;
    uint256 hashPendingCheckpoint = 0;
    CSyncCheckpoint checkpointMessage;
    CSyncCheckpoint checkpointMessagePending;
    uint256 hashInvalidCheckpoint = 0;
    CCriticalSection cs_hashSyncCheckpoint;

    // ppcoin: get last synchronized checkpoint
    CBlockIndex* GetLastSyncCheckpoint()
    {
        LOCK(cs_hashSyncCheckpoint);
        if (!mapBlockIndex.count(hashSyncCheckpoint))
            error("GetSyncCheckpoint: block index missing for current sync-checkpoint %s", hashSyncCheckpoint.ToString().c_str());
        else
            return mapBlockIndex[hashSyncCheckpoint];
        return NULL;
    }

    // ppcoin: only descendant of current sync-checkpoint is allowed
    bool ValidateSyncCheckpoint(uint256 hashCheckpoint)
    {
        if (!mapBlockIndex.count(hashSyncCheckpoint))
            return error("ValidateSyncCheckpoint: block index missing for current sync-checkpoint %s", hashSyncCheckpoint.ToString().c_str());
        if (!mapBlockIndex.count(hashCheckpoint))
            return error("ValidateSyncCheckpoint: block index missing for received sync-checkpoint %s", hashCheckpoint.ToString().c_str());

        CBlockIndex* pindexSyncCheckpoint = mapBlockIndex[hashSyncCheckpoint];
        CBlockIndex* pindexCheckpointRecv = mapBlockIndex[hashCheckpoint];

        if (pindexCheckpointRecv->nHeight <= pindexSyncCheckpoint->nHeight)
        {
            // Received an older checkpoint, trace back from current checkpoint
            // to the same height of the received checkpoint to verify
            // that current checkpoint should be a descendant block
            CBlockIndex* pindex = pindexSyncCheckpoint;
            while (pindex->nHeight > pindexCheckpointRecv->nHeight)
                if (!(pindex = pindex->pprev))
                    return error("ValidateSyncCheckpoint: pprev null - block index structure failure");
            if (pindex->GetBlockHash() != hashCheckpoint)
            {
                hashInvalidCheckpoint = hashCheckpoint;
                return error("ValidateSyncCheckpoint: new sync-checkpoint %s is conflicting with current sync-checkpoint %s", hashCheckpoint.ToString().c_str(), hashSyncCheckpoint.ToString().c_str());
            }
            return false; // ignore older checkpoint
        }

        // Received checkpoint should be a descendant block of the current
        // checkpoint. Trace back to the same height of current checkpoint
        // to verify.
        CBlockIndex* pindex = pindexCheckpointRecv;
        while (pindex->nHeight > pindexSyncCheckpoint->nHeight)
            if (!(pindex = pindex->pprev))
                return error("ValidateSyncCheckpoint: pprev2 null - block index structure failure");
        if (pindex->GetBlockHash() != hashSyncCheckpoint)
        {
            hashInvalidCheckpoint = hashCheckpoint;
            return error("ValidateSyncCheckpoint: new sync-checkpoint %s is not a descendant of current sync-checkpoint %s", hashCheckpoint.ToString().c_str(), hashSyncCheckpoint.ToString().c_str());
        }
        return true;
    }

    bool WriteSyncCheckpoint(const uint256& hashCheckpoint)
    {
        CTxDB txdb;
        txdb.TxnBegin();
        if (!txdb.WriteSyncCheckpoint(hashCheckpoint))
        {
            txdb.TxnAbort();
            return error("WriteSyncCheckpoint(): failed to write to db sync checkpoint %s", hashCheckpoint.ToString().c_str());
        }
        if (!txdb.TxnCommit())
            return error("WriteSyncCheckpoint(): failed to commit to db sync checkpoint %s", hashCheckpoint.ToString().c_str());

        Checkpoints::hashSyncCheckpoint = hashCheckpoint;
        return true;
    }

    bool AcceptPendingSyncCheckpoint()
    {
        LOCK(cs_hashSyncCheckpoint);
        if (hashPendingCheckpoint != 0 && mapBlockIndex.count(hashPendingCheckpoint))
        {
            if (!ValidateSyncCheckpoint(hashPendingCheckpoint))
            {
                hashPendingCheckpoint = 0;
                checkpointMessagePending.SetNull();
                return false;
            }

            CTxDB txdb;
            CBlockIndex* pindexCheckpoint = mapBlockIndex[hashPendingCheckpoint];
            if (!pindexCheckpoint->IsInMainChain())
            {
                CBlock block;
                if (!block.ReadFromDisk(pindexCheckpoint))
                    return error("AcceptPendingSyncCheckpoint: ReadFromDisk failed for sync checkpoint %s", hashPendingCheckpoint.ToString().c_str());
                if (!block.SetBestChain(txdb, pindexCheckpoint))
                {
                    hashInvalidCheckpoint = hashPendingCheckpoint;
                    return error("AcceptPendingSyncCheckpoint: SetBestChain failed for sync checkpoint %s", hashPendingCheckpoint.ToString().c_str());
                }
            }

            if (!WriteSyncCheckpoint(hashPendingCheckpoint))
                return error("AcceptPendingSyncCheckpoint(): failed to write sync checkpoint %s", hashPendingCheckpoint.ToString().c_str());
            hashPendingCheckpoint = 0;
            checkpointMessage = checkpointMessagePending;
            checkpointMessagePending.SetNull();
            printf("AcceptPendingSyncCheckpoint : sync-checkpoint at %s\n", hashSyncCheckpoint.ToString().c_str());
            // relay the checkpoint
            if (!checkpointMessage.IsNull())
            {
                BOOST_FOREACH(CNode* pnode, vNodes)
                    checkpointMessage.RelayTo(pnode);
            }
            return true;
        }
        return false;
    }

    // Automatically select a suitable sync-checkpoint
    uint256 AutoSelectSyncCheckpoint()
    {
        const CBlockIndex *pindex = pindexBest;
        // Search backward for a block within max span and maturity window
        while (pindex->pprev && (pindex->GetBlockTime() + nCheckpointSpan * nTargetSpacing > pindexBest->GetBlockTime() || pindex->nHeight + nCheckpointSpan > pindexBest->nHeight))
            pindex = pindex->pprev;
        return pindex->GetBlockHash();
    }

    // Check against synchronized checkpoint
    bool CheckSync(const uint256& hashBlock, const CBlockIndex* pindexPrev)
    {
        if (TestNet()) return true; // Testnet has no checkpoints
        int nHeight = pindexPrev->nHeight + 1;

        LOCK(cs_hashSyncCheckpoint);
        // sync-checkpoint should always be accepted block
        assert(mapBlockIndex.count(hashSyncCheckpoint));
        const CBlockIndex* pindexSync = mapBlockIndex[hashSyncCheckpoint];

        if (nHeight > pindexSync->nHeight)
        {
            // trace back to same height as sync-checkpoint
            const CBlockIndex* pindex = pindexPrev;
            while (pindex->nHeight > pindexSync->nHeight)
                if (!(pindex = pindex->pprev))
                    return error("CheckSync: pprev null - block index structure failure");
            if (pindex->nHeight < pindexSync->nHeight || pindex->GetBlockHash() != hashSyncCheckpoint)
                return false; // only descendant of sync-checkpoint can pass check
        }
        if (nHeight == pindexSync->nHeight && hashBlock != hashSyncCheckpoint)
            return false; // same height with sync-checkpoint
        if (nHeight < pindexSync->nHeight && !mapBlockIndex.count(hashBlock))
            return false; // lower height than sync-checkpoint
        return true;
    }

    bool WantedByPendingSyncCheckpoint(uint256 hashBlock)
    {
        LOCK(cs_hashSyncCheckpoint);
        if (hashPendingCheckpoint == 0)
            return false;
        if (hashBlock == hashPendingCheckpoint)
            return true;
        if (mapOrphanBlocks.count(hashPendingCheckpoint)
            && hashBlock == WantedByOrphan(mapOrphanBlocks[hashPendingCheckpoint]))
            return true;
        return false;
    }

    // ppcoin: reset synchronized checkpoint to last hardened checkpoint
    bool ResetSyncCheckpoint()
    {
        LOCK(cs_hashSyncCheckpoint);
        const uint256& hash = mapCheckpoints.rbegin()->second;
        if (mapBlockIndex.count(hash) && !mapBlockIndex[hash]->IsInMainChain())
        {
            // checkpoint block accepted but not yet in main chain
            printf("ResetSyncCheckpoint: SetBestChain to hardened checkpoint %s\n", hash.ToString().c_str());
            CTxDB txdb;
            CBlock block;
            if (!block.ReadFromDisk(mapBlockIndex[hash]))
                return error("ResetSyncCheckpoint: ReadFromDisk failed for hardened checkpoint %s", hash.ToString().c_str());
            if (!block.SetBestChain(txdb, mapBlockIndex[hash]))
            {
                return error("ResetSyncCheckpoint: SetBestChain failed for hardened checkpoint %s", hash.ToString().c_str());
            }
        }
        else if(!mapBlockIndex.count(hash))
        {
            // checkpoint block not yet accepted
            hashPendingCheckpoint = hash;
            checkpointMessagePending.SetNull();
            printf("ResetSyncCheckpoint: pending for sync-checkpoint %s\n", hashPendingCheckpoint.ToString().c_str());
        }

        BOOST_REVERSE_FOREACH(const MapCheckpoints::value_type& i, mapCheckpoints)
        {
            const uint256& hash = i.second;
            if (mapBlockIndex.count(hash) && mapBlockIndex[hash]->IsInMainChain())
            {
                if (!WriteSyncCheckpoint(hash))
                    return error("ResetSyncCheckpoint: failed to write sync checkpoint %s", hash.ToString().c_str());
                printf("ResetSyncCheckpoint: sync-checkpoint reset to %s\n", hashSyncCheckpoint.ToString().c_str());
                return true;
            }
        }

        return false;
    }

    void AskForPendingSyncCheckpoint(CNode* pfrom)
    {
        LOCK(cs_hashSyncCheckpoint);
        if (pfrom && hashPendingCheckpoint != 0 && (!mapBlockIndex.count(hashPendingCheckpoint)) && (!mapOrphanBlocks.count(hashPendingCheckpoint)))
            pfrom->AskFor(CInv(MSG_BLOCK, hashPendingCheckpoint));
    }

    bool SetCheckpointPrivKey(std::string strPrivKey)
    {
        // Test signing a sync-checkpoint with genesis block
        CSyncCheckpoint checkpoint;
        // checkpoint.hashCheckpoint = (!fTestNet ? hashGenesisBlock : hashGenesisBlockTestNet);
        checkpoint.hashCheckpoint = Params().HashGenesisBlock();
        CDataStream sMsg(SER_NETWORK, PROTOCOL_VERSION);
        sMsg << (CUnsignedSyncCheckpoint)checkpoint;
        checkpoint.vchMsg = std::vector<unsigned char>(sMsg.begin(), sMsg.end());

        std::vector<unsigned char> vchPrivKey = ParseHex(strPrivKey);
        CKey key;
        key.SetPrivKey(CPrivKey(vchPrivKey.begin(), vchPrivKey.end()), false); // if key is not correct openssl may crash
        if (!key.Sign(Hash(checkpoint.vchMsg.begin(), checkpoint.vchMsg.end()), checkpoint.vchSig))
            return false;

        // Test signing successful, proceed
        CSyncCheckpoint::strMasterPrivKey = strPrivKey;
        return true;
    }

    bool SendSyncCheckpoint(uint256 hashCheckpoint)
    {
        CSyncCheckpoint checkpoint;
        checkpoint.hashCheckpoint = hashCheckpoint;
        CDataStream sMsg(SER_NETWORK, PROTOCOL_VERSION);
        sMsg << (CUnsignedSyncCheckpoint)checkpoint;
        checkpoint.vchMsg = std::vector<unsigned char>(sMsg.begin(), sMsg.end());

        if (CSyncCheckpoint::strMasterPrivKey.empty())
            return error("SendSyncCheckpoint: Checkpoint master key unavailable.");
        std::vector<unsigned char> vchPrivKey = ParseHex(CSyncCheckpoint::strMasterPrivKey);
        CKey key;
        key.SetPrivKey(CPrivKey(vchPrivKey.begin(), vchPrivKey.end()), false); // if key is not correct openssl may crash
        if (!key.Sign(Hash(checkpoint.vchMsg.begin(), checkpoint.vchMsg.end()), checkpoint.vchSig))
            return error("SendSyncCheckpoint: Unable to sign checkpoint, check private key?");

        if(!checkpoint.ProcessSyncCheckpoint(NULL))
        {
            printf("WARNING: SendSyncCheckpoint: Failed to process checkpoint.\n");
            return false;
        }

        // Relay checkpoint
        {
            LOCK(cs_vNodes);
            BOOST_FOREACH(CNode* pnode, vNodes)
                checkpoint.RelayTo(pnode);
        }
        return true;
    }

    // Is the sync-checkpoint outside maturity window?
    bool IsMatureSyncCheckpoint()
    {
        LOCK(cs_hashSyncCheckpoint);
        // sync-checkpoint should always be accepted block
        assert(mapBlockIndex.count(hashSyncCheckpoint));
        const CBlockIndex* pindexSync = mapBlockIndex[hashSyncCheckpoint];
        return (nBestHeight >= pindexSync->nHeight + nCoinbaseMaturity ||
                pindexSync->GetBlockTime() + nStakeMinAge < GetAdjustedTime());
    }
}

// flapx: sync-checkpoint master key
const std::string CSyncCheckpoint::strMasterPubKey = "046b2e58c8c0502bea592b7ec55f5acb8781a2764520f346642a5c2a6e8ef8b00704752e825d9e739daac343fae5027945e018a15e456e98ef2d0b507fbd9a6d13";

std::string CSyncCheckpoint::strMasterPrivKey = "";

bool CSyncCheckpoint::RelayTo(CNode* pnode) const
{
    // don't relay to nodes which haven't sent their version message
    if (pnode->nVersion == 0)
        return false;
    // returns true if wasn't already sent
    if (pnode->hashCheckpointKnown != hashCheckpoint)
    {
        pnode->hashCheckpointKnown = hashCheckpoint;
        pnode->PushMessage("checkpoint", *this);
        return true;
    }
    return false;
}

// ppcoin: verify signature of sync-checkpoint message
bool CSyncCheckpoint::CheckSignature()
{
    // CKey key;
    // if (!key.SetPubKey(ParseHex(CSyncCheckpoint::strMasterPubKey)))
    //     return error("CSyncCheckpoint::CheckSignature() : SetPubKey failed");
    CPubKey key(ParseHex(CSyncCheckpoint::strMasterPubKey));
    if (!key.Verify(Hash(vchMsg.begin(), vchMsg.end()), vchSig))
        return error("CSyncCheckpoint::CheckSignature() : verify signature failed");

    // Now unserialize the data
    CDataStream sMsg(vchMsg, SER_NETWORK, PROTOCOL_VERSION);
    sMsg >> *(CUnsignedSyncCheckpoint*)this;
    return true;
}

// ppcoin: process synchronized checkpoint
bool CSyncCheckpoint::ProcessSyncCheckpoint(CNode* pfrom)
{
    if (!CheckSignature())
        return false;

    LOCK(Checkpoints::cs_hashSyncCheckpoint);
    if (!mapBlockIndex.count(hashCheckpoint))
    {
        // We haven't received the checkpoint chain, keep the checkpoint as pending
        Checkpoints::hashPendingCheckpoint = hashCheckpoint;
        Checkpoints::checkpointMessagePending = *this;
        printf("ProcessSyncCheckpoint: pending for sync-checkpoint %s\n", hashCheckpoint.ToString().c_str());
        // Ask this guy to fill in what we're missing
        if (pfrom)
        {
            // pfrom->PushGetBlocks(pindexBest, hashCheckpoint);
            PushGetBlocks(pfrom, pindexBest, hashCheckpoint);
            // ask directly as well in case rejected earlier by duplicate
            // proof-of-stake because getblocks may not get it this time
            pfrom->AskFor(CInv(MSG_BLOCK, mapOrphanBlocks.count(hashCheckpoint)? WantedByOrphan(mapOrphanBlocks[hashCheckpoint]) : hashCheckpoint));
        }
        return false;
    }

    if (!Checkpoints::ValidateSyncCheckpoint(hashCheckpoint))
        return false;

    CTxDB txdb;
    CBlockIndex* pindexCheckpoint = mapBlockIndex[hashCheckpoint];
    if (!pindexCheckpoint->IsInMainChain())
    {
        // checkpoint chain received but not yet main chain
        CBlock block;
        if (!block.ReadFromDisk(pindexCheckpoint))
            return error("ProcessSyncCheckpoint: ReadFromDisk failed for sync checkpoint %s", hashCheckpoint.ToString().c_str());
        if (!block.SetBestChain(txdb, pindexCheckpoint))
        {
            Checkpoints::hashInvalidCheckpoint = hashCheckpoint;
            return error("ProcessSyncCheckpoint: SetBestChain failed for sync checkpoint %s", hashCheckpoint.ToString().c_str());
        }
    }

    if (!Checkpoints::WriteSyncCheckpoint(hashCheckpoint))
        return error("ProcessSyncCheckpoint(): failed to write sync checkpoint %s", hashCheckpoint.ToString().c_str());
    Checkpoints::checkpointMessage = *this;
    Checkpoints::hashPendingCheckpoint = 0;
    Checkpoints::checkpointMessagePending.SetNull();
    printf("ProcessSyncCheckpoint: sync-checkpoint at %s\n", hashCheckpoint.ToString().c_str());
    return true;
}
