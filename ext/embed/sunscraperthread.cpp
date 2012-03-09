#include <QApplication>
#include <QWebPage>
#include <QWebFrame>
#include <QTimer>
#include "sunscraperthread.h"
#include "sunscraperproxy.h"
#include <QtDebug>
#include <time.h>

#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
pthread_t SunscraperThread::m_thread;
#endif

SunscraperThread *SunscraperThread::m_instance;
QSemaphore SunscraperThread::m_initializationLock;

SunscraperThread::SunscraperThread()
{
}

SunscraperThread *SunscraperThread::instance()
{
    m_initializationLock.acquire(1);
    m_initializationLock.release(1);

    return m_instance;
}

void SunscraperThread::invoke()
{
#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
    pthread_create(&m_thread, NULL, &SunscraperThread::thread_routine, NULL);
#endif
}

void *SunscraperThread::thread_routine(void *)
{
    /* Better error messages. */
    int   argc   = 1;
    char *argv[] = { (char*) "Sunscraper", NULL};

    /* Why (char*)? Because argv can (theoretically) be modified. *
     * But Qt won't do that with argv[0]. I know, trust me.       */

    //qDebug() << "a";
    //usleep(1000000);
    //qDebug() << "b";

    QApplication app(argc, argv);

    if(m_instance != NULL)
        qFatal("Attempt to invoke SunscraperThread more than once");

    m_instance = new SunscraperThread();
    m_initializationLock.release(1);

    /* The magic value 42 means we want exit from the loop. */
    while(app.exec() != 42);

    /* Our application exits. */

    return NULL;
}

void SunscraperThread::commitSuicide()
{
    QApplication::exit(42);

#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
    pthread_join(m_thread, NULL);
#endif
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

void SunscraperThread::setTimeout(unsigned queryId, unsigned timeout)
{
    Q_ASSERT(m_timers[queryId] == NULL);

    QTimer *timer = new QTimer(this);
    timer->setInterval(timeout);
    timer->setSingleShot(true);

    connect(timer, SIGNAL(timeout()), this, SLOT(routeTimeout()));

    timer->start();
    m_timers[queryId] = timer;
}

void SunscraperThread::finalize(unsigned queryId)
{
    Q_ASSERT(m_webPages[queryId] != NULL);

    m_webPages[queryId]->deleteLater();
    m_webPages.remove(queryId);

    if(m_timers.contains(queryId)) {
        m_timers[queryId]->deleteLater();
        m_timers.remove(queryId);
    }
}

QWebPage *SunscraperThread::initializeWebPage(unsigned queryId)
{
    Q_ASSERT(m_webPages[queryId] == NULL);

    QWebPage *webPage = new QWebPage(this);
    connect(webPage->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()),
        this, SLOT(attachAPI()));

    m_webPages[queryId] = webPage;

    return webPage;
}

void SunscraperThread::attachAPI()
{
    QWebFrame *origin = static_cast<QWebFrame *>(QObject::sender());
    QWebPage *page = origin->page();

    unsigned queryId = m_webPages.key(page, 0);
    Q_ASSERT(queryId != 0);

    SunscraperProxy *proxy = new SunscraperProxy(page, queryId);
    connect(proxy, SIGNAL(finished(uint,QString)), this, SIGNAL(finished(uint,QString)));

    origin->addToJavaScriptWindowObject("Sunscraper", proxy, QScriptEngine::QtOwnership);
}

void SunscraperThread::routeTimeout()
{
    QTimer *origin = static_cast<QTimer *>(QObject::sender());

    unsigned queryId = m_timers.key(origin, 0);
    Q_ASSERT(queryId != 0);

    emit timeout(queryId);
}
