#ifndef SUNSCRAPERWORKER_H
#define SUNSCRAPERWORKER_H

#include <QObject>
#include <QSemaphore>
#include <QMap>
#include <QUrl>

class QWebPage;
class QWebFrame;
class QTimer;

class SunscraperWorker : public QObject
{
    Q_OBJECT
public:
    static void invoke();
    static void commitSuicide();
    static SunscraperWorker *instance();

signals:
    void finished(unsigned queryId, QString result);
    void timeout(unsigned queryId);

public slots:
    void loadHtml(unsigned queryId, QString html);
    void loadUrl(unsigned queryId, QString url);
    void setTimeout(unsigned queryId, unsigned timeout);
    void finalize(unsigned queryId);

private slots:
    void attachFrame(QWebFrame *frame);
    void attachAPI();
    void routeTimeout();
    void routeMessage(QString message);

private:
    static SunscraperWorker *m_instance;
    static QSemaphore m_initializationLock;

#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
    static pthread_t m_thread;
#else
#error Your platform is unsupported. Implement SunscraperWorker::invoke() and send a pull request.
#endif

    static void *thread_routine(void *arg);

    QMap<unsigned, QWebPage *> m_webPages;
    QMap<unsigned, QTimer *> m_timers;

    SunscraperWorker();
    SunscraperWorker(SunscraperWorker &);

    QWebPage *initializeWebPage(unsigned queryId);
};

#endif // SUNSCRAPERWORKER_H
