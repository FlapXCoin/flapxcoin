// Copyright (c) 2012-2013 The PPCoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/assign/list_of.hpp>

#include "kernel.h"
#include "txdb.h"

using namespace std;

typedef std::map<int, unsigned int> MapModifierCheckpoints;

// Hard checkpoints of stake modifiers to ensure they are deterministic
// FlapX 2.9.1.0: every 25,000 blocks from genesis to 4,425,000, read from a
// fully synced node (36 matched the earlier checkpoints at the same heights)
static std::map<int, unsigned int> mapStakeModifierCheckpoints =
    boost::assign::map_list_of
        (       0, 0x0e00670bu )
        (   25000, 0x4c161e33u )
        (   50000, 0x9dac1e1fu )
        (   75000, 0xfe33ccefu )
        (  100000, 0x229e2da4u )
        (  125000, 0xbab2547du )
        (  150000, 0xaeb52298u )
        (  175000, 0xdfb2912au )
        (  200000, 0xc213541bu )
        (  225000, 0x04c22f78u )
        (  250000, 0xf514aeb7u )
        (  275000, 0xd7ffbc38u )
        (  300000, 0xab1bc212u )
        (  325000, 0x2b987538u )
        (  350000, 0x260da8e2u )
        (  375000, 0xdf9d9196u )
        (  400000, 0xc94afa10u )
        (  425000, 0x5dcc4c11u )
        (  450000, 0x4b3c9c9cu )
        (  475000, 0x8422b879u )
        (  500000, 0x4e58bf17u )
        (  525000, 0x85fde74fu )
        (  550000, 0x6ed062aeu )
        (  575000, 0xcf9c1caau )
        (  600000, 0xeae8c57du )
        (  625000, 0x6aa313f5u )
        (  650000, 0x8d32fcf5u )
        (  675000, 0xa6c91770u )
        (  700000, 0x4f3491eeu )
        (  725000, 0x484834e4u )
        (  750000, 0x61772732u )
        (  775000, 0x4302310bu )
        (  800000, 0xd10f5e28u )
        (  825000, 0x092cc8a0u )
        (  850000, 0xe6616f44u )
        (  875000, 0xa8cd31f2u )
        (  900000, 0xbb4a4846u )
        (  925000, 0xfdfb6cdeu )
        (  950000, 0x7bb9c765u )
        (  975000, 0x138061c9u )
        ( 1000000, 0x4a513bb6u )
        ( 1025000, 0x9d9011fcu )
        ( 1050000, 0xe1d87ff4u )
        ( 1075000, 0x0250dc8du )
        ( 1100000, 0x98c5dcc4u )
        ( 1125000, 0x5cdf76ddu )
        ( 1150000, 0xf99cd73eu )
        ( 1175000, 0x078ef7cfu )
        ( 1200000, 0x423d0b70u )
        ( 1225000, 0xa3d8be06u )
        ( 1250000, 0x1f61f469u )
        ( 1275000, 0x9fe703fcu )
        ( 1300000, 0x67b7ccd8u )
        ( 1325000, 0x751b69bbu )
        ( 1350000, 0x7ef050f2u )
        ( 1375000, 0x6814123fu )
        ( 1400000, 0xf90b881au )
        ( 1425000, 0x56b73227u )
        ( 1450000, 0x7fd319a1u )
        ( 1475000, 0x6cdf7e8au )
        ( 1500000, 0x41fed79bu )
        ( 1525000, 0xc5b3b228u )
        ( 1550000, 0x982f90c9u )
        ( 1575000, 0x674721b6u )
        ( 1600000, 0x9cc3d94cu )
        ( 1625000, 0xfab67dc1u )
        ( 1650000, 0xd3827c10u )
        ( 1675000, 0x494ed561u )
        ( 1700000, 0x4652e28au )
        ( 1725000, 0x98949d85u )
        ( 1750000, 0x864c07c4u )
        ( 1775000, 0x40b8b7dfu )
        ( 1800000, 0x66512f1fu )
        ( 1825000, 0xa2c6ab5cu )
        ( 1850000, 0x7cee79ddu )
        ( 1875000, 0xf734c91eu )
        ( 1900000, 0x5aa93283u )
        ( 1925000, 0x41b83d90u )
        ( 1950000, 0x71486dadu )
        ( 1975000, 0xa05bc2abu )
        ( 2000000, 0xfae43bccu )
        ( 2025000, 0xc104739au )
        ( 2050000, 0xceef6901u )
        ( 2075000, 0x5b54a43cu )
        ( 2100000, 0xf2139574u )
        ( 2125000, 0xd8044b1fu )
        ( 2150000, 0xd67df19eu )
        ( 2175000, 0x1dc506afu )
        ( 2200000, 0x47668c29u )
        ( 2225000, 0xdcc678a3u )
        ( 2250000, 0x48f9e1e8u )
        ( 2275000, 0x4954ed83u )
        ( 2300000, 0x2d1a5ca7u )
        ( 2325000, 0x34124188u )
        ( 2350000, 0x4643e0eeu )
        ( 2375000, 0xe351ac84u )
        ( 2400000, 0xd20936afu )
        ( 2425000, 0x04d70d67u )
        ( 2450000, 0x0a7ba25fu )
        ( 2475000, 0x969a2877u )
        ( 2500000, 0x19102d2cu )
        ( 2525000, 0xf2bcc765u )
        ( 2550000, 0xec76d550u )
        ( 2575000, 0xeb16f56cu )
        ( 2600000, 0xf0d5c829u )
        ( 2625000, 0xab69a64au )
        ( 2650000, 0x75ec3264u )
        ( 2675000, 0x62360eb8u )
        ( 2700000, 0x7eebd0e3u )
        ( 2725000, 0x0682a2c5u )
        ( 2750000, 0x32d02062u )
        ( 2775000, 0xa3332104u )
        ( 2800000, 0xf352c5b7u )
        ( 2825000, 0xe52f53a6u )
        ( 2850000, 0x0b30b203u )
        ( 2875000, 0x7d7e1997u )
        ( 2900000, 0xef34b842u )
        ( 2925000, 0x70475c0bu )
        ( 2950000, 0x5cea9c68u )
        ( 2975000, 0x95c16a51u )
        ( 3000000, 0x2d9df060u )
        ( 3025000, 0xb743d6b8u )
        ( 3050000, 0xb2dba8e9u )
        ( 3075000, 0xf194632fu )
        ( 3100000, 0x9176daf5u )
        ( 3125000, 0x929c98ccu )
        ( 3150000, 0x49899497u )
        ( 3175000, 0x21c84c4eu )
        ( 3200000, 0x318e68b4u )
        ( 3225000, 0xd05c7c7eu )
        ( 3250000, 0x7d45c64du )
        ( 3275000, 0x9e244a0au )
        ( 3300000, 0x0e1e36c6u )
        ( 3325000, 0x084f4d3bu )
        ( 3350000, 0x6615b213u )
        ( 3375000, 0xf05d3780u )
        ( 3400000, 0xec2a56b3u )
        ( 3425000, 0xaaa8002bu )
        ( 3450000, 0x87284977u )
        ( 3475000, 0x8837f97du )
        ( 3500000, 0xfa0db801u )
        ( 3525000, 0xd5bc3339u )
        ( 3550000, 0x654092a0u )
        ( 3575000, 0xb1a1fe76u )
        ( 3600000, 0xd2d79586u )
        ( 3625000, 0x56dbdcffu )
        ( 3650000, 0x3dd31025u )
        ( 3675000, 0xb27b1768u )
        ( 3700000, 0x2926eac1u )
        ( 3725000, 0xb7b1b8fcu )
        ( 3750000, 0x15f961efu )
        ( 3775000, 0x4a928522u )
        ( 3800000, 0xb9689c71u )
        ( 3825000, 0x0f31e9bbu )
        ( 3850000, 0x84fd64b7u )
        ( 3875000, 0x67909266u )
        ( 3900000, 0xf20635beu )
        ( 3925000, 0xf1828c86u )
        ( 3950000, 0x2d2ad5b3u )
        ( 3975000, 0x883de0bbu )
        ( 4000000, 0x41cb4e69u )
        ( 4025000, 0x71f0d1f5u )
        ( 4050000, 0x5216bd4eu )
        ( 4075000, 0x04c506c1u )
        ( 4100000, 0x139ac794u )
        ( 4125000, 0xed10d199u )
        ( 4150000, 0x20e098fcu )
        ( 4175000, 0x9e9e4ca4u )
        ( 4200000, 0x10ab1c66u )
        ( 4225000, 0xae8b64f0u )
        ( 4250000, 0xef11eeabu )
        ( 4275000, 0x5dba5d57u )
        ( 4300000, 0xbe51af4du )
        ( 4325000, 0x97103a94u )
        ( 4350000, 0x084ef31cu )
        ( 4375000, 0xc076d16eu )
        ( 4400000, 0x1b3d50ffu )
        ( 4425000, 0x247aaf19u )
    ;

// Hard checkpoints of stake modifiers to ensure they are deterministic (testNet)
static std::map<int, unsigned int> mapStakeModifierCheckpointsTestNet =
    boost::assign::map_list_of
        ( 0, 0xfd11f4e7u ) // testnet genesis re-mined with nNonce 522177
    ;

// Get time weight
int64_t GetWeight(int64_t nIntervalBeginning, int64_t nIntervalEnd)
{
    // Kernel hash weight starts from 0 at the min age
    // this change increases active coins participating the hash and helps
    // to secure the network when proof-of-stake difficulty is low

    return min(nIntervalEnd - nIntervalBeginning - nStakeMinAge, (int64_t)nStakeMaxAge);
}

// Get the last stake modifier and its generation time from a given block
static bool GetLastStakeModifier(const CBlockIndex* pindex, uint64_t& nStakeModifier, int64_t& nModifierTime)
{
    if (!pindex)
        return error("GetLastStakeModifier: null pindex");
    while (pindex && pindex->pprev && !pindex->GeneratedStakeModifier())
        pindex = pindex->pprev;
    if (!pindex->GeneratedStakeModifier())
        return error("GetLastStakeModifier: no generation at genesis block");
    nStakeModifier = pindex->nStakeModifier;
    nModifierTime = pindex->GetBlockTime();
    return true;
}

// Get selection interval section (in seconds)
static int64_t GetStakeModifierSelectionIntervalSection(int nSection)
{
    assert (nSection >= 0 && nSection < 64);
    return (nModifierInterval * 63 / (63 + ((63 - nSection) * (MODIFIER_INTERVAL_RATIO - 1))));
}

// Get stake modifier selection interval (in seconds)
static int64_t GetStakeModifierSelectionInterval()
{
    int64_t nSelectionInterval = 0;
    for (int nSection=0; nSection<64; nSection++)
        nSelectionInterval += GetStakeModifierSelectionIntervalSection(nSection);
    return nSelectionInterval;
}

// select a block from the candidate blocks in vSortedByTimestamp, excluding
// already selected blocks in vSelectedBlocks, and with timestamp up to
// nSelectionIntervalStop.
static bool SelectBlockFromCandidates(vector<pair<int64_t, uint256> >& vSortedByTimestamp, map<uint256, const CBlockIndex*>& mapSelectedBlocks,
    int64_t nSelectionIntervalStop, uint64_t nStakeModifierPrev, const CBlockIndex** pindexSelected)
{
    bool fSelected = false;
    uint256 hashBest = 0;
    *pindexSelected = (const CBlockIndex*) 0;
    BOOST_FOREACH(const PAIRTYPE(int64_t, uint256)& item, vSortedByTimestamp)
    {
        if (!mapBlockIndex.count(item.second))
            return error("SelectBlockFromCandidates: failed to find block index for candidate block %s", item.second.ToString().c_str());
        const CBlockIndex* pindex = mapBlockIndex[item.second];
        if (fSelected && pindex->GetBlockTime() > nSelectionIntervalStop)
            break;
        if (mapSelectedBlocks.count(pindex->GetBlockHash()) > 0)
            continue;
        // compute the selection hash by hashing its proof-hash and the
        // previous proof-of-stake modifier
        CDataStream ss(SER_GETHASH, 0);
        ss << pindex->hashProof << nStakeModifierPrev;
        uint256 hashSelection = Hash(ss.begin(), ss.end());
        // the selection hash is divided by 2**32 so that proof-of-stake block
        // is always favored over proof-of-work block. this is to preserve
        // the energy efficiency property
        if (pindex->IsProofOfStake())
            hashSelection >>= 32;
        if (fSelected && hashSelection < hashBest)
        {
            hashBest = hashSelection;
            *pindexSelected = (const CBlockIndex*) pindex;
        }
        else if (!fSelected)
        {
            fSelected = true;
            hashBest = hashSelection;
            *pindexSelected = (const CBlockIndex*) pindex;
        }
    }
    if (fDebug && GetBoolArg("-printstakemodifier"))
        printf("SelectBlockFromCandidates: selection hash=%s\n", hashBest.ToString().c_str());
    return fSelected;
}

// Stake Modifier (hash modifier of proof-of-stake):
// The purpose of stake modifier is to prevent a txout (coin) owner from
// computing future proof-of-stake generated by this txout at the time
// of transaction confirmation. To meet kernel protocol, the txout
// must hash with a future stake modifier to generate the proof.
// Stake modifier consists of bits each of which is contributed from a
// selected block of a given block group in the past.
// The selection of a block is based on a hash of the block's proof-hash and
// the previous stake modifier.
// Stake modifier is recomputed at a fixed time interval instead of every 
// block. This is to make it difficult for an attacker to gain control of
// additional bits in the stake modifier, even after generating a chain of
// blocks.
bool ComputeNextStakeModifier(const CBlockIndex* pindexPrev, uint64_t& nStakeModifier, bool& fGeneratedStakeModifier)
{
    nStakeModifier = 0;
    fGeneratedStakeModifier = false;
    if (!pindexPrev)
    {
        fGeneratedStakeModifier = true;
        return true;  // genesis block's modifier is 0
    }
    // First find current stake modifier and its generation block time
    // if it's not old enough, return the same stake modifier
    int64_t nModifierTime = 0;
    if (!GetLastStakeModifier(pindexPrev, nStakeModifier, nModifierTime))
        return error("ComputeNextStakeModifier: unable to get last modifier");
    if (fDebug)
    {
        printf("ComputeNextStakeModifier: prev modifier=0x%016" PRIx64 " time=%s\n", nStakeModifier, DateTimeStrFormat(nModifierTime).c_str());
    }
    if (nModifierTime / nModifierInterval >= pindexPrev->GetBlockTime() / nModifierInterval)
        return true;

    // Sort candidate blocks by timestamp
    vector<pair<int64_t, uint256> > vSortedByTimestamp;
    vSortedByTimestamp.reserve(64 * nModifierInterval / nTargetSpacing);
    int64_t nSelectionInterval = GetStakeModifierSelectionInterval();
    int64_t nSelectionIntervalStart = (pindexPrev->GetBlockTime() / nModifierInterval) * nModifierInterval - nSelectionInterval;
    const CBlockIndex* pindex = pindexPrev;
    while (pindex && pindex->GetBlockTime() >= nSelectionIntervalStart)
    {
        vSortedByTimestamp.push_back(make_pair(pindex->GetBlockTime(), pindex->GetBlockHash()));
        pindex = pindex->pprev;
    }
    int nHeightFirstCandidate = pindex ? (pindex->nHeight + 1) : 0;
    reverse(vSortedByTimestamp.begin(), vSortedByTimestamp.end());
    sort(vSortedByTimestamp.begin(), vSortedByTimestamp.end());

    // Select 64 blocks from candidate blocks to generate stake modifier
    uint64_t nStakeModifierNew = 0;
    int64_t nSelectionIntervalStop = nSelectionIntervalStart;
    map<uint256, const CBlockIndex*> mapSelectedBlocks;
    for (int nRound=0; nRound<min(64, (int)vSortedByTimestamp.size()); nRound++)
    {
        // add an interval section to the current selection round
        nSelectionIntervalStop += GetStakeModifierSelectionIntervalSection(nRound);
        // select a block from the candidates of current round
        if (!SelectBlockFromCandidates(vSortedByTimestamp, mapSelectedBlocks, nSelectionIntervalStop, nStakeModifier, &pindex))
            return error("ComputeNextStakeModifier: unable to select block at round %d", nRound);
        // write the entropy bit of the selected block
        nStakeModifierNew |= (((uint64_t)pindex->GetStakeEntropyBit()) << nRound);
        // add the selected block from candidates to selected list
        mapSelectedBlocks.insert(make_pair(pindex->GetBlockHash(), pindex));
        if (fDebug && GetBoolArg("-printstakemodifier"))
            printf("ComputeNextStakeModifier: selected round %d stop=%s height=%d bit=%d\n", nRound, DateTimeStrFormat(nSelectionIntervalStop).c_str(), pindex->nHeight, pindex->GetStakeEntropyBit());
    }

    // Print selection map for visualization of the selected blocks
    if (fDebug && GetBoolArg("-printstakemodifier"))
    {
        string strSelectionMap = "";
        // '-' indicates proof-of-work blocks not selected
        strSelectionMap.insert(0, pindexPrev->nHeight - nHeightFirstCandidate + 1, '-');
        pindex = pindexPrev;
        while (pindex && pindex->nHeight >= nHeightFirstCandidate)
        {
            // '=' indicates proof-of-stake blocks not selected
            if (pindex->IsProofOfStake())
                strSelectionMap.replace(pindex->nHeight - nHeightFirstCandidate, 1, "=");
            pindex = pindex->pprev;
        }
        BOOST_FOREACH(const PAIRTYPE(uint256, const CBlockIndex*)& item, mapSelectedBlocks)
        {
            // 'S' indicates selected proof-of-stake blocks
            // 'W' indicates selected proof-of-work blocks
            strSelectionMap.replace(item.second->nHeight - nHeightFirstCandidate, 1, item.second->IsProofOfStake()? "S" : "W");
        }
        printf("ComputeNextStakeModifier: selection height [%d, %d] map %s\n", nHeightFirstCandidate, pindexPrev->nHeight, strSelectionMap.c_str());
    }
    if (fDebug)
    {
        printf("ComputeNextStakeModifier: new modifier=0x%016" PRIx64 " time=%s\n", nStakeModifierNew, DateTimeStrFormat(pindexPrev->GetBlockTime()).c_str());
    }

    nStakeModifier = nStakeModifierNew;
    fGeneratedStakeModifier = true;
    return true;
}

// The stake modifier used to hash for a stake kernel is chosen as the stake
// modifier about a selection interval later than the coin generating the kernel
static bool GetKernelStakeModifier(uint256 hashBlockFrom, uint64_t& nStakeModifier, int& nStakeModifierHeight, int64_t& nStakeModifierTime, bool fPrintProofOfStake)
{
    nStakeModifier = 0;
    if (!mapBlockIndex.count(hashBlockFrom))
        return error("GetKernelStakeModifier() : block not indexed");
    const CBlockIndex* pindexFrom = mapBlockIndex[hashBlockFrom];
    nStakeModifierHeight = pindexFrom->nHeight;
    nStakeModifierTime = pindexFrom->GetBlockTime();
    int64_t nStakeModifierSelectionInterval = GetStakeModifierSelectionInterval();
    const CBlockIndex* pindex = pindexFrom;
    // loop to find the stake modifier later by a selection interval
    while (nStakeModifierTime < pindexFrom->GetBlockTime() + nStakeModifierSelectionInterval)
    {
        if (!pindex->pnext)
        {   // reached best block; may happen if node is behind on block chain
            if (fPrintProofOfStake || (pindex->GetBlockTime() + nStakeMinAge - nStakeModifierSelectionInterval > GetAdjustedTime()))
                return error("GetKernelStakeModifier() : reached best block %s at height %d from block %s",
                    pindex->GetBlockHash().ToString().c_str(), pindex->nHeight, hashBlockFrom.ToString().c_str());
            else
                return false;
        }
        pindex = pindex->pnext;
        if (pindex->GeneratedStakeModifier())
        {
            nStakeModifierHeight = pindex->nHeight;
            nStakeModifierTime = pindex->GetBlockTime();
        }
    }
    nStakeModifier = pindex->nStakeModifier;
    return true;
}

// ppcoin kernel protocol
// coinstake must meet hash target according to the protocol:
// kernel (input 0) must meet the formula
//     hash(nStakeModifier + txPrev.block.nTime + txPrev.offset + txPrev.nTime + txPrev.vout.n + nTime) < bnTarget * nCoinDayWeight
// this ensures that the chance of getting a coinstake is proportional to the
// amount of coin age one owns.
// The reason this hash is chosen is the following:
//   nStakeModifier: scrambles computation to make it very difficult to precompute
//                  future proof-of-stake at the time of the coin's confirmation
//   txPrev.block.nTime: prevent nodes from guessing a good timestamp to
//                       generate transaction for future advantage
//   txPrev.offset: offset of txPrev inside block, to reduce the chance of 
//                  nodes generating coinstake at the same time
//   txPrev.nTime: reduce the chance of nodes generating coinstake at the same
//                 time
//   txPrev.vout.n: output number of txPrev, to reduce the chance of nodes
//                  generating coinstake at the same time
//   block/tx hash should not be used here as they can be generated in vast
//   quantities so as to generate blocks faster, degrading the system back into
//   a proof-of-work situation.
//
bool CheckStakeKernelHash(unsigned int nBits, const CBlock& blockFrom, unsigned int nTxPrevOffset, const CTransaction& txPrev, const COutPoint& prevout, unsigned int nTimeTx, uint256& hashProofOfStake, uint256& targetProofOfStake, bool fPrintProofOfStake)
{
    unsigned int nTimeBlockFrom = blockFrom.GetBlockTime();
    if (nTimeBlockFrom + nStakeMinAge > nTimeTx) // Min age requirement
        return error("CheckStakeKernelHash() : min age violation");

    CBigNum bnTargetPerCoinDay;
    bnTargetPerCoinDay.SetCompact(nBits);
    int64_t nValueIn = txPrev.vout[prevout.n].nValue;

    uint256 hashBlockFrom = blockFrom.GetHash();

    CBigNum bnCoinDayWeight = CBigNum(nValueIn) * GetWeight((int64_t)nTimeBlockFrom, (int64_t)nTimeTx) / COIN / (24 * 60 * 60);
    targetProofOfStake = (bnCoinDayWeight * bnTargetPerCoinDay).getuint256();

    // Calculate hash
    CDataStream ss(SER_GETHASH, 0);
    uint64_t nStakeModifier = 0;
    int nStakeModifierHeight = 0;
    int64_t nStakeModifierTime = 0;

    if (!GetKernelStakeModifier(hashBlockFrom, nStakeModifier, nStakeModifierHeight, nStakeModifierTime, fPrintProofOfStake))
        return false;
    ss << nStakeModifier;

    ss << nTimeBlockFrom << nTxPrevOffset << nTimeBlockFrom << prevout.n << nTimeTx;
    hashProofOfStake = Hash(ss.begin(), ss.end());
    if (fPrintProofOfStake)
    {
        printf("CheckStakeKernelHash() : using modifier 0x%016" PRIx64 " at height=%d timestamp=%s for block from height=%d timestamp=%s\n",
            nStakeModifier, nStakeModifierHeight,
            DateTimeStrFormat(nStakeModifierTime).c_str(),
            mapBlockIndex[hashBlockFrom]->nHeight,
            DateTimeStrFormat(blockFrom.GetBlockTime()).c_str());
        printf("CheckStakeKernelHash() : check modifier=0x%016" PRIx64 " nTimeBlockFrom=%u nTxPrevOffset=%u nTimeTxPrev=%u nPrevout=%u nTimeTx=%u hashProof=%s\n",
            nStakeModifier,
            nTimeBlockFrom, nTxPrevOffset, nTimeBlockFrom, prevout.n, nTimeTx,
            hashProofOfStake.ToString().c_str());
    }

    // Now check if proof-of-stake hash meets target protocol
    if (CBigNum(hashProofOfStake) > bnCoinDayWeight * bnTargetPerCoinDay)
        return false;
    if (fDebug && !fPrintProofOfStake)
    {
        printf("CheckStakeKernelHash() : using modifier 0x%016" PRIx64 " at height=%d timestamp=%s for block from height=%d timestamp=%s\n",
            nStakeModifier, nStakeModifierHeight, 
            DateTimeStrFormat(nStakeModifierTime).c_str(),
            mapBlockIndex[hashBlockFrom]->nHeight,
            DateTimeStrFormat(blockFrom.GetBlockTime()).c_str());
        printf("CheckStakeKernelHash() : pass modifier=0x%016" PRIx64 " nTimeBlockFrom=%u nTxPrevOffset=%u nTimeTxPrev=%u nPrevout=%u nTimeTx=%u hashProof=%s\n",
            nStakeModifier,
            nTimeBlockFrom, nTxPrevOffset, nTimeBlockFrom, prevout.n, nTimeTx,
            hashProofOfStake.ToString().c_str());
    }
    return true;
}

// Check kernel hash target and coinstake signature
bool CheckProofOfStake(const CTransaction& tx, unsigned int txTime, unsigned int nBits, uint256& hashProofOfStake, uint256& targetProofOfStake)
{
    if (!tx.IsCoinStake())
        return error("CheckProofOfStake() : called on non-coinstake %s", tx.GetHash().ToString().c_str());

    // Kernel (input 0) must match the stake hash target per coin age (nBits)
    const CTxIn& txin = tx.vin[0];

    // First try finding the previous transaction in database
    CTxDB txdb("r");
    CTransaction txPrev;
    CTxIndex txindex;
    if (!txPrev.ReadFromDisk(txdb, txin.prevout, txindex))
        return tx.DoS(1, error("CheckProofOfStake() : INFO: read txPrev failed"));  // previous transaction not in main chain, may occur during initial download

    // Verify signature
    if (!VerifySignature(txPrev, tx, 0, SCRIPT_VERIFY_NONE, 0))
        return tx.DoS(100, error("CheckProofOfStake() : VerifySignature failed on coinstake %s", tx.GetHash().ToString().c_str()));

    // Read block header
    CBlock block;
    if (!block.ReadFromDisk(txindex.pos.nFile, txindex.pos.nBlockPos, false))
        return fDebug? error("CheckProofOfStake() : read block failed") : false; // unable to read block of previous transaction

    if (!CheckStakeKernelHash(nBits, block, txindex.pos.nTxPos - txindex.pos.nBlockPos, txPrev, txin.prevout, txTime, hashProofOfStake, targetProofOfStake, fDebug))
        return tx.DoS(1, error("CheckProofOfStake() : INFO: check kernel failed on coinstake %s, hashProof=%s", tx.GetHash().ToString().c_str(), hashProofOfStake.ToString().c_str())); // may occur during initial download or if behind on block chain sync

    return true;
}

// Check whether the coinstake timestamp meets protocol
bool CheckCoinStakeTimestamp(int64_t nTimeBlock, int64_t nTimeTx)
{
    // v0.3 protocol
    return (nTimeBlock == nTimeTx);
}

// Get stake modifier checksum
unsigned int GetStakeModifierChecksum(const CBlockIndex* pindex)
{
    // assert (pindex->pprev || pindex->GetBlockHash() == (!fTestNet ? hashGenesisBlock : hashGenesisBlockTestNet));
    assert (pindex->pprev || pindex->GetBlockHash() == Params().HashGenesisBlock());
    // Hash previous checksum with flags, hashProofOfStake and nStakeModifier
    CDataStream ss(SER_GETHASH, 0);
    if (pindex->pprev)
        ss << pindex->pprev->nStakeModifierChecksum;
    ss << pindex->nFlags << (pindex->IsProofOfStake() ? pindex->hashProof : 0) << pindex->nStakeModifier;
    uint256 hashChecksum = Hash(ss.begin(), ss.end());
    hashChecksum >>= (256 - 32);
    return hashChecksum.GetLow64();
}

// Check stake modifier hard checkpoints
bool CheckStakeModifierCheckpoints(int nHeight, unsigned int nStakeModifierChecksum)
{
    MapModifierCheckpoints& checkpoints = (TestNet() ? mapStakeModifierCheckpointsTestNet : mapStakeModifierCheckpoints);

    if (checkpoints.count(nHeight))
        return nStakeModifierChecksum == checkpoints[nHeight];
    return true;
}
