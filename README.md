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


The default ports: Connection 60042, and RPC 60142 Testnet 60242

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




