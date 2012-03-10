#include <QWebPage>
#include <QWebFrame>
#include <QTimer>
#include <QtDebug>
#include "sunscraperworker.h"
#include "sunscraperwebpage.h"
#include "sunscraperproxy.h"

QSemaphore SunscraperWorker::m_initializationLock(0);

SunscraperWorker::SunscraperWorker(QObject *parent) :
        QObject(parent)
{
    m_initializationLock.acquire(1);
    m_initializationLock.release(1);
}

void SunscraperWorker::unlock()
{
    m_initializationLock.release(1);
}

void SunscraperWorker::loadHtml(unsigned queryId, QString html, QUrl baseUrl)
{
    QWebPage *webPage = initializeWebPage(queryId);
    webPage->mainFrame()->setHtml(html, baseUrl);
}

void SunscraperWorker::loadUrl(unsigned queryId, QUrl url)
{
    QWebPage *webPage = initializeWebPage(queryId);
    webPage->mainFrame()->load(url);
}

void SunscraperWorker::setTimeout(unsigned queryId, unsigned timeout)
{
    Q_ASSERT(m_timers[queryId] == NULL);

    QTimer *timer = new QTimer();
    timer->setInterval(timeout);
    timer->setSingleShot(true);

    connect(timer, SIGNAL(timeout()), this, SLOT(onTimeout()));

    timer->start();
    m_timers[queryId] = timer;
}

void SunscraperWorker::finalize(unsigned queryId)
{
    if(m_webPages.contains(queryId)) {
        m_webPages[queryId]->deleteLater();
        m_webPages.remove(queryId);
    }

    if(m_timers.contains(queryId)) {
        m_timers[queryId]->deleteLater();
        m_timers.remove(queryId);
    }
}

QWebPage *SunscraperWorker::initializeWebPage(unsigned queryId)
{
    Q_ASSERT(m_webPages[queryId] == NULL);

    SunscraperWebPage *webPage = new SunscraperWebPage();
    webPage->settings()->setAttribute(QWebSettings::LocalStorageEnabled, true);

    connect(webPage, SIGNAL(frameCreated(QWebFrame*)), this, SLOT(onFrameCreated(QWebFrame*)));
    connect(webPage, SIGNAL(consoleMessage(QString)), this, SLOT(onMessage(QString)));

    m_webPages[queryId] = webPage;

    return webPage;
}

void SunscraperWorker::onFrameCreated(QWebFrame *frame)
{
    connect(frame, SIGNAL(javaScriptWindowObjectCleared()),
        this, SLOT(onJavascriptObjectCleared()));
}

void SunscraperWorker::onJavascriptObjectCleared()
{
    QWebFrame *origin = static_cast<QWebFrame *>(QObject::sender());
    QWebPage *page = origin->page();

    unsigned queryId = m_webPages.key(page, 0);
    Q_ASSERT(queryId != 0);

    SunscraperProxy *proxy = new SunscraperProxy(page, queryId);
    connect(proxy, SIGNAL(finished(uint)), this, SLOT(onFinish(uint)));

    origin->addToJavaScriptWindowObject("Sunscraper", proxy, QScriptEngine::QtOwnership);
}

void SunscraperWorker::onFinish(unsigned queryId)
{
    Q_ASSERT(m_webPages[queryId] != NULL);

    emit finished(queryId);
}

void SunscraperWorker::onTimeout()
{
    QTimer *origin = static_cast<QTimer *>(QObject::sender());

    unsigned queryId = m_timers.key(origin, 0);
    Q_ASSERT(queryId != 0);

    m_timers[queryId]->deleteLater();
    m_timers.remove(queryId);

    emit timedOut(queryId);
}

void SunscraperWorker::onMessage(QString message)
{
    qDebug() << "Sunscraper Console:" << message;
}

void SunscraperWorker::fetchHtml(unsigned queryId) {
    emit htmlFetched(queryId, m_webPages[queryId]->mainFrame()->toHtml());
}
