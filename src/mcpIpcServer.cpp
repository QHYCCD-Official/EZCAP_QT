#include "mcpIpcServer.h"

#include "ezCap.h"
#include "myStruct.h"

#include <QLocalServer>
#include <QLocalSocket>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QDateTime>
#include <QCoreApplication>
#include <QDebug>

McpIpcServer::McpIpcServer(EZCAP *mainWindow, QObject *parent)
    : QObject(parent),
      m_mainWindow(mainWindow),
      m_server(new QLocalServer(this))
{
    connect(m_server, &QLocalServer::newConnection,
            this, &McpIpcServer::onNewConnection);
}

bool McpIpcServer::start(const QString &serverName, QString *errorMessage)
{
    if (m_server->isListening()) {
        return true;
    }

    m_serverName = serverName;
    QLocalServer::removeServer(serverName);
    if (!m_server->listen(serverName)) {
        if (errorMessage) {
            *errorMessage = m_server->errorString();
        }
        return false;
    }
    return true;
}

void McpIpcServer::stop()
{
    if (!m_server->isListening()) {
        return;
    }
    m_server->close();
    if (!m_serverName.isEmpty()) {
        QLocalServer::removeServer(m_serverName);
    }
    m_buffers.clear();
}

bool McpIpcServer::isRunning() const
{
    return m_server->isListening();
}

bool McpIpcServer::isEnabledByArgs(const QStringList &args)
{
    return args.contains("--mcp-ipc");
}

bool McpIpcServer::isEnabledByEnv()
{
    return qEnvironmentVariableIsSet("EZCAP_MCP_IPC")
        && qgetenv("EZCAP_MCP_IPC") == "1";
}

QString McpIpcServer::defaultServerName()
{
    return QStringLiteral("ezcap_mcp");
}

void McpIpcServer::onNewConnection()
{
    while (m_server->hasPendingConnections()) {
        QLocalSocket *socket = m_server->nextPendingConnection();
        if (!socket) {
            continue;
        }
        m_buffers.insert(socket, QByteArray());
        connect(socket, &QLocalSocket::readyRead,
                this, &McpIpcServer::onSocketReadyRead);
        connect(socket, &QLocalSocket::disconnected,
                this, &McpIpcServer::onSocketDisconnected);
    }
}

void McpIpcServer::onSocketReadyRead()
{
    QLocalSocket *socket = qobject_cast<QLocalSocket *>(sender());
    if (!socket) {
        return;
    }
    QByteArray &buffer = m_buffers[socket];
    buffer.append(socket->readAll());

    int newlineIndex = buffer.indexOf('\n');
    while (newlineIndex >= 0) {
        QByteArray line = buffer.left(newlineIndex);
        buffer.remove(0, newlineIndex + 1);
        handleLine(socket, line);
        newlineIndex = buffer.indexOf('\n');
    }
}

void McpIpcServer::onSocketDisconnected()
{
    QLocalSocket *socket = qobject_cast<QLocalSocket *>(sender());
    if (!socket) {
        return;
    }
    m_buffers.remove(socket);
    socket->deleteLater();
}

void McpIpcServer::handleLine(QLocalSocket *socket, const QByteArray &line)
{
    QByteArray trimmed = line.trimmed();
    if (trimmed.isEmpty()) {
        return;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(trimmed, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        QJsonObject error = makeError(-32700, QStringLiteral("Parse error"), QJsonValue(QJsonValue::Null));
        sendResponse(socket, error);
        return;
    }

    bool isNotification = false;
    int errorCode = 0;
    QString errorMessage;
    QJsonObject response = handleRequest(doc.object(), &isNotification, &errorCode, &errorMessage);
    if (isNotification) {
        return;
    }

    if (errorCode != 0) {
        QJsonObject error = makeError(errorCode, errorMessage, doc.object().value("id"));
        sendResponse(socket, error);
        return;
    }
    sendResponse(socket, response);
}

QJsonObject McpIpcServer::makeError(int code, const QString &message, const QJsonValue &id) const
{
    QJsonObject errorObject;
    errorObject.insert("code", code);
    errorObject.insert("message", message);

    QJsonObject response;
    response.insert("jsonrpc", "2.0");
    response.insert("id", id.isUndefined() ? QJsonValue() : id);
    response.insert("error", errorObject);
    return response;
}

QJsonObject McpIpcServer::handleRequest(const QJsonObject &request, bool *isNotification, int *errorCode, QString *errorMessage) const
{
    if (!request.contains("method") || !request.value("method").isString()) {
        if (errorCode) {
            *errorCode = -32600;
        }
        if (errorMessage) {
            *errorMessage = QStringLiteral("Invalid Request");
        }
        return QJsonObject();
    }

    QString method = request.value("method").toString();
    QJsonValue idValue = request.value("id");
    if (isNotification) {
        *isNotification = idValue.isUndefined();
    }

    QJsonObject result;
    if (method == QLatin1String("app.ping")) {
        result.insert("status", "ok");
        result.insert("time", QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    } else if (method == QLatin1String("app.info")) {
        result.insert("name", QCoreApplication::applicationName());
        result.insert("pid", static_cast<qint64>(QCoreApplication::applicationPid()));
        result.insert("qtVersion", QT_VERSION_STR);
        result.insert("buildTime", QStringLiteral(__DATE__ " " __TIME__));
        if (m_mainWindow) {
            result.insert("version", m_mainWindow->EZCAP_VER);
            result.insert("releaseTime", m_mainWindow->RELEASE_TIME);
        }
    } else if (method == QLatin1String("camera.status")) {
        result.insert("connected", ix.isConnected);
        result.insert("camId", ix.CamID);
        result.insert("camModel", ix.CamModel);
        result.insert("cameraState", ix.cameraState);
        result.insert("workMode", ix.workMode);
        result.insert("fps", ix.fps);
        result.insert("liveMode", ix.onLiveMode);
    } else if (method == QLatin1String("help.methods")) {
        QJsonArray methods;
        methods.append("app.ping");
        methods.append("app.info");
        methods.append("camera.status");
        methods.append("help.methods");
        result.insert("methods", methods);
    } else {
        if (errorCode) {
            *errorCode = -32601;
        }
        if (errorMessage) {
            *errorMessage = QStringLiteral("Method not found");
        }
        return QJsonObject();
    }

    QJsonObject response;
    response.insert("jsonrpc", "2.0");
    response.insert("id", idValue.isUndefined() ? QJsonValue() : idValue);
    response.insert("result", result);
    return response;
}

void McpIpcServer::sendResponse(QLocalSocket *socket, const QJsonObject &response)
{
    if (!socket) {
        return;
    }
    QJsonDocument doc(response);
    QByteArray payload = doc.toJson(QJsonDocument::Compact);
    payload.append('\n');
    socket->write(payload);
    socket->flush();
}
