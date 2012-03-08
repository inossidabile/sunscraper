#include <QFile>
#include <QSocketNotifier>
#include <QTimer>
#include <QDataStream>
#include <QApplication>
#include <QtDebug>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <arpa/inet.h>
#include "sunscraperrpc.h"
#include "sunscraperworker.h"

SunscraperRPC::SunscraperRPC() :
        m_state(StateHeader)
{
    fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL) | O_NONBLOCK);

    m_stdinNotifier = new QSocketNotifier(STDIN_FILENO, QSocketNotifier::Read);
    connect(m_stdinNotifier, SIGNAL(activated(int)), this, SLOT(onStdinReadable()));

    m_worker = new SunscraperWorker();
    connect(m_worker, SIGNAL(finished(uint,QString)), this, SLOT(onPageRendered(uint,QString)));

    write(STDOUT_FILENO, ".", 1);
}

SunscraperRPC::~SunscraperRPC()
{
    delete m_worker;
}

void SunscraperRPC::onStdinReadable()
{
    char buf[1024];

    m_stdinNotifier->setEnabled(false);

    while(true) {
        int result = read(STDIN_FILENO, buf, sizeof(buf));

        if(result > 0) {
            m_buffer += QByteArray(buf, result);
        } else if(result == -1 && errno == EWOULDBLOCK) {
            break;
        } else if(result == 0) {
            QApplication::exit(0);
        } else qFatal("Cannot read: %d, %d", result, errno);
    }

    m_stdinNotifier->setEnabled(true);

    bool moreData = true;
    while(moreData) {
        switch(m_state) {
        case StateHeader:
            if(m_buffer.length() >= sizeof(Header)) {
                memcpy((void*) &m_pendingHeader, m_buffer.constData(), sizeof(Header));
                m_buffer.remove(0, sizeof(Header));
                m_state = StateData;
            } else {
                moreData = false;
            }

            break;

        case StateData:
            unsigned length = ntohl(m_pendingHeader.dataLength);

            if(m_buffer.length() >= length) {
                QByteArray data = m_buffer.left(length);
                m_buffer.remove(0, length);
                processRequest(m_pendingHeader, data);
                m_state = StateHeader;
            } else {
                moreData = false;
            }

            break;
        }
    }
}

void SunscraperRPC::processRequest(Header header, QByteArray data)
{
    unsigned queryId, requestType;

    queryId     = ntohl(header.queryId);
    requestType = ntohl(header.requestType);

    switch(requestType) {
    case RPC_LOAD_HTML: {
            m_worker->loadHtml(queryId, data);

            break;
        }

    case RPC_LOAD_URL: {
            m_worker->loadUrl(queryId, data);

            break;
        }

    case RPC_WAIT: {
            if(m_results.contains(queryId)) {
                Header reply;
                reply.queryId     = htonl(queryId);
                reply.requestType = htonl(RPC_WAIT);

                sendReply(reply, QByteArray());
            } else {
                Q_ASSERT(!m_waitQueue.contains(queryId));
                Q_ASSERT(!m_timers.contains(queryId));

                m_waitQueue.append(queryId);

                unsigned timeout;

                QDataStream stream(data);
                stream >> timeout;

                QTimer *timer = new QTimer(this);
                timer->setInterval(timeout);
                timer->setSingleShot(true);
                timer->start();
                connect(timer, SIGNAL(timeout()), this, SLOT(onTimeout()));

                m_timers[queryId] = timer;
            }

            break;
        }

    case RPC_FETCH: {
            Header reply;
            reply.queryId     = htonl(queryId);
            reply.requestType = htonl(RPC_FETCH);

            if(m_results.contains(queryId)) {
                sendReply(reply, m_results[queryId].toLocal8Bit());
            } else {
                sendReply(reply, "!SUNSCRAPER_TIMEOUT");
            }

            break;
        }

    case RPC_DISCARD: {
            m_results.remove(queryId);
            m_waitQueue.removeAll(queryId);

            if(m_timers.contains(queryId)) {
                QTimer *timer = m_timers[queryId];
                delete timer;

                m_timers.remove(queryId);
            }

            m_worker->finalize(queryId);

            break;
        }
    }
}

void SunscraperRPC::onPageRendered(unsigned queryId, QString data)
{
    m_results[queryId] = data;

    if(m_waitQueue.contains(queryId)) {
        Header reply;
        reply.queryId     = htonl(queryId);
        reply.requestType = htonl(RPC_WAIT);

        sendReply(reply, QByteArray());
    }
}

void SunscraperRPC::onTimeout()
{
    QTimer *timer = static_cast<QTimer*>(QObject::sender());
    unsigned queryId = m_timers.key(timer);

    Header reply;
    reply.queryId     = htonl(queryId);
    reply.requestType = htonl(RPC_WAIT);

    sendReply(reply, QByteArray());
}

void SunscraperRPC::sendReply(Header header, QByteArray data)
{
    header.dataLength = htonl(data.length());

    QByteArray serialized((const char*) &header, sizeof(Header));
    serialized.append(data);

    write(STDOUT_FILENO, serialized.constData(), serialized.length());
}
