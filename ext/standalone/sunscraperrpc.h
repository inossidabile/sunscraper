#ifndef SUNSCRAPERRPC_H
#define SUNSCRAPERRPC_H

#include <QObject>
#include <QVector>
#include <QMap>

class SunscraperWorker;
class QLocalSocket;

class SunscraperRPC : public QObject
{
    Q_OBJECT

    enum State {
        StateHeader = 0,
        StateData,
    };

    enum Request {
        RPC_LOAD_URL  = 1,
        RPC_LOAD_HTML = 2,
        RPC_WAIT      = 3,
        RPC_FETCH     = 4,
    };

public:
    SunscraperRPC(QLocalSocket *socket);
    ~SunscraperRPC();

signals:
    void disconnected();

private slots:
    void onInputReadable();
    void onInputDisconnected();

    void onFinish(unsigned queryId);
    void onTimeout(unsigned queryId);
    void onFetchDone(unsigned queryId, QString data);

private:
    static unsigned m_nextQueryId;
    static SunscraperWorker *m_worker;

    unsigned m_queryId;
    QLocalSocket *m_socket;

    State      m_state;
    unsigned   m_pendingRequest, m_pendingDataLength;
    QByteArray m_buffer;

    bool       m_result;

    SunscraperRPC();

    void processRequest(unsigned requestType, QByteArray data);
    void sendReply(QByteArray data);
};

#endif // SUNSCRAPERRPC_H
