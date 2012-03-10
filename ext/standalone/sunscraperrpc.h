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

    struct Header {
        quint32 queryId;
        quint32 requestType;
        quint32 dataLength;
    };

    struct LoadUrlRequest {
        quint32 htmlLength;
        quint32 baseUrlLength;
    };

    enum Request {
        RPC_LOAD_URL  = 1,
        RPC_LOAD_HTML = 2,
        RPC_WAIT      = 3,
        RPC_FETCH     = 4,
        RPC_FINALIZE  = 5,
    };

public:
    SunscraperRPC(QString socketPath);
    ~SunscraperRPC();

private slots:
    void onInputReadable();
    void onInputDisconnected();

    void onFinish(unsigned queryId);
    void onTimeout(unsigned queryId);
    void onFetchDone(unsigned queryId, QString data);

private:
    QLocalSocket *m_socket;

    State  m_state;
    Header m_pendingHeader;
    QByteArray m_buffer;

    SunscraperWorker *m_worker;

    QMap<unsigned, bool> m_results;

    SunscraperRPC();

    void processRequest(unsigned queryId, unsigned requestType, QByteArray data);
    void sendReply(unsigned queryId, unsigned requestType, QByteArray data);
};

#endif // SUNSCRAPERRPC_H
