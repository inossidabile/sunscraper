#ifndef SUNSCRAPERPROXY_H
#define SUNSCRAPERPROXY_H

#include <QObject>

class QWebPage;

class SunscraperProxy : public QObject
{
    Q_OBJECT
public:
    SunscraperProxy(QWebPage *parent, unsigned queryId);

    Q_INVOKABLE void finish();

signals:
    void finished(unsigned _queryId, QString html);

private:
    QWebPage *_webPage;
    unsigned _queryId;
};

#endif // SUNSCRAPERPROXY_H
