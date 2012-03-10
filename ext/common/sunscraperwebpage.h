#ifndef SUNSCRAPERWEBPAGE_H
#define SUNSCRAPERWEBPAGE_H

#include <QWebPage>

class SunscraperWebPage : public QWebPage
{
    Q_OBJECT

public:
    SunscraperWebPage(QObject *parent = 0);

signals:
    void consoleMessage(QString message);

protected:
    virtual void javaScriptConsoleMessage(const QString & message, int lineNumber, const QString & sourceID);
};

#endif /* SUNSCRAPERWEBPAGE_H */
