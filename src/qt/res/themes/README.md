# FlapX wallet themes

The wallet's default look is the built-in **FlapXCoin** theme. Its style sheet is
`src/qt/res/flapxcoin.qss`, compiled into the wallet through `bitcoin.qrc`, so it
is always available. All themes are picked from the **Themes** menu, and the choice
is remembered.

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
  (navy window and input colours) still sits underneath every theme.
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
