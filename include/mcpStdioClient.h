#ifndef MCP_STDIO_CLIENT_H
#define MCP_STDIO_CLIENT_H

#include <QObject>
#include <QProcess>
#include <QJsonObject>

class McpStdioClient : public QObject
{
    Q_OBJECT

public:
    explicit McpStdioClient(QObject *parent = nullptr);

    bool start(const QString &program, const QStringList &arguments = QStringList());
    void stop();
    bool isRunning() const;

    int sendRequest(const QString &method, const QJsonObject &params = QJsonObject());
    void sendRequestWithId(int id, const QString &method, const QJsonObject &params = QJsonObject());
    void sendNotification(const QString &method, const QJsonObject &params = QJsonObject());
    int initialize(const QString &protocolVersion, const QString &clientName, const QString &clientVersion);
    void sendInitialized();

signals:
    void jsonMessageReceived(const QJsonObject &message);
    void stdioError(const QString &message);
    void processStarted();
    void processStopped(int exitCode, QProcess::ExitStatus exitStatus);
    void processFailed(QProcess::ProcessError error);

private slots:
    void onReadyReadStdout();
    void onReadyReadStderr();
    void onProcessError(QProcess::ProcessError error);
    void onProcessFinished(int exitCode, QProcess::ExitStatus status);

private:
    void handleIncomingLine(const QByteArray &line);
    void writeJson(const QJsonObject &obj);
    int nextId();

    QProcess *m_process;
    QByteArray m_stdoutBuffer;
    QByteArray m_stderrBuffer;
    int m_nextId;
};

#endif // MCP_STDIO_CLIENT_H
