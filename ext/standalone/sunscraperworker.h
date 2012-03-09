#ifndef SUNSCRAPERWORKER_H
#define SUNSCRAPERWORKER_H

#include <QObject>
#include <QMutex>
#include <QMap>

class QWebPage;

class SunscraperWorker : public QObject
{
    Q_OBJECT

public:
    SunscraperWorker(QObject *parent = 0);

signals:
    void finished(unsigned queryId, QString result);

public slots:
    void loadHtml(unsigned queryId, QString html);
    void loadUrl(unsigned queryId, QString url);
    void finalize(unsigned queryId);

private slots:
    void attachAPI();

private:
    QMap<unsigned, QWebPage *> _webPages;

    QWebPage *initializeWebPage(unsigned queryId);
};

#endif // SUNSCRAPERWORKER_H
