#ifndef SUNSCRAPERTHREAD_H
#define SUNSCRAPERTHREAD_H

#include <QObject>
#include <QMap>

class QWebPage;

class SunscraperThread : public QObject
{
    Q_OBJECT
public:
    static void invoke();
    static SunscraperThread *instance();

signals:
    void finished(unsigned queryId, QString result);

public slots:
    void loadHtml(unsigned queryId, QString html);
    void loadUrl(unsigned queryId, QString url);
    void finalize(unsigned queryId);

private slots:
    void attachAPI();

private:
    static SunscraperThread *_instance;
    QMap<unsigned, QWebPage *> _webPages;

    SunscraperThread();
    SunscraperThread(SunscraperThread &);

    static void *thread_routine(void *arg);

    QWebPage *initializeWebPage(unsigned queryId);
};

#endif // SUNSCRAPERTHREAD_H
