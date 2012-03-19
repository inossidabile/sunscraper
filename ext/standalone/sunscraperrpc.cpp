#include <QLocalSocket>
#include <QTimer>
#include <QDataStream>
#include <QApplication>
#include <QtDebug>
#include <arpa/inet.h>
#include <sunscraperworker.h>
#include "sunscraperrpc.h"

//#define DEBUG_SUNSCRAPERRPC

SunscraperWorker *SunscraperRPC::m_worker;
unsigned SunscraperRPC::m_nextQueryId;

SunscraperRPC::SunscraperRPC(QLocalSocket *socket) :
        m_socket(socket), m_state(StateHeader), m_result(false)
{
    m_nextQueryId += 1;
    m_queryId = m_nextQueryId;

    connect(m_socket, SIGNAL(readyRead()), this, SLOT(onInputReadable()));
    connect(m_socket, SIGNAL(disconnected()), this, SLOT(onInputDisconnected()));

    if(m_worker == NULL)
        m_worker = new SunscraperWorker();

    connect(m_worker, SIGNAL(finished(uint)), this, SLOT(onFinish(uint)));
    connect(m_worker, SIGNAL(timedOut(uint)), this, SLOT(onTimeout(uint)));
    connect(m_worker, SIGNAL(htmlFetched(uint,QString)), this, SLOT(onFetchDone(uint,QString)));
}

SunscraperRPC::~SunscraperRPC()
{
    delete m_socket;
}

void SunscraperRPC::onInputReadable()
{
    m_buffer += m_socket->readAll();

    bool moreData = true;
    while(moreData) {
        switch(m_state) {
        case StateHeader:
            if((unsigned) m_buffer.length() >= sizeof(quint32) * 2) {
                QDataStream stream(m_buffer);
                stream >> (quint32&) m_pendingRequest;
                stream >> (quint32&) m_pendingDataLength;

                m_buffer.remove(0, sizeof(quint32) * 2);

                m_state = StateData;
            } else {
                moreData = false;
            }

            break;

        case StateData:
            if((unsigned) m_buffer.length() >= m_pendingDataLength) {
                QByteArray data = m_buffer.left(m_pendingDataLength);
                m_buffer.remove(0, m_pendingDataLength);

                processRequest(m_pendingRequest, data);

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
    m_worker->finalize(m_queryId);

    emit disconnected();
}

void SunscraperRPC::processRequest(unsigned requestType, QByteArray data)
{
#ifdef DEBUG_SUNSCRAPERRPC
    qDebug() << QString("request(%1)").arg(m_queryId) << requestType << data;
#endif

    switch(requestType) {
    case RPC_LOAD_HTML: {
            QDataStream stream(data);

            QByteArray html;
            stream >> html;

            QByteArray baseUrl;
            stream >> baseUrl;

            m_worker->loadHtml(m_queryId, html, QUrl(baseUrl));

            break;
        }

    case RPC_LOAD_URL: {
            m_worker->loadUrl(m_queryId, QUrl(data));

            break;
        }

    case RPC_WAIT: {
            if(!m_result) {
                QDataStream stream(data);

                unsigned timeout;
                stream >> timeout;

                m_worker->setTimeout(m_queryId, timeout);
            }

            break;
        }

    case RPC_FETCH: {
            m_worker->fetchHtml(m_queryId);

            break;
        }
    }
}

void SunscraperRPC::onFinish(unsigned eventQueryId)
{
    if(eventQueryId != m_queryId)
        return;

#ifdef DEBUG_SUNSCRAPERRPC
    qDebug() << QString("finish(%1)").arg(m_queryId);
#endif

    QByteArray data;

    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << (int) true;

    sendReply(data);

    m_result = true;
}

void SunscraperRPC::onTimeout(unsigned eventQueryId)
{
    if(eventQueryId != m_queryId)
        return;

#ifdef DEBUG_SUNSCRAPERRPC
    qDebug() << QString("timeout(%1)").arg(m_queryId);
#endif

    QByteArray data;

    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << (int) false;

    sendReply(data);

    m_result = false;
}

void SunscraperRPC::onFetchDone(unsigned eventQueryId, QString data)
{
    if(eventQueryId != m_queryId)
        return;

#ifdef DEBUG_SUNSCRAPERRPC
    qDebug() << QString("fetchDone(%1)").arg(m_queryId);
#endif

    sendReply(data.toLocal8Bit());
}

void SunscraperRPC::sendReply(QByteArray data)
{
    QByteArray packet;

#ifdef DEBUG_SUNSCRAPERRPC
    qDebug() << QString("reply(%1)").arg(m_queryId) << data;
#endif

    QDataStream stream(&packet, QIODevice::WriteOnly);
    stream << data;

    m_socket->write(packet);
}
