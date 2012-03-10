#include <QApplication>
#include <QtDebug>
#include "sunscraperthread.h"
#include "sunscraperworker.h"

#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
pthread_t SunscraperThread::m_thread;
#endif

void SunscraperThread::invoke()
{
#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
    pthread_create(&m_thread, NULL, &SunscraperThread::thread_routine, NULL);
#endif
}

void *SunscraperThread::thread_routine(void *)
{
    /* Better error messages. */
    int   argc   = 1;
    char *argv[] = { (char*) "Sunscraper", NULL};

    /* Why (char*)? Because argv can (theoretically) be modified. *
     * But Qt won't do that with argv[0]. I know, trust me.       */

    QApplication app(argc, argv);
    app.setApplicationName("Sunscraper-Embed");

    SunscraperWorker::unlock();

    /*
     * The magic value 42 means we want exit from the loop.
     * E.g. alerts from within the page may exit the loop with value 0.
     */
    while(app.exec() != 42);

    /* Our host application exits. */

    return NULL;
}

void SunscraperThread::commitSuicide()
{
    QApplication::exit(42);

#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
    pthread_join(m_thread, NULL);
#endif
}
