# Wallet translations

The wallet picks its language from the computer's language at startup. It can
also be chosen under **Settings → Options → Display → User Interface
language** (after a restart), or forced with `lang=ru` in `flapx.conf` /
`-lang=ru` on the command line.

| Language | File | Status |
|---|---|---|
| English | `src/qt/locale/bitcoin_en.ts` | source language |
| Russian (Русский) | `src/qt/locale/bitcoin_ru.ts` | complete (861 strings) |

Qt supplies some text itself: the standard dialog buttons (OK, Cancel, Yes,
No...), the Copy/Paste menu in text boxes, and the scroll bar menus. The
wallet uses the system's Qt translations for these when it has them, and
otherwise its own `src/qt/locale/qt_<code>.ts` (built in under
`/qttranslations`, so it isn't listed as a language). That matters for the
static Windows build, which has no Qt translations. Only Russian has one so
far; a new language can add its own `qt_<code>.ts` the same way.

The Russian translation reuses the community translations of Bitcoin
(0.8.6, 0.9.5) and NovaCoin for the text FlapX inherited from them, with the
FlapX-specific text translated for FlapX. It uses «кошелёк» for "wallet".
Corrections from Russian-speaking users are welcome: edit the `.ts` file in
Qt Linguist (or any text editor) and send a pull request.

## How it fits together

- `.ts` files (XML, one per language) are compiled to `.qm` by `lrelease`
  during the build (`flapx-qt.pro`, `TSQM`), and the `.qm` files are built
  into the wallet through `src/qt/bitcoin.qrc` (prefix `/translations`).
  A `.qm` that isn't listed in `bitcoin.qrc` is not included.
- Text in the GUI is marked with `tr("...")`; text in the core (help, errors,
  splash screen) with `_("...")`, collected into `src/qt/bitcoinstrings.cpp`
  in the `bitcoin-core` context.

## Updating after the text changes

1. Regenerate `src/qt/bitcoinstrings.cpp` so it lists the core's current
   `_("...")` strings.
2. Run `lupdate flapx-qt.pro` (Qt 5 tools). It adds new strings to every
   `.ts` file as unfinished and marks removed ones obsolete.
3. Translate the unfinished strings (Qt Linguist makes this easy), keeping
   placeholders such as `%1`, `%n` and `%s`, and the `&` keyboard shortcuts.

## Adding a language

1. Copy `bitcoin_en.ts` to `bitcoin_<code>.ts` (e.g. `bitcoin_de.ts`), set
   `language="<code>"` and translate it.
2. Add `<file alias="<code>">locale/bitcoin_<code>.qm</file>` to the
   `/translations` section of `src/qt/bitcoin.qrc`.
