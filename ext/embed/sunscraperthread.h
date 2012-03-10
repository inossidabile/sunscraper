#ifndef SUNSCRAPERTHREAD_H
#define SUNSCRAPERTHREAD_H

#include <QThread>
#include <QSemaphore>

class SunscraperThread : public QThread
{
    Q_OBJECT
public:
    static void invoke();
    static void commitSuicide();

private:
#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
    static pthread_t m_thread;
#else
#error Your platform is unsupported. Implement SunscraperWorker::invoke() and send a pull request.
#endif

    static void *thread_routine(void *arg);
};

#endif /* SUNSCRAPERTHREAD_H */
