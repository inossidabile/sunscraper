#include "sunscraperwebpage.h"

SunscraperWebPage::SunscraperWebPage(QObject *parent) :
        QWebPage(parent)
{
}

void SunscraperWebPage::javaScriptConsoleMessage(const QString &message,
                                                 int lineNumber, const QString &sourceID)
{
    emit consoleMessage(QString("%1:%2> %3").arg(sourceID).arg(lineNumber).arg(message));
}
