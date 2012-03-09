#include <QWebPage>
#include <QWebFrame>
#include <QtDebug>
#include "sunscraperproxy.h"

SunscraperProxy::SunscraperProxy(QWebPage *parent, unsigned queryId) :
    QObject(parent), m_webPage(parent), m_queryId(queryId)
{
}

void SunscraperProxy::finish()
{
    emit finished(m_queryId, m_webPage->mainFrame()->toHtml());
}
