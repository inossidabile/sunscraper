#include <QFile>
#include <QtDebug>
#include <unistd.h>
#include "sunscraperrpc.h"
#include "sunscraperworker.h"

SunscraperRPC::SunscraperRPC()
{
    m_stdin = new QFile();
    m_stdin->open(STDIN_FILENO, QIODevice::ReadOnly | QIODevice::Unbuffered);

    connect(m_stdin, SIGNAL(readyRead()), this, SLOT(onStdinReadable()));

    m_stdout = new QFile();
    m_stdout->open(STDOUT_FILENO, QIODevice::WriteOnly | QIODevice::Unbuffered);

    m_worker = new SunscraperWorker();

    connect(m_worker, SIGNAL(finished(uint,QString)), this, SLOT(onPageRendered(uint,QString)));

    m_stdout->write(".");
}

SunscraperRPC::~SunscraperRPC()
{
    delete m_stdin;
    delete m_stdout;
    delete m_worker;
}

void SunscraperRPC::onStdinReadable()
{
    qDebug() << "buf" << m_buffer.length();

    m_buffer += m_stdin->readAll();

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
            if(m_buffer.length() >= m_pendingHeader.dataLength) {
                QByteArray data = m_buffer.remove(0, m_pendingHeader.dataLength);
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
    qDebug() << "req" << header.queryId << header.requestType;

    switch(header.requestType) {
    case RPC_LOAD_HTML: {
            m_worker->loadHtml(header.queryId, data);

            break;
        }

    case RPC_LOAD_URL: {
            m_worker->loadUrl(header.queryId, data);

            break;
        }

    case RPC_WAIT: {
            m_waitQueue.append(header.queryId);

            break;
        }

    case RPC_FETCH: {
            Header reply;
            reply.queryId     = header.queryId;
            reply.requestType = RPC_FETCH;

            sendReply(reply, m_results[header.queryId].toLocal8Bit());

            break;
        }

    case RPC_DISCARD: {
            m_results.remove(header.queryId);
            m_waitQueue.removeAll(header.queryId);
            m_worker->finalize(header.queryId);

            break;
        }
    }
}

void SunscraperRPC::onPageRendered(unsigned queryId, QString data)
{
    m_results[queryId] = data;

    if(m_waitQueue.contains(queryId)) {
        Header reply;
        reply.queryId     = queryId;
        reply.requestType = RPC_WAIT;

        sendReply(reply, QByteArray());
    }
}

void SunscraperRPC::sendReply(Header header, QByteArray data)
{
    header.dataLength = data.length();

    QByteArray serialized((const char*) &header, sizeof(Header));
    serialized.append(data);

    m_stdout->write(serialized);
    m_stdout->flush();
}
