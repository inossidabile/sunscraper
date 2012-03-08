#ifndef SUNSCRAPERRPC_H
#define SUNSCRAPERRPC_H

#include <QObject>
#include <QVector>
#include <QMap>

class SunscraperWorker;
class QSocketNotifier;
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
    SunscraperRPC();
    ~SunscraperRPC();

private slots:
    void onStdinReadable();
    void onPageRendered(unsigned queryId, QString data);
    void onTimeout();

private:
    QSocketNotifier *m_stdinNotifier;

    State  m_state;
    Header m_pendingHeader;
    QByteArray m_buffer;

    SunscraperWorker *m_worker;

    QList<unsigned> m_waitQueue;
    QMap<unsigned, QTimer*> m_timers;
    QMap<unsigned, QString> m_results;

    void processRequest(Header header, QByteArray data);
    void sendReply(Header header, QByteArray data);
};

#endif // SUNSCRAPERRPC_H
