#ifndef MSGCLIENT_H
#define MSGCLIENT_H

#include <QtCore/QObject>
#include <QTimer>
#include <QtWebSockets/QWebSocket>
#include <QAbstractSocket>

#include "myStruct.h"

class MsgClient : public QObject
{
    Q_OBJECT
public:
    explicit MsgClient(const QUrl &url, QString clientName, bool debug = false, QObject *parent = nullptr);

    void sendTextMessage(QString message);
    void sendEncodingTextMessage(QString message);
    void sendBinaryMessage(QByteArray message);
    void sendTestMessage();
    void sendClientRequest();
    void sendClientEzcapSetAVI(QString destClientName, bool startOrStop);
    void sendClientEzcapSetWindow(QString destClientName, bool maxOrNormal);
    void keepClientAlive();
    void OpenConnection();

    QString protocolVersion;

signals:
    void closed();
    void newMessage(const QString &message,const QString &senderId);
    void connected();
    void setAVI(bool startOrStop);
    void setWindow(bool maxOrNormal);
    void clientConnected(QString clientName);
    void clientDisconnected(QString clientName);
    void initClientList(QStringList clientNames);
    void netWorkError(QString errorMessage);

private Q_SLOTS:
    void onConnected();
    void onError(QAbstractSocket::SocketError error);
    void onTextMessageReceived(QString message);
    void onBinaryMessageReceived(QByteArray message);



private:
    QWebSocket m_webSocket;
    QUrl m_url;
    bool m_debug;
    QString socketClientName;
    QTimer* clientLiveKeepTimer;
};

#endif // MSGCLIENT_H
