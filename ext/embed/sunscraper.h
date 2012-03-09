#ifndef SUNSCRAPER_H
#define SUNSCRAPER_H

#include <QObject>
#include <QString>
#include <QMutex>
#include <QByteArray>

class QWebPage;
class QEventLoop;

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
    void timeout(unsigned queryId);

signals:
    void requestLoadHtml(unsigned queryId, QString html);
    void requestLoadUrl(unsigned queryId, QString html);
    void requestTimeout(unsigned queryId, unsigned timeout);
    void requestFinalize(unsigned queryId);

private:
    static unsigned m_nextQueryId;
    static QMutex m_staticMutex;

    QEventLoop *m_eventLoop;

    unsigned m_queryId;
    QByteArray m_html;
};

#endif // SUNSCRAPER_H
