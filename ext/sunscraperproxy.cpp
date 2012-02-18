#include <QWebPage>
#include <QWebFrame>
#include "sunscraperproxy.h"

SunscraperProxy::SunscraperProxy(QWebPage *parent, unsigned queryId) :
    QObject(parent), _webPage(parent), _queryId(queryId)
{
}

void SunscraperProxy::finish()
{
    emit finished(_queryId, _webPage->mainFrame()->toHtml());
}
