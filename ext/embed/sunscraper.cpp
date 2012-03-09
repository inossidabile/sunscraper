#include <QApplication>
#include <QThread>
#include <QTimer>
#include <QWebPage>
#include <QWebFrame>
#include <QMutexLocker>
#include <QEventLoop>
#include <QtDebug>
#include "sunscraper.h"
#include "sunscraperlibrary.h"
#include "sunscraperworker.h"

unsigned Sunscraper::m_nextQueryId = 1;
QMutex Sunscraper::m_staticMutex;

Sunscraper::Sunscraper()
{
    QMutexLocker locker(&m_staticMutex);

    m_queryId = m_nextQueryId++;

    SunscraperWorker *worker = SunscraperWorker::instance();
    if(worker == NULL)
        qFatal("Attempt to run Sunscraper before thread initialization");

    connect(this, SIGNAL(requestLoadHtml(uint,QString)),
        worker, SLOT(loadHtml(uint,QString)), Qt::QueuedConnection);
    connect(this, SIGNAL(requestLoadUrl(uint,QString)),
        worker, SLOT(loadUrl(uint,QString)), Qt::QueuedConnection);
    connect(this, SIGNAL(requestFinalize(uint)),
        worker, SLOT(finalize(uint)), Qt::QueuedConnection);
    connect(this, SIGNAL(requestTimeout(uint,uint)),
        worker, SLOT(setTimeout(uint, uint)), Qt::QueuedConnection);

    connect(worker, SIGNAL(finished(uint,QString)),
        this, SLOT(finished(uint,QString)), Qt::QueuedConnection);
    connect(worker, SIGNAL(timeout(uint)),
        this, SLOT(timeout(uint)), Qt::QueuedConnection);

    m_eventLoop = new QEventLoop;
}

void Sunscraper::loadHtml(QString html)
{
    emit requestLoadHtml(m_queryId, html);
}

void Sunscraper::loadUrl(QString url)
{
    emit requestLoadUrl(m_queryId, url);
}

void Sunscraper::wait(unsigned timeout)
{
    emit requestTimeout(m_queryId, timeout);

    m_eventLoop->exec();
}

void Sunscraper::finished(unsigned eventQueryId, QString html)
{
    if(eventQueryId != m_queryId)
        return;

    m_eventLoop->quit();

    m_html = html.toUtf8();

    emit requestFinalize(m_queryId);
}

void Sunscraper::timeout(unsigned eventQueryId)
{
    if(eventQueryId != m_queryId)
        return;

    m_eventLoop->quit();

    m_html = "!SUNSCRAPER_TIMEOUT";

    emit requestFinalize(m_queryId);
}

QByteArray Sunscraper::fetch()
{
    return m_html;
}

const char *Sunscraper::fetchAsCString()
{
    return m_html.constData();
}
