#include <QApplication>
#include "sunscraperworker.h"
#include "sunscraperrpc.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    SunscraperRPC rpc;

    return app.exec();
}
