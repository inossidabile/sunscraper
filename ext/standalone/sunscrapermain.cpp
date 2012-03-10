#include <QApplication>
#include <QStringList>
#include "sunscraperworker.h"
#include "sunscraperrpcserver.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    app.setApplicationName("Sunscraper-Standalone");

    SunscraperWorker::unlock();

    SunscraperRPCServer *rpcServer = new SunscraperRPCServer();

    QString socketPath = app.arguments().at(1);
    if(!rpcServer->listen(socketPath)) {
        qFatal("Cannot listen on %s", socketPath.toLocal8Bit().constData());
    }

    app.exec();

    qFatal("finished");
}
