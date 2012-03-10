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
    SunscraperWorker(QObject * parent = 0);
    
    static void unlock();

signals:
    void finished(unsigned queryId);
    void timedOut(unsigned queryId);
    void htmlFetched(unsigned queryId, QString data);

public slots:
    void loadHtml(unsigned queryId, QString html, QUrl baseUrl);
    void loadUrl(unsigned queryId, QUrl url);
    void setTimeout(unsigned queryId, unsigned timeout);
    void fetchHtml(unsigned queryId);
    void finalize(unsigned queryId);

private slots:
    void onFrameCreated(QWebFrame *frame);
    void onJavascriptObjectCleared();
    void onFinish(unsigned queryId);
    void onTimeout();
    void onMessage(QString message);

private:
    static QSemaphore m_initializationLock;
    
    QMap<unsigned, QWebPage *> m_webPages;
    QMap<unsigned, QTimer *> m_timers;

    QWebPage *initializeWebPage(unsigned queryId);
};

#endif // SUNSCRAPERWORKER_H
