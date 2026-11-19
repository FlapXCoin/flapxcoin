#!/usr/bin/env bash
# Builds flapxd and flapx-qt inside an Ubuntu 18.04 container (used by CI:
# docker run -v "$PWD":/src -w /src ubuntu:18.04 bash contrib/ci/build-ubuntu1804.sh)
# 18.04's Boost (1.65) is too old for FlapX, so Boost 1.83 is built here.
# Boost, Berkeley DB, OpenSSL, miniupnpc and qrencode are linked statically,
# so only Qt 5 and the C/C++ runtime come from the system.
set -euo pipefail
export DEBIAN_FRONTEND=noninteractive
apt-get update
apt-get install -y build-essential libssl-dev libdb++-dev libminiupnpc-dev zlib1g-dev \
  qtbase5-dev qttools5-dev-tools git file curl ca-certificates cmake

git config --global --add safe.directory /src
N="$(nproc)"

# Boost 1.83, static, only the libraries FlapX uses
BOOST=/opt/boost
curl -sSfL -o /tmp/boost.tar.gz https://archives.boost.io/release/1.83.0/source/boost_1_83_0.tar.gz
tar -xzf /tmp/boost.tar.gz -C /tmp
(cd /tmp/boost_1_83_0 && ./bootstrap.sh --with-libraries=system,filesystem,program_options,thread,chrono \
  && ./b2 -j"$N" link=static threading=multi variant=release cxxflags=-fPIC --prefix="$BOOST" install > /tmp/boost-build.log 2>&1) \
  || { tail -50 /tmp/boost-build.log; exit 1; }

# qrencode, static (18.04's package has no static library)
QRENCODE=/opt/qrencode
curl -sSfL -o /tmp/qrencode.tar.gz https://github.com/fukuchi/libqrencode/archive/refs/tags/v4.1.1.tar.gz
tar -xzf /tmp/qrencode.tar.gz -C /tmp
mkdir -p /tmp/qrencode-build && cd /tmp/qrencode-build
cmake /tmp/libqrencode-4.1.1 -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF \
  -DWITH_TOOLS=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DCMAKE_INSTALL_PREFIX="$QRENCODE"
make -j"$N" install

cd /src/src
make -f makefile.unix -j"$N" USE_UPNP=1 STATIC=1 \
  BOOST_INCLUDE_PATH="$BOOST/include" BOOST_LIB_PATH="$BOOST/lib"
strip flapxd
cd /src

qmake -qt=qt5 RELEASE=1 USE_UPNP=1 USE_QRCODE=1 "CONFIG+=release" "CONFIG-=debug_and_release" \
  "BOOST_INCLUDE_PATH=$BOOST/include" "BOOST_LIB_PATH=$BOOST/lib" \
  "QRENCODE_INCLUDE_PATH=$QRENCODE/include" "QRENCODE_LIB_PATH=$QRENCODE/lib" flapx-qt.pro
make -j"$N"
strip flapx-qt
ldd flapx-qt
