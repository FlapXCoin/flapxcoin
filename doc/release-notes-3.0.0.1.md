FlapX 3.0.0.1
=============

A sync update to FlapX 3.0: faster, steadier block downloads, by choosing who to
download from better. Everyone on 3.0.0.0 should update; seed nodes especially.

## Downloads

| Platform | Daemon | GUI wallet |
|---|---|---|
| Windows 64-bit | `flapxd-v3.0.0.1-win64.zip` | `flapx-qt-v3.0.0.1-win64.zip` |
| Windows 32-bit | `flapxd-v3.0.0.1-win32.zip` | `flapx-qt-v3.0.0.1-win32.zip` |
| macOS, Apple Silicon | `flapxd-v3.0.0.1-macos-arm64.zip` | `FlapX-Qt-v3.0.0.1-macos-arm64.zip` (.dmg inside) |
| macOS, Intel | `flapxd-v3.0.0.1-macos-x86_64.zip` | `FlapX-Qt-v3.0.0.1-macos-x86_64.zip` (.dmg inside) |
| Linux, Ubuntu 22.04+ / Debian 12+ | `flapxd-v3.0.0.1-linux-x86_64.zip` | `flapx-qt-v3.0.0.1-linux-x86_64.zip` |
| Linux, Ubuntu 18.04+ | `flapxd-v3.0.0.1-ubuntu18.04-x86_64.zip` | `flapx-qt-v3.0.0.1-ubuntu18.04-x86_64.zip` |

Every file is a .zip. The macOS daemon's zip also holds a `libs` folder it needs;
keep it next to `flapxd`.

## What's new

### Sync
- **3.0+ nodes first, always.** Blocks are downloaded from a 3.0+ node whenever one
  can serve them (even one that failed earlier, up to three times); an older wallet
  only when there is no 3.0+ node at all.
- **Lowest ping wins.** Among nodes of the same version, the one with the lowest
  ping is picked, and after two minutes the wallet moves to a node of the same or a
  newer version that is much closer (half the ping or less).
- **Pings that mean something.** Nodes are compared by the best ping measured to
  them. The latest ping to the node you download from can read a minute or more
  while your wallet checks the blocks queued ahead of it, however close it is.
- **Finds 3.0 nodes.** Downloading from an older wallet because no 3.0+ node is
  connected, the wallet swaps one of its older connections for a new one every two
  minutes until it finds a 3.0+ node.

### Peers tab (Help → Debug window → Peers)
- Right-click a peer to **Disconnect** it.
- The Ping column shows each peer's best ping; the details show the best, last
  and waiting pings. `getpeerinfo` shows `minping`.

### Downloads
- Every release file is now a .zip, and Linux and macOS programs stay executable
  after unzipping, no `chmod` needed.

## Upgrading

- From 3.0.0.0: just replace the program; nothing else changes.
- From 2.x: see the [3.0.0.0 notes](release-notes-3.0.0.0.md) (back up
  `wallet.dat` first).
- **macOS:** the app isn't notarised by Apple, so the first time, right-click
  FlapX-Qt and choose **Open**, then confirm. For the daemon, clear the download
  flag once in the folder you unzipped it to:
  `xattr -dr com.apple.quarantine flapxd libs`
