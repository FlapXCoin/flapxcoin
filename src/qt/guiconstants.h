#ifndef GUICONSTANTS_H
#define GUICONSTANTS_H

/* Milliseconds between model updates */
static const int MODEL_UPDATE_DELAY = 250;

/* AskPassphraseDialog -- Maximum passphrase length */
static const int MAX_PASSPHRASE_SIZE = 1024;

/* BitcoinGUI -- Size of icons in status bar */
static const int STATUSBAR_ICONSIZE = 24;

/* Invalid field background style */
#define STYLE_INVALID "background:#3a1622; border:1px solid #ff6b81; color:#ffffff;"

/* Transaction list colours (FlapXCoin theme) */
/* Transaction list -- unconfirmed transaction (muted) */
#define COLOR_UNCONFIRMED QColor(138, 155, 184)
/* Transaction list -- negative amount (silver) */
#define COLOR_NEGATIVE QColor(201, 212, 227)
/* Transaction list -- positive amount (mint) */
#define COLOR_POSITIVE QColor(34, 240, 122)
/* Transaction list -- bare address (without label) (muted) */
#define COLOR_BAREADDRESS QColor(138, 155, 184)

/* Tooltips longer than this (in characters) are converted into rich text,
   so that they can be word-wrapped.
 */
static const int TOOLTIP_WRAP_THRESHOLD = 80;

/* Maximum allowed URI length */
static const int MAX_URI_LENGTH = 255;

/* QRCodeDialog -- size of exported QR Code image */
#define EXPORT_IMAGE_SIZE 256

#endif // GUICONSTANTS_H
