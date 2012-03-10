#include <QApplication>
#include <QThread>
#include <QWebPage>
#include <QWebFrame>
#include <QMutexLocker>
#include <QtDebug>
#include "sunscraperinterface.h"
#include "sunscraperlibrary.h"
#include "sunscraperworker.h"

// #define DEBUG_SUNSCRAPERINTERFACE

QMutex SunscraperInterface::m_initializationMutex;
SunscraperInterface *SunscraperInterface::m_instance;

SunscraperInterface::SunscraperInterface() :
        m_nextQueryId(0)
{
    m_worker = new SunscraperWorker();
    m_worker->moveToThread(QApplication::instance()->thread());

    connect(this, SIGNAL(requestLoadHtml(uint,QString,QUrl)),
            m_worker, SLOT(loadHtml(uint,QString,QUrl)), Qt::QueuedConnection);
    connect(this, SIGNAL(requestLoadUrl(uint,QUrl)),
            m_worker, SLOT(loadUrl(uint,QUrl)), Qt::QueuedConnection);
    connect(this, SIGNAL(requestTimeout(uint,uint)),
            m_worker, SLOT(setTimeout(uint,uint)), Qt::QueuedConnection);
    connect(this, SIGNAL(requestFetch(uint)),
            m_worker, SLOT(fetchHtml(uint)), Qt::QueuedConnection);
    connect(this, SIGNAL(requestFinalize(uint)),
            m_worker, SLOT(finalize(uint)), Qt::QueuedConnection);

    connect(m_worker, SIGNAL(finished(uint)),
            this, SLOT(onFinish(uint)), Qt::DirectConnection);
    connect(m_worker, SIGNAL(timedOut(uint)),
            this, SLOT(onTimeout(uint)), Qt::DirectConnection);
    connect(m_worker, SIGNAL(htmlFetched(uint,QString)),
            this, SLOT(onFetchDone(uint,QString)), Qt::DirectConnection);
}

SunscraperInterface *SunscraperInterface::instance()
{
    QMutexLocker locker(&m_initializationMutex);

    if(m_instance == NULL)
        m_instance = new SunscraperInterface();

    return m_instance;
}

void SunscraperInterface::initSemaphore(unsigned queryId)
{
    QMutexLocker locker(&m_semaphoresMutex);

    Q_ASSERT(m_semaphores[queryId] == NULL);

    QSemaphore *semaphore = new QSemaphore(0);
    m_semaphores[queryId] = semaphore;
}

void SunscraperInterface::waitOnSemaphore(unsigned queryId)
{
    m_semaphoresMutex.lock();

    Q_ASSERT(m_semaphores[queryId] != NULL);

    QSemaphore *semaphore = m_semaphores[queryId];

    m_semaphoresMutex.unlock();

    semaphore->acquire(1);

    m_semaphoresMutex.lock();

    delete semaphore;
    m_semaphores.remove(queryId);

    m_semaphoresMutex.unlock();
}

void SunscraperInterface::signalSemaphore(unsigned queryId)
{
    QMutexLocker locker(&m_semaphoresMutex);

    if(m_semaphores.contains(queryId))
        m_semaphores[queryId]->release(1);
}

unsigned SunscraperInterface::createQuery()
{
    QMutexLocker locker(&m_queryIdMutex);

    m_nextQueryId += 1;

#ifdef DEBUG_SUNSCRAPERINTERFACE
    qDebug() << "createQuery" << m_nextQueryId;
#endif

    return m_nextQueryId;
}

void SunscraperInterface::loadHtml(unsigned queryId, QString html, QUrl baseUrl)
{
#ifdef DEBUG_SUNSCRAPERINTERFACE
    qDebug() << "loadHtml" << queryId << html << baseUrl;
#endif

    emit requestLoadHtml(queryId, html, baseUrl);
}

void SunscraperInterface::loadUrl(unsigned queryId, QUrl url)
{
#ifdef DEBUG_SUNSCRAPERINTERFACE
    qDebug() << "loadUrl" << queryId << url;
#endif

    emit requestLoadUrl(queryId, url);
}

bool SunscraperInterface::wait(unsigned queryId, unsigned timeout)
{
#ifdef DEBUG_SUNSCRAPERINTERFACE
    qDebug() << "wait" << queryId << timeout;
#endif

    initSemaphore(queryId);
    emit requestTimeout(queryId, timeout);
    waitOnSemaphore(queryId);

    /* There was either a finish or timeout */

    {
        QMutexLocker locker(&m_resultsMutex);

        bool success = m_results[queryId];
        m_results.remove(queryId);

        return success;
    }
}

void SunscraperInterface::onFinish(unsigned queryId)
{
#ifdef DEBUG_SUNSCRAPERINTERFACE
    qDebug() << "onFinish" << queryId;
#endif

    QMutexLocker locker(&m_resultsMutex);
    m_results[queryId] = true;

    signalSemaphore(queryId);
}

void SunscraperInterface::onTimeout(unsigned queryId)
{
#ifdef DEBUG_SUNSCRAPERINTERFACE
    qDebug() << "onTimeout" << queryId;
#endif

    QMutexLocker locker(&m_resultsMutex);
    m_results[queryId] = false;

    signalSemaphore(queryId);
}

void SunscraperInterface::onFetchDone(unsigned queryId, QString html)
{
#ifdef DEBUG_SUNSCRAPERINTERFACE
    qDebug() << "onFetchDone" << queryId;
#endif

    QMutexLocker locker(&m_resultsMutex);
    m_htmlCache[queryId] = html.toLocal8Bit();

    signalSemaphore(queryId);
}

QByteArray SunscraperInterface::fetch(unsigned queryId)
{
#ifdef DEBUG_SUNSCRAPERINTERFACE
    qDebug() << "fetch" << queryId;
#endif

    initSemaphore(queryId);
    emit requestFetch(queryId);
    waitOnSemaphore(queryId);

    {
        QMutexLocker locker(&m_resultsMutex);
        return m_htmlCache[queryId];
    }
}

void SunscraperInterface::finalize(unsigned queryId)
{
#ifdef DEBUG_SUNSCRAPERINTERFACE
    qDebug() << "finalize" << queryId;
#endif

    emit requestFinalize(queryId);

    QMutexLocker locker(&m_resultsMutex);
    m_results.remove(queryId);
    m_htmlCache.remove(queryId);
}
