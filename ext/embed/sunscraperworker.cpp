#include <QApplication>
#include <QWebPage>
#include <QWebFrame>
#include <QTimer>
#include <QWebView>
#include "sunscraperworker.h"
#include "sunscraperwebpage.h"
#include "sunscraperproxy.h"
#include <QtDebug>
#include <time.h>

#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
pthread_t SunscraperWorker::m_thread;
#endif

SunscraperWorker *SunscraperWorker::m_instance;
QSemaphore SunscraperWorker::m_initializationLock;

SunscraperWorker::SunscraperWorker()
{
}

SunscraperWorker *SunscraperWorker::instance()
{
    m_initializationLock.acquire(1);
    m_initializationLock.release(1);

    return m_instance;
}

void SunscraperWorker::invoke()
{
#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
    pthread_create(&m_thread, NULL, &SunscraperWorker::thread_routine, NULL);
#endif
}

void *SunscraperWorker::thread_routine(void *)
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
        qFatal("Attempt to invoke SunscraperWorker more than once");

    m_instance = new SunscraperWorker();
    m_initializationLock.release(1);

    /* The magic value 42 means we want exit from the loop. */
    while(app.exec() != 42);

    /* Our application exits. */

    return NULL;
}

void SunscraperWorker::commitSuicide()
{
    QApplication::exit(42);

#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
    pthread_join(m_thread, NULL);
#endif
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

void SunscraperWorker::setTimeout(unsigned queryId, unsigned timeout)
{
    Q_ASSERT(m_timers[queryId] == NULL);

    QTimer *timer = new QTimer(this);
    timer->setInterval(timeout);
    timer->setSingleShot(true);

    connect(timer, SIGNAL(timeout()), this, SLOT(routeTimeout()));

    timer->start();
    m_timers[queryId] = timer;
}

void SunscraperWorker::finalize(unsigned queryId)
{
    Q_ASSERT(m_webPages[queryId] != NULL);

    m_webPages[queryId]->deleteLater();
    m_webPages.remove(queryId);

    if(m_timers.contains(queryId)) {
        m_timers[queryId]->deleteLater();
        m_timers.remove(queryId);
    }
}

QWebPage *SunscraperWorker::initializeWebPage(unsigned queryId)
{
    Q_ASSERT(m_webPages[queryId] == NULL);

    SunscraperWebPage *webPage = new SunscraperWebPage(this);
    webPage->settings()->setAttribute(QWebSettings::LocalStorageEnabled, true);

    connect(webPage, SIGNAL(frameCreated(QWebFrame*)), this, SLOT(attachFrame(QWebFrame*)));
    connect(webPage, SIGNAL(consoleMessage(QString)), this, SLOT(routeMessage(QString)));

    m_webPages[queryId] = webPage;

    return webPage;
}

void SunscraperWorker::attachFrame(QWebFrame *frame)
{
    connect(frame, SIGNAL(javaScriptWindowObjectCleared()),
        this, SLOT(attachAPI()));
}

void SunscraperWorker::attachAPI()
{
    QWebFrame *origin = static_cast<QWebFrame *>(QObject::sender());
    QWebPage *page = origin->page();

    unsigned queryId = m_webPages.key(page, 0);
    Q_ASSERT(queryId != 0);

    SunscraperProxy *proxy = new SunscraperProxy(page, queryId);
    connect(proxy, SIGNAL(finished(uint,QString)), this, SIGNAL(finished(uint,QString)));

    origin->addToJavaScriptWindowObject("Sunscraper", proxy, QScriptEngine::QtOwnership);
}

void SunscraperWorker::routeTimeout()
{
    QTimer *origin = static_cast<QTimer *>(QObject::sender());

    unsigned queryId = m_timers.key(origin, 0);
    Q_ASSERT(queryId != 0);

    emit timeout(queryId);
}

void SunscraperWorker::routeMessage(QString message)
{
    qDebug() << "Sunscraper Console:" << message;
}
