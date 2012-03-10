#include "sunscraperproxy.h"

SunscraperProxy::SunscraperProxy(QObject *parent, unsigned queryId) :
    QObject(parent), m_queryId(queryId)
{
}

void SunscraperProxy::finish()
{
    emit finished(m_queryId);
}
