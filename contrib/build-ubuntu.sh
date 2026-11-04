#!/usr/bin/env bash
# Build the FlapX daemon (flapxd) on a modern Ubuntu/Debian system using the
# distribution's own Boost, OpenSSL 3, Berkeley DB 5.3 and miniupnpc packages.
#
# Usage:  contrib/build-ubuntu.sh               # UPnP built in, off by default
#         USE_UPNP=1 contrib/build-ubuntu.sh    # UPnP on by default
#         NO_UPNP=1 contrib/build-ubuntu.sh     # build without UPnP
#         SKIP_DEPS=1 contrib/build-ubuntu.sh   # don't run apt-get
#
# See doc/build-ubuntu.md for details.
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

if [ -z "${SKIP_DEPS:-}" ]; then
    echo "==> Installing build dependencies (needs sudo)"
    sudo apt-get update
    sudo apt-get install -y \
        build-essential \
        libssl-dev \
        libdb++-dev \
        libboost-system-dev \
        libboost-filesystem-dev \
        libboost-program-options-dev \
        libboost-thread-dev \
        libboost-chrono-dev \
        libminiupnpc-dev \
        zlib1g-dev
fi

# USE_UPNP=0 compiles UPnP in but leaves it off unless flapxd is started with
# -upnp=1 (the makefile's existing default). 1 turns it on by default and "-"
# leaves it out of the build entirely.
if [ -n "${NO_UPNP:-}" ]; then
    UPNP="-"
else
    UPNP="${USE_UPNP:-0}"
fi

echo "==> Building flapxd"
cd "$REPO_ROOT/src"
make -f makefile.unix -j"$(nproc)" USE_UPNP="$UPNP"

echo
echo "==> Done: $REPO_ROOT/src/flapxd"
