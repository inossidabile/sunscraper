#ifndef SUNSCRAPERRPC_H
#define SUNSCRAPERRPC_H

#include <QObject>
#include <QVector>
#include <QMap>

class SunscraperWorker;
class QLocalSocket;
class QTimer;

class SunscraperRPC : public QObject
{
    Q_OBJECT

    enum State {
        StateHeader = 0,
        StateData,
    };

    struct Header {
        quint32 queryId;
        quint32 requestType;
        quint32 dataLength;
    };

    enum Request {
        RPC_LOAD_HTML = 1,
        RPC_LOAD_URL  = 2,
        RPC_WAIT      = 3,
        RPC_FETCH     = 4,
        RPC_DISCARD   = 5,
    };

public:
    SunscraperRPC(QString socketPath);
    ~SunscraperRPC();

private slots:
    void onInputReadable();
    void onInputDisconnected();
    void onPageRendered(unsigned queryId, QString data);
    void onTimeout();

private:
    QLocalSocket *m_socket;

    State  m_state;
    Header m_pendingHeader;
    QByteArray m_buffer;

    SunscraperWorker *m_worker;

    QList<unsigned> m_waitQueue;
    QMap<unsigned, QTimer*> m_timers;
    QMap<unsigned, QString> m_results;

    SunscraperRPC();

    void processRequest(Header header, QByteArray data);
    void sendReply(Header header, QByteArray data);
};

#endif // SUNSCRAPERRPC_H
