#!/usr/bin/env bash
# Builds flapxd and flapx-qt inside an Ubuntu 18.04 container (used by CI:
# docker run -v "$PWD":/src -w /src ubuntu:18.04 bash contrib/ci/build-ubuntu1804.sh)
# Boost, Berkeley DB, OpenSSL, miniupnpc and qrencode are linked statically,
# so only Qt 5 and the C/C++ runtime come from the system.
set -euo pipefail
export DEBIAN_FRONTEND=noninteractive
apt-get update
apt-get install -y build-essential libssl-dev libdb++-dev \
  libboost-system-dev libboost-filesystem-dev libboost-program-options-dev \
  libboost-thread-dev libboost-chrono-dev libminiupnpc-dev zlib1g-dev \
  qtbase5-dev qttools5-dev-tools libqrencode-dev git file

git config --global --add safe.directory /src
N="$(nproc)"

cd /src/src
make -f makefile.unix -j"$N" USE_UPNP=1 STATIC=1
strip flapxd
cd /src

qmake -qt=qt5 RELEASE=1 USE_UPNP=1 USE_QRCODE=1 "CONFIG+=release" "CONFIG-=debug_and_release" flapx-qt.pro
make -j"$N"
strip flapx-qt
ldd flapx-qt
