FlapX 3.0.0.0
=============

FlapX 3.0 is the first full release of the modernised wallet: it builds and runs on
today's systems, syncs the network reliably, and adds staking tools, a built-in block
explorer, twelve themes and eleven languages.

## Downloads

| Platform | Daemon | GUI wallet |
|---|---|---|
| Windows 64-bit | `flapxd-v3.0.0.0-win64.zip` | `flapx-qt-v3.0.0.0-win64.zip` |
| Windows 32-bit | `flapxd-v3.0.0.0-win32.zip` | `flapx-qt-v3.0.0.0-win32.zip` |
| macOS, Apple Silicon | `flapxd-v3.0.0.0-macos-arm64.zip` | `FlapX-Qt-v3.0.0.0-macos-arm64.zip` (.dmg inside) |
| macOS, Intel | `flapxd-v3.0.0.0-macos-x86_64.zip` | `FlapX-Qt-v3.0.0.0-macos-x86_64.zip` (.dmg inside) |
| Linux, Ubuntu 22.04+ / Debian 12+ | `flapxd-v3.0.0.0-linux-x86_64.zip` | `flapx-qt-v3.0.0.0-linux-x86_64.zip` |
| Linux, Ubuntu 18.04+ | `flapxd-v3.0.0.0-ubuntu18.04-x86_64.zip` | `flapx-qt-v3.0.0.0-ubuntu18.04-x86_64.zip` |

Every file is a .zip. The macOS daemon's zip also holds a `libs` folder it needs;
keep it next to `flapxd`.

## What's new

### Builds and platforms
- Builds on modern systems again: current Linux, OpenSSL 3, current Boost and Qt 5.
- Release builds for every platform, made automatically on GitHub: Windows 64-bit and
  32-bit (wallet and daemon, each a single self-contained file), macOS on Apple Silicon
  and Intel (wallet and daemon), and Linux for Ubuntu 22.04+ and Ubuntu 18.04+.

### Network and sync
- Fixes the network fragmentation and syncs the chain reliably; a full sync from zero
  was tested start to finish.
- 3.0 wallets prefer other 3.0 nodes for syncing, and fall back to older ones when needed.
- Protocol version bumped (210044), so a future release can stop serving older wallets.
- A second peer port (60242), so two wallets on the same home network can both take
  connections.

### Security
- A full code audit, with its findings fixed.
- New FlapX alert key: the developers can send network messages to 3.0 wallets
  ("please update before block ..."). The old key was left over from NetCoin.
- New FlapX sync-checkpoint key, replacing NetCoin's: no one else can push checkpoints
  to the network. 3.0 wallets drop any checkpoint signed with the old key on first start.

### Staking
- New **APR Tracking** menu: a **Stake Report** (earnings per day, week, month and
  year) and a redesigned **Interest Estimate**.
- Stake auto-balancing: small inputs are combined and large ones split.
- Change goes back to the address it came from, so wallet backups always include it.

### Wallet
- A built-in block explorer: look up blocks and transactions without leaving the wallet.
- A redesigned overview ("Mission Control") and a Links page.
- Peers and live network-traffic tabs in the debug window.

### Look and feel
- Twelve themes: FlapXCoin, FlappyCoin, Космос, Mars Colony, Synthwave, Deep Sea,
  Winter, DaGoofMan, Silverwing, Flappy Night, Terminal and None. Custom themes can be
  added in a `themes` folder.
- See-through panels with bold white text, and each theme's own colours.
- The new clean FlapX helmet logo on every icon. The Export icon is by DaGoofMan.

### Languages
- Russian, Spanish, German, French, Brazilian Portuguese, Italian, Dutch, Polish,
  Turkish, Ukrainian and Simplified Chinese, all complete, including Qt's own buttons
  and menus. The wallet follows the computer's language, or pick one under
  Settings → Options → Display.

## Upgrading

- **Back up `wallet.dat` first.** These builds use Berkeley DB 5.3; once a 3.0 wallet
  has written to `wallet.dat`, wallets built with Berkeley DB 4.8 can no longer open it.
- No resync is needed: block files from older versions are moved into the `chain`
  folder automatically on first start.
- **Linux GUI:** only Qt 5 is needed from the system. If you get an error about
  `libQt5Widgets`, install it (`sudo apt install libqt5widgets5`, or
  `libqt5widgets5t64` on Ubuntu 24.04+). Unzip with `unzip`, which keeps the files
  executable (if your unzip tool doesn't: `chmod +x flapx-qt flapxd`).
- **macOS:** the app isn't notarised by Apple, so the first time, right-click
  FlapX-Qt and choose **Open**, then confirm. After that it opens normally.
  For the daemon, macOS blocks files downloaded in a browser until you clear the
  download flag once, in the folder you unzipped it to:
  `xattr -dr com.apple.quarantine flapxd libs`
