#ifndef TRAFFICGRAPHWIDGET_H
#define TRAFFICGRAPHWIDGET_H

#include <QWidget>
#include <QQueue>

/** Live graph of network download/upload speed for the Debug window's
 * Network Traffic tab. Feed it the running byte totals with addSample();
 * it keeps the last few minutes and draws download in cyan, upload in mint. */
class TrafficGraphWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TrafficGraphWidget(QWidget *parent = 0);

    /** Add the current totals; nMillis is the time they were read */
    void addSample(quint64 nTotalRecv, quint64 nTotalSent, qint64 nMillis);
    void clear();

    /** Most recent speeds, in bytes per second */
    double currentRecvRate() const { return fRecvRate; }
    double currentSentRate() const { return fSentRate; }

    QSize minimumSizeHint() const;

protected:
    void paintEvent(QPaintEvent *);

private:
    QQueue<double> vRecvRates;
    QQueue<double> vSentRates;
    quint64 nLastRecv;
    quint64 nLastSent;
    qint64 nLastMillis;
    double fRecvRate;
    double fSentRate;
};

#endif // TRAFFICGRAPHWIDGET_H
