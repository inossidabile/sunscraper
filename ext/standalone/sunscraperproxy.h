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
    void finished(unsigned queryId, QString html);

private:
    QWebPage *m_webPage;
    unsigned m_queryId;
};

#endif // SUNSCRAPERPROXY_H
