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
    connect(this, SIGNAL(requestFinalize(uint)),
        m_worker, SLOT(finalize(uint)), Qt::QueuedConnection);

    connect(m_worker, SIGNAL(finished(uint)),
        this, SLOT(onFinish(uint)), Qt::DirectConnection);
    connect(m_worker, SIGNAL(timedOut(uint)),
        this, SLOT(onTimeout(uint)), Qt::DirectConnection);
}

SunscraperInterface *SunscraperInterface::instance()
{
    QMutexLocker locker(&m_initializationMutex);

    if(m_instance == NULL)
        m_instance = new SunscraperInterface();

    return m_instance;
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

    QSemaphore semaphore(0);

    {
        QMutexLocker locker(&m_resultsMutex);

        Q_ASSERT(m_semaphores[queryId] == NULL);

        m_semaphores[queryId] = &semaphore;
    }

    emit requestTimeout(queryId, timeout);

    semaphore.acquire(1);

    /* There was either a finish or timeout */

    bool success;

    {
        QMutexLocker locker(&m_resultsMutex);

        Q_ASSERT(m_semaphores[queryId] != NULL);

        success = m_results[queryId];

        m_semaphores.remove(queryId);
        m_results.remove(queryId);
    }

    return success;
}

void SunscraperInterface::onFinish(unsigned queryId)
{
#ifdef DEBUG_SUNSCRAPERINTERFACE
    qDebug() << "onFinish" << queryId;
#endif

    QMutexLocker locker(&m_resultsMutex);

    Q_ASSERT(m_semaphores[queryId] != NULL);

    m_results[queryId] = true;
    m_semaphores[queryId]->release(1);
}

void SunscraperInterface::onTimeout(unsigned queryId)
{
#ifdef DEBUG_SUNSCRAPERINTERFACE
    qDebug() << "onTimeout" << queryId;
#endif

    QMutexLocker locker(&m_resultsMutex);

    Q_ASSERT(m_semaphores[queryId] != NULL);

    m_results[queryId] = false;
    m_semaphores[queryId]->release(1);
}

QByteArray SunscraperInterface::fetch(unsigned queryId)
{
#ifdef DEBUG_SUNSCRAPERINTERFACE
    qDebug() << "fetch" << queryId;
#endif

    return m_worker->fetchHtml(queryId).toLocal8Bit();
}

void SunscraperInterface::finalize(unsigned queryId)
{
#ifdef DEBUG_SUNSCRAPERINTERFACE
    qDebug() << "finalize" << queryId;
#endif

    emit requestFinalize(queryId);

    m_results.remove(queryId);
}
