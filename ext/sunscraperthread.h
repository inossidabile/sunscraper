#ifndef SUNSCRAPERTHREAD_H
#define SUNSCRAPERTHREAD_H

#include <QThread>
#include <QMap>

class QWebPage;

class SunscraperThread : public QThread
{
    Q_OBJECT
public:
    SunscraperThread();

    void run();

signals:
    void finished(unsigned queryId, QString result);

public slots:
    void loadHtml(unsigned queryId, QString html);
    void loadUrl(unsigned queryId, QString url);
    void finalize(unsigned queryId);

private slots:
    void attachAPI();

private:
    QMap<unsigned, QWebPage *> _webPages;

    QWebPage *initializeWebPage(unsigned queryId);
};

#endif // SUNSCRAPERTHREAD_H
