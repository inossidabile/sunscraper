#ifndef SUNSCRAPERRPCSERVER_H
#define SUNSCRAPERRPCSERVER_H

#include <QObject>

class QLocalServer;

class SunscraperRPCServer : public QObject
{
    Q_OBJECT

public:
    SunscraperRPCServer(QObject *parent = 0);

    bool listen(QString socketPath);

private slots:
    void onNewConnection();

private:
    QLocalServer *m_localServer;
};

#endif
