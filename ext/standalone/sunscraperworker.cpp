#include <QApplication>
#include <QWebPage>
#include <QWebFrame>
#include "sunscraperworker.h"
#include "sunscraperproxy.h"
#include <QtDebug>

SunscraperWorker::SunscraperWorker()
{
}

void SunscraperWorker::loadHtml(unsigned queryId, QString html)
{
    QWebPage *webPage = initializeWebPage(queryId);
    webPage->mainFrame()->setHtml(html);
}

void SunscraperWorker::loadUrl(unsigned queryId, QString url)
{
    QWebPage *webPage = initializeWebPage(queryId);
    webPage->mainFrame()->load(url);
}

void SunscraperWorker::finalize(unsigned queryId)
{
    Q_ASSERT(_webPages[queryId] != NULL);

    _webPages[queryId]->deleteLater();
    _webPages.remove(queryId);
}

QWebPage *SunscraperWorker::initializeWebPage(unsigned queryId)
{
    Q_ASSERT(_webPages[queryId] == NULL);

    QWebPage *webPage = new QWebPage(this);
    connect(webPage->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()),
        this, SLOT(attachAPI()));

    _webPages[queryId] = webPage;

    return webPage;
}

void SunscraperWorker::attachAPI()
{
    QWebFrame *origin = static_cast<QWebFrame *>(QObject::sender());
    QWebPage *page = origin->page();

    unsigned queryId = _webPages.key(page, 0);
    Q_ASSERT(queryId != 0);

    SunscraperProxy *proxy = new SunscraperProxy(page, queryId);
    connect(proxy, SIGNAL(finished(uint,QString)), this, SIGNAL(finished(uint,QString)));

    origin->addToJavaScriptWindowObject("Sunscraper", proxy, QScriptEngine::QtOwnership);
}
