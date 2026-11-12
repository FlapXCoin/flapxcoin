#ifndef EXPLORERPAGE_H
#define EXPLORERPAGE_H

#include <QWidget>
#include <QStringList>

namespace Ui {
    class ExplorerPage;
}
class QUrl;

/** Built-in block explorer, on its own toolbar page. Looks up a block by
    height or hash, or any transaction by its ID, from the local chain
    (transactions through the chain's transaction index, so lookups are quick).
    Blocks and transactions in the results are links; Back returns to the
    previous result. */
class ExplorerPage : public QWidget
{
    Q_OBJECT

public:
    explicit ExplorerPage(QWidget *parent = 0);
    ~ExplorerPage();

public slots:
    /** Look up whatever is in the search box */
    void search();
    /** Show the newest block */
    void showLatest();
    /** Go back to the previous result */
    void back();

protected:
    /** Put the cursor in the search box when the page is opened */
    void showEvent(QShowEvent *event);

private slots:
    void linkClicked(const QUrl &url);

private:
    Ui::ExplorerPage *ui;
    QStringList history;    // previous queries, for Back
    QString current;        // query now shown

    /** Show a query: a height, a 64-character hex hash (block or transaction).
        fRecord adds the previous one to the Back history. */
    void show(const QString &query, bool fRecord = true);
    QString blockHtml(const QString &hash, bool &fFound);
    QString txHtml(const QString &hash, bool &fFound);
};

#endif // EXPLORERPAGE_H
