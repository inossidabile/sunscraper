#ifndef SUNSCRAPER_H
#define SUNSCRAPER_H

#include <QObject>
#include <QString>
#include <QMutex>
#include <QByteArray>
#include <QEventLoop>

class QWebPage;

class Sunscraper : public QObject
{
    Q_OBJECT

public:
    Sunscraper();

    void loadHtml(QString html);
    void loadUrl(QString url);

    void wait(unsigned timeout);

    QByteArray fetch();
    const char *fetchAsCString();

private slots:
    void finished(unsigned queryId, QString html);
    void timeout();

signals:
    void requestLoadHtml(unsigned queryId, QString html);
    void requestLoadUrl(unsigned queryId, QString html);
    void requestFinalize(unsigned queryId);

private:
    static unsigned _nextQueryId;
    static QMutex _staticMutex;

    unsigned _queryId;
    QEventLoop _eventLoop;
    QByteArray _html;
};

#endif // SUNSCRAPER_H
