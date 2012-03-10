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

signals:
    void requestLoadUrl(unsigned queryId, QUrl url);
    void requestLoadHtml(unsigned queryId, QString html, QUrl url);
    void requestTimeout(unsigned queryId, unsigned timeout);
    void requestFinalize(unsigned queryId);

private:
    static QMutex m_initializationMutex;
    static SunscraperInterface *m_instance;

    QMutex m_queryIdMutex;
    unsigned m_nextQueryId;

    QMutex m_resultsMutex;
    QMap<unsigned, QSemaphore *> m_semaphores;
    QMap<unsigned, bool> m_results;

    SunscraperWorker *m_worker;

    SunscraperInterface();
};

#endif // SUNSCRAPERINTERFACE_H
