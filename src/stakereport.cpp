// Copyright (c) 2014 Remy5 (original stake report)
// Copyright (c) 2026 FlapX Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "stakereport.h"
#include "wallet.h"
#include "main.h"

#include <time.h>

using namespace std;

// Local midnight at the start of today (portable: localtime_r isn't on Windows)
static int64_t LocalMidnight(int64_t nNow)
{
    time_t t = (time_t)nNow;
    struct tm tmLocal;
#ifdef WIN32
    localtime_s(&tmLocal, &t);
#else
    localtime_r(&t, &tmLocal);
#endif
    tmLocal.tm_hour = 0;
    tmLocal.tm_min = 0;
    tmLocal.tm_sec = 0;
    tmLocal.tm_isdst = -1;   // let mktime work out daylight saving
    return (int64_t)mktime(&tmLocal);
}

static StakePeriod MakePeriod(const string& strName, int64_t nStart, int64_t nEnd)
{
    StakePeriod p;
    p.strName = strName;
    p.nStart = nStart;
    p.nEnd = nEnd;
    p.nAmount = 0;
    p.nCount = 0;
    return p;
}

// Add up the wallet's stakes into the report's periods (caller holds
// cs_main and the wallet lock)
static void AddStakes(const CWallet* pwallet, StakeReport& report)
{
    for (map<uint256, CWalletTx>::const_iterator it = pwallet->mapWallet.begin(); it != pwallet->mapWallet.end(); ++it)
    {
        const CWalletTx& wtx = it->second;
        if (!wtx.IsCoinStake() || wtx.GetDepthInMainChain() <= 0)
            continue;   // not a stake, or not in the main chain (orphaned)

        // Net reward to this wallet: our stake outputs minus the coins we
        // staked. Worked out per output, because the wallet values an immature
        // stake at 0 and the newest stakes would go missing.
        int64_t nCredit = 0;
        BOOST_FOREACH(const CTxOut& txout, wtx.vout)
            nCredit += pwallet->GetCredit(txout);
        int64_t nAmount = nCredit - wtx.GetDebit();
        int64_t nTime = wtx.nTimeSmart ? wtx.nTimeSmart : wtx.GetTxTime();

        report.nStakesCounted++;
        if (wtx.GetBlocksToMaturity() > 0)
        {
            report.nMaturingAmount += nAmount;
            report.nMaturingCount++;
        }
        if (nTime > report.nLatestTime)
        {
            report.nLatestTime = nTime;
            report.nLatestAmount = nAmount;
        }
        BOOST_FOREACH(StakePeriod& p, report.vDays)
            if (nTime >= p.nStart && nTime <= p.nEnd) { p.nAmount += nAmount; p.nCount++; }
        BOOST_FOREACH(StakePeriod& p, report.vTotals)
            if (nTime >= p.nStart && nTime <= p.nEnd) { p.nAmount += nAmount; p.nCount++; }
    }
}

bool GetStakeReport(const CWallet* pwallet, StakeReport& report, bool fTry)
{
    const int64_t nDay = 24 * 60 * 60;
    const int64_t nNow = GetTime();

    // Periods: the last 30 calendar days (local time), then rolling totals
    report.vDays.clear();
    report.vTotals.clear();
    int64_t nDayStart = LocalMidnight(nNow);
    int64_t nDayEnd = nNow;
    for (int i = 0; i < STAKE_REPORT_DAYS; i++)
    {
        report.vDays.push_back(MakePeriod(DateTimeStrFormat("%Y-%m-%d", nDayStart), nDayStart, nDayEnd));
        nDayEnd = nDayStart - 1;
        nDayStart = LocalMidnight(nDayStart - nDay / 2);   // previous midnight, safe across DST changes
    }
    const int anTotalDays[] = { 1, 7, 30, 365 };
    const char* apszTotalNames[] = { "last24h", "last7days", "last30days", "last365days" };
    for (int i = 0; i < 4; i++)
        report.vTotals.push_back(MakePeriod(apszTotalNames[i], nNow - anTotalDays[i] * nDay, nNow));

    report.nLatestTime = 0;
    report.nLatestAmount = 0;
    report.nMaturingAmount = 0;
    report.nMaturingCount = 0;
    report.nStakesCounted = 0;

    // The wallet and its depth-in-chain checks need both locks. The GUI asks
    // with fTry so it never freezes waiting for them.
    CCriticalSection& cs_wallet = pwallet->cs_wallet;   // mutable
    if (fTry)
    {
        TRY_LOCK(cs_main, lockMain);
        if (!lockMain)
            return false;
        TRY_LOCK(cs_wallet, lockWallet);
        if (!lockWallet)
            return false;
        AddStakes(pwallet, report);
    }
    else
    {
        LOCK2(cs_main, cs_wallet);
        AddStakes(pwallet, report);
    }

    return true;
}
