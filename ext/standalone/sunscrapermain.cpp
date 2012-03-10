#include <QApplication>
#include <QStringList>
#include "sunscraperworker.h"
#include "sunscraperrpc.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    app.setApplicationName("Sunscraper-Standalone");

    SunscraperWorker::unlock();

    SunscraperRPC rpc(app.arguments().at(1));

    return app.exec();
}
