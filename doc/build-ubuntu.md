# Building flapxd on modern Ubuntu / Debian

These instructions build the FlapX daemon (`flapxd`) against the libraries
that current Linux distributions ship: OpenSSL 3, Boost 1.7x–1.8x,
Berkeley DB 5.3 and miniupnpc 2.x. They were tested with Ubuntu 24.04's
versions (GCC 13, Boost 1.83, OpenSSL 3.0, Berkeley DB 5.3.28).

The older `upstream/` dependency bundle (Boost 1.55, OpenSSL 1.0.2,
Berkeley DB 4.8) is no longer needed for this, and several of its download
links are dead.

## Quick start

```bash
contrib/build-ubuntu.sh
```

This installs the build dependencies with `apt-get` (it asks for sudo), then
builds `src/flapxd`. Options:

| Variable | Effect |
| --- | --- |
| `SKIP_DEPS=1` | Don't run `apt-get`; use what's already installed |
| `USE_UPNP=1` | Turn UPnP port mapping on by default |
| `NO_UPNP=1` | Build without UPnP (no miniupnpc needed) |

## Manual build

```bash
sudo apt-get install build-essential libssl-dev libdb++-dev \
    libboost-system-dev libboost-filesystem-dev libboost-program-options-dev \
    libboost-thread-dev libboost-chrono-dev libminiupnpc-dev zlib1g-dev

cd src
make -f makefile.unix -j"$(nproc)"
```

### UPnP

`USE_UPNP` controls UPnP support:

- `USE_UPNP=0` (default): compiled in, off unless you start with `-upnp=1`
- `USE_UPNP=1`: compiled in and on by default
- `USE_UPNP=-`: left out entirely, so `libminiupnpc-dev` isn't needed

## Wallet compatibility (Berkeley DB)

Distributions ship Berkeley DB 5.3, while older FlapX releases were built
with 4.8. A wallet from an older release opens fine in a 5.3 build, but once
the new build has written to it, **older 4.8 builds can no longer open that
wallet**. Back up `wallet.dat` before running a new build on an existing
wallet, and keep the backup if you may need to go back to an older release.

## Building Berkeley DB yourself

If you build Berkeley DB 4.8 or 5.3 from source with a modern GCC, it fails
in `atomic.h` because its `__atomic_compare_exchange` clashes with a GCC
built-in. Rename it before building (the file is `src/dbinc/atomic.h` in
5.3 and `dbinc/atomic.h` in 4.8):

```bash
sed -i 's/__atomic_compare_exchange/__atomic_compare_exchange_db/g' src/dbinc/atomic.h
```

(For 4.8, `upstream/patches/db-4.8.30.NC/` already does this.)
