FlapX 3.0.0.2
=============

A small update to FlapX 3.0: the overview's Unlock/Lock button, translated
properly. Thanks to Serega for testing the Russian translation and reporting it.

## Downloads

| Platform | Daemon | GUI wallet |
|---|---|---|
| Windows 64-bit | `flapxd-v3.0.0.2-win64.zip` | `flapx-qt-v3.0.0.2-win64.zip` |
| Windows 32-bit | `flapxd-v3.0.0.2-win32.zip` | `flapx-qt-v3.0.0.2-win32.zip` |
| macOS, Apple Silicon | `flapxd-v3.0.0.2-macos-arm64.zip` | `FlapX-Qt-v3.0.0.2-macos-arm64.zip` (.dmg inside) |
| macOS, Intel | `flapxd-v3.0.0.2-macos-x86_64.zip` | `FlapX-Qt-v3.0.0.2-macos-x86_64.zip` (.dmg inside) |
| Linux, Ubuntu 22.04+ / Debian 12+ | `flapxd-v3.0.0.2-linux-x86_64.zip` | `flapx-qt-v3.0.0.2-linux-x86_64.zip` |
| Linux, Ubuntu 18.04+ | `flapxd-v3.0.0.2-ubuntu18.04-x86_64.zip` | `flapx-qt-v3.0.0.2-ubuntu18.04-x86_64.zip` |

Every file is a .zip. The macOS daemon's zip also holds a `libs` folder it needs;
keep it next to `flapxd`.

## What's new

- **The Unlock/Lock button on the overview says what it will do:** Unlock while
  the wallet is locked, Lock once it's unlocked for staking. It's translated in
  every language (it used to start as "Lock/Unlock", translated in Russian as
  "lock", and switch to English after a press), and it follows the wallet when
  it's locked or unlocked from the menu.
- On a wallet without a password the button stays disabled, as there's nothing to
  unlock; its tooltip now says so and points to Settings > Encrypt Wallet.

Also in 3.0.0.1 (if you're coming from 3.0.0.0): 3.0+ sync nodes always first,
the lowest ping wins, right-click Disconnect in the peers tab, and the second
peer port setting kept per wallet. See [release-notes-3.0.0.1.md](release-notes-3.0.0.1.md).

## Upgrading

- From 3.0.x: just replace the program; nothing else changes.
- From 2.x: see the [3.0.0.0 notes](release-notes-3.0.0.0.md) (back up
  `wallet.dat` first).
- **macOS:** the app isn't notarised by Apple, so the first time, right-click
  FlapX-Qt and choose **Open**, then confirm. For the daemon, clear the download
  flag once in the folder you unzipped it to:
  `xattr -dr com.apple.quarantine flapxd libs`
