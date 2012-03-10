#ifndef SUNSCRAPERPROXY_H
#define SUNSCRAPERPROXY_H

#include <QObject>

class SunscraperProxy : public QObject
{
    Q_OBJECT

public:
    SunscraperProxy(QObject *parent, unsigned queryId);

    Q_INVOKABLE void finish();

signals:
    void finished(unsigned queryId);

private:
    unsigned m_queryId;

    SunscraperProxy();
};

#endif // SUNSCRAPERPROXY_H
