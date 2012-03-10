#ifndef SUNSCRAPERINTERFACE_H
#define SUNSCRAPERINTERFACE_H

#include <QObject>
#include <QMutex>
#include <QSemaphore>
#include <QMap>
#include <QUrl>
#include <QByteArray>

class SunscraperWorker;

class SunscraperInterface : public QObject
{
    Q_OBJECT

public:
    static SunscraperInterface *instance();

    unsigned createQuery();

    void loadHtml(unsigned queryId, QString html, QUrl baseUrl);
    void loadUrl(unsigned queryId, QUrl url);

    bool wait(unsigned queryId, unsigned timeout);

    QByteArray fetch(unsigned queryId);

    void finalize(unsigned queryId);

private slots:
    void onFinish(unsigned queryId);
    void onTimeout(unsigned queryId);
    void onFetchDone(unsigned queryId, QString html);

signals:
    void requestLoadUrl(unsigned queryId, QUrl url);
    void requestLoadHtml(unsigned queryId, QString html, QUrl url);
    void requestTimeout(unsigned queryId, unsigned timeout);
    void requestFetch(unsigned queryId);
    void requestFinalize(unsigned queryId);

private:
    static QMutex m_initializationMutex;
    static SunscraperInterface *m_instance;

    QMutex m_queryIdMutex;
    unsigned m_nextQueryId;

    QMutex m_semaphoresMutex;
    QMap<unsigned, QSemaphore *> m_semaphores;

    QMutex m_resultsMutex;
    QMap<unsigned, bool> m_results;
    QMap<unsigned, QByteArray> m_htmlCache;

    SunscraperWorker *m_worker;

    SunscraperInterface();

    void initSemaphore(unsigned queryId);
    void waitOnSemaphore(unsigned queryId);
    void signalSemaphore(unsigned queryId);
};

#endif // SUNSCRAPERINTERFACE_H
