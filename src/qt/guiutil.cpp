#include "guiutil.h"
#include "bitcoinaddressvalidator.h"
#include "walletmodel.h"
#include "bitcoinunits.h"
#include "util.h"
#include "init.h"

#include <QString>
#include <QDateTime>
#include <QDoubleValidator>
#include <QFont>
#include <QLineEdit>
#include <QUrl>
#include <QTextDocument> // For Qt::escape
#include <QAbstractItemView>
#include <QApplication>
#include <QClipboard>
#include <QFileDialog>
#include <QDesktopServices>
#include <QThread>
#include <QPalette>
#include <QStyle>
#include <QStyleFactory>
#include <QFile>
#include <QMap>
#include <QMapIterator>
#include <QPainter>
#include <QPainterPath>
#include <QPolygonF>

#include <cmath>
#include <QGraphicsDropShadowEffect>
#include <QPixmap>
#include <QImage>
#include <QBrush>

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#ifdef WIN32
#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
#define _WIN32_WINNT 0x0501
#ifdef _WIN32_IE
#undef _WIN32_IE
#endif
#define _WIN32_IE 0x0501
#define WIN32_LEAN_AND_MEAN 1
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "shlwapi.h"
#include "shlobj.h"
#include "shellapi.h"
#endif

namespace GUIUtil {

QString dateTimeStr(const QDateTime &date)
{
    return date.date().toString(Qt::SystemLocaleShortDate) + QString(" ") + date.toString("hh:mm");
}

QString dateTimeStr(qint64 nTime)
{
    return dateTimeStr(QDateTime::fromTime_t((qint32)nTime));
}

QFont bitcoinAddressFont()
{
    QFont font("Monospace");
    font.setStyleHint(QFont::TypeWriter);
    return font;
}

void setupAddressWidget(QLineEdit *widget, QWidget *parent)
{
    widget->setMaxLength(BitcoinAddressValidator::MaxAddressLength);
    widget->setValidator(new BitcoinAddressValidator(parent));
    widget->setFont(bitcoinAddressFont());
}

void setupAmountWidget(QLineEdit *widget, QWidget *parent)
{
    QDoubleValidator *amountValidator = new QDoubleValidator(parent);
    amountValidator->setDecimals(8);
    amountValidator->setBottom(0.0);
    widget->setValidator(amountValidator);
    widget->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
}

bool parseBitcoinURI(const QUrl &uri, SendCoinsRecipient *out)
{
    // NovaCoin: check prefix
    if(uri.scheme() != QString("flapx"))
        return false;

    SendCoinsRecipient rv;
    rv.address = uri.path();
    rv.amount = 0;
    QList<QPair<QString, QString> > items = uri.queryItems();
    for (QList<QPair<QString, QString> >::iterator i = items.begin(); i != items.end(); i++)
    {
        bool fShouldReturnFalse = false;
        if (i->first.startsWith("req-"))
        {
            i->first.remove(0, 4);
            fShouldReturnFalse = true;
        }

        if (i->first == "label")
        {
            rv.label = i->second;
            fShouldReturnFalse = false;
        }
        else if (i->first == "amount")
        {
            if(!i->second.isEmpty())
            {
                if(!BitcoinUnits::parse(BitcoinUnits::BTC, i->second, &rv.amount))
                {
                    return false;
                }
            }
            fShouldReturnFalse = false;
        }

        if (fShouldReturnFalse)
            return false;
    }
    if(out)
    {
        *out = rv;
    }
    return true;
}

bool parseBitcoinURI(QString uri, SendCoinsRecipient *out)
{
    // Convert flapx:// to flapx:
    //
    //    Cannot handle this later, because bitcoin:// will cause Qt to see the part after // as host,
    //    which will lower-case it (and thus invalidate the address).
    if(uri.startsWith("flapx://"))
    {
        uri.replace(0, 12, "flapx:");
    }
    QUrl uriInstance(uri);
    return parseBitcoinURI(uriInstance, out);
}

QString HtmlEscape(const QString& str, bool fMultiLine)
{
    QString escaped = Qt::escape(str);
    if(fMultiLine)
    {
        escaped = escaped.replace("\n", "<br>\n");
    }
    return escaped;
}

QString HtmlEscape(const std::string& str, bool fMultiLine)
{
    return HtmlEscape(QString::fromStdString(str), fMultiLine);
}

void copyEntryData(QAbstractItemView *view, int column, int role)
{
    if(!view || !view->selectionModel())
        return;
    QModelIndexList selection = view->selectionModel()->selectedRows(column);

    if(!selection.isEmpty())
    {
        // Copy first item
        QApplication::clipboard()->setText(selection.at(0).data(role).toString());
    }
}

QString getSaveFileName(QWidget *parent, const QString &caption,
                                 const QString &dir,
                                 const QString &filter,
                                 QString *selectedSuffixOut)
{
    QString selectedFilter;
    QString myDir;
    if(dir.isEmpty()) // Default to user documents location
    {
        myDir = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
    }
    else
    {
        myDir = dir;
    }
    QString result = QFileDialog::getSaveFileName(parent, caption, myDir, filter, &selectedFilter);

    /* Extract first suffix from filter pattern "Description (*.foo)" or "Description (*.foo *.bar ...) */
    QRegExp filter_re(".* \\(\\*\\.(.*)[ \\)]");
    QString selectedSuffix;
    if(filter_re.exactMatch(selectedFilter))
    {
        selectedSuffix = filter_re.cap(1);
    }

    /* Add suffix if needed */
    QFileInfo info(result);
    if(!result.isEmpty())
    {
        if(info.suffix().isEmpty() && !selectedSuffix.isEmpty())
        {
            /* No suffix specified, add selected suffix */
            if(!result.endsWith("."))
                result.append(".");
            result.append(selectedSuffix);
        }
    }

    /* Return selected suffix if asked to */
    if(selectedSuffixOut)
    {
        *selectedSuffixOut = selectedSuffix;
    }
    return result;
}

Qt::ConnectionType blockingGUIThreadConnection()
{
    if(QThread::currentThread() != QCoreApplication::instance()->thread())
    {
        return Qt::BlockingQueuedConnection;
    }
    else
    {
        return Qt::DirectConnection;
    }
}

bool checkPoint(const QPoint &p, const QWidget *w)
{
    QWidget *atW = qApp->widgetAt(w->mapToGlobal(p));
    if (!atW) return false;
    return atW->topLevelWidget() == w;
}

bool isObscured(QWidget *w)
{
    return !(checkPoint(QPoint(0, 0), w)
        && checkPoint(QPoint(w->width() - 1, 0), w)
        && checkPoint(QPoint(0, w->height() - 1), w)
        && checkPoint(QPoint(w->width() - 1, w->height() - 1), w)
        && checkPoint(QPoint(w->width() / 2, w->height() / 2), w));
}

void openDebugLogfile()
{
    boost::filesystem::path pathDebug = GetDataDir() / "debug.log";

    /* Open debug.log with the associated application */
    if (boost::filesystem::exists(pathDebug))
        QDesktopServices::openUrl(QUrl::fromLocalFile(QString::fromStdString(pathDebug.string())));
}

ToolTipToRichTextFilter::ToolTipToRichTextFilter(int size_threshold, QObject *parent) :
    QObject(parent), size_threshold(size_threshold)
{

}

bool ToolTipToRichTextFilter::eventFilter(QObject *obj, QEvent *evt)
{
    if(evt->type() == QEvent::ToolTipChange)
    {
        QWidget *widget = static_cast<QWidget*>(obj);
        QString tooltip = widget->toolTip();
        if(tooltip.size() > size_threshold && !tooltip.startsWith("<qt>") && !Qt::mightBeRichText(tooltip))
        {
            // Prefix <qt/> to make sure Qt detects this as rich text
            // Escape the current message as HTML and replace \n by <br>
            tooltip = "<qt>" + HtmlEscape(tooltip, true) + "<qt/>";
            widget->setToolTip(tooltip);
            return true;
        }
    }
    return QObject::eventFilter(obj, evt);
}

#ifdef WIN32
boost::filesystem::path static StartupShortcutPath()
{
    return GetSpecialFolderPath(CSIDL_STARTUP) / "FlapX.lnk";
}

bool GetStartOnSystemStartup()
{
    // check for Bitcoin.lnk
    return boost::filesystem::exists(StartupShortcutPath());
}

bool SetStartOnSystemStartup(bool fAutoStart)
{
    // If the shortcut exists already, remove it for updating
    boost::filesystem::remove(StartupShortcutPath());

    if (fAutoStart)
    {
        CoInitialize(NULL);

        // Get a pointer to the IShellLink interface.
        IShellLink* psl = NULL;
        HRESULT hres = CoCreateInstance(CLSID_ShellLink, NULL,
                                CLSCTX_INPROC_SERVER, IID_IShellLink,
                                reinterpret_cast<void**>(&psl));

        if (SUCCEEDED(hres))
        {
            // Get the current executable path
            TCHAR pszExePath[MAX_PATH];
            GetModuleFileName(NULL, pszExePath, sizeof(pszExePath));

            TCHAR pszArgs[5] = TEXT("-min");

            // Set the path to the shortcut target
            psl->SetPath(pszExePath);
            PathRemoveFileSpec(pszExePath);
            psl->SetWorkingDirectory(pszExePath);
            psl->SetShowCmd(SW_SHOWMINNOACTIVE);
            psl->SetArguments(pszArgs);

            // Query IShellLink for the IPersistFile interface for
            // saving the shortcut in persistent storage.
            IPersistFile* ppf = NULL;
            hres = psl->QueryInterface(IID_IPersistFile,
                                       reinterpret_cast<void**>(&ppf));
            if (SUCCEEDED(hres))
            {
                WCHAR pwsz[MAX_PATH];
                // Ensure that the string is ANSI.
                MultiByteToWideChar(CP_ACP, 0, StartupShortcutPath().string().c_str(), -1, pwsz, MAX_PATH);
                // Save the link by calling IPersistFile::Save.
                hres = ppf->Save(pwsz, TRUE);
                ppf->Release();
                psl->Release();
                CoUninitialize();
                return true;
            }
            psl->Release();
        }
        CoUninitialize();
        return false;
    }
    return true;
}

#elif defined(LINUX)

// Follow the Desktop Application Autostart Spec:
//  http://standards.freedesktop.org/autostart-spec/autostart-spec-latest.html

boost::filesystem::path static GetAutostartDir()
{
    namespace fs = boost::filesystem;

    char* pszConfigHome = getenv("XDG_CONFIG_HOME");
    if (pszConfigHome) return fs::path(pszConfigHome) / "autostart";
    char* pszHome = getenv("HOME");
    if (pszHome) return fs::path(pszHome) / ".config" / "autostart";
    return fs::path();
}

boost::filesystem::path static GetAutostartFilePath()
{
    return GetAutostartDir() / "flapx.desktop";
}

bool GetStartOnSystemStartup()
{
    boost::filesystem::ifstream optionFile(GetAutostartFilePath());
    if (!optionFile.good())
        return false;
    // Scan through file for "Hidden=true":
    std::string line;
    while (!optionFile.eof())
    {
        getline(optionFile, line);
        if (line.find("Hidden") != std::string::npos &&
            line.find("true") != std::string::npos)
            return false;
    }
    optionFile.close();

    return true;
}

bool SetStartOnSystemStartup(bool fAutoStart)
{
    if (!fAutoStart)
        boost::filesystem::remove(GetAutostartFilePath());
    else
    {
        char pszExePath[MAX_PATH+1];
        memset(pszExePath, 0, sizeof(pszExePath));
        if (readlink("/proc/self/exe", pszExePath, sizeof(pszExePath)-1) == -1)
            return false;

        boost::filesystem::create_directories(GetAutostartDir());

        boost::filesystem::ofstream optionFile(GetAutostartFilePath(), std::ios_base::out|std::ios_base::trunc);
        if (!optionFile.good())
            return false;
        // Write a bitcoin.desktop file to the autostart directory:
        optionFile << "[Desktop Entry]\n";
        optionFile << "Type=Application\n";
        optionFile << "Name=FlapX\n";
        optionFile << "Exec=" << pszExePath << " -min\n";
        optionFile << "Terminal=false\n";
        optionFile << "Hidden=false\n";
        optionFile.close();
    }
    return true;
}
#else

// TODO: OSX startup stuff; see:
// https://developer.apple.com/library/mac/#documentation/MacOSX/Conceptual/BPSystemStartup/Articles/CustomLogin.html

bool GetStartOnSystemStartup() { return false; }
bool SetStartOnSystemStartup(bool fAutoStart) { return false; }

#endif

HelpMessageBox::HelpMessageBox(QWidget *parent) :
    QMessageBox(parent)
{
    header = tr("FlapX-Qt") + " " + tr("version") + " " +
        QString::fromStdString(FormatFullVersion()) + "\n\n" +
        tr("Usage:") + "\n" +
        "  flapx-qt [" + tr("command-line options") + "]                     " + "\n";

    coreOptions = QString::fromStdString(HelpMessage());

    uiOptions = tr("UI options") + ":\n" +
        "  -lang=<lang>           " + tr("Set language, for example \"de_DE\" (default: system locale)") + "\n" +
        "  -min                   " + tr("Start minimized") + "\n" +
        "  -splash                " + tr("Show splash screen on startup (default: 1)") + "\n";

    setWindowTitle(tr("FlapX-Qt"));
    setTextFormat(Qt::PlainText);
    // setMinimumWidth is ignored for QMessageBox so put in non-breaking spaces to make it wider.
    setText(header + QString(QChar(0x2003)).repeated(50));
    setDetailedText(coreOptions + "\n" + uiOptions);
}

void HelpMessageBox::printToConsole()
{
    // On other operating systems, the expected action is to print the message to the console.
    QString strUsage = header + "\n" + coreOptions + "\n" + uiOptions;
    fprintf(stdout, "%s", strUsage.toStdString().c_str());
}

void HelpMessageBox::showOrPrint()
{
#if defined(WIN32)
        // On Windows, show a message box, as there is no stderr/stdout in windowed applications
        exec();
#else
        // On other operating systems, print help text to console
        printToConsole();
#endif
}

// The FlapX starfield, drawn in code (no image): the same stars as the block explorer's
// stylesheet, on a 420x420 tile that repeats across the window background.
static QPixmap flapxStarfieldTile(const QColor &space)
{
    struct Star { double x, y, size; int r, g, b; double alpha; };
    static const Star stars[] = {
        { 0.12, 0.18, 1.0, 255, 255, 255, 0.90 },
        { 0.68, 0.09, 1.0, 255, 255, 255, 0.80 },
        { 0.85, 0.42, 1.5, 200, 246, 251, 0.90 },   // pale cyan
        { 0.33, 0.64, 1.0, 255, 255, 255, 0.75 },
        { 0.52, 0.88, 1.0, 255, 255, 255, 0.70 },
        { 0.07, 0.79, 1.5, 200, 251, 222, 0.85 },   // pale mint
    };
    const int tile = 420;

    QPixmap pixmap(tile, tile);
    pixmap.fill(space);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);
    for (unsigned int i = 0; i < sizeof(stars) / sizeof(stars[0]); i++)
    {
        const Star &star = stars[i];
        QColor color(star.r, star.g, star.b);
        color.setAlphaF(star.alpha);
        painter.setBrush(color);
        const double radius = star.size / 2.0 + 0.25;
        painter.drawEllipse(QPointF(star.x * tile, star.y * tile), radius, radius);
    }
    painter.end();

    return pixmap;
}

// ---------- the dark built-in themes ----------

static void drawKosmosFallback(QPainter &painter, const QRect &rect);
static void drawFlappyNightScene(QPainter &painter, const QRect &rect);
static void drawTerminalScene(QPainter &painter, const QRect &rect);

// The dark built-in themes, in Themes-menu order. Each one is the FlapXCoin layout
// (res/flapxcoin.qss) with its own variables (res/<theme>.qss), palette and background.
struct DarkTheme
{
    const char *name;        // in the Themes menu (UTF-8)
    const char *qss;         // its variables, and any extra rules
    const char *picture;     // background picture, or 0
    void (*scene)(QPainter &, const QRect &);   // drawn in code: the background if there's no picture, or if it's missing
    bool stars;              // starfield behind dialogs
    // palette
    QRgb space, space2, panel, border, light, mid, dark, text, disabled;
    QRgb accent, accent2, highlight, bright;
    // colours set in code (the traffic graph uses space and border)
    QRgb positive, negative, negativeAmount, muted;
};

static const DarkTheme darkThemes[] = {
    // FlapXCoin: the moon base, a rocket lifting off and a Voyager-style probe;
    // Flappy Bird yellow and FlapX green
    { "FlapXCoin", ":/themes/flapxcoin.qss", ":/themes/flapx.jpg", 0, true,
      0x0b1220, 0x111a2e, 0x15203a, 0x24345a, 0x2c3e68, 0x1b2848, 0x070c17, 0xffffff, 0x5b6a86,
      0xfcd535, 0x22f07a, 0x1f6f80, 0xff6b81,
      0x22f07a, 0xff6b81, 0xffffff, 0xced6e4 },
    // Космос: Soyuz, Sputnik and Venera; Soviet red and star gold
    { "\xd0\x9a\xd0\xbe\xd1\x81\xd0\xbc\xd0\xbe\xd1\x81", ":/themes/kosmos.qss", ":/themes/kosmos.jpg", drawKosmosFallback, true,
      0x0f1430, 0x161c3e, 0x1d2450, 0x3a3f78, 0x484e8c, 0x282d5c, 0x090c1e, 0xffffff, 0x6e6a8c,
      0xe53935, 0xf9c80e, 0x961e1e, 0xff5a5f,
      0xf9c80e, 0xff8a80, 0xffffff, 0xd6d0e2 },
    // Mars Colony: domes and a rocket at dusk; dusty orange, sand gold and cyan
    { "Mars Colony", ":/themes/mars.qss", ":/themes/mars.jpg", 0, true,
      0x1a100c, 0x24160f, 0x2e1c14, 0x5c3a28, 0x6e4632, 0x3a2418, 0x0e0806, 0xffffff, 0x7a6458,
      0xe07a3f, 0x4dd0e1, 0x8a3f1c, 0xff6b5a,
      0x4dd0e1, 0xff7a6a, 0xffffff, 0xe2d2c6 },
    // Synthwave: neon sunset over the grid; hot pink, cyan and orange
    { "Synthwave", ":/themes/synthwave.qss", ":/themes/synthwave.jpg", 0, true,
      0x140824, 0x1c0c30, 0x261040, 0x542480, 0x642c96, 0x30144e, 0x0a0414, 0xffffff, 0x7a6290,
      0xff2e97, 0x22e5ff, 0x8a1a66, 0xff4d6d,
      0x22e5ff, 0xff6b8b, 0xffffff, 0xe0d0ee },
    // Deep Sea: jellyfish, coral and a yellow submarine; jellyfish cyan and coral
    { "Deep Sea", ":/themes/deepsea.qss", ":/themes/deepsea.jpg", 0, false,
      0x04141f, 0x061c2c, 0x0a263a, 0x184a64, 0x1e5a78, 0x0e3044, 0x020a10, 0xffffff, 0x50707e,
      0x3ee8e0, 0xff7f6a, 0x0f6a78, 0xff6b81,
      0xff7f6a, 0xff8a80, 0xffffff, 0xc8e0e6 },
    // Winter: a snowy moon base under the Earth; icy cyan, gold and holiday green
    { "Winter", ":/themes/winter.qss", ":/themes/winter.jpg", 0, true,
      0x0a101e, 0x0e1628, 0x141e36, 0x344a6e, 0x3e5a84, 0x1c2a46, 0x050810, 0xffffff, 0x5e6e88,
      0x8fe3ff, 0x3ddc84, 0x1f5f8a, 0xff6b81,
      0x3ddc84, 0xff7b7b, 0xffffff, 0xd6e2f0 },
    // DaGoofMan: the crowned fortress and its fleet; his orange
    { "DaGoofMan", ":/themes/dagoofman.qss", ":/themes/dagoofman.jpg", 0, true,
      0x0e0a0a, 0x16100e, 0x201612, 0x50301e, 0x603a24, 0x2a1c14, 0x060404, 0xffffff, 0x6e5e56,
      0xff7a1a, 0xffb347, 0x8a3a08, 0xff5a4a,
      0xffb347, 0xff6b5a, 0xffffff, 0xe6d6cc },
    // Flappy Night: the FlappyCoin city after dark; yellow and pipe green
    { "Flappy Night", ":/themes/flappynight.qss", 0, drawFlappyNightScene, true,
      0x0e1630, 0x141e3e, 0x1a284e, 0x344878, 0x40568a, 0x1e2c56, 0x080c1a, 0xffffff, 0x5e6a8a,
      0xfcd535, 0x8ee05a, 0x3a6a1e, 0xff6b6b,
      0x8ee05a, 0xff7b7b, 0xffffff, 0xd4dcec },
    // Terminal: green on black, monospace, scan lines
    { "Terminal", ":/themes/terminal.qss", 0, drawTerminalScene, false,
      0x000000, 0x030a04, 0x061208, 0x145a22, 0x1a7a2e, 0x0a2a10, 0x000000, 0x33ff66, 0x1a6a30,
      0x33ff66, 0xb8ff5c, 0x0f5a22, 0xff5555,
      0x7dff9a, 0xff5555, 0x33ff66, 0x22b84a },
};
static const int nDarkThemes = sizeof(darkThemes) / sizeof(darkThemes[0]);
static int nDarkTheme = 0;

int darkThemeCount()
{
    return nDarkThemes;
}

QString darkThemeName(int index)
{
    if (index < 0 || index >= nDarkThemes)
        return QString();
    return QString::fromUtf8(darkThemes[index].name);
}

int darkThemeIndex(const QString &name)
{
    for (int i = 0; i < nDarkThemes; i++)
        if (name == QString::fromUtf8(darkThemes[i].name))
            return i;
    return -1;
}

void applyDarkTheme(int index)
{
    if (index < 0 || index >= nDarkThemes)
        index = 0;
    nDarkTheme = index;
    const DarkTheme &t = darkThemes[index];

    if (QStyle *style = QStyleFactory::create("Fusion"))
        QApplication::setStyle(style);

    const QColor space(t.space), text(t.text), disabled(t.disabled), border(t.border);
    QPalette p;
    p.setColor(QPalette::Window, space);
    if (t.stars)
    {
        // dialogs get a starfield too; the main window draws the full scene itself
        p.setBrush(QPalette::Active, QPalette::Window, QBrush(flapxStarfieldTile(space)));
        p.setBrush(QPalette::Inactive, QPalette::Window, QBrush(flapxStarfieldTile(space)));
    }
    p.setColor(QPalette::WindowText, text);
    p.setColor(QPalette::Base, QColor(t.space2));
    p.setColor(QPalette::AlternateBase, QColor(t.panel));
    p.setColor(QPalette::ToolTipBase, QColor(t.space2));
    p.setColor(QPalette::ToolTipText, text);
    p.setColor(QPalette::Text, text);
    p.setColor(QPalette::Button, QColor(t.panel));
    p.setColor(QPalette::ButtonText, text);
    p.setColor(QPalette::BrightText, QColor(t.bright));
    p.setColor(QPalette::Link, QColor(t.accent));
    p.setColor(QPalette::LinkVisited, QColor(t.accent2));
    p.setColor(QPalette::Highlight, QColor(t.highlight));
    p.setColor(QPalette::HighlightedText, text);
    p.setColor(QPalette::Light, QColor(t.light));
    p.setColor(QPalette::Midlight, border);
    p.setColor(QPalette::Mid, QColor(t.mid));
    p.setColor(QPalette::Dark, QColor(t.dark));
    p.setColor(QPalette::Shadow, Qt::black);
    p.setColor(QPalette::Disabled, QPalette::WindowText, disabled);
    p.setColor(QPalette::Disabled, QPalette::Text, disabled);
    p.setColor(QPalette::Disabled, QPalette::ButtonText, disabled);
    p.setColor(QPalette::Disabled, QPalette::Highlight, border);
    QApplication::setPalette(p);
}

static QString readTextFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QFile::ReadOnly))
        return QString();
    return QString::fromUtf8(file.readAll());
}

// Fill in a style sheet's [VARS] variables and $theme-dir
static QString fillStyleSheet(QString styleSheet, const QString &themeDir)
{
    // Variables are declared between "/** [VARS]" and "[/VARS] */", one "@name = value" per line
    QMap<QString, QString> variables;
    bool readingVariables = false;
    foreach (const QString &rawLine, styleSheet.split('\n'))
    {
        const QString line = rawLine.trimmed();
        if (line == "/** [VARS]") {
            readingVariables = true;
            continue;
        }
        if (line == "[/VARS] */")
            break;
        if (readingVariables && line.length() > 3 && line.contains('=')) {
            const int eq = line.indexOf('=');
            variables[line.left(eq).trimmed()] = line.mid(eq + 1).trimmed();
        }
    }

    // $theme-dir lets a theme point at its own files, e.g. url($theme-dir/image.png)
    styleSheet.replace("$theme-dir", themeDir);

    // Longest names first (reverse map order), so @space2 is replaced before @space
    QMapIterator<QString, QString> variable(variables);
    variable.toBack();
    while (variable.hasPrevious()) {
        variable.previous();
        styleSheet.replace(variable.key(), variable.value());
    }

    return styleSheet;
}

QString loadStyleSheet(const QString &path, const QString &themeDir)
{
    QString styleSheet = readTextFile(path);
    if (styleSheet.isEmpty())
        return QString();
    return fillStyleSheet(styleSheet, themeDir);
}

QString darkThemeStyleSheet(int index)
{
    if (index < 0 || index >= nDarkThemes)
        index = 0;
    // The FlapXCoin layout is everything after its variables; the theme's own file
    // (its variables, and any extra rules) goes after the layout, so its rules win
    QString layout = readTextFile(":/themes/flapxcoin.qss");
    const QString endVars = "[/VARS] */";
    const int cutAt = layout.indexOf(endVars);
    if (cutAt < 0)
        return QString();
    if (index == 0)
        return fillStyleSheet(layout, ":/themes");
    QString own = readTextFile(QString::fromUtf8(darkThemes[index].qss));
    if (own.isEmpty())
        return QString();
    return fillStyleSheet(own + "\n" + layout.mid(cutAt + endVars.length()), ":/themes");
}

QString flappyStyleSheet()
{
    return loadStyleSheet(":/themes/flappycoin.qss", ":/themes");
}

QString plainStyleSheet()
{
    return loadStyleSheet(":/themes/plain.qss", ":/themes");
}

void applyPlainPalette()
{
    // Plain grey and black: Qt's Fusion style with its standard light palette
    if (QStyle *style = QStyleFactory::create("Fusion"))
    {
        QApplication::setStyle(style);
        QApplication::setPalette(style->standardPalette());
    }
}

static ThemeLook nThemeLook = LOOK_DARK;

void setThemeLook(ThemeLook look)
{
    nThemeLook = look;
}

ThemeLook themeLook()
{
    return nThemeLook;
}

bool isFlappyTheme()
{
    return nThemeLook == LOOK_FLAPPY;
}

QColor themeColor(ThemeColor which)
{
    if (nThemeLook == LOOK_NONE)
    {
        switch (which)
        {
        case TC_POSITIVE:        return QColor(0, 110, 0);
        case TC_NEGATIVE:        return QColor(180, 0, 0);
        case TC_NEGATIVE_AMOUNT: return QColor(0, 0, 0);
        case TC_MUTED:           return QColor(96, 96, 96);
        case TC_ACCENT:          return QColor(40, 40, 40);
        case TC_ACCENT2:         return QColor(110, 110, 110);
        case TC_GRAPH_BG:        return QColor(255, 255, 255);
        case TC_GRAPH_GRID:      return QColor(200, 200, 200);
        }
    }
    if (nThemeLook == LOOK_FLAPPY)
    {
        switch (which)
        {
        case TC_POSITIVE:        return QColor(47, 125, 42);    // bush green, darkened for text
        case TC_NEGATIVE:        return QColor(210, 60, 60);
        case TC_NEGATIVE_AMOUNT: return QColor(43, 58, 31);     // normal text
        case TC_MUTED:           return QColor(94, 115, 86);
        case TC_ACCENT:          return QColor(240, 130, 30);   // orange
        case TC_ACCENT2:         return QColor(47, 125, 42);
        case TC_GRAPH_BG:        return QColor(253, 255, 247);
        case TC_GRAPH_GRID:      return QColor(156, 201, 143);
        }
    }
    // the dark themes (and themes from a themes folder, on FlapXCoin's colours)
    const DarkTheme &t = darkThemes[nDarkTheme];
    switch (which)
    {
    case TC_POSITIVE:        return QColor(t.positive);
    case TC_NEGATIVE:        return QColor(t.negative);
    case TC_NEGATIVE_AMOUNT: return QColor(t.negativeAmount);
    case TC_MUTED:           return QColor(t.muted);
    case TC_ACCENT:          return QColor(t.accent);
    case TC_ACCENT2:         return QColor(t.accent2);
    case TC_GRAPH_BG:        return QColor(t.space);
    case TC_GRAPH_GRID:      return QColor(t.border);
    }
    return QColor();
}

QString positiveColor()
{
    return themeColor(TC_POSITIVE).name();
}

QString negativeColor()
{
    return themeColor(TC_NEGATIVE).name();
}

QString mutedColor()
{
    return themeColor(TC_MUTED).name();
}

void applyTextOutline(QWidget *widget)
{
    if (!widget)
        return;
    if (nThemeLook == LOOK_FLAPPY)
    {
        if (!widget->graphicsEffect())
        {
            QGraphicsDropShadowEffect *outline = new QGraphicsDropShadowEffect(widget);
            outline->setBlurRadius(0);
            outline->setOffset(1.5, 1.5);
            outline->setColor(QColor(84, 56, 71));   // Flappy Bird's dark brown outline
            widget->setGraphicsEffect(outline);
        }
    }
    else if (widget->graphicsEffect())
    {
        widget->setGraphicsEffect(0);   // deletes the effect
    }
}

void applyFlappyPalette()
{
    if (QStyle *style = QStyleFactory::create("Fusion"))
        QApplication::setStyle(style);

    const QColor sky(112, 197, 206), cream(244, 251, 233), cream2(253, 255, 247);
    const QColor text(43, 58, 31), outline(81, 127, 75), border(156, 201, 143);
    const QColor disabled(140, 155, 132);

    QPalette p;
    p.setColor(QPalette::Window, sky);
    p.setColor(QPalette::WindowText, text);
    p.setColor(QPalette::Base, cream2);
    p.setColor(QPalette::AlternateBase, cream);
    p.setColor(QPalette::ToolTipBase, cream2);
    p.setColor(QPalette::ToolTipText, text);
    p.setColor(QPalette::Text, text);
    p.setColor(QPalette::Button, cream);
    p.setColor(QPalette::ButtonText, text);
    p.setColor(QPalette::BrightText, QColor(210, 60, 60));
    p.setColor(QPalette::Link, QColor(200, 97, 12));
    p.setColor(QPalette::LinkVisited, outline);
    p.setColor(QPalette::Highlight, QColor(249, 194, 127));
    p.setColor(QPalette::HighlightedText, text);
    p.setColor(QPalette::Light, Qt::white);
    p.setColor(QPalette::Midlight, cream);
    p.setColor(QPalette::Mid, border);
    p.setColor(QPalette::Dark, outline);
    p.setColor(QPalette::Shadow, QColor(43, 58, 31));
    p.setColor(QPalette::Disabled, QPalette::WindowText, disabled);
    p.setColor(QPalette::Disabled, QPalette::Text, disabled);
    p.setColor(QPalette::Disabled, QPalette::ButtonText, disabled);
    p.setColor(QPalette::Disabled, QPalette::Highlight, border);
    QApplication::setPalette(p);
}

// One 192 px wide strip of the FlappyCoin scenery: clouds, a pale city, and
// bushes, in the style of the original wallet's footer. Drawn once, then tiled.
static QPixmap flappySceneryTile()
{
    const int w = 192, h = 96;
    QPixmap pixmap(w, h);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    p.setPen(Qt::NoPen);

    // Clouds: cream puffs with a pale edge, along the top of the strip
    const QColor cloudEdge(205, 236, 214), cloud(233, 250, 217);
    static const int puffs[][3] = { {10,30,22}, {38,22,26}, {70,30,20}, {96,26,24}, {126,20,28},
                                    {158,28,22}, {182,24,24}, {-8,24,24}, {202,30,22} };
    for (unsigned int i = 0; i < sizeof(puffs) / sizeof(puffs[0]); i++)
    {
        p.setBrush(cloudEdge);
        p.drawEllipse(QPoint(puffs[i][0], puffs[i][1]), puffs[i][2] + 2, puffs[i][2] + 2);
    }
    for (unsigned int i = 0; i < sizeof(puffs) / sizeof(puffs[0]); i++)
    {
        p.setBrush(cloud);
        p.drawEllipse(QPoint(puffs[i][0], puffs[i][1]), puffs[i][2], puffs[i][2]);
    }
    p.fillRect(0, 34, w, h - 34, cloud);

    // City: pale green-blue buildings with rows of windows
    const QColor bldEdge(170, 221, 212), bld(214, 240, 222), win(170, 221, 212);
    static const int towers[][3] = { {4,34,22}, {28,24,16}, {46,40,24}, {72,18,18}, {92,32,20},
                                     {114,26,26}, {142,38,18}, {162,22,26} };   // x, top, width
    for (unsigned int i = 0; i < sizeof(towers) / sizeof(towers[0]); i++)
    {
        const int x = towers[i][0], top = towers[i][1], tw = towers[i][2];
        p.fillRect(x, top, tw, h - top, bldEdge);
        p.fillRect(x + 2, top + 2, tw - 4, h - top - 2, bld);
        for (int y = top + 6; y < h - 30; y += 7)
            for (int wx = x + 4; wx + 3 < x + tw - 2; wx += 6)
                p.fillRect(wx, y, 3, 3, win);
    }

    // Bushes: rounded green mounds with a darker outline, across the bottom
    const QColor bushEdge(111, 194, 150), bush(135, 226, 148), bushLight(160, 234, 168);
    static const int mounds[][3] = { {0,74,20}, {28,70,22}, {58,76,18}, {84,70,24}, {116,74,20},
                                     {144,68,24}, {176,74,20}, {200,72,20} };
    for (unsigned int i = 0; i < sizeof(mounds) / sizeof(mounds[0]); i++)
    {
        p.setBrush(bushEdge);
        p.drawEllipse(QPoint(mounds[i][0], mounds[i][1]), mounds[i][2] + 2, mounds[i][2] + 2);
    }
    for (unsigned int i = 0; i < sizeof(mounds) / sizeof(mounds[0]); i++)
    {
        p.setBrush(bush);
        p.drawEllipse(QPoint(mounds[i][0], mounds[i][1]), mounds[i][2], mounds[i][2]);
        p.setBrush(bushLight);
        p.drawEllipse(QPoint(mounds[i][0] - mounds[i][2] / 3, mounds[i][1] - mounds[i][2] / 3), 4, 3);
    }
    p.fillRect(0, 80, w, h - 80, bush);
    p.end();
    return pixmap;
}

void drawFlappyScenery(QPainter &painter, const QRect &rect)
{
    static QPixmap tile = flappySceneryTile();

    // Sky: a little lighter towards the top, like the game
    QLinearGradient sky(rect.topLeft(), rect.bottomLeft());
    sky.setColorAt(0.0, QColor(132, 207, 214));
    sky.setColorAt(1.0, QColor(112, 197, 206));
    painter.fillRect(rect, sky);

    // Scenery strip along the bottom
    const int y = rect.bottom() + 1 - tile.height();
    painter.drawTiledPixmap(QRect(rect.left(), y, rect.width(), tile.height()), tile);
}

// pi, without relying on M_PI (not defined in strict C++ mode on Windows)
static const double KOSMOS_PI = 3.14159265358979323846;

// A four-pointed sparkle, for the brighter stars
static void drawSparkle(QPainter &p, const QPointF &c, double r)
{
    QPolygonF star;
    for (int i = 0; i < 8; i++)
    {
        const double a = i * KOSMOS_PI / 4.0 - KOSMOS_PI / 2.0;
        const double d = (i % 2 == 0) ? r : r * 0.25;
        star << QPointF(c.x() + d * std::cos(a), c.y() + d * std::sin(a));
    }
    p.drawPolygon(star);
}

// Soyuz, facing right, centred on the descent module: service module with
// its solar panels and "СССР", the bell-shaped descent module and the round
// orbital module with its docking probe
static void drawSoyuz(QPainter &p)
{
    // solar panels, with gold edges and a grid
    const QColor panel(36, 64, 128), grid(242, 177, 52);
    for (int side = -1; side <= 1; side += 2)
    {
        const QRectF wing(-27, side < 0 ? -41 : 11, 20, 30);
        p.setPen(QPen(QColor(150, 150, 160), 1.2));
        p.drawLine(QPointF(-17, side * 8), QPointF(-17, side * 11));
        p.setPen(QPen(grid, 1));
        p.setBrush(panel);
        p.drawRect(wing);
        p.setPen(QPen(QColor(242, 177, 52, 120), 0.6));
        for (int i = 1; i < 4; i++)
            p.drawLine(QPointF(wing.left(), wing.top() + i * wing.height() / 4), QPointF(wing.right(), wing.top() + i * wing.height() / 4));
        p.drawLine(QPointF(wing.center().x(), wing.top()), QPointF(wing.center().x(), wing.bottom()));
    }
    p.setPen(Qt::NoPen);

    // engine nozzle and service module
    p.setBrush(QColor(90, 94, 104));
    QPolygonF nozzle;
    nozzle << QPointF(-33, -4) << QPointF(-40, -7) << QPointF(-40, 7) << QPointF(-33, 4);
    p.drawPolygon(nozzle);
    QLinearGradient body(QPointF(0, -9), QPointF(0, 9));
    body.setColorAt(0.0, QColor(150, 166, 122));
    body.setColorAt(1.0, QColor(96, 110, 78));
    p.setBrush(body);
    p.drawRoundedRect(QRectF(-34, -9, 34, 18), 3, 3);

    QFont font;
    font.setPixelSize(7);
    font.setBold(true);
    p.setFont(font);
    p.setPen(QColor(214, 40, 40));
    p.drawText(QRectF(-33, -6, 32, 12), Qt::AlignCenter, QString::fromUtf8("СССР"));
    p.setPen(Qt::NoPen);

    // descent module (bell) and orbital module (sphere)
    QLinearGradient bell(QPointF(0, -9), QPointF(0, 9));
    bell.setColorAt(0.0, QColor(222, 214, 184));
    bell.setColorAt(1.0, QColor(160, 152, 126));
    p.setBrush(bell);
    QPainterPath descent;
    descent.moveTo(0, -9);
    descent.lineTo(13, -6);
    descent.quadTo(16, 0, 13, 6);
    descent.lineTo(0, 9);
    descent.closeSubpath();
    p.drawPath(descent);
    QRadialGradient orbital(QPointF(21, -3), 12);
    orbital.setColorAt(0.0, QColor(214, 212, 180));
    orbital.setColorAt(1.0, QColor(128, 142, 104));
    p.setBrush(orbital);
    p.drawEllipse(QPointF(24, 0), 9, 9);

    p.setPen(QPen(QColor(190, 190, 200), 1.2, Qt::SolidLine, Qt::RoundCap));
    p.drawLine(QPointF(33, 0), QPointF(40, 0));          // docking probe
    p.drawLine(QPointF(24, -9), QPointF(28, -16));       // antennas
    p.drawLine(QPointF(-6, 9), QPointF(-10, 16));
    p.setPen(Qt::NoPen);
}

// The Venera descent probe under its striped parachute: aerobrake disc on
// top, the round pressure sphere, and the landing ring underneath
static void drawVenera(QPainter &p)
{
    const QRectF canopy(-24, -62, 48, 30);
    static const QColor gores[2] = { QColor(248, 150, 30), QColor(246, 236, 220) };
    for (int i = 0; i < 6; i++)
    {
        p.setBrush(gores[i % 2]);
        p.drawPie(canopy, (int)(i * 30 * 16), 30 * 16);
    }
    p.setPen(QPen(QColor(246, 236, 220, 170), 0.8));
    static const double rim[] = { -23, -11, 0, 11, 23 };
    for (unsigned int i = 0; i < sizeof(rim) / sizeof(rim[0]); i++)
        p.drawLine(QPointF(rim[i], -47), QPointF(0, -15));
    p.setPen(Qt::NoPen);

    p.setBrush(QColor(170, 176, 190));
    p.drawRect(QRectF(-1.5, -14, 3, 5));                  // mast
    p.setBrush(QColor(200, 206, 218));
    p.drawEllipse(QPointF(0, -14), 14, 3.5);              // aerobrake disc
    QRadialGradient sphere(QPointF(-3, -3), 13);
    sphere.setColorAt(0.0, QColor(246, 240, 222));
    sphere.setColorAt(1.0, QColor(160, 150, 128));
    p.setBrush(sphere);
    p.drawEllipse(QPointF(0, 0), 10, 10);
    p.setBrush(QColor(116, 124, 146));
    p.drawEllipse(QPointF(0, 10), 17, 5);                 // landing ring
    p.setBrush(QColor(80, 86, 106));
    p.drawEllipse(QPointF(0, 10), 11, 2.5);
}

static void drawSputnik(QPainter &p)
{
    p.setPen(QPen(QColor(200, 200, 210), 1.3, Qt::SolidLine, Qt::RoundCap));
    static const double antennas[][2] = { {-30, 7}, {-27, 13}, {-31, 1}, {-24, 19} };
    for (unsigned int i = 0; i < sizeof(antennas) / sizeof(antennas[0]); i++)
        p.drawLine(QPointF(0, 0), QPointF(antennas[i][0], antennas[i][1]));
    p.setPen(Qt::NoPen);
    QRadialGradient sphere(QPointF(-2.5, -2.5), 9);
    sphere.setColorAt(0.0, QColor(255, 244, 214));
    sphere.setColorAt(1.0, QColor(170, 150, 110));
    p.setBrush(sphere);
    p.drawEllipse(QPointF(0, 0), 7, 7);
}

// The drawn version of the Kosmos scene, used if the picture can't be loaded
static void drawKosmosFallback(QPainter &painter, const QRect &rect)
{
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);
    const double w = rect.width(), h = rect.height();
    const double x0 = rect.left(), y0 = rect.top();

    // Deep space: indigo at the top, a little bluer near the planets
    QLinearGradient space(rect.topLeft(), rect.bottomLeft());
    space.setColorAt(0.0, QColor(12, 16, 41));
    space.setColorAt(0.65, QColor(21, 27, 69));
    space.setColorAt(1.0, QColor(29, 45, 85));
    painter.fillRect(rect, space);

    // A soft violet nebula, upper right
    QRadialGradient nebula(QPointF(x0 + 0.78 * w, y0 + 0.22 * h), 0.55 * std::max(w, h));
    nebula.setColorAt(0.0, QColor(120, 60, 150, 55));
    nebula.setColorAt(1.0, QColor(120, 60, 150, 0));
    painter.fillRect(rect, nebula);

    // Stars: the same scattering every time (a fixed pseudo-random sequence)
    quint32 seed = 1957;   // the year of Sputnik
    for (int i = 0; i < 170; i++)
    {
        seed = seed * 1103515245u + 12345u; const double fx = ((seed >> 8) & 0xffff) / 65535.0;
        seed = seed * 1103515245u + 12345u; const double fy = ((seed >> 8) & 0xffff) / 65535.0;
        seed = seed * 1103515245u + 12345u; const int bright = (seed >> 8) & 0xff;
        QColor star = (bright % 5 == 0) ? QColor(255, 209, 102) : QColor(246, 236, 220);
        star.setAlpha(90 + bright * 150 / 255);
        painter.setBrush(star);
        const double r = 0.5 + (bright % 3) * 0.35;
        painter.drawEllipse(QPointF(x0 + fx * w, y0 + fy * h), r, r);
    }
    painter.setBrush(QColor(255, 209, 102, 220));
    static const double sparkles[][3] = { {0.14, 0.18, 6}, {0.43, 0.10, 4.5}, {0.62, 0.34, 5}, {0.90, 0.12, 6},
                                          {0.30, 0.47, 4}, {0.56, 0.62, 5.5}, {0.05, 0.55, 4.5} };
    for (unsigned int i = 0; i < sizeof(sparkles) / sizeof(sparkles[0]); i++)
        drawSparkle(painter, QPointF(x0 + sparkles[i][0] * w, y0 + sparkles[i][1] * h), sparkles[i][2]);

    // Orbit lines: two wide dashed ellipses sweeping across
    painter.setBrush(Qt::NoBrush);
    QPen orbitPen(QColor(246, 236, 220, 55), 1.2, Qt::DashLine);
    painter.setPen(orbitPen);
    painter.drawEllipse(QPointF(x0 + 0.30 * w, y0 + h * 1.02), 0.62 * w, 0.42 * h);
    painter.drawEllipse(QPointF(x0 + 0.86 * w, y0 + h * 1.08), 0.55 * w, 0.30 * h);
    painter.setPen(Qt::NoPen);

    // The Earth, bottom left: teal oceans, a pale atmosphere
    const double rEarth = std::max(110.0, std::min(300.0, 0.40 * h));
    const QPointF earth(x0 + 0.07 * w, y0 + h + 0.42 * rEarth);
    QRadialGradient halo(earth, rEarth * 1.14);
    halo.setColorAt(0.86, QColor(46, 196, 182, 90));
    halo.setColorAt(1.0, QColor(46, 196, 182, 0));
    painter.setBrush(halo);
    painter.drawEllipse(earth, rEarth * 1.14, rEarth * 1.14);
    QRadialGradient ocean(earth + QPointF(0.35 * rEarth, -0.55 * rEarth), 1.4 * rEarth);
    ocean.setColorAt(0.0, QColor(74, 214, 198));
    ocean.setColorAt(0.5, QColor(30, 120, 150));
    ocean.setColorAt(1.0, QColor(16, 54, 92));
    painter.setBrush(ocean);
    painter.drawEllipse(earth, rEarth, rEarth);
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(QColor(246, 236, 220, 70), 3, Qt::SolidLine, Qt::RoundCap));
    painter.drawArc(QRectF(earth.x() - 0.75 * rEarth, earth.y() - 0.90 * rEarth, 1.2 * rEarth, 0.5 * rEarth), 20 * 16, 120 * 16);
    painter.drawArc(QRectF(earth.x() - 0.30 * rEarth, earth.y() - 0.78 * rEarth, 1.0 * rEarth, 0.4 * rEarth), 30 * 16, 110 * 16);
    painter.setPen(Qt::NoPen);

    // Venus, bottom right: a retro striped disc in mustard, orange and red
    const double rVenus = std::max(130.0, std::min(340.0, 0.46 * h));
    const QPointF venus(x0 + 0.93 * w, y0 + h + 0.38 * rVenus);
    QRadialGradient glow(venus, rVenus * 1.25);
    glow.setColorAt(0.78, QColor(248, 150, 30, 70));
    glow.setColorAt(1.0, QColor(248, 150, 30, 0));
    painter.setBrush(glow);
    painter.drawEllipse(venus, rVenus * 1.25, rVenus * 1.25);
    const double venusTop = venus.y() - rVenus;
    QLinearGradient bands(QPointF(0, venusTop), QPointF(0, y0 + h));
    bands.setColorAt(0.0, QColor(255, 209, 102));
    bands.setColorAt(0.55, QColor(248, 150, 30));
    bands.setColorAt(1.0, QColor(228, 87, 46));
    painter.setBrush(bands);
    painter.drawEllipse(venus, rVenus, rVenus);
    {
        // the 70s stripes across the lower part, getting wider towards the bottom
        QPainterPath disc;
        disc.addEllipse(venus, rVenus, rVenus);
        painter.save();
        painter.setClipPath(disc);
        const double visible = y0 + h - venusTop;
        for (int i = 0; i < 6; i++)
        {
            const double y = venusTop + visible * (0.42 + i * 0.105);
            painter.fillRect(QRectF(venus.x() - rVenus, y, 2 * rVenus, 1.5 + i * 1.6), QColor(29, 45, 85));
        }
        painter.restore();
    }

    // Venera, parachuting down towards Venus
    painter.save();
    painter.translate(x0 + 0.80 * w, venusTop - 26);
    painter.scale(1.35, 1.35);
    drawVenera(painter);
    painter.restore();

    // Soyuz in orbit above the Earth
    painter.save();
    painter.translate(x0 + 0.30 * w, y0 + h - std::min(0.36 * h, 0.95 * rEarth));
    painter.rotate(-8);
    painter.scale(1.5, 1.5);
    drawSoyuz(painter);
    painter.restore();

    // Sputnik, further out
    painter.save();
    painter.translate(x0 + 0.53 * w, y0 + h - 0.24 * h);
    drawSputnik(painter);
    painter.restore();

    painter.restore();
}

// Draw a background picture filling rect, keeping its shape and anchored to the
// bottom, so what's at the bottom of the picture always sits along the bottom
// edge; anything that doesn't fit is cut off at the top and the sides. The
// scaled copy is kept per window size. Returns false if the picture is missing.
static bool drawBottomAnchored(QPainter &painter, const QRect &rect, const QImage &picture,
                               QPixmap &scaled, QSize &scaledFor)
{
    if (picture.isNull() || rect.isEmpty())
        return false;
    if (scaledFor != rect.size())
    {
        QImage fill = picture.scaled(rect.size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        const int x = (fill.width() - rect.width()) / 2;
        const int y = fill.height() - rect.height();
        scaled = QPixmap::fromImage(fill.copy(x, y, rect.width(), rect.height()));
        scaledFor = rect.size();
    }
    painter.drawPixmap(rect.topLeft(), scaled);
    return true;
}

// Flappy Night: the FlappyCoin city and bushes after dark, under the stars and a
// crescent moon
static void drawFlappyNightScene(QPainter &painter, const QRect &rect)
{
    static QPixmap tile = flappySceneryTile();
    painter.save();

    QLinearGradient sky(rect.topLeft(), rect.bottomLeft());
    sky.setColorAt(0.0, QColor(7, 11, 30));
    sky.setColorAt(0.65, QColor(20, 30, 70));
    sky.setColorAt(1.0, QColor(38, 52, 102));
    painter.fillRect(rect, sky);

    // Stars: the same scatter every time, fewer near the horizon
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);
    quint32 seed = 20141;
    const int count = rect.width() * rect.height() / 2600;
    for (int i = 0; i < count; i++)
    {
        seed = seed * 1103515245u + 12345u; const double x = (seed >> 8) % 10000 / 10000.0;
        seed = seed * 1103515245u + 12345u; const double y = (seed >> 8) % 10000 / 10000.0;
        seed = seed * 1103515245u + 12345u; const int bright = 120 + (seed >> 8) % 136;
        const double fy = y * y;   // bunch them towards the top
        QColor star(255, 255, 235, (int)(bright * (1.0 - fy * 0.8)));
        painter.setBrush(star);
        const double r = (bright > 235) ? 1.3 : 0.8;
        painter.drawEllipse(QPointF(rect.left() + x * rect.width(), rect.top() + fy * rect.height() * 0.8), r, r);
    }

    // Crescent moon, top right, with a soft glow
    const QPointF moon(rect.left() + rect.width() * 0.82, rect.top() + rect.height() * 0.16);
    const double mr = qMax(22.0, rect.height() * 0.05);
    QRadialGradient glow(moon, mr * 3.5);
    glow.setColorAt(0.0, QColor(255, 244, 190, 70));
    glow.setColorAt(1.0, QColor(255, 244, 190, 0));
    painter.setBrush(glow);
    painter.drawEllipse(moon, mr * 3.5, mr * 3.5);
    QPainterPath disc, bite;
    disc.addEllipse(moon, mr, mr);
    bite.addEllipse(moon + QPointF(mr * 0.45, -mr * 0.25), mr * 0.85, mr * 0.85);
    painter.setBrush(QColor(255, 240, 170));
    painter.drawPath(disc.subtracted(bite));

    // The scenery strip, in moonlight
    const int y = rect.bottom() + 1 - tile.height();
    const QRect strip(rect.left(), y, rect.width(), tile.height());
    painter.drawTiledPixmap(strip, tile);
    painter.fillRect(strip, QColor(10, 18, 52, 165));
    painter.restore();
}

// Terminal: a black screen with faint green scan lines and a dim glow in the middle
static void drawTerminalScene(QPainter &painter, const QRect &rect)
{
    painter.save();
    painter.fillRect(rect, Qt::black);
    QRadialGradient glow(rect.center(), qMax(rect.width(), rect.height()) * 0.7);
    glow.setColorAt(0.0, QColor(0, 46, 14));
    glow.setColorAt(1.0, QColor(0, 0, 0));
    painter.fillRect(rect, glow);
    painter.setPen(QColor(51, 255, 102, 14));
    for (int y = rect.top(); y <= rect.bottom(); y += 3)
        painter.drawLine(rect.left(), y, rect.right(), y);

    // a prompt in the bottom corner
    QFont font("DejaVu Sans Mono");
    font.setStyleHint(QFont::Monospace);
    font.setPixelSize(14);
    painter.setFont(font);
    painter.setPen(QColor(51, 255, 102, 70));
    painter.drawText(QRect(rect.left() + 12, rect.top(), rect.width() - 24, rect.height() - 10),
                     Qt::AlignLeft | Qt::AlignBottom, "flapx@node:~$ _");
    painter.restore();
}

bool drawDarkThemeScene(QPainter &painter, const QRect &rect)
{
    // The picture or the drawn scene, kept per theme and window size
    static QImage picture;
    static int pictureFor = -1;
    static QPixmap scaled;
    static QSize scaledFor;
    static int scaledTheme = -1;

    const DarkTheme &t = darkThemes[nDarkTheme];
    if (rect.isEmpty())
        return false;
    if (scaledTheme != nDarkTheme)
    {
        scaledFor = QSize();
        scaledTheme = nDarkTheme;
    }
    if (t.picture)
    {
        if (pictureFor != nDarkTheme)
        {
            picture = QImage(QString::fromUtf8(t.picture));
            pictureFor = nDarkTheme;
        }
        if (drawBottomAnchored(painter, rect, picture, scaled, scaledFor))
            return true;
    }
    if (!t.scene)
        return false;
    if (scaledFor != rect.size())
    {
        scaled = QPixmap(rect.size());
        QPainter scene(&scaled);
        t.scene(scene, QRect(QPoint(0, 0), rect.size()));
        scene.end();
        scaledFor = rect.size();
    }
    painter.drawPixmap(rect.topLeft(), scaled);
    return true;
}

QString formatBytes(quint64 bytes)
{
    if (bytes < 1024)
        return QObject::tr("%1 B").arg(bytes);
    if (bytes < 1024 * 1024)
        return QObject::tr("%1 kB").arg(bytes / 1024.0, 0, 'f', 1);
    if (bytes < 1024 * 1024 * 1024)
        return QObject::tr("%1 MB").arg(bytes / (1024.0 * 1024), 0, 'f', 2);
    return QObject::tr("%1 GB").arg(bytes / (1024.0 * 1024 * 1024), 0, 'f', 2);
}

QString formatBytesPerSecond(double bytes)
{
    return QObject::tr("%1/s").arg(formatBytes((quint64)(bytes + 0.5)));
}

QString formatDurationStr(qint64 secs)
{
    if (secs < 0)
        secs = 0;
    qint64 d = secs / 86400, h = (secs % 86400) / 3600, m = (secs % 3600) / 60, s = secs % 60;
    if (d > 0)
        return QObject::tr("%1 d %2 h").arg(d).arg(h);
    if (h > 0)
        return QObject::tr("%1 h %2 m").arg(h).arg(m);
    if (m > 0)
        return QObject::tr("%1 m %2 s").arg(m).arg(s);
    return QObject::tr("%1 s").arg(s);
}

} // namespace GUIUtil

