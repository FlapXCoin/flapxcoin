#include "trafficgraphwidget.h"
#include "guiutil.h"

#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>

#include <algorithm>
#include <cmath>

// One sample every TRAFFIC_SAMPLE_MS (set by the Debug window); keep 10 minutes
static const int MAX_SAMPLES = 300;
static const int MARGIN = 10;

TrafficGraphWidget::TrafficGraphWidget(QWidget *parent) :
    QWidget(parent),
    nLastRecv(0), nLastSent(0), nLastMillis(0),
    fRecvRate(0), fSentRate(0)
{
    setMinimumHeight(160);
}

QSize TrafficGraphWidget::minimumSizeHint() const
{
    return QSize(300, 160);
}

void TrafficGraphWidget::clear()
{
    vRecvRates.clear();
    vSentRates.clear();
    nLastMillis = 0;
    fRecvRate = fSentRate = 0;
    update();
}

void TrafficGraphWidget::addSample(quint64 nTotalRecv, quint64 nTotalSent, qint64 nMillis)
{
    if (nLastMillis != 0 && nMillis > nLastMillis)
    {
        double fSeconds = (nMillis - nLastMillis) / 1000.0;
        fRecvRate = (nTotalRecv >= nLastRecv ? nTotalRecv - nLastRecv : 0) / fSeconds;
        fSentRate = (nTotalSent >= nLastSent ? nTotalSent - nLastSent : 0) / fSeconds;
        vRecvRates.enqueue(fRecvRate);
        vSentRates.enqueue(fSentRate);
        while (vRecvRates.size() > MAX_SAMPLES) vRecvRates.dequeue();
        while (vSentRates.size() > MAX_SAMPLES) vSentRates.dequeue();
    }
    nLastRecv = nTotalRecv;
    nLastSent = nTotalSent;
    nLastMillis = nMillis;
    if (isVisible())
        update();
}

// Round a speed up to a tidy scale: 1, 2 or 5 x 10^n bytes per second
static double NiceScale(double fMax)
{
    if (fMax < 1024) return 1024;   // at least 1 kB/s
    double fMag = std::pow(10.0, std::floor(std::log10(fMax)));
    double f = fMax / fMag;
    if (f <= 1) return fMag;
    if (f <= 2) return 2 * fMag;
    if (f <= 5) return 5 * fMag;
    return 10 * fMag;
}

static void DrawRates(QPainter &painter, const QQueue<double> &vRates, const QRect &area,
                      double fScale, const QColor &color)
{
    if (vRates.size() < 2)
        return;
    double fStep = (double)area.width() / (MAX_SAMPLES - 1);
    // Newest sample at the right edge
    double x0 = area.right() - fStep * (vRates.size() - 1);
    QPainterPath line;
    for (int i = 0; i < vRates.size(); i++)
    {
        double x = x0 + fStep * i;
        double y = area.bottom() - area.height() * std::min(vRates[i] / fScale, 1.0);
        if (i == 0) line.moveTo(x, y); else line.lineTo(x, y);
    }
    QPainterPath fill = line;
    fill.lineTo(area.right(), area.bottom());
    fill.lineTo(x0, area.bottom());
    fill.closeSubpath();

    QColor fillColor = color;
    fillColor.setAlpha(45);
    painter.fillPath(fill, fillColor);
    painter.setPen(QPen(color, 1.6));
    painter.drawPath(line);
}

void TrafficGraphWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const QColor bg("#0b1220"), grid("#24345a"), muted("#8a9bb8");
    const QColor cyan("#2ad8ea"), mint("#22f07a");

    painter.fillRect(rect(), bg);
    QRect area = rect().adjusted(MARGIN + 60, MARGIN, -MARGIN, -MARGIN);
    if (area.width() < 10 || area.height() < 10)
        return;

    double fMax = 0;
    foreach (double f, vRecvRates) fMax = std::max(fMax, f);
    foreach (double f, vSentRates) fMax = std::max(fMax, f);
    double fScale = NiceScale(fMax);

    // Grid: 4 horizontal lines with speed labels
    painter.setFont(font());
    for (int i = 0; i <= 4; i++)
    {
        int y = area.bottom() - area.height() * i / 4;
        painter.setPen(QPen(grid, 1, i == 0 ? Qt::SolidLine : Qt::DotLine));
        painter.drawLine(area.left(), y, area.right(), y);
        painter.setPen(muted);
        painter.drawText(QRect(0, y - 8, area.left() - 6, 16), Qt::AlignRight | Qt::AlignVCenter,
                         GUIUtil::formatBytesPerSecond(fScale * i / 4));
    }

    DrawRates(painter, vRecvRates, area, fScale, cyan);
    DrawRates(painter, vSentRates, area, fScale, mint);

    painter.setPen(muted);
    painter.drawText(area.adjusted(4, 2, -4, -2), Qt::AlignLeft | Qt::AlignBottom, tr("10 minutes"));
    painter.drawText(area.adjusted(4, 2, -4, -2), Qt::AlignRight | Qt::AlignBottom, tr("now"));
}
