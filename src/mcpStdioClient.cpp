#include "mcpStdioClient.h"

#include <QJsonDocument>
#include <QJsonParseError>

McpStdioClient::McpStdioClient(QObject *parent)
    : QObject(parent),
      m_process(new QProcess(this)),
      m_nextId(1)
{
    m_process->setProcessChannelMode(QProcess::SeparateChannels);

    connect(m_process, &QProcess::readyReadStandardOutput,
            this, &McpStdioClient::onReadyReadStdout);
    connect(m_process, &QProcess::readyReadStandardError,
            this, &McpStdioClient::onReadyReadStderr);
    connect(m_process, &QProcess::errorOccurred,
            this, &McpStdioClient::onProcessError);
    connect(m_process,
            static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this, &McpStdioClient::onProcessFinished);
    connect(m_process, &QProcess::started,
            this, &McpStdioClient::processStarted);
}

bool McpStdioClient::start(const QString &program, const QStringList &arguments)
{
    if (isRunning()) {
        return true;
    }

    m_stdoutBuffer.clear();
    m_stderrBuffer.clear();
    m_process->setProgram(program);
    m_process->setArguments(arguments);
    m_process->start();
    return m_process->state() != QProcess::NotRunning;
}

void McpStdioClient::stop()
{
    if (!isRunning()) {
        return;
    }
    m_process->terminate();
    if (!m_process->waitForFinished(1500)) {
        m_process->kill();
    }
}

bool McpStdioClient::isRunning() const
{
    return m_process->state() != QProcess::NotRunning;
}

int McpStdioClient::sendRequest(const QString &method, const QJsonObject &params)
{
    int id = nextId();
    sendRequestWithId(id, method, params);
    return id;
}

void McpStdioClient::sendRequestWithId(int id, const QString &method, const QJsonObject &params)
{
    QJsonObject obj;
    obj.insert("jsonrpc", "2.0");
    obj.insert("id", id);
    obj.insert("method", method);
    obj.insert("params", params);
    writeJson(obj);
}

void McpStdioClient::sendNotification(const QString &method, const QJsonObject &params)
{
    QJsonObject obj;
    obj.insert("jsonrpc", "2.0");
    obj.insert("method", method);
    obj.insert("params", params);
    writeJson(obj);
}

int McpStdioClient::initialize(const QString &protocolVersion,
                               const QString &clientName,
                               const QString &clientVersion)
{
    QJsonObject clientInfo;
    clientInfo.insert("name", clientName);
    clientInfo.insert("version", clientVersion);

    QJsonObject params;
    params.insert("protocolVersion", protocolVersion);
    params.insert("clientInfo", clientInfo);
    params.insert("capabilities", QJsonObject());

    return sendRequest("initialize", params);
}

void McpStdioClient::sendInitialized()
{
    sendNotification("initialized", QJsonObject());
}

void McpStdioClient::onReadyReadStdout()
{
    m_stdoutBuffer.append(m_process->readAllStandardOutput());
    int newlineIndex = m_stdoutBuffer.indexOf('\n');
    while (newlineIndex >= 0) {
        QByteArray line = m_stdoutBuffer.left(newlineIndex);
        m_stdoutBuffer.remove(0, newlineIndex + 1);
        handleIncomingLine(line);
        newlineIndex = m_stdoutBuffer.indexOf('\n');
    }
}

void McpStdioClient::onReadyReadStderr()
{
    m_stderrBuffer.append(m_process->readAllStandardError());
    int newlineIndex = m_stderrBuffer.indexOf('\n');
    while (newlineIndex >= 0) {
        QByteArray line = m_stderrBuffer.left(newlineIndex);
        m_stderrBuffer.remove(0, newlineIndex + 1);
        line = line.trimmed();
        if (!line.isEmpty()) {
            emit stdioError(QString::fromUtf8(line));
        }
        newlineIndex = m_stderrBuffer.indexOf('\n');
    }
}

void McpStdioClient::onProcessError(QProcess::ProcessError error)
{
    emit processFailed(error);
}

void McpStdioClient::onProcessFinished(int exitCode, QProcess::ExitStatus status)
{
    emit processStopped(exitCode, status);
}

void McpStdioClient::handleIncomingLine(const QByteArray &line)
{
    QByteArray trimmed = line.trimmed();
    if (trimmed.isEmpty()) {
        return;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(trimmed, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        emit stdioError(QString::fromUtf8("Invalid JSON from MCP server: %1")
                        .arg(QString::fromUtf8(trimmed)));
        return;
    }

    emit jsonMessageReceived(doc.object());
}

void McpStdioClient::writeJson(const QJsonObject &obj)
{
    if (!isRunning()) {
        emit stdioError("MCP process is not running.");
        return;
    }

    QJsonDocument doc(obj);
    QByteArray payload = doc.toJson(QJsonDocument::Compact);
    payload.append('\n');
    m_process->write(payload);
}

int McpStdioClient::nextId()
{
    return m_nextId++;
}
