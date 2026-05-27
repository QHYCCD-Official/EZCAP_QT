#ifndef MCP_IPC_SERVER_H
#define MCP_IPC_SERVER_H

#include <QObject>
#include <QHash>
#include <QJsonObject>
#include <QPointer>

class QLocalServer;
class QLocalSocket;
class EZCAP;

class McpIpcServer : public QObject
{
    Q_OBJECT

public:
    explicit McpIpcServer(EZCAP *mainWindow, QObject *parent = nullptr);

    bool start(const QString &serverName, QString *errorMessage = nullptr);
    void stop();
    bool isRunning() const;

    static bool isEnabledByArgs(const QStringList &args);
    static bool isEnabledByEnv();
    static QString defaultServerName();

private slots:
    void onNewConnection();
    void onSocketReadyRead();
    void onSocketDisconnected();

private:
    void handleLine(QLocalSocket *socket, const QByteArray &line);
    void sendResponse(QLocalSocket *socket, const QJsonObject &response);
    QJsonObject makeError(int code, const QString &message, const QJsonValue &id) const;
    QJsonObject handleRequest(const QJsonObject &request, bool *isNotification, int *errorCode, QString *errorMessage) const;

    QPointer<EZCAP> m_mainWindow;
    QLocalServer *m_server;
    QString m_serverName;
    QHash<QLocalSocket *, QByteArray> m_buffers;
};

#endif // MCP_IPC_SERVER_H
