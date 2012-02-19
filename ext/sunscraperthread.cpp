#include <QApplication>
#include <QWebPage>
#include <QWebFrame>
#include "sunscraperthread.h"
#include "sunscraperproxy.h"

SunscraperThread *SunscraperThread::_instance;

SunscraperThread::SunscraperThread()
{
}

SunscraperThread *SunscraperThread::instance()
{
    return _instance;
}

void SunscraperThread::invoke()
{
#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
    pthread_t thread;

    pthread_create(&thread, NULL, &SunscraperThread::thread_routine, NULL);
#else
#error Your platform is unsupported. Implement SunscraperThread::invoke() and send a pull request.
#endif
}

void *SunscraperThread::thread_routine(void *)
{
    static int argc;
    static char **argv = {NULL};

    QApplication app(argc, argv);

    _instance = new SunscraperThread();

    app.exec();

    qFatal("Sunscraper apartment thread event loop should never end");

    return NULL;
}

void SunscraperThread::loadHtml(unsigned queryId, QString html)
{
    QWebPage *webPage = initializeWebPage(queryId);
    webPage->mainFrame()->setHtml(html);
}

void SunscraperThread::loadUrl(unsigned queryId, QString url)
{
    QWebPage *webPage = initializeWebPage(queryId);
    webPage->mainFrame()->load(url);
}

void SunscraperThread::finalize(unsigned queryId)
{
    Q_ASSERT(_webPages[queryId] != NULL);

    _webPages[queryId]->deleteLater();
    _webPages.remove(queryId);
}

QWebPage *SunscraperThread::initializeWebPage(unsigned queryId)
{
    Q_ASSERT(_webPages[queryId] == NULL);

    QWebPage *webPage = new QWebPage(this);
    connect(webPage->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()),
        this, SLOT(attachAPI()));

    _webPages[queryId] = webPage;

    return webPage;
}

void SunscraperThread::attachAPI()
{
    QWebFrame *origin = static_cast<QWebFrame *>(QObject::sender());
    QWebPage *page = origin->page();

    unsigned queryId = _webPages.key(page, 0);
    Q_ASSERT(queryId != 0);

    SunscraperProxy *proxy = new SunscraperProxy(page, queryId);
    connect(proxy, SIGNAL(finished(uint,QString)), this, SIGNAL(finished(uint,QString)));

    origin->addToJavaScriptWindowObject("Sunscraper", proxy, QScriptEngine::QtOwnership);
}
