# FlapX wallet themes

The wallet has four built-in themes, compiled in through `bitcoin.qrc`, so they
are always available:

- **FlapXCoin** (the default): deep navy space with a picture of a moon base,
  a rocket lifting off its pad and a Voyager-style probe under cyan and mint
  nebulas (`src/qt/res/images/flapx.jpg`, filling the window and anchored to
  the bottom). See-through panels framed in FlapX green, Flappy Bird yellow
  highlights, bold white text. Style sheet `src/qt/res/flapxcoin.qss`.
- **FlappyCoin**: the look of the original 2014 FlappyCoin-Qt wallet: Flappy Bird
  sky blue with clouds, city and bushes along the bottom (drawn in code, no
  images), cream panels, green outlines and orange buttons. Style sheet
  `src/qt/res/flappycoin.qss`, palette `GUIUtil::applyFlappyPalette()`.
- **Космос** (Kosmos): the Soviet space programme of the 1970s. The FlapXCoin
  layout with Soviet red frames, title tabs and buttons, star gold for amounts
  and bold white text, over a picture of Soyuz over the Earth, Sputnik, and a
  Venera lander parachuting down to Venus (`src/qt/res/images/kosmos.jpg`).
  Style sheet `src/qt/res/kosmos.qss`, palette `GUIUtil::applyKosmosPalette()`.
- **None**: plain grey and black, Qt's standard look with no pictures or
  colours (`src/qt/res/plain.qss`). Always the last entry in the Themes menu,
  after any themes from a themes folder.

All themes are picked from the **Themes** menu, and the choice is remembered.

## Adding a theme

1. Make a folder named after the theme, holding a `styles.qss`:

   ```
   themes/
     MyTheme/
       styles.qss
       (any images the theme uses)
   ```

2. Put the `themes` folder in one of these places (the first one found is used):
   - the FlapX data directory, e.g. `~/.flapx/themes` on Linux
   - next to the wallet program
   - this folder (`src/qt/res/themes`) when running a development build

3. Restart the wallet and pick the theme from the **Themes** menu.

The easiest start is a copy of `src/qt/res/flapxcoin.qss` with new colours.

## Style sheet notes

- A theme replaces the FlapXCoin style sheet completely. The FlapX palette
  (navy window and input colours) still sits underneath every theme folder.
- Variables go in a block at the top and can be used anywhere below:

  ```
  /** [VARS]
  @accent = #2ad8ea
  [/VARS] */

  QPushButton { color: @accent; }
  ```

- `$theme-dir` is replaced with the theme's folder, for its own files:
  `url($theme-dir/button.png)`.
- A theme that can't be read falls back to FlapXCoin.
- While designing a theme, press `$` in the wallet to reload it without restarting.
