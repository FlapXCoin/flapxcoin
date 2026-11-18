// Copyright (c) 2014 Remy5 (original stake report)
// Copyright (c) 2026 FlapX Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
//
// Stake report: what this wallet earned from staking per day (last 30 days)
// and over the last 24 hours, 7, 30 and 365 days. Shared by the GUI's
// APR Tracking > Stake Report window and the getstakereport RPC.
#ifndef FLAPX_STAKEREPORT_H
#define FLAPX_STAKEREPORT_H

#include <stdint.h>
#include <string>
#include <vector>

class CWallet;

/** Earnings from stakes made in one period (start and end inclusive, unix time) */
struct StakePeriod
{
    std::string strName;
    int64_t nStart;
    int64_t nEnd;
    int64_t nAmount;      // net reward to this wallet (stake outputs minus stake inputs)
    int nCount;           // number of stakes
};

struct StakeReport
{
    std::vector<StakePeriod> vDays;     // [0] = today (since local midnight), [1] = yesterday, ...
    std::vector<StakePeriod> vTotals;   // last 24 hours, 7 days, 30 days, 365 days
    int64_t nLatestTime;                // time of the most recent stake (0 = none)
    int64_t nLatestAmount;
    int64_t nMaturingAmount;            // part of the totals not yet mature (still counted)
    int nMaturingCount;
    int nStakesCounted;                 // stakes in the main chain that were looked at
};

static const int STAKE_REPORT_DAYS = 30;

/** Fill in the report for pwallet. With fTry, return false at once if the
 *  wallet or chain is busy (for the GUI, which must not wait on the locks);
 *  otherwise wait for them. */
bool GetStakeReport(const CWallet* pwallet, StakeReport& report, bool fTry);

#endif // FLAPX_STAKEREPORT_H
