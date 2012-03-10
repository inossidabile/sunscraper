#include <QLocalSocket>
#include <QTimer>
#include <QDataStream>
#include <QApplication>
#include <QtDebug>
#include <arpa/inet.h>
#include <sunscraperworker.h>
#include "sunscraperrpc.h"

SunscraperRPC::SunscraperRPC(QString socketPath) :
        m_state(StateHeader)
{
    m_socket = new QLocalSocket(this);
    m_socket->connectToServer(socketPath);
    connect(m_socket, SIGNAL(readyRead()), this, SLOT(onInputReadable()));
    connect(m_socket, SIGNAL(disconnected()), this, SLOT(onInputDisconnected()));

    m_worker = new SunscraperWorker(this);
    connect(m_worker, SIGNAL(finished(uint)), this, SLOT(onFinish(uint)));
    connect(m_worker, SIGNAL(timedOut(uint)), this, SLOT(onTimeout(uint)));
    connect(m_worker, SIGNAL(htmlFetched(uint,QString)), this, SLOT(onFetchDone(uint,QString)));
}

SunscraperRPC::~SunscraperRPC()
{
    delete m_worker;
}

void SunscraperRPC::onInputReadable()
{
    m_buffer += m_socket->readAll();

    bool moreData = true;
    while(moreData) {
        switch(m_state) {
        case StateHeader:
            if((unsigned) m_buffer.length() >= sizeof(Header)) {
                memcpy((void*) &m_pendingHeader, m_buffer.constData(), sizeof(Header));
                m_buffer.remove(0, sizeof(Header));
                m_state = StateData;
            } else {
                moreData = false;
            }

            break;

        case StateData:
            unsigned length = ntohl(m_pendingHeader.dataLength);

            if((unsigned) m_buffer.length() >= length) {
                QByteArray data = m_buffer.left(length);
                m_buffer.remove(0, length);

                unsigned queryId, requestType;

                queryId     = ntohl(m_pendingHeader.queryId);
                requestType = ntohl(m_pendingHeader.requestType);

                processRequest(queryId, requestType, data);

                m_state = StateHeader;
            } else {
                moreData = false;
            }

            break;
        }
    }
}

void SunscraperRPC::onInputDisconnected()
{
    QApplication::exit();
}

void SunscraperRPC::processRequest(unsigned queryId, unsigned requestType, QByteArray data)
{
    switch(requestType) {
    case RPC_LOAD_HTML: {
            QDataStream stream(data);

            QByteArray html;
            stream >> html;

            QByteArray baseUrl;
            stream >> baseUrl;

            m_worker->loadHtml(queryId, html, QUrl(baseUrl));

            break;
        }

    case RPC_LOAD_URL: {
            m_worker->loadUrl(queryId, QUrl(data));

            break;
        }

    case RPC_WAIT: {
            if(m_results[queryId]) {
                onFinish(queryId);
            } else {
                QDataStream stream(data);

                unsigned timeout;
                stream >> timeout;

                m_worker->setTimeout(queryId, timeout);
            }

            break;
        }

    case RPC_FETCH: {
            m_worker->fetchHtml(queryId);

            break;
        }

    case RPC_FINALIZE: {
            m_results.remove(queryId);

            m_worker->finalize(queryId);

            break;
        }
    }
}

void SunscraperRPC::onFinish(unsigned queryId)
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    stream << (int) true;

    sendReply(queryId, RPC_WAIT, data);

    m_results[queryId] = true;
}

void SunscraperRPC::onTimeout(unsigned queryId)
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    stream << (int) false;

    sendReply(queryId, RPC_WAIT, data);

    m_results[queryId] = true;
}

void SunscraperRPC::onFetchDone(unsigned queryId, QString data)
{
    sendReply(queryId, RPC_FETCH, data.toLocal8Bit());
}

void SunscraperRPC::sendReply(unsigned queryId, unsigned requestType, QByteArray data)
{
    Header header;

    header.queryId     = ntohl(queryId);
    header.requestType = ntohl(requestType);
    header.dataLength  = htonl(data.length());

    QByteArray serialized((const char*) &header, sizeof(Header));
    serialized.append(data);

    m_socket->write(serialized);
}
