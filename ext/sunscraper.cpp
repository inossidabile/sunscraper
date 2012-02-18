#include <QApplication>
#include <QThread>
#include <QTimer>
#include <QWebPage>
#include <QWebFrame>
#include <QMutexLocker>
#include <QtDebug>
#include "sunscraper.h"
#include "sunscraperlibrary.h"
#include "sunscraperthread.h"

unsigned Sunscraper::_nextQueryId = 1;
QMutex Sunscraper::_staticMutex;

Sunscraper::Sunscraper()
{
    QMutexLocker locker(&_staticMutex);

    _queryId = _nextQueryId++;

    SunscraperThread *worker = SunscraperLibrary::instance()->thread();

    connect(this, SIGNAL(requestLoadHtml(uint,QString)),
        worker, SLOT(loadHtml(uint,QString)), Qt::QueuedConnection);
    connect(this, SIGNAL(requestLoadUrl(uint,QString)),
        worker, SLOT(loadUrl(uint,QString)), Qt::QueuedConnection);
    connect(this, SIGNAL(requestFinalize(uint)),
        worker, SLOT(finalize(uint)), Qt::QueuedConnection);

    connect(worker, SIGNAL(finished(uint,QString)),
        this, SLOT(finished(uint,QString)), Qt::QueuedConnection);
}

void Sunscraper::loadHtml(QString html)
{
    emit requestLoadHtml(_queryId, html);
}

void Sunscraper::loadUrl(QString url)
{
    emit requestLoadUrl(_queryId, url);
}

void Sunscraper::wait(unsigned timeout)
{
    QTimer _timeoutTimer;
    connect(&_timeoutTimer, SIGNAL(timeout()), this, SLOT(timeout()));

    _timeoutTimer.setInterval(timeout);
    _timeoutTimer.start();

    _eventLoop.exec();

    _timeoutTimer.stop();
}

void Sunscraper::finished(unsigned eventQueryId, QString html)
{
    if(eventQueryId != _queryId)
        return;

    _eventLoop.quit();

    _html = html.toUtf8();

    emit requestFinalize(_queryId);
}

void Sunscraper::timeout()
{
    _eventLoop.quit();

    _html = "!SUNSCRAPER_TIMEOUT";

    emit requestFinalize(_queryId);
}

QByteArray Sunscraper::fetch()
{
    return _html;
}

const char *Sunscraper::fetchAsCString()
{
    return _html.constData();
}
