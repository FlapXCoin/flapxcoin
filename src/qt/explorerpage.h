#ifndef EXPLORERPAGE_H
#define EXPLORERPAGE_H

#include <QWidget>

namespace Ui {
    class ExplorerPage;
}

/** Built-in block explorer: look up a block by height and show its hash, merkle root,
    nonce, bits, timestamp, difficulty and hashrate. Has its own toolbar page (it used
    to be a tab on the Links page). */
class ExplorerPage : public QWidget
{
    Q_OBJECT

public:
    explicit ExplorerPage(QWidget *parent = 0);
    ~ExplorerPage();

public slots:
    void blockClicked();
    void txClicked();
    void updateExplorer(bool block);

private:
    Ui::ExplorerPage *ui;
};

#endif // EXPLORERPAGE_H
