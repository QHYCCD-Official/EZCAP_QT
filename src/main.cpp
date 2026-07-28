#include <QApplication>
#include <QPushButton>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QDir>
#include <QDebug>
#include <QMessageBox>
#include <QStyleFactory>
#include <QLibrary>
#include <QDebug>
#include <QScreen>

#include "ezCap.h"
#include "mcpIpcServer.h"
#include "qhyccdstruct.h"
#include "windowsTimerResolution.h"

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
#include <QTextCodec>
#endif

QString logFileFullName;

char _mali_clz_lut[256] = {
    8, 7, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};
//回调函数实现debug信息到文件
void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    QString logText = "";
    switch (type)
    {
    case QtDebugMsg:
        //logText = QString("\tDebug\t %1 (%2:%3, %4)").arg(localMsg.constData()).arg(context.file).arg(context.line).arg(context.function);
        logText = QString("\tDebug\t %1").arg(localMsg.constData());
        break;
    case QtInfoMsg:
        logText = QString("\tInfo\t %1").arg(localMsg.constData());
        break;
    case QtWarningMsg:
        logText = QString("\tWarning\t %1").arg(localMsg.constData());
        break;
    case QtCriticalMsg:
        logText = QString("\tCritical\t %1").arg(localMsg.constData());
        break;
    case QtFatalMsg:
        logText = QString("\tFatal\t %1").arg(localMsg.constData());
        abort();
    }

    QDateTime dt = QDateTime::currentDateTime();//获取系统现在的时间
    QString current_date = dt.toString("yyyy/MM/dd hh:mm:ss");

    QFile outFile(logFileFullName);
    if(outFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
    {
        QTextStream logTextStream(&outFile);
        logTextStream << current_date << logText << QT_ENDL;

        outFile.flush();
        outFile.close();
    }
    else
    {
        qDebug() << "Failed to open log file.";
    }
}

static void installMsgHandler()
{
    //create a logfilename with current datatime
    QDateTime dt = QDateTime::currentDateTime();//get the current system datatime
    QString current_date = dt.toString("yyyy-MM-dd.hh-mm-ss");

    QString logPath = QCoreApplication::applicationDirPath() + "/log";
    logPath = QDir::toNativeSeparators(logPath);

    QDir logdir;
    if(!logdir.exists(logPath))
        logdir.mkdir(logPath); //if log folder not exists, make it

    QString logfileName = current_date + ".log";
    logFileFullName = logPath + "/" + logfileName;
    logFileFullName = QDir::toNativeSeparators(logFileFullName);

#ifdef QT_NO_DEBUG_OUTPUT
    qInstallMessageHandler(myMessageOutput);
#else
    qInstallMessageHandler(0);  //To restore the message handler, will output to console
#endif

}

int main(int argc, char *argv[])
{
#ifdef Q_OS_WIN32

#if(QT_VERSION >= QT_VERSION_CHECK(4,0,0))
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QApplication a(argc, argv);

    const float DEFAULT_DPI = 96.0;
    QList<QScreen*> screens = QApplication::screens();

    if (screens.size() > 0) {
        QScreen* screen = screens[0];
        double dpiX = screen->logicalDotsPerInch();
          //rate = dpiX / 96.0;
        float fontSize = dpiX / DEFAULT_DPI;
        if (fontSize < 1.1) {
            fontSize = 1.0;
        } else if (fontSize < 1.4) {
            fontSize = 1.25;
        } else if (fontSize < 1.6) {
            fontSize = 1.5;
        } else if (fontSize < 1.8) {
            fontSize = 1.75;
        } else {
            fontSize = 2.0;
        }
        QFont font = a.font();
        font.setPointSize(font.pointSize()*fontSize);
        a.setFont(font);
    }
#else
    QApplication a(argc, argv);
#endif

    // 统一应用程序样式（在可能弹出任何对话框之前设置）
    WindowsTimerResolutionGuard processTimerResolution(1);

    QApplication::setStyle(QStyleFactory::create("Fusion"));
    // 设置应用样式表，确保 QMessageBox 与主界面风格一致
    QString qss;
    QFile qssFile(":/qss/style.qss");
    qssFile.open(QFile::ReadOnly);
    if(qssFile.isOpen())
    {
        qss = QLatin1String(qssFile.readAll());
        a.setStyleSheet(qss);
        qssFile.close();
    }

    // 检测SDK文件是否存在（需在 QApplication 创建后再进行，以便可弹窗提示）
    QString dll_path;
#ifdef Q_OS_WIN
    dll_path = QCoreApplication::applicationDirPath() + "/qhyccd.dll";
#endif
#ifdef Q_OS_UNIX
    dll_path = "/usr/local/lib/libqhyccd.so";
#endif
#ifdef Q_OS_MAC
    dll_path = QCoreApplication::applicationDirPath() + "/../Frameworks/libqhyccd.20.dylib";
#endif
    QFile dll_file(dll_path);
    if(!dll_file.exists())
    {
        QMessageBox::critical(nullptr,
                              QObject::tr("Error"),
                              QObject::tr("No SDK file be found!"),
                              QMessageBox::Ok);
        return -1;
    }

    installMsgHandler();

//    libqhyccd =new dllqhyccd();
//    unsigned int ret=NULL;
    QDir *temp = new QDir;
    bool exist = temp->exists("./Bias");
    if(!exist)
    {
        bool ok = temp->mkdir("./Bias");
        if(ok)
        {
            qDebug() << "mkdir Bias failed" << QT_ENDL;
        }
    }
    exist = temp->exists("./Black");
    if(!exist)
    {
        bool ok = temp->mkdir("./Black");
        if(ok)
        {
            qDebug() << "mkdir Black failed" << QT_ENDL;
        }
    }
//    ret =libqhyccd->InitQHYCCDResource(); //ret = InitQHYCCDResource();
//    if(ret != QHYCCD_SUCCESS)
//    {
//        qCritical("InitQHYCCDResource: failed");
//        if(libqhyccd)
//        {
//            delete libqhyccd;
//            libqhyccd=NULL;
//        }
//    }
//    else
//    {
//        qDebug() << "EZCAP   InitQHYCCDSDK success";
//    }
//20200426 lyl 修正样式
    // 样式与样式表已在 SDK 检测前设置，避免重复设置
//    double rate = 0;
//    QList<QScreen*> screens = QApplication::screens();
//    if (screens.size() > 0) {
//        QScreen* screen = screens[0];
//        double dpi = screen->logicalDotsPerInch();
//        rate = dpi / 96.0;
//        if (rate < 1.1) {
//            rate = 1.0;
//        } else if (rate < 1.4) {
//            rate = 1.25;
//        } else if (rate < 1.6) {
//            rate = 1.5;
//        } else if (rate < 1.8) {
//            rate = 1.75;
//        } else {
//            rate = 2.0;
//        }
//    }
//    qputenv("QT_SCALE_FACTOR", QString::number(rate).toLatin1());
    EZCAP w;
    McpIpcServer mcpServer(&w);
    const QStringList args = QCoreApplication::arguments();
    if (McpIpcServer::isEnabledByEnv() || McpIpcServer::isEnabledByArgs(args)) {
        QString errorMessage;
        if (!mcpServer.start(McpIpcServer::defaultServerName(), &errorMessage)) {
            qWarning() << "MCP IPC server failed to start:" << errorMessage;
        } else {
            qInfo() << "MCP IPC server listening on"
                    << McpIpcServer::defaultServerName();
        }
    }
    w.resize(1250,800);//*rate *rate
    w.show();
    return a.exec();
}
