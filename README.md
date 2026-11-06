2024 updates in progress
FLAPXZERO DAY 3
========


PUNCH LIST 



>TX page right click explorer link 404
>Change RPC Port
>Another round windows debug (sizing issues)
>flip unlock icon and make new mining a ctive icon 
>tool tip ghosts everywhere
>


FlapX (FLAPX) - Litecoin / PeerCoin Hybrid POW/POSV hybrid with Personal Investment Rate


The default ports: mainnet peer 60042 and RPC 60142; testnet peer 60043 and RPC 60143
Second peer port (for a second wallet on the same network): mainnet 60242, testnet 60243

POW Details.

60 seconds block time POW.
Scrypt algorithm.
15 POW coins per block.
30 confirmations for mined blocks.
Difficulty retargets on every block.


POS Details.

POS activated
POSV stake once each month to earn highest possible rewards incentivises running full nodes to increase POS security. Coin age gradually slows after 1 month with an aging half life of 90 days.

Personal Investment Rate or PIR is a special reward algorithm unique for a PoS coin. The APR Stake reward increases in line with the value of the coins staked on a sliding scale.In other words the more FlapX you own and stake the bigger the reward you earn. The last 3 years FlapX had 3 different levels of PIR and it reached the last and final level end 2016.
The reward rates now are as followed: 0-999N --> #%
                                      1000-9999N --> #%
                                      10000-99999N --> #%
                                      100000-999999N --> #%
                                      1000000-9999999N --> #%
                                      10000000N->>> --> #%

120 seconds block time POS
1 hour minimum age for staking.  30 days max age.
FAIR-WEIGHT protocol replaces the idea of 'block splitting' to prevent very large FLAPX holders signing lots of POS blocks.

70 confirmations for minted blocks
   	
Build notes.

##################################################################################################
Running the prebuilt binaries
##################################################################################################

flapxd (Linux daemon)
Needs nothing extra: Boost, Berkeley DB, OpenSSL and miniupnpc are built in.
Runs on Ubuntu 22.04+ / Debian 12+ (64-bit).

flapx-qt (Linux GUI wallet)
Boost, Berkeley DB, OpenSSL, miniupnpc and qrencode are built in; only Qt 5 is
needed from the system. Most desktops already have it. If you get an error about
libQt5Widgets, libQt5Gui or libQt5Core, install it:

* Ubuntu 24.04+ / Debian 13: `sudo apt install libqt5widgets5t64`
* Ubuntu 22.04 / Debian 12: `sudo apt install libqt5widgets5`

Make it executable before the first run: `chmod +x flapx-qt flapxd`

flapx-qt.exe (Windows GUI wallet)
Needs nothing extra. Single self-contained file for 64-bit Windows.

All binaries are built with UPnP on.

Block files: the blockchain is stored in a "chain" folder inside the data
directory, in files of about 25 MB (chain/blk0001.dat, chain/blk0002.dat, ...).
On first start, block files from older versions are moved into it
automatically; no resync is needed. To go back to an older version, move the
blk*.dat files from chain/ back into the data directory first.

Running two wallets on the same home network
Your router can send incoming connections on a port to only one computer, so two
wallets behind the same internet connection can't both use port 60042. From
2.9.0.4 there is a second peer port, 60242, that wallets treat just like 60042:

* First wallet (e.g. the one that stakes 24/7): leave it as it is, on port 60042.
* Second wallet: GUI: Settings > Options > Network > "Use the second peer port
  (60242)", then restart. flapxd: add altport=1 to flapx.conf.
* On the router, forward TCP 60042 to the first computer and TCP 60242 to the
  second (or leave UPnP on in both wallets).

Other 2.9.0.4+ wallets will then connect to both: they allow up to two
connections to the same internet address, one on each of the two ports. Older
wallets connect to the 60242 one less readily. (Two wallets on the same computer
also need separate data directories: -datadir.)

Wallet note: these builds use Berkeley DB 5.3. Back up wallet.dat before
opening it with them: once a 5.3 build has written to a wallet, older builds
made with Berkeley DB 4.8 can no longer open it.

##################################################################################################
Building from source on Ubuntu / Debian
##################################################################################################

Requires Boost 1.66 or newer, OpenSSL 1.1 or 3, Berkeley DB 5.3 and miniupnpc 2.x,
all of which come from the distribution's packages. Tested on Ubuntu 22.04 and 24.04.

Daemon (flapxd)

```
sudo apt install build-essential libssl-dev libdb++-dev zlib1g-dev libminiupnpc-dev \
    libboost-system-dev libboost-filesystem-dev libboost-program-options-dev \
    libboost-thread-dev libboost-chrono-dev

USE_UPNP=1 SKIP_DEPS=1 contrib/build-ubuntu.sh
# or: make -C src -f makefile.unix USE_UPNP=1
```

GUI wallet (flapx-qt): the daemon packages above, plus

```
sudo apt install qtbase5-dev qttools5-dev-tools libqrencode-dev

qmake USE_UPNP=1 USE_QRCODE=1 "CONFIG+=release" "CONFIG-=debug_and_release" flapx-qt.pro
make
```

If qmake is missing or reports Qt 6, run Qt 5's directly: /usr/lib/qt5/bin/qmake

USE_UPNP=1 turns UPnP on by default; USE_UPNP=0 builds it in but off,
USE_UPNP=- leaves it out. More detail: doc/build-ubuntu.md

The Windows wallet and the release binaries are built by GitHub Actions
(.github/workflows/build.yml).

##################################################################################################
Legacy build (old upstream/ bundle) - NO LONGER WORKS with this code
##################################################################################################
Kept for reference only. The bundle uses Boost 1.55 and Berkeley DB 4.8;
the code now needs Boost 1.66 or newer. Use the instructions above instead.

Berkeley db 4.8 Deprecation 
Users on Ubuntu 20+ and Debian 11+ respectively 
must use Cmake to compile

##################################################################################################
sudo apt install build-essential libpng-dev zlib1g-dev libbz2-dev python3-pip python3-requirements

sudo make -C upstream unarchive BASE_OS=ubuntu20


*for 4.8 atleast do * sudo make -C upstream patches
answer y no next steps if prompted patch exists


sudo make -C upstream build BASE_OS=ubuntu20

daemon

sudo make -C src -f makefile.unix STATIC=1

sudo apt-get install qttools5-dev-tools qtbase5-dev qtchooser qt5-qmake qtbase5-dev-tools

qmake "CONFIG += static" "CONFIG += release"
make

#############################################################################################

Development process
===================

Developers work in their own trees, then submit pull requests when
they think their feature or bug fix is ready.

The patch will be accepted if there is broad consensus that it is a
good thing.  Developers should expect to rework and resubmit patches
if they don't match the project's coding conventions (see coding.txt)
or are controversial.

The master branch is regularly built and tested, but is not guaranteed
to be completely stable. Tags are regularly created to indicate new
official, stable release versions of FlapX coin.

Feature branches are created when there are major new features being
worked on by several people.

From time to time a pull request will become outdated. If this occurs, and
the pull is no longer automatically mergeable; a comment on the pull will
be used to issue a warning of closure. The pull will be closed 15 days
after the warning if action is not taken by the author. Pull requests closed
in this manner will have their corresponding issue labeled 'stagnant'.

Issues with no commits will be given a similar warning, and closed after
15 days from their last activity. Issues closed in this manner will be 
labeled 'stale'. 
# FlapX




