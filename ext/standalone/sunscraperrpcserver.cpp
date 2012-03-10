#include <QLocalServer>
#include "sunscraperrpcserver.h"
#include "sunscraperrpc.h"

SunscraperRPCServer::SunscraperRPCServer(QObject *parent) :
        QObject(parent)
{
    m_localServer = new QLocalServer();

    connect(m_localServer, SIGNAL(newConnection()), this, SLOT(onNewConnection()));
}

bool SunscraperRPCServer::listen(QString socketPath)
{
    return m_localServer->listen(socketPath);
}

void SunscraperRPCServer::onNewConnection()
{
    while(m_localServer->hasPendingConnections()) {
        QLocalSocket *socket = m_localServer->nextPendingConnection();

        SunscraperRPC *rpc = new SunscraperRPC(socket);
        connect(rpc, SIGNAL(disconnected()), rpc, SLOT(deleteLater()));
    }
}
