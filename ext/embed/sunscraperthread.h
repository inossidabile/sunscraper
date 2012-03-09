#ifndef SUNSCRAPERTHREAD_H
#define SUNSCRAPERTHREAD_H

#include <QObject>
#include <QSemaphore>
#include <QMap>

class QWebPage;
class QTimer;

class SunscraperThread : public QObject
{
    Q_OBJECT
public:
    static void invoke();
    static void commitSuicide();
    static SunscraperThread *instance();

signals:
    void finished(unsigned queryId, QString result);
    void timeout(unsigned queryId);

public slots:
    void loadHtml(unsigned queryId, QString html);
    void loadUrl(unsigned queryId, QString url);
    void setTimeout(unsigned queryId, unsigned timeout);
    void finalize(unsigned queryId);

private slots:
    void attachAPI();
    void routeTimeout();

private:
    static SunscraperThread *m_instance;
    static QSemaphore m_initializationLock;

#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
    static pthread_t m_thread;
#else
#error Your platform is unsupported. Implement SunscraperThread::invoke() and send a pull request.
#endif

    static void *thread_routine(void *arg);

    QMap<unsigned, QWebPage *> m_webPages;
    QMap<unsigned, QTimer *> m_timers;

    SunscraperThread();
    SunscraperThread(SunscraperThread &);

    QWebPage *initializeWebPage(unsigned queryId);
};

#endif // SUNSCRAPERTHREAD_H
