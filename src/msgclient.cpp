#include "msgclient.h"
#include "protocol.h"
#include "QJsonDocument"
#include "QVariantMap"
#include "QJsonObject"
#include <QAbstractSocket>


QT_USE_NAMESPACE

//! [constructor]
MsgClient::MsgClient(const QUrl &url, QString clientName, bool debug, QObject *parent) :
    QObject(parent),
    m_url(url),
    m_debug(debug),
    socketClientName(clientName)
{

    QTextStream(stdout) << tr("WebSocket server: %1 \n").arg(url.toString());
//    connect(&m_webSocket, &QWebSocket::error, [=](QAbstractSocket::SocketError error)
//    {
//        QTextStream(stdout)  << tr("socket Error: %1 \n").arg(m_webSocket.errorString());
//    });
//    connect(&m_webSocket, &QWebSocket::error,this,&MsgClient::onError);
    if(!connect(&m_webSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError))))
    {
        QTextStream(stdout) << "Failed to connect to QWebSocket::error" <<  QT_ENDL;

    }
    connect(&m_webSocket, &QWebSocket::connected, this, &MsgClient::onConnected);
    connect(&m_webSocket, &QWebSocket::disconnected, this, &MsgClient::closed);
////    m_webSocket.setProxy(QNetworkProxy::NoProxy);
//    m_webSocket.open(QUrl(url));
//    protocolVersion = protocol_version;
//    clientLiveKeepTimer = new QTimer(this);
//    connect(clientLiveKeepTimer,&QTimer::timeout,this,&MsgClient::keepClientAlive);
//    clientLiveKeepTimer->start(keep_alive_time_cycle);

}
//! [constructor]

//! [onConnected]
void MsgClient::onConnected()
{

    QTextStream(stdout) << "WebSocket connected";
    connect(&m_webSocket, &QWebSocket::textMessageReceived,
            this, &MsgClient::onTextMessageReceived);
    connect(&m_webSocket, &QWebSocket::binaryMessageReceived,
            this, &MsgClient::onBinaryMessageReceived);
    sendTestMessage();
}
void MsgClient::onError(QAbstractSocket::SocketError error)
{
    QTextStream(stdout)  << tr("socket Error: %1 \n").arg(error);
    emit netWorkError(tr("socket Error: %1 \n").arg(error));
}
//! [onConnected]

//! [onTextMessageReceived]
void MsgClient::onTextMessageReceived(QString message)
{
     QTextStream(stdout) << message << "  -> text from server \n";

     QTextStream(stdout) << "Message received:" << message;
//    m_webSocket.close();
    emit newMessage(message," [todo] ");

    QString		json = message;
    QJsonParseError error;
    QJsonDocument	jsonDocument = QJsonDocument::fromJson( json.toUtf8(), &error );
    QString messageSenderName="";
    QString messageTargetName="";

    int cmd_code = 0;
    if ( error.error == QJsonParseError::NoError )
    {
        if ( jsonDocument.isObject() )
        {
            QVariantMap result = jsonDocument.toVariant().toMap();
            cmd_code = result["cmd_code"].toInt();
            messageSenderName = result["cmd_src_client_name"].toString();
            messageTargetName = result["cmd_des_client_name"].toString();

            if(messageSenderName.isEmpty()){
                 QTextStream(stdout) <<tr("error: [%1] from [%2]").arg(cmd_code).arg(messageSenderName);
                return;
            }

            switch (cmd_code) {
                case cmd_code_test:
                    QTextStream(stdout) <<tr("test return: %1 %2").arg(result["cmd_name"].toString(),result["cmd_status"].toString());
                    sendClientRequest();
                    break;
                case cmd_code_client_list:{
                    QStringList clients = result["cmd_value_client_name"].toStringList();
                    emit initClientList(clients);
                    break;
                }
                case cmd_code_client_connect:{
                    QString clientName = result["cmd_value_client_name"].toString();
                    QTextStream(stdout) << tr("%1   =%2 \n").arg(cmd_name_client_connect).arg(clientName);
                    emit clientConnected(clientName);
                    break;
                }
                case cmd_code_client_remove:{
                    QString clientName = result["cmd_value_client_name"].toString();
                    QTextStream(stdout) << tr("%1   =%2 \n").arg(cmd_name_client_remove).arg(clientName);
                    emit clientDisconnected(clientName);
                    break;
                }
                case cmd_code_client_ezcap_save_avi:{
                    bool avi_value = result["cmd_value_client_ezcap_set_avi"].toBool();
                    QTextStream(stdout) << tr("%1   =%2 \n").arg(cmd_name_client_ezcap_save_avi).arg(avi_value);
                    emit setAVI(avi_value);
                    break;
                }
                case cmd_code_client_ezcap_max_window:{
                    bool window_value = result["cmd_value_client_ezcap_set_window"].toBool();
                    QTextStream(stdout) << tr("%1   =%2 \n").arg(cmd_name_client_ezcap_max_window).arg(window_value);
                    emit setWindow(window_value);
                    break;
                }
                case cmd_code_client_keep_alive:{
                    QTextStream(stdout)  << tr("keep alive \n");

                    break;
                }
                default:
                    QTextStream(stdout)  << tr("CMD code Error code = [%1]").arg(cmd_code);
                    break;

            }
        }else{
            QTextStream(stdout)  << tr("JsonDocument Error \n");
        }
    }else {
        QTextStream(stdout)  << tr("Error msg: %1 \n").arg(json);
    }
}
//! [onTextMessageReceived]


void MsgClient::sendTextMessage(QString message)
{
     QTextStream(stdout) << message << "  -> send to server \n";
    qint64 len = m_webSocket.sendTextMessage(message);
    QTextStream(stdout) << tr("  -> send to server [%1] \n").arg(len);
    if(len == 0){
        emit netWorkError("Send error occur");
    }
}


void MsgClient::onBinaryMessageReceived(QByteArray message)
{
     QTextStream(stdout) << message.length() << "  -> text from server new Message\n";
     emit newMessage(QString::fromStdString(message.toStdString()),"todo from server");
}

void MsgClient::sendEncodingTextMessage(QString message)
{

     QTextStream(stdout) << "Message Send:" << message;
     QTextStream(stdout) << message << "  ->Bytes send to server \n";
     QTextStream(stdout) << "Send length   =  " << m_webSocket.sendBinaryMessage(message.toUtf8());
}


void MsgClient::sendBinaryMessage(QByteArray message)
{

     QTextStream(stdout) << "Message Send:" << message.length();
     QTextStream(stdout) << message.length() << "  ->Bytes send to server \n";
     QTextStream(stdout) << "Send length   =  " << m_webSocket.sendBinaryMessage(message);
}

void MsgClient::sendTestMessage()
{
    QJsonObject commandDiscovery;
    commandDiscovery.insert("cmd_code",cmd_code_test);
    commandDiscovery.insert("cmd_name",cmd_name_test);
    commandDiscovery.insert("cmd_status",cmd_status_send);
    commandDiscovery.insert("cmd_src_client_name",socketClientName);


    QJsonDocument doc(commandDiscovery);
    QByteArray byteArray = doc.toJson();
    sendTextMessage(QString(byteArray));
}
void MsgClient::sendClientRequest()
{
    QJsonObject commandDiscovery;
    commandDiscovery.insert("cmd_code",cmd_code_client_list);
    commandDiscovery.insert("cmd_name",cmd_name_client_list);
    commandDiscovery.insert("cmd_status",cmd_status_send);
    commandDiscovery.insert("cmd_src_client_name",socketClientName);


    QJsonDocument doc(commandDiscovery);
    QByteArray byteArray = doc.toJson();
    sendTextMessage(QString(byteArray));
}

void MsgClient::sendClientEzcapSetAVI(QString destClientName, bool startOrStop)
{
    QJsonObject commandDiscovery;
    commandDiscovery.insert("cmd_code",cmd_code_client_ezcap_save_avi);
    commandDiscovery.insert("cmd_name",cmd_name_client_ezcap_save_avi);
    commandDiscovery.insert("cmd_status",cmd_status_send);
    commandDiscovery.insert("cmd_src_client_name",socketClientName);
    commandDiscovery.insert("cmd_des_client_name",destClientName);
    commandDiscovery.insert("cmd_value_client_ezcap_set_avi",startOrStop);

    QJsonDocument doc(commandDiscovery);
    QByteArray byteArray = doc.toJson();
    sendTextMessage(QString(byteArray));
}
void MsgClient::sendClientEzcapSetWindow(QString destClientName, bool maxOrNormal)
{
    QJsonObject commandDiscovery;
    commandDiscovery.insert("cmd_code",cmd_code_client_ezcap_max_window);
    commandDiscovery.insert("cmd_name",cmd_name_client_ezcap_max_window);
    commandDiscovery.insert("cmd_status",cmd_status_send);
    commandDiscovery.insert("cmd_src_client_name",socketClientName);
    commandDiscovery.insert("cmd_des_client_name",destClientName);
    commandDiscovery.insert("cmd_value_client_ezcap_set_window",maxOrNormal);

    QJsonDocument doc(commandDiscovery);
    QByteArray byteArray = doc.toJson();
    sendTextMessage(QString(byteArray));
}
void MsgClient::keepClientAlive()
{
    QJsonObject commandDiscovery;
    commandDiscovery.insert("cmd_code",cmd_code_client_keep_alive);
    commandDiscovery.insert("cmd_name",cmd_name_client_keep_alive);
    commandDiscovery.insert("cmd_status",cmd_status_send);
    commandDiscovery.insert("cmd_src_client_name",socketClientName);
//    commandDiscovery.insert("cmd_des_client_name",socketClientName);

    QJsonDocument doc(commandDiscovery);
    QByteArray byteArray = doc.toJson();
    sendTextMessage(QString(byteArray));
}
void MsgClient::OpenConnection()
{
        m_webSocket.setProxy(QNetworkProxy::NoProxy);
        m_webSocket.open(QUrl(m_url));
        protocolVersion = protocol_version;
        clientLiveKeepTimer = new QTimer(this);
        connect(clientLiveKeepTimer,&QTimer::timeout,this,&MsgClient::keepClientAlive);
        clientLiveKeepTimer->start(keep_alive_time_cycle);
}
