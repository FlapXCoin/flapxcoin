#ifndef GUICONSTANTS_H
#define GUICONSTANTS_H

/* Milliseconds between model updates */
static const int MODEL_UPDATE_DELAY = 250;
/* Block height display refresh while syncing, in milliseconds (10 times a second) */
static const int LIVE_HEIGHT_DELAY = 100;
/* Sync time-left estimate: recompute every 5 seconds, averaging the download rate over the last 3 minutes */
static const int SYNC_ESTIMATE_DELAY = 5000;
static const int SYNC_ESTIMATE_WINDOW_MS = 3 * 60 * 1000;

/* AskPassphraseDialog -- Maximum passphrase length */
static const int MAX_PASSPHRASE_SIZE = 1024;

/* BitcoinGUI -- Size of icons in status bar */
static const int STATUSBAR_ICONSIZE = 24;

/* Invalid field background style */
#define STYLE_INVALID "background:#3a1622; border:1px solid #ff6b81; color:#ffffff;"

/* Transaction list colours; they follow the active theme (see GUIUtil::themeColor,
   needs guiutil.h) */
/* Transaction list -- unconfirmed transaction (muted) */
#define COLOR_UNCONFIRMED GUIUtil::themeColor(GUIUtil::TC_MUTED)
/* Transaction list -- negative amount */
#define COLOR_NEGATIVE GUIUtil::themeColor(GUIUtil::TC_NEGATIVE_AMOUNT)
/* Transaction list -- positive amount */
#define COLOR_POSITIVE GUIUtil::themeColor(GUIUtil::TC_POSITIVE)
/* Transaction list -- bare address (without label) (muted) */
#define COLOR_BAREADDRESS GUIUtil::themeColor(GUIUtil::TC_MUTED)

/* Tooltips longer than this (in characters) are converted into rich text,
   so that they can be word-wrapped.
 */
static const int TOOLTIP_WRAP_THRESHOLD = 80;

/* Maximum allowed URI length */
static const int MAX_URI_LENGTH = 255;

/* QRCodeDialog -- size of exported QR Code image */
#define EXPORT_IMAGE_SIZE 256

#endif // GUICONSTANTS_H
