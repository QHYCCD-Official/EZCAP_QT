#include "ezCap.h"
#include "ui_ezCap.h"
#include "mainMenu.h"
#include "borderLayout.h"
#include "planner.h"
#include "ui_planner.h"
#include "favorite.h"
#include "ui_favorite.h"
#include "gpsTool.h"
#include "ui_gpstool.h"
#include "tempControl.h"
#include "ui_tempControl.h"
#include "phdLink.h"
#include "darkFrameTool.h"
#include "frameToolCal.h"
#include "frameToolCapCal.h"
#include "ui_phdLink.h"
#include "about.h"
#include "fitHeader.h"
#include "ui_fitHeader.h"
#include "cameraChooser.h"
#include "ui_cameraChooser.h"
#include "managementMenu.h"
#include "ui_managementMenu.h"
#include "downloadPreThread.h"
#include "downloadCapThread.h"
#include "downloadFocThread.h"
#include "liveCapThread.h"
#include "threadProcessImage.h"
#include "videoShowThread.h"
#include "cfwControl.h"
#include "cfwSetup.h"
#include "otherCameraSetup.h"

//20200220 lyl Add ReadMode Dialog
#include "readmode.h"
#include "ui_readmode.h"
#include "technicalsupport.h"
#include "threadTempControl.h"
#include "imgAnalyze.h"
#include "ui_imgAnalyze.h"
#include "toolBurst.h"
#include "ui_toolBurst.h"
#include "toolTrigger.h"
#include "ui_toolTrigger.h"
#include "pixelMagnifier.h"
#include "toolCorrectCenter.h"
#include "ui_toolCorrectCenter.h"

//#include "qhyccdStatus.h"
#include "include/dllqhyccd.h"
#include <QPointer>
#include "outputdebug.h"

#include "qthread.h"
#include "protocol.h"
#include "msgclient.h"

#include <QImage>
#include <qfiledialog.h>
#include <QPainter>
#include <QtCore>
#include <QScrollArea>
#include <QScrollBar>
#include <QCloseEvent>
#include <QMessageBox>
#include <QMenu>
#include <QDesktopServices>
#include <QException>
#include <QMutex>
#include <QScreen>
#include <QDebug>
#include <QTemporaryFile>
#include <QCursor>

EZCAP *mainWidget;
const bool EZCAP::TESTED_PID = false;

struct IX ix;
struct FOCUSINFO FocusInfo;
struct INIFILEPARAM iniFileParams;

//CvFont QHYFont;

//QQueue<unsigned char *> frameQueue;
//QQueue<unsigned char *> histQueue;

//QMutex capImgMutex;
QMutex fpsMutex;
QMutex copyMutex;
QMutex showMutex;
QMutex histMutex;
QMutex saveMutex;
QMutex calMutex;

qhyccd_handle *camhandle;

char camid[64];
bool show_disconnect_confirm_box = true;
uint32_t pnp_counter =0;
uint32_t transfer_error_counter =0;
uint32_t frame_count = 0;


#ifdef CALAB_YAU_PLANETARIUM
//websoket p2p connection
QWebSocket *_ws;

bool glClientConnected;
QString glClientIP;
quint16 glClientPort;

#endif

void EZCAP::updateWindowsTitle()
{
    uint8_t superSpeed ='-';
    uint8_t camStatus = '-';
    uint32_t sdk_build_version='-';

    sdk_build_version = libqhyccd->GetQHYCCDSDKBuildVersion();
    if(ix.isConnected)
    {
        superSpeed = libqhyccd->GetCameraIsSuperSpeedFromID(camid);
        superSpeed = superSpeed + 2 +48;//48=0(ascii code) In SDK 0=usb2.0 1=usb3.0 , so speed+=2
        camStatus = libqhyccd->GetCameraStatusFromID(camid)+48;//48=0(ascii code)

        if(libqhyccd->IsQHYCCDControlAvailable(camhandle,CAM_Sensor_ULVO_Status)==QHYCCD_SUCCESS)
        {
            int configLength = 64;
            char configString_raw64[configLength];

            libqhyccd->QHYCCDReadInitConfigFlash(camhandle,configString_raw64);
            QByteArray qData(configString_raw64,configLength);
            if(QChar(configString_raw64[0]) != 'c'  || QChar(configString_raw64[1]) != 'f' || QChar(configString_raw64[2]) != 'g')
            {
                qDebug() << "---Flash check:["<<QChar(configString_raw64[0]) <<"]"<<QString(configString_raw64[0]);
                if(QChar(configString_raw64[0]) != '0' && QString(configString_raw64[0]) != "\uFFFF" ) //fiber camera do have flash check so it return 0 is ok
                {
                    QMessageBox msgBox_flash_Check;
                    msgBox_flash_Check.setText(QString("Camera [%1] Flash Error").arg(ix.CamID));
                    msgBox_flash_Check.exec();
                }
                else
                {
                    mainWidget->statusLabel_frame_status->setText(QString("can not verify flash"));
                }
            }

            if(configString_raw64[15] == 1)
            {
                QMessageBox msgBox_Ulvo_Check;
                msgBox_Ulvo_Check.setText(QString("Camera [%1] may not able to achieve maximum cooling power").arg(ix.CamID));
                msgBox_Ulvo_Check.exec();
            }
            else
            {
                DBGOPT_INFO("skip ULVO warning");
            }
        }
        else
        {
            DBGOPT_INFO("skip ULVO check");
        }

    }
    else
    {
        ix.CamID = "-";
        ix.FPGAVer = "-";
        ix.FPGAVer1 = "-";
        ix.driverVer = "-";
        statusLabel_msg->clear();
        statusLabel_Temp->clear();
        statusLabel_RH->clear();
        statusLabel_PRESS->clear();
    }

    if(ix.CamID.contains("PCIE") || ix.CamID.contains("ERIS"))
        this->setWindowTitle(tr("EZCAP:[") + EZCAP_VER + "] Cam-ID:[" + ix.CamID +"] FPGA.A:[" + ix.FPGAVer + "] FPGA.B:[" + ix.FPGAVer1 + "] PCIEVersion:[" + ix.driverVer +"] Conn:[" + QString((char)camStatus) + "] Build:[" + QString::number(sdk_build_version) +"]");
    else
        this->setWindowTitle(tr("EZCAP:[") + EZCAP_VER + "] Cam-ID:[" + ix.CamID +"] FPGA.A:[" + ix.FPGAVer + "] FPGA.B:[" + ix.FPGAVer1 + "] FWVersion:[" + ix.driverVer +"] USB:[" + QString((char)superSpeed) + "] Conn:[" + QString((char)camStatus) + "] Build:[" + QString::number(sdk_build_version) +"]");
}

QString device_status_string_out = QString("①②③④⑤⑥⑦⑧");
QString device_status_string_connected = QString("❶❷❸❹❺❻❼❽");
QMap<int,QMap<QString,int>> dev_status_map;

//dev_status 0 = init, 1 = connected
void update_device_status(char *id,int dev_status)
{
    QString param_id = QString("[%1]").arg(id);
    DBGOPT_INFO("update  [%s] [%d]", param_id.toStdString().c_str(), dev_status);
    QMapIterator<int,QMap<QString,int>> i_search(dev_status_map);

    bool dev_found = false;
    int found_dev_connection_count =0;
    int found_index = -1;
    while (i_search.hasNext())
    {
        found_index++;
        i_search.next();
        QMapIterator<QString, int> i(i_search.value());

        while (i.hasNext())
        {
            i.next();
            qDebug() << i.key() << ": " << i.value();
        }

        if(i_search.value().contains(param_id))
        {
            dev_found = true;
            found_dev_connection_count = i_search.value().value("connection_count",-1);
            break;
        }
    }

    if(!dev_found)
    {
        DBGOPT_INFO("add +++++++++  [%s] [%d] ", param_id.toStdString().c_str(), dev_status);
        QMap<QString,int> new_dev;
        new_dev[param_id] = dev_status;
        new_dev["connection_count"] = 1;
        dev_status_map[dev_status_map.size()] = new_dev;
    }
    else
    {
        dev_status_map[found_index][param_id] = dev_status;
        switch (dev_status) {
        case 0:
            dev_status_map[found_index]["connection_count"] = found_dev_connection_count;
            break;
        case 1:
            dev_status_map[found_index]["connection_count"] = found_dev_connection_count + 1;
            break;
        default:
            qDebug( "found count error  [%s] [%d] ", param_id.toStdString().c_str(), dev_status_map[found_index][param_id]);
            break;

        }
        DBGOPT_INFO("found  [%s] [%d] ", param_id.toStdString().c_str(), dev_status_map[found_index][param_id]);
    }

    QString device_status_string_update = QString("");
    QMapIterator<int,QMap<QString,int>> i_update(dev_status_map);
    int status_update = 0;
    int index_update = 0;

    while (i_update.hasNext())
    {
        i_update.next();
        if(index_update < (dev_status_map.size()-8)){
            continue;//show last 8 cameras
        }

        QMap<QString,int> mapItem_update = i_update.value();
        status_update = mapItem_update.values().first();
        QString id_update = mapItem_update.keys().first();
        QMapIterator<QString, int> i(mapItem_update);

        while (i.hasNext()) {
            i.next();
            qDebug() << i.key() << ": " << i.value();
        }

        switch (status_update)
        {
        case 0:
            device_status_string_update += device_status_string_out.mid(index_update,1);
            qDebug( "cam list updating 0 [%s] [%d] [%s] ", id_update.toStdString().c_str(), status_update,QString(" - %1").arg(index_update).toStdString().c_str());
            break;
        case 1:
            device_status_string_update += device_status_string_connected.mid(index_update,1);
            qDebug( "cam list updating 1 [%s] [%d] [%s] ", id_update.toStdString().c_str(), status_update,QString(" + %1").arg(index_update).toStdString().c_str());
            break;
        default:
            device_status_string_update += "?";
            break;
        }

        device_status_string_update += QString::number(mapItem_update["connection_count"]) +" ";
        if((index_update+1) %4 ==0){device_status_string_update += "| ";}
        index_update++;
    }

    DBGOPT_INFO("update_device_status  [%s] ", device_status_string_update.toStdString().c_str());
    mainWidget->statusLabel_dev_status->setText(device_status_string_update);
}

void pnpEventExFunc()
{
    DBGOPT_INFO(".....pnpEventExFunc......");

    pnp_counter++;
    mainWidget->statusLabel_SDKmsg->setText("PNP: " + QString::number(pnp_counter) + "  Err: " + QString::number(transfer_error_counter));
    mainWidget->ui->plainTextEdit_debug->appendPlainText(QString("pnpEvent  [%1]").arg(mainWidget->statusLabel_SDKmsg->text()));
}
void transferEventErrorFunc(){
    DBGOPT_INFO(".....transferEventErrorFunc......");

    transfer_error_counter++;
    mainWidget->statusLabel_SDKmsg->setText("P:" + QString::number(pnp_counter) + " E: " + QString::number(transfer_error_counter));
    mainWidget->ui->plainTextEdit_debug->appendPlainText(QString("Err  [%1]").arg(mainWidget->statusLabel_SDKmsg->text()));
}
void pnp_Event_In_Func(char *id){
    DBGOPT_INFO(".....pnp_Event_In_Func......");
    DBGOPT_INFO("Cam +  [%s]", id);

    mainWidget->ui->plainTextEdit_debug->appendPlainText(QString("Cam +  [%1]").arg(id));
    update_device_status(id,1);
    QString eventID = QString(id);

    DBGOPT_INFO("autoconnect..............2.1");
    if(ix.isConnected)
    {
        DBGOPT_INFO("autoconnect..............2.2");
        DBGOPT_INFO("Cam In [%s] , But Cam[%s] already connected ", id, ix.CamID.toStdString().c_str());
        return ;
    }

    DBGOPT_INFO("autoconnect..............2.3");
    DBGOPT_INFO("autoConnectLive = %d", iniFileParams.autoConnectLive);
    if(iniFileParams.autoConnectLive)
    {
        DBGOPT_INFO("autoconnect..............2.4 - Triggering actCntLive");
        mainMenuBar->actCntLive->trigger();
        DBGOPT_INFO("autoconnect..............2.5 - actCntLive triggered");
    }
    else
    {
        DBGOPT_INFO("autoConnectLive is false, skipping auto connect");
    }
}

void pnp_Event_Out_Func(char *id)
{
    DBGOPT_INFO("Cam -  [%s]", id);

    mainWidget->ui->plainTextEdit_debug->appendPlainText(QString("Cam -  [%1]").arg(id));
    update_device_status(id,0);
    QString eventID = QString(id);
    if(eventID.compare(ix.CamID) != 0)
    {
        OutputDebug("EZCAP | %s | %s | Cam Out Compare false  [%s] [%s]", __FILE__, __FUNCTION__, id, ix.CamID.toStdString().c_str());
        return ;
    }
    if(managerMenu->ui->pBtn_live_preview->isEnabled())
    {
        ix.cameraState = Camera_Idle; //todo need to extract a disconnect function and enable the preview button
        QThread::msleep(200);
    }
    show_disconnect_confirm_box = false;
    if(ix.camStreamMode == 0)
        mainMenuBar->actConnect->trigger(); //Do disconnect
    else if(ix.camStreamMode == 1)
        mainMenuBar->actCntLive->trigger(); //Do disconnect
}
void data_Event_Single_Func(char *id, uint8_t *imageData)
{
    OutputDebug("EZCAP | %s | %s | img +Single  [%s]", __FILE__, __FUNCTION__, id);
}
void data_Event_Live_Func(char *id, uint8_t *imageData)
{
    OutputDebug("EZCAP | %s | %s img +Live  [%s]", __FILE__, __FUNCTION__, id);
}
void pnp_Event_UVLO_Func(char *id, uint8_t status){
    DBGOPT_INFO("id = %s status = %d", id, status);
}
#ifdef CALAB_YAU_PLANETARIUM
QString getLocalIp()
{
    QString localIp;
    QList<QHostAddress> list = QNetworkInterface::allAddresses();
    for(int index = 0; index < list.size(); index++)
    {
        if(list.at(index).protocol() == QAbstractSocket::IPv4Protocol)
        {
            //IPv4地址
            if (list.at(index).toString().contains("127.0."))
            {
                continue;
            }
            localIp = list.at(index).toString();
            break;
        }
    }
    return localIp;
}

#endif 

/**
 * @brief EZCAP::EZCAP
 * @param parent
 */
EZCAP::EZCAP(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::EZCAP)
{
    //define soft version
    //date in macro is like Nov 21 2020,but in QT, it use local time, mean 十一月 21 2020,  That why we have to use Qlocale
    QLocale   macro_locale(QLocale::English, QLocale::UnitedStates);
    QDateTime macro_Date = macro_locale.toDateTime(__DATE__, "MMM dd yyyy");
    QDateTime macro_Date_short = macro_locale.toDateTime(__DATE__, "MMM  d yyyy");
    EZCAP_VER = macro_Date.toString("yyyy-MM-dd");
    EZCAP_VER_SHORT = macro_Date_short.toString("yyyy-MM-dd");
    if(EZCAP_VER_SHORT.length()>0){EZCAP_VER = EZCAP_VER_SHORT;}
    RELEASE_TIME = EZCAP_VER + QStringLiteral("   ") + QStringLiteral(__TIME__);

    ui->setupUi(this);
    libqhyccd = new dllqhyccd();

    DBGOPT_INFO("=== EZCAP Constructor called ===");
    DBGOPT_INFO("EZCAP    EZCAP    START-----------");
    DBGOPT_INFO("macro date before convert = %s", __DATE__);
    DBGOPT_INFO("EZCAP Version: %s", qPrintable(EZCAP_VER));

    //---------------------create the sub-window dialog class---------------------------------------------------
    about_dialog             = new About(this,EZCAP_VER,RELEASE_TIME); //About //pass the version info when create the dialog
    favorite_dialog          = new Favorite(this);                     //偏好等设置
    gpsTool_dialog           = new gpsTool(this);
    toolBurst_dialog         = new ToolBurst(this);
    toolTrigger_dialog       = new ToolTrigger(this);
    fitHeader_dialog         = new FitHeader(this);                    //FIT头编辑
    planner_dialog           = new Planner(this);                      //计划列表
    imgAnalyze_dialog        = new ImgAnalyze(this);                   //图像分析
    tempControl_dialog       = new TempControl(this, TESTED_PID);      //温度控制
    phdLink_dialog           = new PHDLink(this);                      //PHD控制
    darkFrameTool_dialog     = new DarkFrameTool(this);                //单帧暗场工具
    frameToolCapCal_dialog   = new FrameToolCapCal(this);        //连续暗场工具
    frameToolCal_dialog      = new FrameToolCal(this);
    toolCorrectCenter_dialog = new ToolCorrectCenter(this);
    cameraChooser            = new CameraChooser(this);                //相机选择 //need to passed by the father point
    managerMenu              = new ManagementMenu(this);               //管理菜单
    mainMenuBar              = new MainMenu(0);                        //顶部主菜单
    cfwControl_dialog        = new CFWControl(this);                   //滤镜轮控制
    cfwSetup_dialog          = new CFWSetup(cfwControl_dialog);        //滤镜轮设置
    technicalSupport_dialog  = new TechnicalSupport(this);
    otherCameraSetup_dialog  = new OtherCameraSetup(this);

    //----------------------------main menu setting-------------------------------------------------------------
    this->setMenuBar(mainMenuBar);
    mainMenuBar->raise();//mainmenu show on the top,suit for Mac os

    //-------------------------manager Menu mainlayout-----------------------------------------------------------
    managerLayout = new QVBoxLayout();
    managerLayout->setContentsMargins(0, 0, 0, 0);
    managerLayout->setSpacing(0);
    managerLayout->setSizeConstraint(QLayout::SetFixedSize);
    managerLayout->addWidget(managerMenu);
    ui->scrollAreaWidgetContents_2->setLayout(managerLayout);
    ui->scrollArea_manager->setWidget(ui->scrollAreaWidgetContents_2);//managerMenu contains the ui

    //------------------add the scroll in the image showing area -------------------------------------------------
    ui->label_ImgShow->setBackgroundRole(QPalette::Base);
    ui->label_ImgShow->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    ui->label_ImgShow->setScaledContents(true);//设置QLabel自动缩放,既:显示图像大小自动调整为Qlabel大小

    scrollArea_ImgShow = new QScrollArea(this);
    scrollArea_ImgShow->setObjectName("scrollArea_ImgShow");//设置objectName，用于qss中设置其显示风格
    scrollArea_ImgShow->setBackgroundRole(QPalette::Dark);
    scrollArea_ImgShow->setWidget(ui->label_ImgShow);

    //像素放大镜浮动控件（浮于图像显示区之上，鼠标悬停时显示）
    pixelMagnifier = new PixelMagnifierWidget(scrollArea_ImgShow->viewport());

    gridLayout_ImgShow_Total = new QGridLayout();
    gridLayout_ImgShow_Total->addWidget(scrollArea_ImgShow,              0, 0, 1, 1);
    gridLayout_ImgShow_Total->addWidget(ui->verticalScrollBar_ImgShow,   0, 1, 1, 1);
    gridLayout_ImgShow_Total->addWidget(ui->horizontalScrollBar_ImgShow, 1, 0, 1, 1);
    gridLayout_ImgShow_Total->addWidget(ui->widgetFocusAssistant,        2, 0, 1, 2);
    gridLayout_ImgShow_Total->setContentsMargins(0, 0, 0, 0);
    gridLayout_ImgShow_Total->setRowStretch(0, 1);
    gridLayout_ImgShow_Total->setColumnStretch(0, 1);

    scrollArea_ImgShow_Total = new QScrollArea(this);
    scrollArea_ImgShow_Total->setObjectName("scrollArea_ImgShow_2");//设置objectName，用于qss中设置其显示风格
    scrollArea_ImgShow_Total->setBackgroundRole(QPalette::Dark);
    scrollArea_ImgShow_Total->setLayout(gridLayout_ImgShow_Total);
    scrollArea_ImgShow_Total->layout()->setContentsMargins(0, 0, 0, 0);
    scrollArea_ImgShow_Total->layout()->setSpacing(0);

    ui->horizontalScrollBar_ImgShow->setVisible(false);
    ui->verticalScrollBar_ImgShow->setVisible(false);
    ui->widgetFocusAssistant->setVisible(false);

    //---------------------状态栏显示区域----------------------------------------------------------
    statusLabel_imgSize = new QLabel(this);//用于显示图像分辨率
    statusLabel_imgSize->setFrameShape(QFrame::NoFrame);
    statusLabel_imgSize->setFixedSize(75,18);
    statusLabel_mousePos = new QLabel(this);//用于显示鼠标指向的像素在图像中的位置坐标
    statusLabel_mousePos->setFrameShape(QFrame::NoFrame);
    statusLabel_mousePos->setFixedSize(75,18);
    statusLabel_rgb = new QLabel(this);//用于显示鼠标指向的像素的rbg值
    statusLabel_rgb->setFrameShape(QFrame::NoFrame);
    statusLabel_rgb->setFixedSize(155,18);
    statusLabel_Temp = new QLabel(this);//显示温度
    statusLabel_Temp->setFrameShape(QFrame::NoFrame);
    statusLabel_Temp->setFixedSize(75, 18);
    statusLabel_RH = new QLabel(this);
    statusLabel_RH->setFixedSize(60, 18);
    statusLabel_PRESS =new QLabel(this);
    statusLabel_PRESS->setFixedSize(100,18);
    statusLabel_msg = new QLabel(this);//显示提示信息
    statusLabel_SDKmsg = new QLabel(this);//显示提示信息
    statusLabel_dev_status = new QLabel(this);
    statusLabel_frame_status = new QLabel(this);
    statusLabel_ImgMean = new QLabel(this);
    ui->statusBar->addWidget(statusLabel_imgSize);
    ui->statusBar->addWidget(statusLabel_mousePos);
    ui->statusBar->addWidget(statusLabel_rgb);
    ui->statusBar->addWidget(statusLabel_Temp);
    ui->statusBar->addWidget(statusLabel_RH);
    ui->statusBar->addWidget(statusLabel_PRESS);
    ui->statusBar->addWidget(statusLabel_msg);
    ui->statusBar->addWidget(statusLabel_SDKmsg);
    ui->statusBar->addWidget(statusLabel_dev_status);
    ui->statusBar->addWidget(statusLabel_frame_status);
    ui->statusBar->addWidget(statusLabel_ImgMean);

    //----------------------整体borderlayout布局-------------------------------------------------
    mainLayout = new BorderLayout();
    mainLayout->setSpacing(1);
    mainLayout->addWidget(ui->scrollArea_manager, BorderLayout::West);
    mainLayout->addWidget(scrollArea_ImgShow_Total, BorderLayout::Center);
    mainLayout->addWidget(mainMenuBar, BorderLayout::North);//test menubar in Mac os
    mainLayout->addWidget(ui->statusBar, BorderLayout::South);
    ui->centralWidget->setLayout(mainLayout);

    //-------------------init members-----------------------------------------------------------
    cfwTimer = new QTimer();// 色轮状态查询  定时器
    cfwTimer->setInterval(300);

    ditherTimer = new QTimer(); //Dither 状态查询定时器
    ditherTimer->setInterval(500);
    isSettleDone = false; //settle 默认状态

    PumpTimer = new QTimer();//循环泵 计时器
    PumpTimer->setInterval(1000);

    PumpV2CycleTimer = new QTimer();//Test Pump V2 Cycle定时器
    PumpV2CycleTimer->setInterval(60000);//60秒=1分钟

    PumpV2CycleSecondTimer = new QTimer();//Test Pump V2 Cycle Second定时器
    PumpV2CycleSecondTimer->setInterval(60000);//60秒=1分钟

    connect(scrollArea_ImgShow->horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(hScrollBarValueChanged(int)));
    connect(scrollArea_ImgShow->verticalScrollBar(),   SIGNAL(valueChanged(int)), this, SLOT(vScrollBarValueChanged(int)));

    //像素放大镜开关：取消勾选时立即隐藏
    connect(mainMenuBar->actPixelMagnifier, &QAction::toggled, this, [this](bool checked) {
        if(!checked && pixelMagnifier) pixelMagnifier->hide();
    });

    //顶部菜单栏
    connect(mainMenuBar->actOpenVideo,            SIGNAL(triggered()),     this, SLOT(openVideo()));
    connect(mainMenuBar->actOpenFolder,           SIGNAL(triggered()),     this, SLOT(openFolder()));
    connect(mainMenuBar->actSaveFIT,              SIGNAL(triggered()),     this, SLOT(saveAsFIT()));
    connect(mainMenuBar->actSaveBMP,              SIGNAL(triggered()),     this, SLOT(saveAsBMP()));
    connect(mainMenuBar->actSaveJPG,              SIGNAL(triggered()),     this, SLOT(saveAsJPG()));
    connect(mainMenuBar->actSavePNG,              SIGNAL(triggered()),     this, SLOT(saveAsPNG()));
    connect(mainMenuBar->actSaveTIF,              SIGNAL(triggered()),     this, SLOT(saveAsTIF()));
    connect(mainMenuBar->actFitHeaderEditor,      SIGNAL(triggered()),     this, SLOT(showFITHeaderEditor()));
    connect(mainMenuBar->actIgnoreOverScanArea,   SIGNAL(triggered(bool)), this, SLOT(ignoreOverScanAreaClicked(bool)));
    connect(mainMenuBar->actCalibrateOverScan,    SIGNAL(triggered(bool)), this, SLOT(calibrateOverScanClicked(bool)));
    connect(mainMenuBar->actSaveTHPFile,          SIGNAL(triggered(bool)), this, SLOT(enableSaveTHPFile(bool)));
    connect(mainMenuBar->actExit,                 SIGNAL(triggered()),     this, SLOT(exitMainWindow()));
    connect(mainMenuBar->actConnect,              SIGNAL(triggered()),     this, SLOT(showCameraChooser()));
    connect(mainMenuBar->actCntLive,              SIGNAL(triggered()),     this, SLOT(showCameraChooser()));
    connect(mainMenuBar->actShowPlanTable,        SIGNAL(triggered()),     this, SLOT(showPlanTable()));
    connect(mainMenuBar->actImgAnalyze,           SIGNAL(triggered()),     this, SLOT(showImgAnalyze()));
    connect(mainMenuBar->actImgRotate180,         SIGNAL(triggered()),     this, SLOT(imageRotateMirror()));
    connect(mainMenuBar->actImgRotate90L,         SIGNAL(triggered()),     this, SLOT(imageRotateMirror()));
    connect(mainMenuBar->actImgRotate90R,         SIGNAL(triggered()),     this, SLOT(imageRotateMirror()));
    connect(mainMenuBar->actImgMirrorH,           SIGNAL(triggered()),     this, SLOT(imageRotateMirror()));
    connect(mainMenuBar->actImgMirrorV,           SIGNAL(triggered()),     this, SLOT(imageRotateMirror()));
    connect(mainMenuBar->actFavorite,             SIGNAL(triggered()),     this, SLOT(showFavoriteSetting()));
    connect(mainMenuBar->actGPSControl,           SIGNAL(triggered()),     this, SLOT(showGPSTool()));
    connect(mainMenuBar->actToolBurst,            SIGNAL(triggered()),     this, SLOT(showToolBurst()));
    connect(mainMenuBar->actToolTrigger,          SIGNAL(triggered()),     this, SLOT(showToolTrigger()));
    connect(mainMenuBar->actPHDLink,              SIGNAL(triggered()),     this, SLOT(showPHDLink()));
    connect(mainMenuBar->actTempControl,          SIGNAL(triggered()),     this, SLOT(showTempControl()));
    connect(mainMenuBar->actAbout,                SIGNAL(triggered()),     this, SLOT(showAbout()));
    connect(mainMenuBar->actManual,               SIGNAL(triggered()),     this, SLOT(showManual()));
    connect(mainMenuBar->actTestMode,             SIGNAL(triggered()),     this, SLOT(activeTestMode()));
    connect(mainMenuBar->actDebug,                SIGNAL(triggered()),     this, SLOT(switchDebug()));
    connect(mainMenuBar->actTestGuid,             SIGNAL(triggered()),     this, SLOT(switchTestGuid()));
    connect(mainMenuBar->actTestPumpV2,           SIGNAL(triggered(bool)),     this, SLOT(switchTestPumpV2(bool)));
    connect(mainMenuBar->actTestPumpV2_second,    SIGNAL(triggered(bool)),     this, SLOT(switchTestPumpV2_second(bool)));
    connect(mainMenuBar->actTestPumpV2_cycle,     SIGNAL(triggered(bool)),     this, SLOT(switchTestPumpV2_cycle(bool)));
    connect(mainMenuBar->actTestPumpV2_cycle_second, SIGNAL(triggered(bool)),  this, SLOT(switchTestPumpV2_cycle_second(bool)));
    connect(mainMenuBar->actTestErrorLed,    SIGNAL(triggered(bool)),     this, SLOT(switchTestErrorLed(bool)));
    connect(mainMenuBar->actTestIMG1,             SIGNAL(triggered(bool)),     this, SLOT(switchTestIMG1(bool)));
    connect(mainMenuBar->actTestIMG3,             SIGNAL(triggered(bool)),     this, SLOT(switchTestIMG3(bool)));
    connect(mainMenuBar->actCFWControl,           SIGNAL(triggered(bool)), this, SLOT(showCFWControl()));
    connect(mainMenuBar->actFrameToolCapCal,      SIGNAL(triggered()),     this, SLOT(showCaptureDarkFrameTool()));
    connect(mainMenuBar->actFrameToolCal,         SIGNAL(triggered()),     this, SLOT(darkFrameCalibration()));
    connect(mainMenuBar->actCorrectCenter,        SIGNAL(triggered()),     this, SLOT(showToolCorrectCenter()));
    connect(mainMenuBar->actEnglish,              SIGNAL(triggered()),     this, SLOT(changeToEnglish()));
    connect(mainMenuBar->actChinese,              SIGNAL(triggered()),     this, SLOT(changeToChinese()));
    connect(mainMenuBar->actJapanese,             SIGNAL(triggered()),     this, SLOT(changeToJapanese()));
    connect(mainMenuBar->actFitWindow,            SIGNAL(triggered()),     this, SLOT(zoomFitWindow()));
    connect(mainMenuBar->actFillWindow,           SIGNAL(triggered()),     this, SLOT(zoomFillWindow()));
    connect(mainMenuBar->act0_25X,                SIGNAL(triggered()),     this, SLOT(zoom0_25X()));
    connect(mainMenuBar->act0_5X,                 SIGNAL(triggered()),     this, SLOT(zoom0_5X()));
    connect(mainMenuBar->act0_75X,                SIGNAL(triggered()),     this, SLOT(zoom0_75X()));
    connect(mainMenuBar->act1X,                   SIGNAL(triggered()),     this, SLOT(zoom1X()));
    connect(mainMenuBar->act1_5X,                 SIGNAL(triggered()),     this, SLOT(zoom1_5X()));
    connect(mainMenuBar->act2X,                   SIGNAL(triggered()),     this, SLOT(zoom2X()));
    connect(mainMenuBar->actTechnicalSupport,     SIGNAL(triggered()),     this, SLOT(technicalSupport()));
    connect(mainMenuBar->actOtherCameraSetup,     SIGNAL(triggered()),     this, SLOT(showOtherCameraSetup()));

    //语言切换
    connect(this, SIGNAL(changeLanguage()), about_dialog,       SLOT(resetUI()));
    connect(this, SIGNAL(changeLanguage()), cameraChooser,      SLOT(resetUI()));
    connect(this, SIGNAL(changeLanguage()), favorite_dialog,    SLOT(resetUI()));
    connect(this, SIGNAL(changeLanguage()), fitHeader_dialog,   SLOT(resetUI()));
    connect(this, SIGNAL(changeLanguage()), mainMenuBar,        SLOT(resetUI()));
    connect(this, SIGNAL(changeLanguage()), managerMenu,        SLOT(resetUI()));
    connect(this, SIGNAL(changeLanguage()), planner_dialog,     SLOT(resetUI()));
    connect(this, SIGNAL(changeLanguage()), tempControl_dialog, SLOT(resetUI()));
    //20200220 lyl Add ReadMode Dialog
    //connect(this, SIGNAL(changeLanguage()), readMode, SLOT(resetUI()));

    //滤镜轮控制
    connect(cfwControl_dialog, SIGNAL(changeCFWPosition()), this,           SLOT(cfwPositionChanged()));
    connect(cfwControl_dialog, SIGNAL(endCFWProgress()),    this->cfwTimer, SLOT(stop()));

    //滤镜轮设置
    connect(cfwSetup_dialog, SIGNAL(updateFilterNames()), cfwControl_dialog, SLOT(filterNames_updated()));
    connect(cfwSetup_dialog, SIGNAL(updateFilterNames()), planner_dialog,    SLOT(cfwSetup_updated()));

    //管理菜单
    connect(managerMenu->ui->pBtn_cross,          SIGNAL(clicked()),           this, SLOT(mgrMenu_pBtn_cross_clicked()));
    connect(managerMenu->ui->pBtn_grid,           SIGNAL(clicked()),           this, SLOT(mgrMenu_pBtn_grid_clicked()));
    connect(managerMenu->ui->pBtn_circle,         SIGNAL(clicked()),           this, SLOT(mgrMenu_pBtn_circle_clicked()));
    connect(managerMenu->ui->pBtn_preview,        SIGNAL(clicked()),           this, SLOT(mgrMenu_pBtn_preview_clicked()));
    connect(managerMenu->ui->pBtn_live_preview,   SIGNAL(clicked()),           this, SLOT(mgrMenu_pBtn_live_preview_clicked()));
    connect(managerMenu->ui->pBtn_focus,          SIGNAL(clicked()),           this, SLOT(mgrMenu_pBtn_focus_clicked()));
    connect(managerMenu->ui->pBtn_live_focus,     SIGNAL(clicked()),           this, SLOT(mgrMenu_pBtn_live_focus_clicked()));
    connect(managerMenu->ui->pBtn_capture,        SIGNAL(clicked()),           this, SLOT(mgrMenu_pBtn_capture_clicked()));
    connect(managerMenu->ui->pBtn_stop,           SIGNAL(clicked()),           this, SLOT(mgrMenu_pBtn_stop_clicked()));
    connect(managerMenu->ui->hSlider_bPos,        SIGNAL(sliderReleased()),    this, SLOT(mgrMenu_hSlider_bPos_sliderReleased()));
    connect(managerMenu->ui->hSlider_wPos,        SIGNAL(sliderReleased()),    this, SLOT(mgrMenu_hSlider_wPos_sliderReleased()));
    connect(managerMenu->ui->pBtn_stretchMinusB,  SIGNAL(clicked()),           this, SLOT(mgrMenu_pBtn_stretchMinusB_clicked()));
    connect(managerMenu->ui->pBtn_stretchPlusB,   SIGNAL(clicked()),           this, SLOT(mgrMenu_pBtn_stretchPlusB_clicked()));
    connect(managerMenu->ui->pBtn_stretchMinusW,  SIGNAL(clicked()),           this, SLOT(mgrMenu_pBtn_stretchMinusW_clicked()));
    connect(managerMenu->ui->pBtn_stretchPlusW,   SIGNAL(clicked()),           this, SLOT(mgrMenu_pBtn_stretchPlusW_clicked()));
    connect(managerMenu->ui->pBtn_auto_histogram, SIGNAL(clicked()),           this, SLOT(mgrMenu_pBtn_auto_histogram_clicked()));
    connect(managerMenu->ui->btnStartSaveCap,     SIGNAL(clicked()),           this, SLOT(saveVideo()));
    connect(managerMenu->ui->btnSnapShot,         SIGNAL(clicked()),           this, SLOT(saveSnapShot()));
    connect(managerMenu->ui->btnOpenSaveFolder,   SIGNAL(clicked()),           this, SLOT(openFolder()));
    connect(managerMenu,                          SIGNAL(switchWorkMode(int)), this, SLOT(currentWorkingModeChanged(int)));

    //多相机选择窗口的相机连接操作
    connect(cameraChooser, SIGNAL(connect_camera()), mainMenuBar,       SLOT(camera_connected()));
    connect(cameraChooser, SIGNAL(connect_camera()), managerMenu,       SLOT(camera_connected()));
    connect(cameraChooser, SIGNAL(connect_camera()), planner_dialog,    SLOT(camera_connected()));
    connect(cameraChooser, SIGNAL(connect_camera()), this,              SLOT(camera_connected()));
    connect(cameraChooser, SIGNAL(connect_camera()), cfwControl_dialog, SLOT(camera_connected()));
    connect(cameraChooser, SIGNAL(connect_camera()), favorite_dialog,   SLOT(camera_connected()));
    connect(cameraChooser, SIGNAL(connect_camera()), fitHeader_dialog,  SLOT(camera_connected()));
    connect(cameraChooser, SIGNAL(connect_camera()), toolBurst_dialog,  SLOT(camera_connected()));
    connect(cameraChooser, SIGNAL(connect_camera()), toolTrigger_dialog,SLOT(camera_connected()));
    connect(cameraChooser, SIGNAL(connect_camera()), toolCorrectCenter_dialog, SLOT(camera_connected()));
    connect(cameraChooser, SIGNAL(connect_camera()), otherCameraSetup_dialog,  SLOT(camera_connected()));

    //主窗口的相机连接操作
    connect(this, SIGNAL(connect_camera()),    managerMenu,       SLOT(camera_connected()));
    connect(this, SIGNAL(connect_camera()),    mainMenuBar,       SLOT(camera_connected()));
    connect(this, SIGNAL(connect_camera()),    planner_dialog,    SLOT(camera_connected()));
    connect(this, SIGNAL(connect_camera()),    this,              SLOT(camera_connected()));
    connect(this, SIGNAL(connect_camera()),    cfwControl_dialog, SLOT(camera_connected()));
    connect(this, SIGNAL(connect_camera()),    favorite_dialog,   SLOT(camera_connected()));
    connect(this, SIGNAL(connect_camera()),    fitHeader_dialog,  SLOT(camera_connected()));
    connect(this, SIGNAL(connect_camera()),    toolBurst_dialog,  SLOT(camera_connected()));
    connect(this, SIGNAL(connect_camera()),    toolTrigger_dialog,SLOT(camera_connected()));
    connect(this, SIGNAL(connect_camera()),    toolCorrectCenter_dialog, SLOT(camera_connected()));
    connect(this, SIGNAL(connect_camera()),    otherCameraSetup_dialog,  SLOT(camera_connected()));

    //主窗口的相机断开操作
    connect(this, SIGNAL(disconnect_camera()), mainMenuBar,       SLOT(camera_disconnected()));
    connect(this, SIGNAL(disconnect_camera()), managerMenu,       SLOT(camera_disconnected()));
    connect(this, SIGNAL(disconnect_camera()), this,              SLOT(camera_disconnected()));
    connect(this, SIGNAL(disconnect_camera()), toolBurst_dialog,  SLOT(camera_disconnected()));
    connect(this, SIGNAL(disconnect_camera()), toolTrigger_dialog,SLOT(camera_disconnected()));
    connect(this, SIGNAL(disconnect_camera()), toolCorrectCenter_dialog, SLOT(camera_disconnected()));
    connect(this, SIGNAL(disconnect_camera()), otherCameraSetup_dialog,  SLOT(camera_disconnected()));

    //FIT头改变时触发的操作
    connect(this, SIGNAL(change_fitHeaderInfo()), fitHeader_dialog, SLOT(fitHeaderInfo_changed()));

    //favorite控制面板
    connect(favorite_dialog->ui->pBtn_calibrateFrame,                SIGNAL(clicked()), this, SLOT(favorite_pBtn_calibrateFrame_clicked()));
    connect(favorite_dialog->ui->pBtn_getRealTemp,                   SIGNAL(clicked()), this, SLOT(favorite_pBtn_getRealTemp_clicked()));
    connect(favorite_dialog->ui->pBtn_controlSensorChamberCyclePUMP, SIGNAL(clicked()), this, SLOT(favorite_pBtn_controlSensorChamberCyclePUMP_clicked()));

    //关联plannerdialog中forceStop信号到capture中的stop
    connect(planner_dialog->ui->pBtn_forceStop_planner, SIGNAL(clicked()), managerMenu->ui->pBtn_stop, SLOT(click()));

    //关联定时器信号
//    connect(tempTimer,   SIGNAL(timeout()), this, SLOT(tempTimer_timeout()));
    connect(cfwTimer,    SIGNAL(timeout()), this, SLOT(cfwTimer_timeout()));
    connect(ditherTimer, SIGNAL(timeout()), this, SLOT(ditherTimer_timeout()));
    connect(PumpTimer,   SIGNAL(timeout()), this, SLOT(PumpTimer_timeout()));
    connect(PumpV2CycleTimer, SIGNAL(timeout()), this, SLOT(PumpV2CycleTimer_timeout()));
    connect(PumpV2CycleSecondTimer, SIGNAL(timeout()), this, SLOT(PumpV2CycleSecondTimer_timeout()));

    //--------------------init-------------------------------------------------
    //-------------------------------------------------------------------------
    //初始化preview，focus,capture “收起/展开功能”关闭
    managerMenu->ui->head_preview->setCheckable(false);
    managerMenu->ui->head_focus->setCheckable(false);
    managerMenu->ui->head_capture->setCheckable(false);
    managerMenu->ui->head_save->setCheckable(false);
    managerMenu->ui->head_liveimageformat->setCheckable(false);
    managerMenu->ui->head_livecamerasetup->setCheckable(false);
    managerMenu->ui->head_liveimagesetup->setCheckable(false);
    managerMenu->ui->head_Roi->setCheckable(false);
    managerMenu->ui->head_screenView->setCheckable(false);
    managerMenu->ui->head_hist->setCheckable(false);

    //设置部分菜单及功能选项默认为不可控状态
    mainMenuBar->menuPlanner->setEnabled(false);
    mainMenuBar->actCFWControl->setEnabled(false);
    mainMenuBar->menuZoom->setEnabled(false);
    mainMenuBar->actSaveBMP->setEnabled(false);
    mainMenuBar->actOpenFolder->setEnabled(false);
    mainMenuBar->actSaveFIT->setEnabled(false);
    mainMenuBar->actSaveJPG->setEnabled(false);
    mainMenuBar->actSavePNG->setEnabled(false);
    mainMenuBar->actSaveTIF->setEnabled(false);
    mainMenuBar->actCalibrateOverScan->setEnabled(false);
    mainMenuBar->actIgnoreOverScanArea->setEnabled(false);
    mainMenuBar->actOpenVideo->setEnabled(true);
    mainMenuBar->actOpenFolder->setEnabled(false);
    mainMenuBar->actFrameToolCapCal->setEnabled(false);
    mainMenuBar->actFrameToolCal->setEnabled(false);
    mainMenuBar->actFavorite->setEnabled(false);//true

    //初始FocusAssistant panel不显示
    // ui->widgetFocusAssistant->close();
//    ui->widgetFocusAssistant->setVisible(false);

    //初始显示在focusAssistant的图像
    QImage img_focusAssistant = QPixmap(":/image/black.bmp").toImage();
    DrawGridBox(&img_focusAssistant);//绘制表格线
    ui->image2_focusAssistant->setPixmap(QPixmap::fromImage(img_focusAssistant));
    ui->image3_focusAssistant->setPixmap(QPixmap::fromImage(img_focusAssistant));

    //初始化screenview显示的图像
    managerMenu->ui->img_Roi->setPixmap(QPixmap(":/image/screenviewblack.bmp"));

    //初始化screenview显示的图像
    managerMenu->ui->img_screenView->setPixmap(QPixmap(":/image/screenviewblack.bmp"));

    //初始伪彩色图片
    LoadFalseColor("Linear.bmp");

    //为控件添加事件过滤注册（对于控件事件过滤，必须添加注册，否则不能生效）
    ui->label_ImgShow->installEventFilter(this);
    managerMenu->ui->img_Roi->installEventFilter(this);
    managerMenu->ui->img_screenView->installEventFilter(this);
    ui->image1_focusAssistant->installEventFilter(this);

    //字体结构初始化
//    cvInitFont(&QHYFont,CV_FONT_HERSHEY_SIMPLEX,0.5,0.5,0,1,8);

    ///
    /// 初始化ix变量
    ///
    //初始化读出模式
    ix.ReadMode               = 0;
    ix.ReadMode_Last          = 0;
    ix.ReadMode_Num           = 0;
    memset(ix.ReadMode_Name, 0, 50);
    ix.ReadMode_List.clear();
    //初始化stream mode
    ix.camStreamMode             = 0;
    ix.lastCamStreamMode         = 0;
    //初始化ID和型号
    ix.CamID                     = "";
    ix.CamModel                  = "";
    //初始化固件版本号
    ix.driverVer                 = "";
    //初始化FPGA版本
    ix.FPGAVer                   = "";
    ix.FPGAVer1                  = "";
    //
    ix.maxScreenW                = 0;
    ix.maxScreenH                = 0;
    //初始化芯片信息
    ix.CCD_ChipW                     = 0.0;
    ix.CCD_ChipH                     = 0.0;
    ix.CCD_PixelW                    = 0.0;
    ix.CCD_PixelH                    = 0.0;
    ix.CCD_ImageW                    = 0;
    ix.CCD_ImageH                    = 0;
    ix.CCD_ImageB                    = 0;
    //初始化BIN
    ix.Bin11_Fun                  = true;
    ix.Bin22_Fun                  = false;
    ix.Bin33_Fun                  = false;
    ix.Bin44_Fun                  = false;
    ix.Bin66_Fun                  = false;
    ix.Bin88_Fun                  = false;
    ix.BinX                      = 1;
    ix.BinY                      = 1;
    ix.BinX_Max                  = 1;
    ix.BinY_Max                   = 1;
    ix.BinX_Last                  = 0;
    ix.BinY_Last                  = 0;
    //初始化图像尺寸
    ix.ImageW_Max                 = 0;
    ix.ImageH_Max                 = 0;
    ix.Resolution_Max             = 0;
    ix.ImageW_Min                 = 0;
    ix.ImageH_Min                 = 0;
    ix.Resolution_Min             = 0;
    ix.EffectiveX                = 0;
    ix.EffectiveY                = 0;
    ix.EffectiveW                 = 0;
    ix.EffectiveH                 = 0;
    ix.OverscanX            = 0;
    ix.OverscanY            = 0;
    ix.OverscanW             = 0;
    ix.OverscanH             = 0;
    ix.RoiX                 = 0;
    ix.RoiY                 = 0;
    ix.RoiW                  = 0;
    ix.RoiH                  = 0;
    ix.RoiX_Last             = 0;
    ix.RoiY_Last             = 0;
    ix.RoiW_Last              = 0;
    ix.RoiH_Last              = 0;
    ix.FrameW                    = 0;
    ix.FrameH                    = 0;
    ix.FrameB                    = 0;
    ix.FrameC               = 0;
    ix.FrameW_Last                = 0;
    ix.FrameH_Last                = 0;
    ix.FrameB_Last                = 0;
    ix.FrameC_Last           = 0;
    //初始化图像位数
    ix.Bits8_Fun                  = false;
    ix.Bits16_Fun                 = false;
    ix.Bits                      = 16;
    ix.Bits_Last                  = 16;
    //初始化彩色模式
    ix.Color_Fun                  = false;
    ix.Color                = false;
    ix.Color_Last            = false;
    ix.IsCvtColor                = false;
    ix.Bayer               = 0;
    ix.CamBayer                  = 0;
    //初始化曝光时间
    ix.ExpTime_Fun                = true;
    ix.ExpUnit                   = 1000.0;
    ix.ExpUnit_Last               = 1000.0;
    ix.ExpTime                   = 20.0;
    ix.ExpTime_Last               = 20.0;
    //初始化增益
    ix.Gain_Fun                   = false;
    ix.Gain                      = 0.0;
    ix.Gain_Last                  = 0.0;
    //初始化偏置
    ix.Offset_Fun                 = false;
    ix.Offset                    = 0.0;
    ix.Offset_Last                = 0.0;
    //初始化下载速度
    ix.Speed_Fun              = false;
    ix.Speed             = 0.0;
    ix.Speed_Last         = 0.0;
    //初始化Traffic
    ix.Traffic_Fun             = false;
    ix.Traffic                = 0.0;
    ix.Traffic_Last            = 0.0;
    //初始化亮度
    ix.Brightness_Fun             = false;
    ix.Brightness                = 0.0;
    ix.Brightness_Last            = 0.0;
    //初始化对比度
    ix.Contrast_Fun               = false;
    ix.Contrast                  = 0.0;
    ix.Contrast_Last              = 0.0;
    //初始化Gamma
    ix.Gamma_Fun                  = false;
    ix.Gamma                     = 1.0;
    ix.Gamma_Last                 = 1.0;
    //初始化WBR
    ix.WBR_Fun                    = false;
    ix.WBR                       = 0.0;
    ix.WBR_Last                   = 0.0;
    //初始化WBG
    ix.WBG_Fun                    = false;
    ix.WBG                       = 0.0;
    ix.WBG_Last                   = 0.0;
    //初始化WBB
    ix.WBB_Fun                    = false;
    ix.WBB                       = 0.0;
    ix.WBB_Last                   = 0.0;
    //初始化DDR
    ix.DDR_Fun                    = false;
    ix.DDR                  = false;
    //初始化AMPV
    ix.AMPV_Fun                   = false;
    ix.AMPV                 = false;
    //初始化GPS
    ix.GPS_Fun                    = false;
    ix.GPS                   = false;
    //Burst
    ix.Burst_Fun                  = false;
    //初始化cooler
    ix.Cooler_Fun                 = false;
    ix.Cooler_Mode                = Cooler_Manual;
    ix.Temp_Now                   = 0.0;
    ix.PWM_Now                    = 0.0;
    ix.Voltage_Now                = 0;
    ix.Temp_Target                = 0.0;
    ix.Voltage_Target             = 0;
    //初始化湿度
    ix.Humidity_Fun               = false;
    ix.Humidity               = 0.0;
    //初始化压力
    ix.Pressure_Fun               = false;
    ix.Pressure               = 0.0;
    //初始化快门
    ix.canMechanicalShutter      = false;
    ix.MechanicalShutterMode     = 0;
    ix.LastMechanicalShutterMode = 0;
    //初始化FineTone
    ix.canFineTone               = false;
    //初始化MotorHeating
    ix.canMotorHeating           = false;
    //初始化制冷保护
    ix.canTecOverProtect         = false;
    ix.tecPretect                = false;
    //初始化clamp
    ix.canSignalClamp            = false;
    ix.clamp                     = false;
    //初始化FPN
    ix.canCalibrateFPN           = false;
    ix.isCalibrateFrame          = false;
    //初始化SlowestDownload
    ix.canSlowestDownload        = false;
    ix.slowestDowload            = false;
    //初始化Chip Temperature
    ix.canChipTemp               = false;
    //初始化外触发
    ix.canTriger                 = false;
    ix.trigerInterface           = 0;
    ix.trigerInterfaceList.clear();
    //初始化滤镜轮
    ix.canFilterWheel            = false;
    ix.CFW_Plugged               = false;
    ix.dstCfwPos                 = '0';
    memset(ix.curCfwPos, 0, 64);
    ix.CFWStatus                 = CFW_Idle;
    ix.CFWSlotsNum               = 0;
    ix.filterNames_2.clear();
    //20201127 lyl SensorChamberCyclePUMP
    ix.canContolSensorChamberCyclePUMP = false;
    ix.cyclePUMBStatus           = false;

    ix.Circle_Correct            = false;
    ix.circle1                   = 0;
    ix.circle2                   = 0;

    ix.locked = false;
    //初始化图像数据内存
    ix.ImgData                   = NULL;
    ix.ImgData_Last              = NULL;
    ix.ImgData_Save              = NULL;
    ix.ImgData_Dark              = NULL;
    //初始化相机连接状态
    ix.isConnected               = false;
    //初始化相机状态
    ix.cameraState               = Camera_Idle;
    //初始化工作模式
    ix.workMode                  = 0;//workMode: 1 preview  2 focus 3 capture
    ix.lastWorkMode              = -1;
    //初始化图像读取状态
    ix.imageReady                = GetSingleFrame_Waiting;
    //初始化cross状态
    ix.crossBtnState             = 0;
    //初始化grid状态
    ix.gridBtnState              = 0;
    //初始化circle状态
    ix.circleBtnState            = 0;
    //初始化FIT头编辑状态
    ix.fitHeadEditState          = FitHeader_Set;
    //初始化计划表状态
    ix.plannerState              = PlannerStatus_Done;
    //初始化强制退出状态
    ix.ForceStop                 = false;
    //初始化连续拍摄状态
    ix.onLiveMode                = false;
    //初始化过扫区校正
    ix.calConstant               = 1000;
    ix.IgnoreOverscan            = false;
    ix.CalibrateOverscan         = false;
    ix.saveTHPFile               = false;
    //初始化图像拉伸
    ix.autoStretchMode           = 0;
    ix.StretchStep               = 256;
    memset(ix.Histogram, 0, 256);
    memset(ix.LUT_1, 0, 3);
    memset(ix.StretchLUT, 0, 65536);
    //初始化图像显示控件参数
//    ix.imgShowWidth              = 0;
//    ix.imgShowHeight             = 0;
    ix.scaleFactor               = 1.0;
//    scaleFactor                  = 1.0;
    //初始化语言
    ix.lang                      = "EN";
    //初始化图像缩放
    ix.zoomMode                  = Zoom_SpecifyScaling;
    ix.lastZoomMode              = Zoom_SpecifyScaling;
    //初始化OBS
    ix.dateOBS                   = "";
    //初始化OSD
    ix.OSDList.clear();

    ///
    /// Init EZCAP params
    ///
    //初始化图像显示内存
    qImg_show              = NULL;
    qImg_focus             = NULL;
    qImg_video             = NULL;
    hoverLabelPos          = QPoint(0, 0);
    hoverLabelPosValid     = false;

    translator             = NULL;//init translator object is null.
    cmenu_captureExp       = NULL;
    cmenu_imgArea          = NULL;

    downloadPre            = NULL;
    downloadFoc            = NULL;
    downloadCap            = NULL;
    liveCap                = NULL;
    threadProcessImage     = NULL;
    videoShowThread        = NULL;
    exePlanTable           = NULL;
    runCFWOrder            = NULL;
    runCFWOrder            = new ExecuteCFWOrder();

    fwhm_x                 = 0;//focus assistant中fwhm图的起始点坐标
    fwhm_y                 = 0;
    peak_x                 = 0;//focus assistant中peak图的起始点坐标
    peak_y                 = 0;

    Preview_WPOS           = 65535;//初始化W B位置值
    Preview_BPOS           = 0;
    Focus_WPOS             = 65535;
    Focus_BPOS             = 0;
    Capture_WPOS           = 65535;
    Capture_BPOS           = 0;
    Live_WPOS              = 65535;
    Live_BPOS              = 0;

    noImgInWorkMode        = true;

    OverScanRMS            = 0;

    FocusCenterX_Pre       = 300;//初始化focus中心坐标
    FocusCenterY_Pre       = 300;
    focusAreaStartX        = 0;
    focusAreaStartY        = 200;
    focusAreaSizeX         = 800;
    focusAreaSizeY         = 200;
    ZoomFocus_X            = 0;
    ZoomFocus_Y            = 0;
    FocusZoomMode          = 0;
    FocusCurveX            = 0;

    viewBoxCX              = 98;
    viewBoxCY              = 64;
    viewBoxW               = 100;
    viewBoxH               = 100;

    //ini file parameters init
    iniFileParams.iniFileExist = false;
    iniFileParams.lang = "EN";
    iniFileParams.Gain = 0;
    iniFileParams.Offset = 0;
    iniFileParams.tecPretect = false;
    iniFileParams.slowestDowload = false;
    iniFileParams.clamp = false;
    iniFileParams.bPos_Preview = 0;
    iniFileParams.wPos_Preview = 65535;
    iniFileParams.bPos_Focus = 0;
    iniFileParams.wPos_Focus = 65535;
    iniFileParams.bPos_Capture = 0;
    iniFileParams.wPos_Capture = 65535;
    iniFileParams.autoStretchMode = 0;
    iniFileParams.ignoreOverScan = false;
    iniFileParams.calibrateOverScan = false;
    iniFileParams.calConstant = 1000;
    iniFileParams.CFWSlotsNum = 0;

    loadParamFromIni("SoftSetting", "TestGuider", &iniFileParams.testGuider, false);
    mainMenuBar->actTestGuid->setChecked(iniFileParams.testGuider);
    mainMenuBar->actTestMode->setChecked(iniFileParams.autoConnect);

    // switch to current language.
    languageChanged();

    //init lang menu
    if(ix.lang.compare("CN") == 0)
        mainMenuBar->actChinese->setChecked(true);
    else if(ix.lang.compare("JP") == 0)
        mainMenuBar->actJapanese->setChecked(true);
    else
        mainMenuBar->actEnglish->setChecked(true);

    //load the FIT Header data form CSV file
    QString path_csv = QApplication::applicationDirPath() + "/" + "FITHEADER.csv";
    path_csv = QDir::toNativeSeparators(path_csv);
    fitHeader_dialog->loadCSV(path_csv);

    QString path_ezcap = QCoreApplication::applicationDirPath() + "/" + "EZCAP.ini";
    path_ezcap = QDir::toNativeSeparators(path_ezcap);
    QFile *settingFile = new QFile(path_ezcap);
    if(settingFile->exists())
    {
        QSettings *iniRead = new QSettings(path_ezcap, QSettings::IniFormat);

        iniRead->beginGroup("SoftSetting");
        ix.saveTHPFile                    = iniRead->value("SaveTHPFile",           false).toBool();
        iniFileParams.autoConnectLive     = iniRead->value("autoConnectLive",      false).toBool();
        iniFileParams.fullScreen          = iniRead->value("fullScreen",           false).toBool();
        iniFileParams.enableMsgClient     = iniRead->value("enableMsgClient",      false).toBool();
        iniFileParams.msgClientName       = iniRead->value("msgClientName",    "NotName").toString();
        iniRead->endGroup();

        DBGOPT_INFO("Constructor: autoConnectLive = %d", iniFileParams.autoConnectLive);
        DBGOPT_INFO("Constructor: fullScreen = %d", iniFileParams.fullScreen);
        DBGOPT_INFO("Constructor: enableMsgClient = %d", iniFileParams.enableMsgClient);
        DBGOPT_INFO("Constructor: msgClientName = %s", qPrintable(iniFileParams.msgClientName));

        delete iniRead;
    }
    //20201125lyl温度湿度压力数据保存
#if THP_File_Saved
    if(ix.saveTHPFile)
    {
        mainMenuBar->actSaveTHPFile->setChecked(true);

        QString filenameTEMP = QFileDialog::getSaveFileName(this,tr("Save"),"",tr("temp Files(*.txt)")); //选择路径txt   xlsx
        if(filenameTEMP.isEmpty())
        {
            qDebug() << "save Excel Files: fileName is empty, can not save";
        }
        else
        {
            fileTHP=filenameTEMP;
            if(QFile::exists(filenameTEMP))
            {
                qDebug() << "exists  Files: " << filenameTEMP;
                QFile::remove(filenameTEMP);
            }
            qDebug() << "save Excel Files: " << filenameTEMP;
            QFile fileTEMP(filenameTEMP);
            fileTEMP.open(QIODevice::Text | QIODevice::WriteOnly);
            QTextStream out(&fileTEMP);
            out<<qSetFieldWidth(5)<<left<<"\t\t time\t Temp\t Humidity\t Press\t rate\t";
            fileTEMP.close();
        }
    }
#endif
    if(iniFileParams.oldSDK){
        qDebug() << "Skip Event when use Old SDK";
        return;
    }

    libqhyccd->RegisterPnpEventOut(pnp_Event_Out_Func);
    libqhyccd->RegisterPnpEventIn(pnp_Event_In_Func);
    libqhyccd->RegisterDataEventSingle(data_Event_Single_Func);
    libqhyccd->RegisterDataEventLive(data_Event_Live_Func);
    libqhyccd->RegisterPnpEvent(pnpEventExFunc);
    libqhyccd->RegisterTransferEventError(transferEventErrorFunc);
    libqhyccd->RegisterPnpEventUVLO(pnp_Event_UVLO_Func);
    updateWindowsTitle();
    ui->plainTextEdit_debug->hide();

    updateImgTimer = new QTimer(this);
    QObject::connect(updateImgTimer, SIGNAL(timeout()), this, SLOT(displayLiveImage()));

    updateFrameTimer = new QTimer(this);
    QObject::connect(updateFrameTimer, SIGNAL(timeout()), this, SLOT(showFrameCount()));

    liveCap = new LiveCapThread();
    connect(liveCap, SIGNAL(gotGPSData()), gpsTool_dialog, SLOT(updateGPSInfo()));
    connect(liveCap, SIGNAL(gotFPSData()), this, SLOT(showFPS()));
    connect(liveCap, SIGNAL(gotHistData()), this, SLOT(displayHistogramImageLive()));
    ix.darkSave = false;
    ix.canDarkSave = false;
    ix.darkNum = 0;
    ix.addedNum = 0;

    threadProcessImage = new ThreadProcessImage();
    connect(threadProcessImage, SIGNAL(gotCalData()), this, SLOT(saveCal2Image()));

    videoShowThread = new VideoShowThread();

    threadTempControl = new ThreadTempControl();

    ImgHist.create(cv::Size(managerMenu->ui->img_hist->width(), managerMenu->ui->img_hist->height()), CV_8UC3);

    ImgView.create(cv::Size(managerMenu->ui->img_screenView->width(), managerMenu->ui->img_screenView->height()), CV_8UC3);
    memset(ImgView.data, 0, managerMenu->ui->img_screenView->width() * managerMenu->ui->img_screenView->height() * 3);

#ifdef CALAB_YAU_PLANETARIUM

    //websocket for p2p connection
    //websocket server
    wstimer = new QTimer;
    wstimer->setInterval(300);
    wstimer->stop();

    _sslMode= QWebSocketServer::NonSecureMode;
    _wss = new QWebSocketServer("wss_test",_sslMode,this);
    connect(_wss,SIGNAL(newConnection()),this,SLOT(wss_newconnection()));
    connect(_wss,SIGNAL(closed()),this,SLOT(wss_closed()));
    connect(_wss,SIGNAL(serverError(QWebSocketProtocol::CloseCode)),this,SLOT(wss_serverError(QWebSocketProtocol::CloseCode)));

    QHostAddress hostAddress;

    hostAddress.setAddress(getLocalIp());
#endif

//    toolBurst_dialog->show();
}


#ifdef CALAB_YAU_PLANETARIUM
//websocket

void EZCAP::onConnected(){
    wstimer->stop();
    qDebug()<<"websocket connected successful"<<_ws->peerAddress()<<_ws->peerPort();
}

void EZCAP::onDisconnected(){
    wstimer->start();
    qDebug("websocket is disconnected");
}

void EZCAP::onTextReceived(QString msg){
    qDebug("get one msg");
    qDebug()<<msg;
}

void EZCAP::reconnect(){
    qDebug("websocket reconnected");
    _ws->abort();
    _ws->open(QUrl("ws://192.168.31.7:8800"));
}



//*******************************************************************
//*******************   websocket server  ***************************
//*******************************************************************

void EZCAP::sendQMountInfo(void){
    /*
    QJsonObject mountInfo;


    mountInfo.insert("TextCode","QMountInfo");
    mountInfo.insert("Model","QMount21");
    mountInfo.insert("SerialNumber","000001");
    mountInfo.insert("CanSync",true);
    mountInfo.insert("CanSyncAltAz",true);
    mountInfo.insert("CanSlew",true);
    mountInfo.insert("CanSlewAsync",false);
    mountInfo.insert("CanSlewAltAz",false);
    mountInfo.insert("CanSlowAltAzAsync",false);
    mountInfo.insert("CanSetTracking",true);
    mountInfo.insert("CanPulseGuide",true);
    mountInfo.insert("CanPark",true);
    mountInfo.insert("CanUnpark",true);
    mountInfo.insert("CanSetPark",true);

    mountInfo.insert("DoesFraction",true);
    mountInfo.insert("AlignmentMode",0);
    mountInfo.insert("SideOfPier",-1);
    mountInfo.insert("CanMoveAxisPrimary",true);
    mountInfo.insert("CanMoveAxisSecondary",true);

    mountInfo.insert("Desription","QMount Firmware V2021.11.24");  //mount firmware version
    mountInfo.insert("Owner","Qiu");

    mountInfo.insert("SiteElevation",glElevation);  //?? there is both altitude and elevation in ascom driver
    mountInfo.insert("SiteLatitude",RadToDegree(glLatitude_radian));
    mountInfo.insert("SiteLongitude",RadToDegree(glLongitude_radian));

    mountInfo.insert("ApertureArea",10.0);
    mountInfo.insert("ApertureDiameter",60.0);
    mountInfo.insert("FocalLenth",1000.0);

    mountInfo.insert("CanFindHome",false);
    mountInfo.insert("AtHome",false);
    mountInfo.insert("AtPark",false);

    mountInfo.insert("Azimuth",glCurrentAZ_Degree);     //current AZ    unit:
    mountInfo.insert("Altitude",glCurrentALT_Degree);    //current ALT   unit:
    mountInfo.insert("RA",glCurrentRA_Degree);          //current RA    unit:
    mountInfo.insert("DEC",glCurrentDEC_Degree);         //current DEC   unit:


    mountInfo.insert("GuideRateRA",1.0);
    mountInfo.insert("GuideRateDEC",1.0);

    mountInfo.insert("CanSetDEC",true);
    mountInfo.insert("CanSetRA",true);

    mountInfo.insert("CanSetPierSide",true);
    mountInfo.insert("CanSetRARates",true);
    mountInfo.insert("CanSetDECRates",true);

    mountInfo.insert("CanSetGuideRates",true);

    mountInfo.insert("DECRate",0.0);
    mountInfo.insert("RARate",0.0);

    mountInfo.insert("TrackingRate",0);  //0=sideral 1= lunar  2= solar  3=king

    mountInfo.insert("TargetRA",0.0);
    mountInfo.insert("TargetDEC",0.0);

    mountInfo.insert("Tracking",true); //if mount is traking

    mountInfo.insert("IsPulseGuiding",false);

    mountInfo.insert("Slewing",false);

    mountInfo.insert("EquatorialSystem",2); //0=other 1= topo 2=j2000 3= j2050  4=B1950


    double jd=0;
    //getJDFromDate(&jd,2021,11,25,22,8,5.35);
    jd=getJDFromSystem();
    mountInfo.insert("MountTimeJD",jd);







    QJsonDocument document;
    document.setObject(mountInfo);
    QByteArray byteArray = document.toJson(QJsonDocument::Compact);
    QString strMountInfo("GetQMountInfo..."+byteArray);
    //qDebug()<<strMountInfo;
    wss_sendText(glClientIP,glClientPort,strMountInfo);

*/
}

void decodeClientCommand(QString context,QString &CommandName,double &value1,double &value2,double &value3){
    QJsonParseError jsonError;
    QByteArray bytes=context.toUtf8();

    QJsonDocument doucment = QJsonDocument::fromJson(bytes, &jsonError);  // ×ª»¯Îª JSON ÎÄµµ
    if (!doucment.isNull() && (jsonError.error == QJsonParseError::NoError)) {  // ½âÎöÎ´·¢Éú´íÎó
        if (doucment.isObject()) { // JSON ÎÄµµÎª¶ÔÏó
            QJsonObject object = doucment.object();  // ×ª»¯Îª¶ÔÏó
            if (object.contains("CommandName")) {  // °üº¬Ö¸¶¨µÄ key
                QJsonValue value = object.value("CommandName");  // »ñÈ¡Ö¸¶¨ key ¶ÔÓ¦µÄ value
                if (value.isString()) {  // ÅÐ¶Ï value ÊÇ·ñÎª×Ö·û´®
                    CommandName = value.toString();  // ½« value ×ª»¯Îª×Ö·û´®
                 //   int length=CommandName.length();
                 //   CommandName=CommandName.mid(1,length-1);

                    qDebug() << "CommandName : " << CommandName;
                }
            }
            if (object.contains("Value1")) {
                QJsonValue value = object.value("Value1");
                if (value.isDouble()) {
                    value1 = value.toVariant().toDouble();
                    qDebug() << "v1 : " << value1;
                }
            }
            if (object.contains("Value2")) {
                QJsonValue value = object.value("Value2");
                if (value.isDouble()) {
                    value2 = value.toVariant().toDouble();
                    qDebug() << "v2 : " << value2;
                }
            }

            if (object.contains("Value3")) {
                QJsonValue value = object.value("Value3");
                if (value.isDouble()) {
                    value3 = value.toVariant().toDouble();
                    qDebug() << "v3 : " << value3;
                }
            }

        }
    }
}


void EZCAP::decodeTextMessage(QString message){

    int length;
    length=message.length();

    if(length>16){

        QString head;
        QString context;

        head    = message.mid(0,16);
        context = message.mid(16,length-16);


        qDebug()<<head;
        qDebug()<<context;


      if(head=="ClientCommand...") {

        qDebug()<<"process command from client";

        //v1=Gain
        //v2=exposure
        //v3=Offset

        double v1,v2,v3;
        QString CommandName="";
        decodeClientCommand(context,CommandName,v1,v2,v3);
        //managerMenu->ui->sliderLiveGain->setValue((int)v1);


        if(CommandName=="Gain")               managerMenu->ui->sliderLiveGain->setValue((int)v1);
        else if(CommandName=="EXPOSURE")      managerMenu->ui->sliderLiveExposure->setValue((int)v1);
        else if(CommandName=="Offset")        managerMenu->ui->sliderLiveOffset->setValue((int)v1);
        else if(CommandName=="TRANSFER")  {

            QString imageInfo;

            imageInfo="imsageX:"+QString::number(ix.FrameW_Last)+" FrameH:"+QString::number(ix.FrameH_Last)+" imageBpp:"+QString::number(ix.FrameB_Last);

            wss_sendText(glClientIP,glClientPort,imageInfo);

            wss_listAllConnection();
            qDebug()<<"begin to transfer";

            QByteArray ImageDataToTransfer;
            ImageDataToTransfer.resize(ix.FrameW_Last*ix.FrameH_Last*ix.FrameB_Last/8);
            memcpy(ImageDataToTransfer.data(),ix.ImgData_Last,ImageDataToTransfer.length());

            qDebug()<<glClientIP<<glClientPort<<ImageDataToTransfer.size();

            wss_sendByte(glClientIP,glClientPort,ImageDataToTransfer);
            qDebug()<<"end to transfer";
        }



        //sendQMountInfo();
        //glClientConnected = true;  //in temp.  from this time , the server will begin to send message to client in period

       }
    }

    else{
        //received message does not meet the standard of client command
    }


}





void EZCAP::wss_start(QHostAddress hostAddress,qint32 port){

}
void EZCAP::wss_stop(){

}
void EZCAP::wss_sendText(QString ip,qint32 port , QString message){
 QString key = QString("%1-%2").arg(ip).arg(port);
 //qDebug()<<key<<message;
 if(_hashIpPort2PWebSocket.contains(key)){
   _hashIpPort2PWebSocket.value(key)->sendTextMessage(message);
 }

}

void EZCAP::wss_sendByte(QString ip,qint32 port , QByteArray message){
 QString key = QString("%1-%2").arg(ip).arg(port);
 //qDebug()<<key<<message;
 if(_hashIpPort2PWebSocket.contains(key)){
   _hashIpPort2PWebSocket.value(key)->sendBinaryMessage(message);
 }

}


QString EZCAP::wss_getState(QString ip,qint32 port){
 QString key = QString("%1-%2").arg(ip).arg(port);
 QAbstractSocket::SocketState s;

 if(_hashIpPort2PWebSocket.contains(key)){
   s=_hashIpPort2PWebSocket.value(key)->state();
 }
 qDebug()<<"wss_state"<<key<<s;
 QMetaEnum metaEnum = QMetaEnum::fromType<QAbstractSocket::SocketState>();
 //TO BE FINISHED




 return "";
}

bool EZCAP::wss_checkAlive(QString ip,qint32 port){
    QString key = QString("%1-%2").arg(ip).arg(port);
    bool value= false;
    QByteArray s;
    s.resize(100);
    for(int i=0;i<100;i++){
        s[i]=i;
    }


    if(_hashIpPort2PWebSocket.contains(key)){
      _hashIpPort2PWebSocket.value(key)->ping(s);
        value=true;
    }
    else{
        value=false;
    }


    qDebug()<<"wss_checkAlive"<<key<<s;

}



void EZCAP::wss_listAllConnection(){
    QList<QWebSocket *> _listWebSocket = _hashIpPort2PWebSocket.values();

    qDebug()<<"************ all websocket connection ***********";
    for(int index = 0; index < _listWebSocket.size(); index++)
    {
        qDebug()<<index<<_listWebSocket.at(index)->peerAddress()<<_listWebSocket.at(index)->peerPort();
    }

}



void EZCAP::wss_newconnection(){
  QWebSocket *pWebSocket = _wss->nextPendingConnection();
  connect(pWebSocket,SIGNAL(disconnected()),this,SLOT(wss_disconnected()));
  connect(pWebSocket,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(wss_error(QAbstractSocket::SocketError)));
  connect(pWebSocket,SIGNAL(textMessageReceived(QString)),this,SLOT(wss_textMessageReceived(QString)));
  connect(pWebSocket,SIGNAL(binaryMessageReceived(QByteArray)),this,SLOT(wss_binaryMessageReceived(QByteArray)));

  connect(pWebSocket,SIGNAL(pong(quint64,QByteArray)),this,SLOT(wss_pong(quint64,QByteArray)));
  connect(pWebSocket,SIGNAL(stateChanged(QAbstractSocket::SocketState)),this,SLOT(wss_stateChanged(QAbstractSocket::SocketState)));



  _hashIpPort2PWebSocket.insert(QString("%1-%2").arg(pWebSocket->peerAddress().toString()).arg(pWebSocket->peerPort()),pWebSocket);
  qDebug()<<__FILE__<<__LINE__<< "_wws:new connection established" << pWebSocket->peerAddress().toString()<<pWebSocket->peerPort();

  glClientIP   = pWebSocket->peerAddress().toString();
  glClientPort = pWebSocket->peerPort();



  wss_listAllConnection();


  //emit signal_conncted(pWebSocket->peerAddress().toString(), pWebSocket->peerPort());
}
void EZCAP::wss_serverError(QWebSocketProtocol::CloseCode closeCode){

    QWebSocket *pWebSocket = dynamic_cast<QWebSocket *>(sender());
     if(!pWebSocket)
     {
         return;
     }

     qDebug()<<"server error detected";
     //emit signal_error(pWebSocket->peerAddress().toString(), pWebSocket->peerPort(), _pWebSocketServer->errorString());

}




void EZCAP::wss_closed(){
    QList<QWebSocket *> _listWebSocket = _hashIpPort2PWebSocket.values();
    for(int index = 0; index < _listWebSocket.size(); index++)
    {
        _listWebSocket.at(index)->close();
    }
    _hashIpPort2PWebSocket.clear();
    //emit signal_close();
}




void EZCAP::wss_disconnected(){
   qDebug()<<"_wws:disconnected event";
   QWebSocket *pWebSocket = dynamic_cast<QWebSocket *>(sender());
   if(!pWebSocket) return;
   //emit signal_disconncted(pWebSocket->peerAddress().toString(), pWebSocket->peerPort());
   _hashIpPort2PWebSocket.remove(QString("%1-%2").arg(pWebSocket->peerAddress().toString()).arg(pWebSocket->peerPort()));
   wss_listAllConnection();
}


void EZCAP::wss_error(QAbstractSocket::SocketError error){
    QWebSocket *pWebSocket = dynamic_cast<QWebSocket *>(sender());
      if(!pWebSocket)
      {
          return;
      }
      qDebug()<<"connection error detected";
      //emit signal_error(pWebSocket->peerAddress().toString(), pWebSocket->peerPort(), pWebSocket->errorString());
}


void EZCAP::wss_textFrameReceived(const QString &frame, bool isLastFrame){
    QWebSocket *pWebSocket = dynamic_cast<QWebSocket *>(sender());
      if(!pWebSocket)
      {
          return;
      }
      qDebug() << __FILE__ << __LINE__ << frame << "isLastFrame=" << isLastFrame;
      //emit signal_textFrameReceived(pWebSocket->peerAddress().toString(), pWebSocket->peerPort(), frame, isLastFrame);

}
void EZCAP::wss_textMessageReceived(const QString &message){
    QWebSocket *pWebSocket = dynamic_cast<QWebSocket *>(sender());
       if(!pWebSocket)
       {
           return;
       }
       qDebug()<<"wss_textMessageReceived"<<message;
       //emit signal_textMessageReceived(pWebSocket->peerAddress().toString(), pWebSocket->peerPort(), message);

       decodeTextMessage(message);
}

void EZCAP::wss_binaryMessageReceived(const QByteArray &message){
    QWebSocket *pWebSocket = dynamic_cast<QWebSocket *>(sender());
       if(!pWebSocket)
       {
           return;
       }
       qDebug()<<"wss_binaryMessageReceived"<<message;
       //emit signal_textMessageReceived(pWebSocket->peerAddress().toString(), pWebSocket->peerPort(), message);
}

void EZCAP::wss_pong(quint64 elapsedTime, const QByteArray &payload){

       QWebSocket *pWebSocket = dynamic_cast<QWebSocket *>(sender());
       if(!pWebSocket)
       {
           return;
       }
       qDebug()<<"wss_pong|pong"<<payload;
       //emit signal_textMessageReceived(pWebSocket->peerAddress().toString(), pWebSocket->peerPort(), message);
}


void EZCAP::wss_stateChanged(QAbstractSocket::SocketState state){
    QWebSocket *pWebSocket = dynamic_cast<QWebSocket *>(sender());
       if(!pWebSocket)
       {
           return;
       }

    QString ip   = pWebSocket->peerAddress().toString();
    int     port = pWebSocket->peerPort();

       qDebug()<<"wss_stateChanged"<<ip<<":"<<port<<state;
       //emit signal_textMessageReceived(pWebSocket->peerAddress().toString(), pWebSocket->peerPort(), message);
}


#endif




void EZCAP::showEvent(QShowEvent *)
{
    DBGOPT_INFO("=== showEvent() called ===");
    DBGOPT_INFO("=== About to check INI file ===");
//    QTimer::singleShot(100, this, SLOT(initLibqhyccd()));
//        this->loadParasFromIni();
        initLibqhyccd();

        if(libqhyccd->GetQHYCCDPCIECardNum && libqhyccd->GetQHYCCDPCIECardVer)
        {
            QString strVer = "";
            uint32_t num, year,month,day,subday;
            libqhyccd->GetQHYCCDPCIECardNum(&num);

            if(num != 0)
            {
                for(uint32_t i = 0; i < num; i++)
                {
                    libqhyccd->GetQHYCCDPCIECardVer(i, &year, &month, &day, &subday);
                    strVer += "PCIE Card " + QString::number(i+1) + " Ver : " + QString::number(year) + "-" + QString::number(month) + "-" + QString::number(day) + "-" + QString::number(subday);
                    DBGOPT_INFO("strVer = %s", qPrintable(strVer));
                    if(i < num - 1) strVer += "\n";
                }
            }
            else
            {
                strVer = "No PCIE Card be found";
            }

            about_dialog->updatePCIECardVer(strVer);
        }
        else
        {
            DBGOPT_ERROR("can't support GetQHYCCDPCIECardNum&GetQHYCCDPCIECardVer functions");
            about_dialog->updatePCIECardVer("-");
        }

        this->loadParamFromIni("SoftSetting", "autoConnectLive", &iniFileParams.autoConnectLive, false);
        DBGOPT_INFO("showEvent: autoConnectLive = %d", iniFileParams.autoConnectLive);

        QString iniPath = QDir::toNativeSeparators(QCoreApplication::applicationDirPath() + "/EZCAP.ini");
        DBGOPT_INFO("INI file path: %s", qPrintable(iniPath));
        QFile iniFile(iniPath);
        if(iniFile.exists()){
            DBGOPT_INFO("INI file exists");
        } else {
            DBGOPT_ERROR("INI file does NOT exist!");
        }

        this->loadParamFromIni("SoftSetting", "fullScreen", &iniFileParams.fullScreen, false);
        DBGOPT_INFO("showEvent: fullScreen = %d", iniFileParams.fullScreen);
        if(iniFileParams.fullScreen){
            eventFilter(this->ui->label_ImgShow, new  QEvent(QEvent::MouseButtonDblClick));
        }
        this->loadParamFromIni("SoftSetting", "enableMsgClient", &iniFileParams.enableMsgClient, false);
        this->loadParamFromIni("SoftSetting", "msgClientName", &iniFileParams.msgClientName, QString("NotName"));
        DBGOPT_INFO("showEvent: enableMsgClient = %d", iniFileParams.enableMsgClient);
        DBGOPT_INFO("showEvent: msgClientName = %s", qPrintable(iniFileParams.msgClientName));
        if(iniFileParams.enableMsgClient){
            DBGOPT_INFO("showEvent: Calling UDPServerScan()...");
            UDPServerScan();
        } else {
            DBGOPT_INFO("showEvent: enableMsgClient is false, UDP scan skipped");
        }
}

void EZCAP::initLibqhyccd(){
    unsigned int ret=NULL;
    ret =libqhyccd->InitQHYCCDResource(); //ret = InitQHYCCDResource();
    if(ret != QHYCCD_SUCCESS)
    {
        DBGOPT_WARNING("InitQHYCCDResource() failed!");
        if(libqhyccd)
        {
            delete libqhyccd;
            libqhyccd=NULL;
        }
    }
    else
    {
        qDebug() << "EZCAP   InitQHYCCDSDK success";
    }

    if(about_dialog)
    {
        DBGOPT_INFO("about created");
    }
}


EZCAP::~EZCAP()
{
    if(mainMenuBar)
    {
        delete mainMenuBar;
        mainMenuBar = NULL;
    }

    if(scrollArea_ImgShow)
    {
        delete scrollArea_ImgShow;
        scrollArea_ImgShow = NULL;
        pixelMagnifier = NULL;//随 viewport 一起被释放
    }
    if(about_dialog)
    {
        delete about_dialog;
        about_dialog = NULL;
    }
    if(favorite_dialog)
    {
        delete favorite_dialog;
        favorite_dialog = NULL;
    }
    if(gpsTool_dialog)
    {
        delete gpsTool_dialog;
        gpsTool_dialog = NULL;
    }
    if(phdLink_dialog)
    {
        delete phdLink_dialog;
        phdLink_dialog = NULL;
    }

    if(darkFrameTool_dialog)
    {
        delete darkFrameTool_dialog;
        darkFrameTool_dialog = NULL;
    }

    if(frameToolCapCal_dialog)
    {
        delete frameToolCapCal_dialog;
        frameToolCapCal_dialog = NULL;
    }

    if(frameToolCal_dialog)
    {
        delete frameToolCal_dialog;
        frameToolCal_dialog = NULL;
    }

    if(fitHeader_dialog)
    {
        delete fitHeader_dialog;
        fitHeader_dialog = NULL;
    }
    if(planner_dialog)
    {
        delete planner_dialog;
        planner_dialog = NULL;
    }
    if(tempControl_dialog)
    {
        delete tempControl_dialog;
        tempControl_dialog = NULL;
    }
    if(cameraChooser)
    {
        delete cameraChooser;
        cameraChooser = NULL;
    }

    //20200220 lyl Add ReadMode Dialog
    if(readMode)
    {
        delete readMode;
        readMode = NULL;
    }

    if(threadTempControl)
    {
        delete threadTempControl;
        threadTempControl = NULL;
    }

    if(statusLabel_imgSize)
    {
        delete statusLabel_imgSize;
        statusLabel_imgSize = NULL;
    }
    if(statusLabel_mousePos)
    {
        delete statusLabel_mousePos;
        statusLabel_mousePos = NULL;
    }

    if(statusLabel_rgb)
    {
        delete statusLabel_rgb;
        statusLabel_rgb = NULL;
    }

    if(statusLabel_Temp)
    {
        delete statusLabel_Temp;
        statusLabel_Temp = NULL;
    }

    if(statusLabel_RH)
    {
        delete statusLabel_RH;
        statusLabel_RH = NULL;
    }
    if(statusLabel_PRESS)
    {
        delete statusLabel_PRESS;
        statusLabel_PRESS = NULL;
    }
    if(statusLabel_msg)
    {
        delete statusLabel_msg;
        statusLabel_msg = NULL;
    }

    if(statusLabel_SDKmsg)
    {
        delete statusLabel_SDKmsg;
        statusLabel_SDKmsg = NULL;
    }
    if(statusLabel_dev_status)
    {
        delete statusLabel_dev_status;
        statusLabel_dev_status =NULL;
    }
    if(statusLabel_frame_status)
    {
        delete statusLabel_frame_status;
        statusLabel_frame_status =NULL;
    }
    if(statusLabel_ImgMean)
    {
        delete statusLabel_ImgMean;
        statusLabel_ImgMean = NULL;
    }

    if(cfwTimer)
    {
        delete cfwTimer;
        cfwTimer = NULL;
    }
    if(PumpTimer)
    {
        delete PumpTimer;
        PumpTimer = NULL;
    }
    if(PumpV2CycleTimer)
    {
        delete PumpV2CycleTimer;
        PumpV2CycleTimer = NULL;
    }
    if(PumpV2CycleSecondTimer)
    {
        delete PumpV2CycleSecondTimer;
        PumpV2CycleSecondTimer = NULL;
    }
    if(updateImgTimer)
    {
        delete updateImgTimer;
        updateImgTimer = NULL;
    }
    if(updateFrameTimer)
    {
        delete updateFrameTimer;
        updateFrameTimer = NULL;
    }

    if(managerLayout)
    {
        delete managerLayout;
        managerLayout = NULL;
    }
    if(mainLayout)
    {
        delete mainLayout;
        mainLayout = NULL;
    }

    if(ix.ImgData_Last)
    {
        delete ix.ImgData_Last;
        ix.ImgData_Last = NULL;
    }
    if(ix.ImgData)
    {
        delete ix.ImgData;
        ix.ImgData = NULL;
    }
    if(ix.ImgData_GPS)
    {
        delete ix.ImgData_GPS;
        ix.ImgData_GPS = NULL;
    }
    if(ix.ImgData_FB)
    {
        delete ix.ImgData_FB;
        ix.ImgData_FB = NULL;
    }

    if(ix.ImgData_Save)
    {
        delete ix.ImgData_Save;
        ix.ImgData_Save = NULL;
    }
    if(ix.ImgData_Dark)
    {
        delete ix.ImgData_Dark;
        ix.ImgData_Dark = NULL;
    }

    if(!ImgShow.empty())
    {
        ImgShow.release();
    }
    if(!ImgView.empty())
    {
        ImgView.release();
    }
    if(!ImgHist.empty())
    {
        ImgHist.release();
    }

    if(runCFWOrder)
    {
        delete(runCFWOrder);
        runCFWOrder = NULL;
    }

    if(libqhyccd)
    {
        delete libqhyccd;
        libqhyccd=NULL;
    }

    delete ui;
}

//根据相机初始参数设置界面中控件是否可用     相机打开成功后，设置界面初值
bool EZCAP::getParamsFromCamera()
{
    uint32_t ret;
    double value = 0;
    unsigned char buf[32];

    //Firmware Version
    ret = libqhyccd->GetQHYCCDFWVersion(camhandle, buf); //Firmware版本号
    if(ret == QHYCCD_ERROR)
    {
        ix.driverVer = "Unknown";
        DBGOPT_WARNING("GetQHYCCDFWVersion() Failed!");
    }
    else
    {
        //可正确显示2010年~2025的相机驱动版本
        unsigned char year = buf[0] >> 4;
        if(buf[4] == 1)//FX3 year > 25
            ix.driverVer = QString::number(buf[0], 10) + "-" + QString::number(buf[1], 10) + "-" + QString::number(buf[2], 10);
        else if(year <= 9)//FX3 year <= 25
            ix.driverVer = QString::number(year + 0x10,10) + "-" + QString::number((buf[0]&~0xf0),10) + "-" + QString::number(buf[1], 10);
        else//FX2
            ix.driverVer = QString::number(year,10)        + "-" + QString::number((buf[0]&~0xf0),10) + "-" + QString::number(buf[1], 10);
        if(ix.CamID.contains("PCIE") || ix.CamID.contains("ERIS")) ix.driverVer +=  "-" + QString::number(buf[2], 10);
    }

    //20200303lyl FPGAVersion
    ret = libqhyccd->GetQHYCCDFPGAVersion(camhandle, 0, buf);
    if(ret == QHYCCD_ERROR)
    {
        ix.FPGAVer= "-";
        DBGOPT_WARNING("GetQHYCCDFPGAVersion() index 0 Failed!");
    }
    else
    {
        ix.FPGAVer = QString::number(buf[0],10) + "-" + QString::number(buf[1],10) + "-" + QString::number(buf[2],10) + "-" + QString::number(buf[3],10);
    }

    ret = libqhyccd->GetQHYCCDFPGAVersion(camhandle, 1, buf);//获得第二个FPGAversion
    if(ret == QHYCCD_ERROR)
    {
        ix.FPGAVer1= "-";
        DBGOPT_WARNING("GetQHYCCDFPGAVersion() index 1 Failed!");
    }
    else
    {
        ix.FPGAVer1 = QString::number(buf[0],10) + "-" + QString::number(buf[1],10) + "-" + QString::number(buf[2],10) + "-" + QString::number(buf[3],10);
    }

    //----------------------Chip Info----------------------
    ret = libqhyccd->GetQHYCCDChipInfo(camhandle, &ix.CCD_ChipW, &ix.CCD_ChipH, &ix.CCD_ImageW, &ix.CCD_ImageH, &ix.CCD_PixelW, &ix.CCD_PixelH, &ix.CCD_ImageB);
    if(ret == QHYCCD_SUCCESS && ix.CCD_ChipW != ix.CCD_ChipH != ix.CCD_ImageW != ix.CCD_ImageH != ix.CCD_PixelW != ix.CCD_PixelH != ix.CCD_ImageB != 0)
    {
        DBGOPT_INFO("GetQHYCCDChipInfo() w = %d h = %d", ix.CCD_ImageW, ix.CCD_ImageH);
        ix.ImageW_Max = ix.CCD_ImageW;
        ix.ImageH_Max = ix.CCD_ImageH;
        ix.Resolution_Max = ix.CCD_ImageW * ix.CCD_ImageH;

        ix.ImageW_Min = ix.CCD_ImageW;
        ix.ImageH_Min = ix.CCD_ImageH;
        ix.Resolution_Min = ix.CCD_ImageW * ix.CCD_ImageH;
    }
    else
    {
        DBGOPT_WARNING("GetQHYCCDChipInfo() Failed!", __FILE__, __FUNCTION__);
    }

    //------------------------ReadMode Resolution---------------------------
    ret = libqhyccd->GetQHYCCDReadModeResolution(camhandle, ix.ReadMode, &ix.ReadMode_ImageW, &ix.ReadMode_ImageH);
    if(ret == QHYCCD_SUCCESS && ix.ReadMode_ImageW != ix.ReadMode_ImageH != 0)
    {
        DBGOPT_INFO("GetQHYCCDReadModeResolution() w = %d h = %d", ix.ReadMode_ImageW, ix.ReadMode_ImageH);
        if(ix.Resolution_Min > ix.ReadMode_ImageW * ix.ReadMode_ImageH)
        {
            ix.ImageW_Min = ix.ReadMode_ImageW;
            ix.ImageH_Min = ix.ReadMode_ImageH;
            ix.Resolution_Min = ix.ReadMode_ImageW * ix.ReadMode_ImageH;
        }
    }
    else
    {
        DBGOPT_WARNING("GetQHYCCDReadModeResolution() Failed!");
    }

    //-------------------Resolution Error Detection-----------------------
    if(ix.CCD_ImageW != ix.ReadMode_ImageW || ix.CCD_ImageH != ix.ReadMode_ImageH)
    {
        DBGOPT_WARNING("GetQHYCCDChipInfo() Image Size != GetQHYCCDReadModeResolution Image Size");
    }

    for(int i = 0; i < ix.ReadMode_Num; i ++)
    {
        ret = libqhyccd->GetQHYCCDReadModeResolution(camhandle, i, &ix.ReadMode_ImageW, &ix.ReadMode_ImageH);
        if(ret == QHYCCD_SUCCESS && ix.ReadMode_ImageW != ix.ReadMode_ImageH != 0)
        {
            DBGOPT_INFO("GetQHYCCDReadModeResolution() 1 w = %d h = %d", ix.ReadMode_ImageW, ix.ReadMode_ImageH);
            if(ix.Resolution_Max < ix.ReadMode_ImageW*ix.ReadMode_ImageH)
            {
                ix.ImageW_Max = ix.ReadMode_ImageW;
                ix.ImageH_Max = ix.ReadMode_ImageH;
                ix.Resolution_Max = ix.ReadMode_ImageW * ix.ReadMode_ImageH;
            }
        }
        else
        {
            DBGOPT_WARNING("GetQHYCCDReadModeResolution() Failed!");
        }
    }

    DBGOPT_INFO("min w = %d h = %d", ix.ImageW_Min, ix.ImageH_Min);
    DBGOPT_INFO("max w = %d h = %d", ix.ImageW_Max, ix.ImageH_Max);

    //------------------------------有效区域范围--------------------------------
    ret = libqhyccd->GetQHYCCDEffectiveArea(camhandle, &ix.EffectiveX, &ix.EffectiveY, &ix.EffectiveW, &ix.EffectiveH);
    DBGOPT_INFO("Effective startx = %d starty = %d sizex = %d sizey = %d", ix.EffectiveX, ix.EffectiveY, ix.EffectiveW, ix.EffectiveH);
    if(ret != QHYCCD_SUCCESS || (ix.EffectiveX+ix.EffectiveW) > ix.ImageW_Min || (ix.EffectiveY+ix.EffectiveH) > ix.ImageH_Min)
    {
        DBGOPT_WARNING("GetQHYCCDEffectiveArea() Failed!");
    }

    //------------------------过扫区范围----------------------------
    ret = libqhyccd->GetQHYCCDOverScanArea(camhandle, &ix.OverscanX, &ix.OverscanY, &ix.OverscanW, &ix.OverscanH);
    DBGOPT_INFO("Overscan startx = %d starty = %d sizex = %d sizey = %d", ix.OverscanX, ix.OverscanY, ix.OverscanW, ix.OverscanH);
    if(ret != QHYCCD_SUCCESS || (ix.OverscanX+ix.OverscanW) > ix.ImageW_Min || (ix.OverscanY+ix.OverscanH) > ix.ImageH_Min)
    {
        DBGOPT_WARNING("GetQHYCCDOverScanArea() Failed! %d", ret);
    }

    if((ix.EffectiveW != ix.CCD_ImageW && ix.EffectiveH != ix.CCD_ImageH)
        && (ix.OverscanW == 0 && ix.OverscanH == 0))
    {
        DBGOPT_WARNING("Wrong Overscan Size!!!");
    }

    //----------------------------BIN Mode---------------------------
    ret =libqhyccd->IsQHYCCDControlAvailable(camhandle,CAM_BIN1X1MODE);
    if(ret != QHYCCD_SUCCESS)
    {
        DBGOPT_WARNING("IsQHYCCDControlAvailable() CAM_BIN1X1MODE Failed!");
    }
    ret = libqhyccd->IsQHYCCDControlAvailable(camhandle,CAM_BIN2X2MODE);
    if(ret == QHYCCD_SUCCESS)
    {
        ix.Bin22_Fun = true;
        ix.BinX_Max = 2;
        ix.BinY_Max = 2;
    }
    else
    {
        ix.Bin22_Fun = false;
        DBGOPT_INFO("IsQHYCCDControlAvailable() CAM_BIN2X2MODE Failed!");
    }
    ret =libqhyccd->IsQHYCCDControlAvailable(camhandle,CAM_BIN3X3MODE);
    if(ret == QHYCCD_SUCCESS)
    {
        ix.Bin33_Fun = true;
        ix.BinX_Max = 3;
        ix.BinY_Max = 3;
    }
    else
    {
        ix.Bin33_Fun = false;
        DBGOPT_INFO("IsQHYCCDControlAvailable() CAM_BIN3X3MODE Failed!");
    }
    ret =libqhyccd->IsQHYCCDControlAvailable(camhandle,CAM_BIN4X4MODE);
    if(ret == QHYCCD_SUCCESS)
    {
        ix.Bin44_Fun = true;
        ix.BinX_Max = 4;
        ix.BinY_Max = 4;
    }
    else
    {
        ix.Bin44_Fun = false;
        DBGOPT_INFO("IsQHYCCDControlAvailable() CAM_BIN4X4MODE Failed!");
    }
    ret =libqhyccd->IsQHYCCDControlAvailable(camhandle,CAM_BIN6X6MODE);
    if(ret == QHYCCD_SUCCESS)
    {
        ix.Bin66_Fun = true;
        ix.BinX_Max = 6;
        ix.BinY_Max = 6;
    }
    else
    {
        ix.Bin66_Fun = false;
        DBGOPT_INFO("IsQHYCCDControlAvailable() CAM_BIN6X6MODE Failed!");
    }
    ret =libqhyccd->IsQHYCCDControlAvailable(camhandle,CAM_BIN8X8MODE);
    if(ret == QHYCCD_SUCCESS)
    {
        ix.Bin88_Fun = true;
        ix.BinX_Max = 8;
        ix.BinY_Max = 8;
    }
    else
    {
        ix.Bin88_Fun = false;
        DBGOPT_INFO("IsQHYCCDControlAvailable() CAM_BIN8X8MODE Failed!");
    }

    ix.BinX = 1;
    ix.BinY = 1;

    //-----------------------------Bits Mode-----------------------------------
    ret = libqhyccd->IsQHYCCDControlAvailable(camhandle, CONTROL_TRANSFERBIT);
    if(ret != QHYCCD_SUCCESS)
    {
        DBGOPT_WARNING("IsQHYCCDControlAvailable() CONTROL_TRANSFERBIT Failed!");
    }
    ret = libqhyccd->IsQHYCCDControlAvailable(camhandle, CAM_8BITS);
    if(ret != QHYCCD_SUCCESS)
    {
        DBGOPT_WARNING("IsQHYCCDControlAvailable() CAM_8Bits False!");
    }

    ret = libqhyccd->IsQHYCCDControlAvailable(camhandle, CAM_16BITS);
    if(ret != QHYCCD_SUCCESS)
    {
        DBGOPT_WARNING("IsQHYCCDControlAvailable() CAM_16Bits Failed!");
    }

    value = libqhyccd->GetQHYCCDParam(camhandle, CONTROL_TRANSFERBIT);
    if((int)value%8 != 0)
    {
        DBGOPT_WARNING("GetQHYCCDParam() CONTROL_TRANSFERBIT value%8 != 0");
    }

    if(ix.camStreamMode == 0)
    {
        ix.Bits = 16;
    }
    else if(ix.camStreamMode == 1 && !ix.FoundCam)
    {
        ix.Bits = 8;
    }
    else
    {
        DBGOPT_ERROR("camStreamMode Value Error!");
    }

    DBGOPT_INFO("GetQHYCCDParam() CONTROL_TRANSFERBIT Bits = %d", ix.Bits);

    //----------------------------彩色模式及Bayer序列-------------------------
    ret = libqhyccd->IsQHYCCDControlAvailable(camhandle, CAM_IS_COLOR);
    if(ret == QHYCCD_ERROR)
    {
        ix.Color_Fun = false;
    }
    else
    {
        ix.Color_Fun = true;
        ix.CamBayer = libqhyccd->IsQHYCCDControlAvailable(camhandle, CAM_COLOR);
        if(ix.CamBayer > 4 || ix.CamBayer < 1)
        {
            DBGOPT_WARNING("IsQHYCCDControlAvailable() CAM_COLOR Failed!");
        }
        else
        {
            if(!ix.FoundCam)
            {
                //初次连接默认不开启彩色模式
                ix.IsCvtColor = false;
                ix.Color = false;
                ix.Color_Last = false;
            }
            ix.Bayer = ix.CamBayer;
        }
    }

    //////////////////////////////////////////////////
    /////////////////// 获取相机参数 ///////////////////
    //////////////////////////////////////////////////

    //-----------------------------Expose Time-----------------------------------
    ret = libqhyccd->IsQHYCCDControlAvailable(camhandle, CONTROL_EXPOSURE);
    if(ret != QHYCCD_SUCCESS)
    {
        DBGOPT_WARNING("IsQHYCCDControlAvailable() CONTROL_EXPOSURE Failed!");
    }
    ret = libqhyccd->GetQHYCCDParamMinMaxStep(camhandle, CONTROL_EXPOSURE, &ix.ExpTime_Min, &ix.ExpTime_Max, &ix.ExpTime_Step);
    if(ret != QHYCCD_SUCCESS && ix.ExpTime_Min >= ix.ExpTime_Max ||
      (ix.ExpTime_Min == 0 && ix.ExpTime_Max == 0) || ix.ExpTime_Step == 0)
    {
        DBGOPT_WARNING("GetQHYCCDParamMinMaxStep() CONTROL_TEXPOSURE Failed! min = %f max = %f step = %f",
                    ix.ExpTime_Min, ix.ExpTime_Max, ix.ExpTime_Step);
    }

    value = libqhyccd->GetQHYCCDParam(camhandle, CONTROL_EXPOSURE);
    if(value == QHYCCD_ERROR || value >= ix.ExpTime_Max || value <= ix.ExpTime_Min)
    {
        DBGOPT_WARNING("GetQHYCCDParam() CONTROL_EXPOSURE Failed! Wrong Value = %f", value);
    }

    if(!ix.FoundCam)
    {
        ix.ExpTime = 20.0; //默认20ms
        ix.ExpUnit = 1000.0;
    }

    //-------------------------------Gain------------------------------------
    ret = libqhyccd->IsQHYCCDControlAvailable(camhandle, CONTROL_GAIN);
    if(ret == QHYCCD_SUCCESS)
    {
        ix.Gain_Fun = true;
        ret = libqhyccd->GetQHYCCDParamMinMaxStep(camhandle, CONTROL_GAIN, &ix.Gain_Min, &ix.Gain_Max, &ix.Gain_Step);
        if(ret != QHYCCD_SUCCESS || ix.Gain_Min >= ix.Gain_Max ||
          (ix.Gain_Min == 0 && ix.Gain_Max == 0) || ix.Gain_Step == 0)
        {
            DBGOPT_WARNING("GetQHYCCDParamMinMaxStep() CONTROL_GAIN Failed! min = %f max = %f step = %f", ix.Gain_Min, ix.Gain_Max, ix.Gain_Step);
        }
        else
        {
            value = libqhyccd->GetQHYCCDParam(camhandle, CONTROL_GAIN);
            if(value == QHYCCD_ERROR || value < ix.Gain_Min || value > ix.Gain_Max)
            {
                DBGOPT_WARNING("GetQHYCCDParam() CONTROL_GAIN Failed! Wrong Value = %f", value);
            }
        }
        if(!ix.FoundCam) ix.Gain = 1.0;
    }
    else
    {
        ix.Gain_Fun = false;
    }

    //---------------------------------Offset-------------------------------
    ret = libqhyccd->IsQHYCCDControlAvailable(camhandle,CONTROL_OFFSET);
    if(ret == QHYCCD_SUCCESS)
    {
        ix.Offset_Fun = true;
        ret = libqhyccd->GetQHYCCDParamMinMaxStep(camhandle,CONTROL_OFFSET, &ix.Offset_Min, &ix.Offset_Max, &ix.Offset_Step);
        if(ret != QHYCCD_SUCCESS || ix.Offset_Min >= ix.Offset_Max ||
          (ix.Offset_Min == 0 && ix.Offset_Max == 0) || ix.Offset_Step == 0)
        {
            DBGOPT_WARNING("GetQHYCCDParamMinMaxStep() CONTROL_OFFSET Failed! min = %f max = %f step = %f", ix.Offset_Min, ix.Offset_Max, ix.Offset_Step);
        }
        else
        {
            value = libqhyccd->GetQHYCCDParam(camhandle, CONTROL_OFFSET);
            if(value == QHYCCD_ERROR || value < ix.Offset_Min || value > ix.Offset_Max)
            {
                DBGOPT_WARNING("GetQHYCCDParam() CONTROL_OFFSET Failed! Wrong Value = %f", value);
            }
        }
        if(!ix.FoundCam) ix.Offset = 1.0;
    }
    else
    {
        ix.Offset_Fun = false;
    }

    //--------------------------Speed-----------------------
    ret = libqhyccd->IsQHYCCDControlAvailable(camhandle,CONTROL_SPEED);
    if(ret == QHYCCD_SUCCESS)
    {
        ix.Speed_Fun = true;
        ret = libqhyccd->GetQHYCCDParamMinMaxStep(camhandle, CONTROL_SPEED, &ix.Speed_Min, &ix.Speed_Max, &ix.Speed_Step);
        if(ret != QHYCCD_SUCCESS || ix.Speed_Min >= ix.Speed_Max ||
           ix.Speed_Max == 0 || ix.Speed_Step == 0)
        {
            DBGOPT_WARNING("GetQHYCCDParamMinMaxStep() CONTROL_SPEED Failed! min = %f max = %f step = %f", ix.Speed_Min, ix.Speed_Max, ix.Speed_Step);
        }
        else
        {
            value = libqhyccd->GetQHYCCDParam(camhandle, CONTROL_SPEED);
            if(value == QHYCCD_ERROR || value < ix.Speed_Min || value > ix.Speed_Max)
            {
                DBGOPT_WARNING("GetQHYCCDParam() CONTROL_SPEED Failed! Wrong Value = %f", value);
            }
        }
        if(!ix.FoundCam) ix.Speed = 0.0;
    }
    else
    {
        ix.Speed_Fun = false;
    }

    //---------------------------Traffic-------------------------
    ret = libqhyccd->IsQHYCCDControlAvailable(camhandle, CONTROL_USBTRAFFIC);
    if(ret == QHYCCD_SUCCESS)
    {
        ix.Traffic_Fun = true;
        ret = libqhyccd->GetQHYCCDParamMinMaxStep(camhandle, CONTROL_USBTRAFFIC, &ix.Traffic_Min, &ix.Traffic_Max, &ix.Traffic_Step);
        if(ret != QHYCCD_SUCCESS || ix.Traffic_Min >= ix.Traffic_Max || ix.Traffic_Max == 0 || ix.Traffic_Step == 0)
        {
            DBGOPT_WARNING("GetQHYCCDParamMinMaxStep() CONTROL_TRAFFIC Failed! min = %f max = %f step = %f", ix.Traffic_Min, ix.Traffic_Max, ix.Traffic_Step);
        }
        else
        {
            value = libqhyccd->GetQHYCCDParam(camhandle, CONTROL_USBTRAFFIC);
            if(value == QHYCCD_ERROR || value < ix.Traffic_Min || value > ix.Traffic_Max)
            {
                DBGOPT_WARNING("GetQHYCCDParam() CONTROL_TRAFFIC Failed! Wrong Value = %f", value);
            }
        }
        if(!ix.FoundCam) ix.Traffic = 30.0;
    }
    else
    {
        ix.Traffic_Fun = false;
    }

    if(ix.camStreamMode == 1)
    {
        //---------------------Brightness-------------------------
        ret = libqhyccd->IsQHYCCDControlAvailable(camhandle, CONTROL_BRIGHTNESS);
        if(ret == QHYCCD_SUCCESS)
        {
            ix.Brightness_Fun = true;
            ret = libqhyccd->GetQHYCCDParamMinMaxStep(camhandle, CONTROL_BRIGHTNESS, &ix.Brightness_Min, &ix.Brightness_Max, &ix.Brightness_Step);
            if(ret != QHYCCD_SUCCESS || ix.Brightness_Min >= ix.Brightness_Max || ix.Brightness_Max == 0 || ix.Brightness_Step == 0)
            {
                DBGOPT_ERROR("GetQHYCCDParamMinMaxStep() CONTROL_BRIGHTNESS Failed! min = %f max = %f step = %f",
                            ix.Brightness_Min, ix.Brightness_Max, ix.Brightness_Step);
            }
            else
            {
                value = libqhyccd->GetQHYCCDParam(camhandle, CONTROL_BRIGHTNESS);
                if(value < ix.Brightness_Min || value > ix.Brightness_Max)
                {
                    DBGOPT_ERROR("GetQHYCCDParam() CONTROL_BRIGHTNESS Failed! Wrong Value = %f", value);
                }
            }
            if(!ix.FoundCam) ix.Brightness = 0.0;
        }
        else
        {
            ix.Brightness_Fun = false;
        }

        //---------------------------Contrast---------------------------
        ret = libqhyccd->IsQHYCCDControlAvailable(camhandle, CONTROL_CONTRAST);
        if(ret == QHYCCD_SUCCESS)
        {
            ix.Contrast_Fun = true;
            ret = libqhyccd->GetQHYCCDParamMinMaxStep(camhandle, CONTROL_CONTRAST, &ix.Contrast_Min, &ix.Contrast_Max, &ix.Contrast_Step);
            if(ret != QHYCCD_SUCCESS || ix.Contrast_Min >= ix.Contrast_Max || ix.Contrast_Max == 0 || ix.Contrast_Step == 0)
            {
                DBGOPT_ERROR("GetQHYCCDParamMinMaxStep() CONTROL_CONTRAST Failed! min = %f max = %f step = %f",
                            ix.Contrast_Min, ix.Contrast_Max, ix.Contrast_Step);
            }
            else
            {
                value = libqhyccd->GetQHYCCDParam(camhandle, CONTROL_CONTRAST);
                if(value < ix.Contrast_Min || value > ix.Contrast_Max)
                {
                    DBGOPT_ERROR("GetQHYCCDParam() CONTROL_CONTRAST Failed! Wrong Value = %f", value);
                }
            }
            if(!ix.FoundCam) ix.Contrast = 0.0;
        }
        else
        {
            ix.Contrast_Fun = false;
            OutputDebug("EZCAPWARNING | %s | %s | IsQHYCCDControlAvailable() CONTROL_CONTRAST Failed!", __FILE__, __FUNCTION__);
        }

        //--------------------------Gamma---------------------------------
        ret = libqhyccd->IsQHYCCDControlAvailable(camhandle, CONTROL_GAMMA);
        if(ret == QHYCCD_SUCCESS)
        {
            ix.Gamma_Fun = true;
            ret = libqhyccd->GetQHYCCDParamMinMaxStep(camhandle, CONTROL_GAMMA, &ix.Gamma_Min, &ix.Gamma_Max, &ix.Gamma_Step);
            if(ret != QHYCCD_SUCCESS || ix.Gamma_Min >= ix.Gamma_Max)
            {
                DBGOPT_ERROR("GetQHYCCDParamMinMaxStep() CONTROL_GAMMA Failed! min = %f max = %f step = %f",
                            ix.Gamma_Min, ix.Gamma_Max, ix.Gamma_Step);
            }
            else
            {
                value = libqhyccd->GetQHYCCDParam(camhandle, CONTROL_GAMMA);
                if(value == QHYCCD_ERROR || value < ix.Gamma_Min || value > ix.Gamma_Max)
                {
                    DBGOPT_ERROR("GetQHYCCDParam() CONTROL_GAMMA Failed! Wrong Value = %f", value);
                }
            }
            if(!ix.FoundCam) ix.Gamma = 1.0;
        }
        else
        {
            ix.Gamma_Fun = false;
        }

        //-------------------------------WBR-----------------------------
        ret = libqhyccd->IsQHYCCDControlAvailable(camhandle, CONTROL_WBR);
        if(ret == QHYCCD_SUCCESS)
        {
            ix.WBR_Fun = true;
            ret = libqhyccd->GetQHYCCDParamMinMaxStep(camhandle, CONTROL_WBR, &ix.WBR_Min, &ix.WBR_Max, &ix.WBR_Step);
            if(ret != QHYCCD_SUCCESS || ix.WBR_Min >= ix.WBR_Max || ix.WBR_Max == 0 || ix.WBR_Step == 0)
            {
                DBGOPT_ERROR("GetQHYCCDParamMinMaxStep() CONTROL_WBR Failed! min = %f max = %f step = %f",
                            ix.WBR_Min, ix.WBR_Max, ix.WBR_Step);
            }
            else
            {
                value = libqhyccd->GetQHYCCDParam(camhandle, CONTROL_WBR);
                if(value == QHYCCD_ERROR || value < ix.WBR_Min || value > ix.WBR_Max)
                {
                    DBGOPT_ERROR("GetQHYCCDParam() CONTROL_WBR Failed! Wrong Value = %f", value);
                }
            }
            if(!ix.FoundCam) ix.WBR = 32.0;
        }
        else
        {
            ix.WBR_Fun = false;
        }

        //-----------------------------WBG---------------------------------
        ret = libqhyccd->IsQHYCCDControlAvailable(camhandle, CONTROL_WBG);
        if(ret == QHYCCD_SUCCESS)
        {
            ix.WBG_Fun = true;
            ret = libqhyccd->GetQHYCCDParamMinMaxStep(camhandle, CONTROL_WBG, &ix.WBG_Min, &ix.WBG_Max, &ix.WBG_Step);
            if(ret != QHYCCD_SUCCESS || ix.WBG_Min >= ix.WBG_Max || ix.WBG_Max == 0 || ix.WBG_Step == 0)
            {
                DBGOPT_ERROR("GetQHYCCDParamMinMaxStep() CONTROL_WBG Failed! min = %f max = %f step = %f",
                            ix.WBG_Min, ix.WBG_Max, ix.WBG_Step);
            }
            else
            {
                value = libqhyccd->GetQHYCCDParam(camhandle, CONTROL_WBG);
                if(value == QHYCCD_ERROR || value < ix.WBG_Min || value > ix.WBG_Max)
                {
                    DBGOPT_ERROR("GetQHYCCDParam() CONTROL_WBG Failed! Wrong Value = %f", value);
                }
            }
            if(!ix.FoundCam) ix.WBG = 32.0;
        }
        else
        {
            ix.WBG_Fun = false;
        }

        //---------------------------------WBB-----------------------------------
        ret = libqhyccd->IsQHYCCDControlAvailable(camhandle, CONTROL_WBB);
        if(ret == QHYCCD_SUCCESS)
        {
            ix.WBB_Fun = true;
            ret = libqhyccd->GetQHYCCDParamMinMaxStep(camhandle, CONTROL_WBB, &ix.WBB_Min, &ix.WBB_Max, &ix.WBB_Step);
            if(ret != QHYCCD_SUCCESS || ix.WBB_Min >= ix.WBB_Max || ix.WBB_Max == 0 || ix.WBB_Step == 0)
            {
                DBGOPT_ERROR("GetQHYCCDParamMinMaxStep() CONTROL_WBB Failed! min = %f max = %f step = %f",
                            ix.WBB_Min, ix.WBB_Max, ix.WBB_Step);
            }
            else
            {
                value = libqhyccd->GetQHYCCDParam(camhandle, CONTROL_WBB);
                if(value == QHYCCD_ERROR || value < ix.WBB_Min || value > ix.WBB_Max)
                {
                    DBGOPT_ERROR("GetQHYCCDParam() CONTROL_WBB Failed! Wrong Value = %f", value);
                }
            }
            if(!ix.FoundCam) ix.WBB = 32.0;
        }
        else
        {
            ix.WBB_Fun = false;
        }

#if 0
        //------------------------DDR---------------------------
        ret = libqhyccd->IsQHYCCDControlAvailable(camhandle, CONTROL_DDR);
        if(ret == QHYCCD_SUCCESS)
        {
            ix.DDR_Fun = true;
            ix.DDR = true;//默认开启DDR
        }
        else
        {
            ix.DDR_Fun = false;
        }
#endif

        //--------------------------AMPV------------------------------
        ret = libqhyccd->IsQHYCCDControlAvailable(camhandle, CONTROL_AMPV);
        if(ret == QHYCCD_SUCCESS)
        {
            ix.AMPV_Fun = true;
            ix.AMPV = false;//默认关闭AMPV
        }
        else
        {
            ix.AMPV_Fun = false;
        }
    }

    //------------------------GPS------------------------------
    ret = libqhyccd->IsQHYCCDControlAvailable(camhandle, CAM_GPS);
    if(ret == QHYCCD_SUCCESS)
    {
        ix.GPS_Fun = true;
        ix.GPS = false;
    }
    else
    {
        ix.GPS_Fun = false;
    }

    //------------------------Burst------------------------------
    ret = libqhyccd->IsQHYCCDControlAvailable(camhandle, CAM_BURST_MODE);
    if(ret == QHYCCD_SUCCESS)
    {
        ix.Burst_Fun = true;
    }
    else
    {
        ix.Burst_Fun = false;
    }

    //--------------------------Cooler-----------------------------------
    ret = libqhyccd->IsQHYCCDControlAvailable(camhandle,CONTROL_COOLER);
    if(ret == QHYCCD_SUCCESS)
    {
        ix.Cooler_Fun = true;
    }
    else
    {
        ix.Cooler_Fun = false;
    }

    //------------------------------Humidity-----------------------
    ret = libqhyccd->IsQHYCCDControlAvailable(camhandle,CAM_HUMIDITY);
    if(ret == QHYCCD_SUCCESS)
    {
        ix.Humidity_Fun = true;
    }
    else
    {
        ix.Humidity_Fun = false;
    }

    //----------------------Pressure--------------------------
    ret = libqhyccd->IsQHYCCDControlAvailable(camhandle,CAM_PRESSURE);
    if(ret == QHYCCD_SUCCESS)
    {
        ix.Pressure_Fun = true;
    }
    else
    {
        ix.Pressure_Fun = false;
    }

    //-----------------------------------Mechanical Shutter-------------------------------
    ret = libqhyccd->IsQHYCCDControlAvailable(camhandle, CAM_MECHANICALSHUTTER);
    if(ret == QHYCCD_SUCCESS)
    {
        ix.canMechanicalShutter = true;
        ix.MechanicalShutterMode = 0;
    }
    else
    {
        ix.canMechanicalShutter = false;
    }

    //-----------------------------Finetone--------------------------
    ret = libqhyccd->IsQHYCCDControlAvailable(camhandle,CAM_FINETONE_INTERFACE);
    if(ret == QHYCCD_SUCCESS)
    {
        ix.canFineTone = true;
        ix.fineToneOnOff = false;
    }
    else
    {
        ix.canFineTone = false;
    }

    //-----------------------------------Motor Heating------------------------------
    ret = libqhyccd->IsQHYCCDControlAvailable(camhandle,CAM_SHUTTERMOTORHEATING_INTERFACE);
    if(ret == QHYCCD_SUCCESS)
    {
        ix.canMotorHeating = true;
        ix.motorHeatingOnOff = false;
    }
    else
    {
        ix.canMotorHeating = false;
    }

    //--------------------------TEC Over Protect---------------------------
    ret = libqhyccd->IsQHYCCDControlAvailable(camhandle,CAM_TECOVERPROTECT_INTERFACE);
    if(ret == QHYCCD_SUCCESS)
    {
        ix.canTecOverProtect = true;
        ix.tecPretect = false;
    }
    else
    {
        ix.canTecOverProtect = false;
    }

    //-----------------------------------Clamp-----------------------------
    ret = libqhyccd->IsQHYCCDControlAvailable(camhandle,CAM_SINGNALCLAMP_INTERFACE);
    if(ret == QHYCCD_SUCCESS)
    {
        ix.canSignalClamp = true;
        ix.clamp = false;
    }
    else
    {
        ix.canSignalClamp = false;
    }

    //--------------------------------Calibrate FPN---------------------------
    ret = libqhyccd->IsQHYCCDControlAvailable(camhandle,CAM_CALIBRATEFPN_INTERFACE);
    if(ret == QHYCCD_SUCCESS)
    {
        ix.canCalibrateFPN = true;
        ix.calibrateFPNOnOff = false;
    }
    else
    {
        ix.canCalibrateFPN = false;
    }

    //----------------------Slowest Download--------------------------
    ret = libqhyccd->IsQHYCCDControlAvailable(camhandle,CAM_USBREADOUTSLOWEST_INTERFACE);
    if(ret == QHYCCD_SUCCESS)
    {
        ix.canSlowestDownload = true;
        ix.slowestDowload = false;
    }
    else
    {
        ix.canSlowestDownload = false;
    }

    //---------------------------Chip temperature--------------------------
    ret = libqhyccd->IsQHYCCDControlAvailable(camhandle,CAM_CHIPTEMPERATURESENSOR_INTERFACE);
    if(ret == QHYCCD_SUCCESS)
    {
        ix.canChipTemp = true;
        ix.chipTempOnOff = false;
    }
    else
    {
        ix.canChipTemp = false;
    }

    //-------------------------Trigger-------------------------
    ret = libqhyccd->IsQHYCCDControlAvailable(camhandle,CAM_TRIGER_INTERFACE);
    if(ret == QHYCCD_SUCCESS)
    {
        ix.canTriger = true;
        ix.trigerMode = 0;
    }
    else
    {
        ix.canTriger = false;
    }

    //------------------------CFW--------------------------
    ret = libqhyccd->IsQHYCCDControlAvailable(camhandle,CONTROL_CFWPORT);
    if(ret == QHYCCD_SUCCESS)
    {
        ix.canFilterWheel = true;

        ret = libqhyccd->IsQHYCCDCFWPlugged(camhandle);
        if(ret == QHYCCD_SUCCESS)
        {
            ix.CFW_Plugged = true;
            ix.CFWSlotsNum = libqhyccd->GetQHYCCDParam(camhandle, CONTROL_CFWSLOTSNUM);
            if(ix.CFWSlotsNum < 2)
            {
                OutputDebug("EZCAPWARNING | %s | %s | GetQHYCCDParam() CONTROL_CFWSLOTSNUM Failed!", __FILE__, __FUNCTION__);
                ix.CFWSlotsNum = 9;
            }
        }
        else
        {
            ix.CFW_Plugged = false;
            ix.CFWSlotsNum = 9;
        }

        QString str;
        for(int i = 0; i < ix.CFWSlotsNum; i++)
        {
            str = QString("Pos %1").arg(i+1);
            ix.filterNames_2.append(str);
        }
    }
    else
    {
         ix.canFilterWheel = false;
    }

    //---------------------20201127 lyl SensorChamberCyclePUMP------------------------------
    ret = libqhyccd->IsQHYCCDControlAvailable(camhandle,CONTROL_SensorChamberCycle_PUMP);
    if(ret == QHYCCD_SUCCESS)
    {
        ix.canContolSensorChamberCyclePUMP = true;
    }
    else
    {
        ix.canContolSensorChamberCyclePUMP = false;
    }

    int len = 0;
    len = libqhyccd->GetQHYCCDMemLength(camhandle);
    if(len <= 0)
    {
        QMessageBox::critical(this,tr("Warning"),tr("Can not get max frame length,Please contact the developer!"),QMessageBox::Ok);
        return false;
    }

    if(ix.ImgData)
    {
        delete ix.ImgData;
        ix.ImgData = NULL;
    }
    if(ix.ImgData_Last)
    {
        delete ix.ImgData_Last;
        ix.ImgData_Last = NULL;
    }
    if(ix.ImgData_GPS)
    {
        delete ix.ImgData_GPS;
        ix.ImgData_GPS = NULL;
    }
    if(ix.ImgData_FB)
    {
        delete ix.ImgData_FB;
        ix.ImgData_FB = NULL;
    }
    if(ix.ImgData_Save)
    {
        delete ix.ImgData_Save;
        ix.ImgData_Save = NULL;
    }
    if(ix.ImgData_Dark)
    {
        delete ix.ImgData_Dark;
        ix.ImgData_Dark = NULL;
    }

    if(ix.ImgData_CalSave)
    {
        delete ix.ImgData_CalSave;
        ix.ImgData_CalSave = NULL;
    }

    QList<QScreen *> listScreen =  QGuiApplication::screens();  //多显示器
    for(int i = 0; i < listScreen.count(); i ++)
    {
        QRect rectScreen = listScreen.at(0)->geometry();
        if(rectScreen.width()  > ix.maxScreenW) ix.maxScreenW = rectScreen.width();
        if(rectScreen.height() > ix.maxScreenH) ix.maxScreenH = rectScreen.height();
    }
    ix.ImgData         = new unsigned char[ix.ImageW_Max * ix.ImageH_Max * 3];//分配内存
    ix.ImgData_Last    = new unsigned char[ix.ImageW_Max * ix.ImageH_Max * 2 * 3/*ix.maxScreenW * ix.maxScreenH * 4 * 3*/];//分配内存 4:0.25X 3:channels=3
    ix.ImgData_GPS     = new unsigned char[1024];
    ix.ImgData_Save    = new unsigned char[(ix.ImageW_Max+3)/4*4 * ix.ImageH_Max * 3];
    ix.ImgData_Dark    = new unsigned char[(ix.ImageW_Max+3)/4*4 * ix.ImageH_Max * 3];
    ix.ImgData_CalSave = new uint32_t[(ix.ImageW_Max+3)/4*4 * ix.ImageH_Max * 3];
    ix.ImgData_FB      = new uint16_t[(ix.ImageW_Max+3)/4*4 * ix.ImageH_Max * 3];

    if(qImg_show)
    {
        delete qImg_show;
        qImg_show = NULL;
    }

    return true;
}

void EZCAP::camera_connected()
{
    updateWindowsTitle();
    if(cameraChooser->isVisible())
        cameraChooser->close();

    ix.cameraState = Camera_Idle;
    ix.imageReady = GetSingleFrame_Waiting;
    ix.ForceStop = false;
    ix.onLiveMode = false;
    ix.plannerState = PlannerStatus_Done;

    if(ix.Cooler_Fun)
    {
        threadTempControl->start();
    }
}

void EZCAP::camera_disconnected()
{
    resetFrameCount();

    if(planner_dialog->isVisible())
        planner_dialog->close();

    if(cameraChooser->isVisible())
        cameraChooser->close();

    if(ix.Cooler_Fun)
    {
        if(threadTempControl->isRunning()) threadTempControl->stop();
    }

    //20200323lyl断开连接preview，focus,capture 功能关闭
    managerMenu->ui->head_preview->setCheckable(false);
    managerMenu->ui->head_focus->setCheckable(false);
    managerMenu->ui->head_capture->setCheckable(false);
    managerMenu->ui->head_save->setCheckable(false);
    managerMenu->ui->head_liveimageformat->setCheckable(false);
    managerMenu->ui->head_livecamerasetup->setCheckable(false);
    managerMenu->ui->head_liveimagesetup->setCheckable(false);
    managerMenu->ui->head_Roi->setCheckable(false);
    managerMenu->ui->head_screenView->setCheckable(false);
    managerMenu->ui->head_hist->setCheckable(false);
    mainMenuBar->menuPlanner->setEnabled(false);
    mainMenuBar->actCFWControl->setEnabled(false);
    mainMenuBar->menuZoom->setEnabled(false);
    mainMenuBar->actSaveBMP->setEnabled(false);
    mainMenuBar->actOpenFolder->setEnabled(false);
    mainMenuBar->actSaveFIT->setEnabled(false);
    mainMenuBar->actSaveJPG->setEnabled(false);
    mainMenuBar->actSavePNG->setEnabled(false);
    mainMenuBar->actSaveTIF->setEnabled(false);
    mainMenuBar->actCalibrateOverScan->setEnabled(false);
    mainMenuBar->actIgnoreOverScanArea->setEnabled(false);
    mainMenuBar->actOpenVideo->setEnabled(true);
    mainMenuBar->actOpenFolder->setEnabled(false);
    mainMenuBar->actFrameToolCapCal->setEnabled(false);
    mainMenuBar->actFrameToolCal->setEnabled(false);
    mainMenuBar->actCorrectCenter->setEnabled(false);
    mainMenuBar->actFavorite->setEnabled(false);
    mainWidget->updateWindowsTitle();
}

//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
void EZCAP::showFPS()
{
    fpsMutex.lock();

    ix.fps = static_cast<double>(ix.frameCount) / static_cast<double>(ix.timeCount) * 1000.0;
    this->statusLabel_msg->setText("FPS : " + QString::number(ix.fps, 'f', 1));

    fpsMutex.unlock();
}

void EZCAP::showFrameCount()
{
    mainWidget->statusLabel_frame_status->setText("Count : " + QString::number(ix.frame));
}

void EZCAP::resetFrameCount()
{
    ix.frame = 0;
    ix.frameLast = 0;
    ix.frameCount = 0;
    showFrameCount();
}

void EZCAP::displayLiveImage()
{
    showMutex.lock();

    if(ImgShow.empty()) { showMutex.unlock(); return; }

    qImg_show = MatToQImage(ImgShow);

    if(qImg_show)
    {
        ui->label_ImgShow->setPixmap(QPixmap::fromImage(*qImg_show));

        QString str1 = QString::number(ix.FrameW_Last) + "x" + QString::number(ix.FrameH_Last);
        statusLabel_imgSize->setText(str1);

        if(updateHoverLabelPosFromCursor())
        {
            updateHoverPixelStatus(hoverLabelPos, ImgShow);
            updatePixelMagnifier(hoverLabelPos);
        }
    }

    ImgShow.release();

    showMutex.unlock();
}

/*********************************************************************************/
/************************* Save Image Data For Live Mode *************************/
/*********************************************************************************/
void EZCAP::saveVideo()
{
    if(managerMenu->ui->btnStartSaveCap->text() == "Start Capture")
    {
        managerMenu->ui->btnStartSaveCap->setText("Stop Capture");

        if(ix.video.isOpened())
        {
            ix.video.release();
        }

        QString dirPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "/EZCAP Captures";
        QDir dir(dirPath);
        if(!dir.exists())
        {
            QDir *folder = new QDir();
            folder->mkdir(dirPath);
        }
        dirPath += "/Videos";
        QDir dir1(dirPath);
        if(!dir1.exists())
        {
            QDir *folder = new QDir();
            folder->mkdir(dirPath);
        }
        QString fileName = dirPath + "/" + ix.CamModel + "-" +
                QDateTime::currentDateTime().toString("yyyyMMddhhmmss") + "." +
                managerMenu->ui->comBoxCapFormat->currentText().toLower();

        DBGOPT_INFO("fileName = %s", qPrintable(fileName));

        bool isColor = ix.FrameC_Last==3 ? true : false;
        double fps   = ix.fps;
        int FrameW   = static_cast<int>(ix.FrameW_Last);
        int frameH   = static_cast<int>(ix.FrameH_Last);
        bool ok = false;
#ifdef Q_OS_MAC
        if(fps < 1.0) fps = 1.0;
        ok = ix.video.open(fileName.toStdString().c_str(), cv::VideoWriter::fourcc('a', 'v', 'c', '1'), fps, Size(FrameW, frameH), isColor);
#else
        ok = ix.video.open(fileName.toStdString().c_str(), cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), fps, Size(FrameW, frameH), isColor);
#endif
        if(!ok)
        {
            ix.saveFlag = false;
            QMessageBox::critical(this, tr("Error"), tr("Save video failed!"), QMessageBox::Ok);
            return;
        }

        disconnect(threadProcessImage, SIGNAL(gotSaveData()), this, SLOT(saveSnapShot2Image()));
        disconnect(threadProcessImage, SIGNAL(gotSaveData()), this, SLOT(saveCal2Image()));
        connect(   threadProcessImage, SIGNAL(gotSaveData()), this, SLOT(saveImage2Video()));

        managerMenu->ui->btnSnapShot->setEnabled(false);
        managerMenu->ui->comBoxSnapshotFormat->setEnabled(false);
        managerMenu->ui->comBoxCapFormat->setEnabled(false);

        ix.saveFlag = true;
        lastSavedPath = dirPath;
    }
    else
    {
        disconnect(threadProcessImage, SIGNAL(gotSaveData()), this, SLOT(saveImage2Video()));

        ix.saveFlag = false;
        ix.video.release();

        managerMenu->ui->btnStartSaveCap->setText("Start Capture");
        managerMenu->ui->btnSnapShot->setEnabled(true);
        managerMenu->ui->comBoxSnapshotFormat->setEnabled(true);
        managerMenu->ui->comBoxCapFormat->setEnabled(true);
    }
}

void EZCAP::saveImage2Video()
{
    if(ix.video.isOpened())
    {
        int type = ix.FrameC_Save == 1 ? CV_8UC1 : CV_8UC3;
        Mat image(Size(static_cast<int>(ix.FrameW_Save), static_cast<int>(ix.FrameH_Save)), type);

        if(ix.FrameB_Save == 16)
        {
            for(int i = 0; i < static_cast<int>(ix.FrameW_Save*ix.FrameH_Save*ix.FrameC_Save); i++)
            {
                image.data[i] = ix.ImgData_Save[2*i+1];
            }
        }
        else
        {
            for(int i = 0; i < static_cast<int>(ix.FrameW_Save*ix.FrameH_Save*ix.FrameC_Save); i++)
            {
                image.data[i] = ix.ImgData_Save[i];
            }
        }

        ix.video.write(image);
        image.release();

        ix.saveFlag = true;
    }
}

void EZCAP::saveSnapShot()
{
    disconnect(threadProcessImage, SIGNAL(gotSaveData()), this, SLOT(saveImage2Video()));
    disconnect(threadProcessImage, SIGNAL(gotSaveData()), this, SLOT(saveCal2Image()));
    connect(   threadProcessImage, SIGNAL(gotSaveData()), this, SLOT(saveSnapShot2Image()));

    managerMenu->ui->btnSnapShot->setEnabled(false);
    managerMenu->ui->comBoxSnapshotFormat->setEnabled(false);
    managerMenu->ui->btnStartSaveCap->setEnabled(false);
    managerMenu->ui->comBoxCapFormat->setEnabled(false);

    ix.saveFlag = true;
}

void EZCAP::saveSnapShot2Image()
{
    QString savePath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "/EZCAP Captures";
    QDir saveDir1(savePath);
    if(!saveDir1.exists())
    {
        QDir *folder = new QDir();
        folder->mkdir(savePath);
    }
    savePath += "/Images";
    QDir saveDir2(savePath);
    if(!saveDir2.exists())
    {
        QDir *folder = new QDir();
        folder->mkdir(savePath);
    }
    lastSavedPath = savePath;

    QString fileName = savePath + "/" + ix.CamModel + "-" +
            QDateTime::currentDateTime().toString("yyyyMMddhhmmss") + "." +
            managerMenu->ui->comBoxSnapshotFormat->currentText().toLower();

    if(managerMenu->ui->comBoxSnapshotFormat->currentText() == "FIT")
    {
        fitHeader_dialog->FITwrite_Live(fileName, ix.FrameW_Save, ix.FrameH_Save, ix.FrameB_Save, ix.FrameC_Save, ix.ImgData_Save);
    }
    else if(managerMenu->ui->comBoxSnapshotFormat->currentText() == "TIFF")
    {
        int type;
        if(ix.FrameB_Save == 8  && ix.FrameC_Save == 1)      type = CV_8UC1;
        else if(ix.FrameB_Save == 8  && ix.FrameC_Save == 3) type = CV_8UC3;
        else if(ix.FrameB_Save == 16 && ix.FrameC_Save == 1) type = CV_16UC1;
        else if(ix.FrameB_Save == 16 && ix.FrameC_Save == 3) type = CV_16UC3;

        Mat matSave(Size(ix.FrameW_Save, ix.FrameH_Save), type);
        matSave.data = ix.ImgData_Save;
        std::vector<int> compression_params;
        compression_params.push_back(IMWRITE_TIFF_COMPRESSION);  //选择jpeg
        compression_params.push_back(1); //在这个填入你要的图片质量
        imwrite(fileName.toStdString().c_str(), matSave, compression_params);
        matSave.release();
    }
    else
    {
        int type = ix.FrameC_Save == 1 ? CV_8UC1 : CV_8UC3;
        Mat matSave(Size(ix.FrameW_Save, ix.FrameH_Save), type);
        if(ix.FrameB_Save == 16)
            for (int i = 0; i < ix.FrameW_Save*ix.FrameH_Save*ix.FrameC_Save; i++)
                matSave.data[i] = ix.ImgData_Save[2*i+1];
        else
            matSave.data = ix.ImgData_Save;

        std::vector<int> compression_params;
        if(managerMenu->ui->comBoxSnapshotFormat->currentText() == "JPG")
        {
            compression_params.push_back(IMWRITE_JPEG_QUALITY);  //选择jpg
            compression_params.push_back(100); //在这个填入你要的图片质量
            imwrite(fileName.toStdString().c_str(), matSave, compression_params);
        }
        else if(managerMenu->ui->comBoxSnapshotFormat->currentText() == "PNG")
        {
            compression_params.push_back(IMWRITE_PNG_COMPRESSION);  //选择png
            compression_params.push_back(0); //在这个填入你要的图片质量
            imwrite(fileName.toStdString().c_str(), matSave, compression_params);
        }
        else if(managerMenu->ui->comBoxSnapshotFormat->currentText() == "BMP")
        {
            imwrite(fileName.toStdString().c_str(), matSave);
        }
        matSave.release();
    }

    disconnect(threadProcessImage, SIGNAL(gotSaveData()), this, SLOT(saveSnapShot2Image()));
    ix.saveFlag = false;

    managerMenu->ui->btnSnapShot->setEnabled(true);
    managerMenu->ui->comBoxSnapshotFormat->setEnabled(true);
    managerMenu->ui->btnStartSaveCap->setEnabled(true);
    managerMenu->ui->comBoxCapFormat->setEnabled(true);
}

void EZCAP::saveCal()
{
    disconnect(threadProcessImage, SIGNAL(gotSaveData()), this, SLOT(saveImage2Video()));
    disconnect(threadProcessImage, SIGNAL(gotSaveData()), this, SLOT(saveSnapShot2Image()));
    connect(   threadProcessImage, SIGNAL(gotSaveData()), this, SLOT(saveCal2Image()));
}

void EZCAP::adjustScrollBar(uint32_t w, uint32_t h)
{
//    if(ix.camStreamMode == 0) return;

    if((double)w/*ix.RoiW_Last*/ * ix.scaleFactor <= ix.showLabelW || ix.zoomMode == Zoom_FitWindow || ix.zoomMode == Zoom_FillWindow)
    {
        ui->horizontalScrollBar_ImgShow->setMinimum(0);
        ui->horizontalScrollBar_ImgShow->setMaximum(0);
        ui->horizontalScrollBar_ImgShow->setVisible(false);
    }
    else
    {
        ui->horizontalScrollBar_ImgShow->setMaximum((double)w/*ix.RoiW_Last*/ * ix.scaleFactor - ix.showLabelW);
        ui->horizontalScrollBar_ImgShow->setPageStep(ix.showLabelW);
        ui->horizontalScrollBar_ImgShow->setValue(ix.showLabelX);
        ui->horizontalScrollBar_ImgShow->setVisible(true);
    }
    if((double)h/*ix.RoiH_Last*/ * ix.scaleFactor <= ix.showLabelH || ix.zoomMode == Zoom_FitWindow || ix.zoomMode == Zoom_FillWindow)
    {
        ui->verticalScrollBar_ImgShow->setMinimum(0);
        ui->verticalScrollBar_ImgShow->setMaximum(0);
        ui->verticalScrollBar_ImgShow->setVisible(false);
    }
    else
    {
        ui->verticalScrollBar_ImgShow->setMaximum((double)h/*ix.RoiH_Last*/ * ix.scaleFactor - ix.showLabelH);
        ui->verticalScrollBar_ImgShow->setPageStep(ix.showLabelH);
        ui->verticalScrollBar_ImgShow->setValue(ix.showLabelY);
        ui->verticalScrollBar_ImgShow->setVisible(true);
    }
}

void EZCAP::saveCal2Image()
{
    if(ix.addedNum < ix.calNum)
    {
        for(int i = 0; i < ix.FrameW_Save*ix.FrameH_Save*ix.FrameB_Save*ix.FrameC_Save/8; i++)
        {
            ix.ImgData_CalSave[i] = ix.ImgData_CalSave[i] + ix.ImgData_Save[i];
        }
        ix.addedNum ++;
        ix.saveFlag = true;
    }
    else
    {
        disconnect(threadProcessImage, SIGNAL(gotSaveData()), this, SLOT(saveCal2Image()));
        ix.saveFlag = false;

        unsigned char *calImg = (unsigned char *)malloc(ix.FrameW_Save*ix.FrameH_Save*ix.FrameB_Save*ix.FrameC_Save/8);
        int i;
        for(i = 0; i < ix.FrameW_Save*ix.FrameH_Save*ix.FrameB_Save*ix.FrameC_Save/8; i++)
        {
            calImg[i] = static_cast<unsigned char>(ix.ImgData_CalSave[i] / ix.calNum);
        }

        QString dirPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "/EZCAP Captures";
        QDir dir(dirPath);
        if(!dir.exists())
        {
            QDir *folder = new QDir();
            folder->mkdir(dirPath);
        }

        if(frameToolCapCal_dialog->capCalFlag == 0)
            dirPath += "/Dark";
        else if(frameToolCapCal_dialog->capCalFlag == 1)
            dirPath += "/Flat";
        else if(frameToolCapCal_dialog->capCalFlag == 2)
            dirPath += "/Bias";

        QDir dir1(dirPath);
        if(!dir1.exists())
        {
            QDir *folder = new QDir();
            folder->mkdir(dirPath);
        }

        frameToolCapCal_dialog->savePath = dirPath;

        QString fileName = dirPath + "/" + ix.CamModel + "_" +
                managerMenu->ui->comBoxLiveBits->currentText() + "_" +
                QString::number(ix.FrameW_Save) + "x" + QString::number(ix.FrameH_Save) +
                "_Exp" + QString::number(ix.ExpTime) +
                "_Gain" + QString::number(ix.Gain) +
                "_Offset" + QString::number(ix.Offset) +
                "_Temp" + QString::number((int)ix.Temp_Now) + "C"
                "_Repeat" + QString::number(ix.calNum) +
                ".fit";

        fitHeader_dialog->FITwrite_Live(fileName, ix.FrameW_Save, ix.FrameH_Save, ix.FrameB_Save, ix.FrameC_Save, calImg);

        ix.calNum = 0;
        ix.addedNum = 0;
        frameToolCapCal_dialog->setEnableUI();

        free(calImg);
    }
}

/*******************************************************************/
/********************* Open and show local files *******************/
/*******************************************************************/
void EZCAP::openVideo()
{
    if(mainMenuBar->actOpenVideo->text() == "Open Video")
    {
        QString filePath, loadPath = QApplication::applicationDirPath();
        QDir dir(loadPath + "/Video");

        if(dir.exists())
        {
            loadPath = dir.path();
        }

        filePath = QFileDialog::getOpenFileName(this, tr("Open"), loadPath, tr("AVI Files(*.avi)"));
        if(filePath.isEmpty() || !filePath.contains(".avi"))
            return;

        if(videoShowThread->isRunning())
        {
            ix.canReadVideo = false;
            videoShowThread->closeThread();
            videoShowThread = new VideoShowThread();
        }

        ix.canClose = false;
        ix.canReadVideo = true;
        videoShowThread->fileName = filePath;
        connect(videoShowThread, SIGNAL(gotVideoFrame()), this, SLOT(displayVideoImage()));
        connect(videoShowThread, SIGNAL(finished()), videoShowThread, SLOT(deleteLater()));
        videoShowThread->start();

        mainMenuBar->actOpenVideo->setText("Close Video");
        mainMenuBar->menuCamera->setEnabled(false);
        mainMenuBar->menuImageProcess->setEnabled(false);
        mainMenuBar->menuCameraSetup->setEnabled(false);
        mainMenuBar->menuTools->setEnabled(false);
        mainMenuBar->menuZoom->setEnabled(true);
    }
    else if(mainMenuBar->actOpenVideo->text() == "Close Video")
    {
        if(videoShowThread->isRunning())
        {
            ix.canReadVideo = false;
            videoShowThread->closeThread();
            videoShowThread = new VideoShowThread();
        }

        mainMenuBar->actOpenVideo->setText("Open Video");
        mainMenuBar->menuCamera->setEnabled(true);
        mainMenuBar->menuImageProcess->setEnabled(true);
        mainMenuBar->menuCameraSetup->setEnabled(true);
        mainMenuBar->menuTools->setEnabled(true);
        mainMenuBar->menuZoom->setEnabled(false);
    }
    else
    {
        OutputDebug("EZCAPERROR | %s | %s | actOpenVideo Text Error!", __FILE__, __FUNCTION__);
    }
}

void EZCAP::displayVideoImage()
{
    if(ix.videoImage.depth() == CV_8U)
    {
        qImg_video = this->MatToQImage(ix.videoImage);
    }
    else if(ix.videoImage.depth() == CV_16U)
    {
        int type = ix.videoImage.channels() == 1 ? CV_8UC1 : CV_8UC3;
        Mat image(Size(ix.videoImage.cols, ix.videoImage.rows), type);
        qImg_video = this->MatToQImage(image);
        image.release();
    }
    else
    {
        ix.canReadVideo = true;
        return;
    }

    if(ix.videoImage.cols == 0 || ix.videoImage.rows == 0)
    {
        ix.canReadVideo = true;
        return;
    }

    if(ix.videoImage.channels() != 1 && ix.videoImage.channels() != 3)
    {
        ix.canReadVideo = true;
        return;
    }

    if(qImg_video)
    {
        if(ix.zoomMode == Zoom_FitWindow || ix.zoomMode == Zoom_FillWindow)
        {
            scrollArea_ImgShow->setWidgetResizable(true);
            ui->label_ImgShow->setPixmap(QPixmap::fromImage(*qImg_video));
        }
        else
        {
            scrollArea_ImgShow->setWidgetResizable(false);
            ui->label_ImgShow->setPixmap(QPixmap::fromImage(*qImg_video));
            ui->label_ImgShow->resize(ix.videoImage.cols*ix.scaleFactor, ix.videoImage.rows*ix.scaleFactor);
        }
    }

    ix.canReadVideo = true;
}

//------------------------------------------------------------------------------------
//------------------translate language------------------------------------------------

void EZCAP::languageChanged()
{
    bool isLoaded = false;

    if(translator)
    {
        qApp->removeTranslator(translator);
        delete translator;
        translator = NULL;
    }
    translator = new QTranslator(this);

    if(ix.lang.compare("EN") == 0)
        isLoaded = translator->load(":/language/lan_en_us.qm");
    else if(ix.lang.compare("CN") == 0)
        isLoaded = translator->load(":/language/lan_zh_cn.qm");
    else if(ix.lang.compare("JP") == 0)
        isLoaded = translator->load(":/language/lan_ja_jp.qm");
    else
        isLoaded = translator->load(":/language/lan_en_us.qm");

    if(isLoaded)
    {
        qApp->installTranslator(translator);
        ui->retranslateUi(this);
        emit changeLanguage();//emit the signal to retranslate the ui of each dialog
    }
    else
    {
        qCritical() << "translator loaded failure, can not change language";
    }
}

void EZCAP::changeToEnglish()
{
    ix.lang = "EN";
    iniFileParams.lang = "EN";

    languageChanged();
}

void EZCAP::changeToChinese()
{
    ix.lang = "CN";
    iniFileParams.lang = "CN";

    languageChanged();
}

void EZCAP::changeToJapanese()
{
    ix.lang = "JP";
    iniFileParams.lang = "JP";

    languageChanged();
}

void EZCAP::switchDebug()
{
    libqhyccd->EnableQHYCCDMessage(mainMenuBar->actDebug->isChecked());
    ui->plainTextEdit_debug->setVisible(mainMenuBar->actDebug->isChecked());
    ui->plainTextEdit_debug->appendPlainText("debug changed ");
}
void EZCAP::switchTestGuid()
{
    iniFileParams.testGuider = mainMenuBar->actTestGuid->isChecked();
    saveParamToIni("SoftSetting", "TestGuider", iniFileParams.testGuider);

    for (int guidCount = 0; guidCount < 11; ++guidCount)
    {
        libqhyccd->ControlQHYCCDGuide(camhandle, guidCount%4, 250);
        QThread::msleep(200);
    }
}
void EZCAP::switchTestPumpV2(bool checked)
{
    libqhyccd->SetQHYCCDParam(camhandle, CONTROL_OUTSIDE_PUMP_V2, checked? 1.0: 0.0);
}
void EZCAP::switchTestPumpV2_second(bool checked)
{
    libqhyccd->SetQHYCCDParam(camhandle, CONTROL_OUTSIDE_PUMP_V2, checked? 3.0: 2.0);
}
void EZCAP::switchTestPumpV2_cycle(bool checked)
{
    if(checked)
    {
        PumpV2CycleTimer->start();
        libqhyccd->SetQHYCCDParam(camhandle, CONTROL_OUTSIDE_PUMP_V2, 1.0);
    }
    else
    {
        PumpV2CycleTimer->stop();
        libqhyccd->SetQHYCCDParam(camhandle, CONTROL_OUTSIDE_PUMP_V2, 0.0);
    }
}
void EZCAP::switchTestPumpV2_cycle_second(bool checked)
{
    if(checked)
    {
        PumpV2CycleSecondTimer->start();
        libqhyccd->SetQHYCCDParam(camhandle, CONTROL_OUTSIDE_PUMP_V2, 3.0);
    }
    else
    {
        PumpV2CycleSecondTimer->stop();
        libqhyccd->SetQHYCCDParam(camhandle, CONTROL_OUTSIDE_PUMP_V2, 2.0);
    }
}
void EZCAP::switchTestErrorLed(bool checked)
{
    libqhyccd->SetQHYCCDParam(camhandle, CONTROL_Error_Led, checked? 1.0: 0.0);
}
void EZCAP::switchTestIMG1(bool checked)
{
    qDebug("----------- switch img1 ----[%d]", checked);
    libqhyccd->SetQHYCCDWriteCMOS(camhandle, 0, 0xc4, checked? 1: 0);
}
void EZCAP::switchTestIMG3(bool checked)
{
    qDebug("----------- switch img3 ----[%d]", checked);
    libqhyccd->SetQHYCCDWriteCMOS(camhandle, 0, 0xc4, checked? 3: 0);
}
void EZCAP::activeTestMode()
{
    iniFileParams.autoConnectLive = mainMenuBar->actTestMode->isChecked();
}

//*******************************************************************
//        菜单栏操作
//*******************************************************************
/**
 * @brief EZCAP::saveParasAsIni
 */
void EZCAP::saveParasAsIni()
{
    //Qt中使用QSettings类读写ini文件
    //QSettings构造函数的第一个参数是ini文件的路径,第二个参数表示针对ini文件,第三个参数可以缺省
    QString path_ezcap = QCoreApplication::applicationDirPath() + "/" + "EZCAP.ini";
    path_ezcap = QDir::toNativeSeparators(path_ezcap);
    QSettings *iniWrite = new QSettings(path_ezcap, QSettings::IniFormat);

    iniWrite->beginGroup("SoftSetting");
    iniWrite->setValue("Language",           ix.lang);//iniFileParams.lang);
    iniWrite->setValue("AutoStretchMode",    ix.autoStretchMode);
    iniWrite->setValue("IgnoreOverScanArea", ix.IgnoreOverscan);
    iniWrite->setValue("OverScanCalibrate",  ix.CalibrateOverscan);
    iniWrite->setValue("CalConstant",        ix.calConstant);
    iniWrite->setValue("SaveTHPFile",        ix.saveTHPFile);
    iniWrite->setValue("Preview_BPos",       Preview_BPOS);
    iniWrite->setValue("Preview_WPos",       Preview_WPOS);
    iniWrite->setValue("Focus_BPos",         Focus_BPOS);
    iniWrite->setValue("Focus_WPos",         Focus_WPOS);
    iniWrite->setValue("Capture_BPos",       Capture_BPOS);
    iniWrite->setValue("Capture_WPos",       Capture_WPOS);
    iniWrite->setValue("Live_WPOS",          Live_WPOS);
    iniWrite->setValue("Live_BPOS",          Live_BPOS);
    iniWrite->endGroup();

    if(ix.CamModel != "")
    {
        iniWrite->beginGroup(ix.CamModel + "-" + QString::number(ix.camStreamMode));

        iniWrite->setValue("ReadMode",              ix.ReadMode);

        iniWrite->setValue("Bits",                  ix.Bits);

        iniWrite->setValue("Color",            ix.Color);

        iniWrite->setValue("ConvertColor",          ix.IsCvtColor);

        iniWrite->setValue("ExpUnit",               ix.ExpUnit);
        iniWrite->setValue("ExpTime",               ix.ExpTime);

        iniWrite->setValue("Gain",                  ix.Gain);

        iniWrite->setValue("Offset",                ix.Offset);

        iniWrite->setValue("Speed",         ix.Speed);

        iniWrite->setValue("Traffic",            ix.Traffic);

        if(ix.CamID.contains("miniCam8"))
            iniWrite->setValue("CFWCalculateValue", ix.CFWCalculateValue);

        if(ix.camStreamMode == 1)
        {
            iniWrite->setValue("Brightness",            ix.Brightness);

            iniWrite->setValue("Contrast",              ix.Contrast);

            iniWrite->setValue("Gamma",                 ix.Gamma);

            iniWrite->setValue("WBR",                   ix.WBR);

            iniWrite->setValue("WBG",                   ix.WBG);

            iniWrite->setValue("WBB",                   ix.WBB);

            iniWrite->setValue("DDR",              ix.DDR);

            iniWrite->setValue("AMPV",             ix.AMPV);

            iniWrite->setValue("Circle1",             ix.circle1);
            iniWrite->setValue("Circle2",             ix.circle2);
        }

        iniWrite->endGroup();
    }

    iniWrite->beginGroup("FilterNames");
    iniWrite->setValue("SlotsNum", iniFileParams.CFWSlotsNum);
    QString key;
    for(int i=0; i<iniFileParams.filterNames_2.count(); i++)
    {
        key = QString("Slots%1").arg(i+1);
        iniWrite->setValue(key, iniFileParams.filterNames_2.at(i));
    }

    iniWrite->endGroup();

    //写入完成后删除指针
    delete iniWrite;
}

/**
 * @brief EZCAP::loadParasFromIni
 */
bool EZCAP::loadParasFromIni()
{
    bool ret = false;

    //读取EZCAP.ini中的内容
    QString path_ezcap = QCoreApplication::applicationDirPath() + "/" + "EZCAP.ini";
    path_ezcap = QDir::toNativeSeparators(path_ezcap);

    QFile *settingFile = new QFile(path_ezcap);
    if(settingFile->exists())
    {
        QSettings *iniRead = new QSettings(path_ezcap, QSettings::IniFormat);

        iniRead->beginGroup("SoftSetting");
        ix.lang                  = iniRead->value("Language",               "EN").toString();
        ix.autoStretchMode       = iniRead->value("AutoStretchMode",           0).toInt();
        ix.IgnoreOverscan        = iniRead->value("IgnoreOverScanArea",    false).toBool();
        ix.CalibrateOverscan     = iniRead->value("OverScanCalibrate",     false).toBool();
        ix.calConstant           = iniRead->value("CalConstant",            1000).toInt();
        ix.saveTHPFile           = iniRead->value("SaveTHPFile",           false).toBool();
        Preview_BPOS             = iniRead->value("Preview_BPos",              0).toUInt();
        Preview_WPOS             = iniRead->value("Preview_WPos",          65535).toUInt();
        Focus_BPOS               = iniRead->value("Focus_BPos",                0).toUInt();
        Focus_WPOS               = iniRead->value("Focus_WPos",            65535).toUInt();
        Capture_BPOS             = iniRead->value("Capture_BPos",              0).toUInt();
        Capture_WPOS             = iniRead->value("Capture_WPos",          65535).toUInt();
        Live_BPOS                = iniRead->value("Live_BPOS",                 0).toUInt();
        Live_WPOS                = iniRead->value("Live_WPOS",             65535).toUInt();
        iniFileParams.fullScreen      = iniRead->value("fullScreen",        false).toBool();
        iniFileParams.enableMsgClient = iniRead->value("enableMsgClient",   false).toBool();
        iniFileParams.msgClientName   = iniRead->value("msgClientName", "NotName").toString();
        iniRead->endGroup();

        iniRead->beginGroup(ix.CamModel + "-" + QString::number(ix.camStreamMode));
        ix.ReadMode           = iniRead->value("ReadMode",                  0).toUInt();
        ix.Bits                  = iniRead->value("Bits",                     16).toUInt();

        ix.Color            = iniRead->value("Color",            false).toBool();
        ix.IsCvtColor            = iniRead->value("ConvertColor",          false).toBool();

        ix.ExpUnit               = iniRead->value("ExpUnit",              1000.0).toDouble();
        ix.ExpTime               = iniRead->value("ExpTime",                20.0).toDouble();

        ix.Gain                  = iniRead->value("Gain",                    0.0).toDouble();

        ix.Offset                = iniRead->value("Offset",                  0.0).toDouble();

        ix.Speed         = iniRead->value("Speed",           0.0).toDouble();

        ix.Traffic            = iniRead->value("Traffic",              0.0).toDouble();

        ix.CFWCalculateValue = iniRead->value("CFWCalculateValue", 40).toInt();

        if (ix.camStreamMode == 1)
        {
            ix.Brightness        = iniRead->value("Brightness",              0.0).toDouble();

            ix.Contrast          = iniRead->value("Contrast",                0.0).toDouble();

            ix.Gamma             = iniRead->value("Gamma",                   1.0).toDouble();

            ix.WBR               = iniRead->value("WBR",                    32.0).toDouble();

            ix.WBG               = iniRead->value("WBG",                    32.0).toDouble();

            ix.WBB               = iniRead->value("WBB",                      32.0).toDouble();

            ix.DDR          = iniRead->value("DDR",                    true).toBool();

            ix.AMPV         = iniRead->value("AMPV",                  false).toBool();

            ix.circle1       = iniRead->value("Circle1",                  5000).toInt();
            ix.circle2       = iniRead->value("Circle2",                  5000).toInt();
        }

        iniRead->endGroup();

        iniRead->beginGroup("FilterNames");
        iniFileParams.CFWSlotsNum = iniRead->value("SlotsNum", 0).toInt();
        QString key;
        QString defStr;
        for(int i=0; i<iniFileParams.CFWSlotsNum; i++)
        {
            key = QString("Slots%1").arg(i+1);
            defStr = QString("Pos %1").arg(i+1);
            iniFileParams.filterNames_2.append(iniRead->value(key, defStr).toString());

        }

        iniRead->endGroup();

        delete iniRead;

        ret = true;
    }

    delete settingFile;

    return ret;
}

void EZCAP::saveParamToIni(QString group, QString key, QString value)
{
    QString path_ezcap = QDir::toNativeSeparators(QCoreApplication::applicationDirPath() + "/EZCAP.ini");
    QSettings *iniWrite = new QSettings(path_ezcap, QSettings::IniFormat);

    iniWrite->beginGroup(group);
    iniWrite->setValue(key, value);
    iniWrite->endGroup();

    delete iniWrite;
}

void EZCAP::saveParamToIni(QString group, QString key, int value)
{
    saveParamToIni(group, key, QString::number(value));
}

void EZCAP::saveParamToIni(QString group, QString key, double value)
{
    saveParamToIni(group, key, QString::number(value, 'f', 1));
}

void EZCAP::saveParamToIni(QString group, QString key, bool value)
{
    saveParamToIni(group, key, QString::number(value));
}

void EZCAP::loadParamFromIni(QString group, QString key, QString *value, QString defaultValue)
{
    QString iniPath = QDir::toNativeSeparators(QCoreApplication::applicationDirPath() + "/EZCAP.ini");
    QFile *file = new QFile(iniPath);

    if(file->exists())
    {
        QSettings *iniRead = new QSettings(iniPath, QSettings::IniFormat);

        iniRead->beginGroup(group);
        *value = iniRead->value(key, defaultValue).toString();
        iniRead->endGroup();

        DBGOPT_INFO("loadParamFromIni(QString): group=%s, key=%s, value=%s",
                    qPrintable(group), qPrintable(key), qPrintable(*value));

        delete iniRead;
    }
    else
    {
        *value = defaultValue;
        DBGOPT_WARNING("loadParamFromIni(QString): INI file not found, using default. group=%s, key=%s, default=%s",
                       qPrintable(group), qPrintable(key), qPrintable(defaultValue));
    }

    delete file;
}

bool EZCAP::loadParamFromIni(QString group, QString key, int *value, int defaultValue)
{
    QString val = "";
    bool isOK = false;

    loadParamFromIni(group, key, &val, QString::number(defaultValue));
    *value = val.toInt(&isOK);

    return isOK;
}

bool EZCAP::loadParamFromIni(QString group, QString key, double *value, double defaultValue)
{
    QString val = "";
    bool isOK = false;

    loadParamFromIni(group, key, &val, QString::number(defaultValue, 'f', 1));
    *value = val.toInt(&isOK);

    return isOK;
}

bool EZCAP::loadParamFromIni(QString group, QString key, bool *value, bool defaultValue)
{
    QString val = "";
    bool isOK = false;

    loadParamFromIni(group, key, &val, QString::number(defaultValue));

    // 支持多种布尔值格式：true/false, 1/0, yes/no, on/off
    val = val.trimmed().toLower();
    if(val == "true" || val == "1" || val == "yes" || val == "on"){
        *value = true;
        isOK = true;
    } else if(val == "false" || val == "0" || val == "no" || val == "off"){
        *value = false;
        isOK = true;
    } else {
        // 如果都不匹配，尝试转换为整数
        *value = val.toInt(&isOK);
    }

    DBGOPT_INFO("loadParamFromIni(bool): group=%s, key=%s, val=%s, result=%d, isOK=%d",
                qPrintable(group), qPrintable(key), qPrintable(val), *value, isOK);

    return isOK;
}

bool EZCAP::SearchCamFromIni(QString camName)
{
    QString iniPath = QDir::toNativeSeparators(QCoreApplication::applicationDirPath() + "/EZCAP.ini");
    QFile *file = new QFile(iniPath);
    bool found = false;

    if(file->exists())
    {
        QSettings *iniRead = new QSettings(iniPath, QSettings::IniFormat);

        for(int i = 0; i < iniRead->childGroups().length(); i++)
        {
            if(camName == iniRead->childGroups().at(i))
            {
                found = true;
                break;
            }
        }

        return found;
    }
    else
    {
        return false;
    }
}

/**
 * @brief EZCAP::CheckWritePath
 * @param filename
 * @return
 */
bool EZCAP::CheckWritePath(QString filename)
{
    //尝试写一个文件，测试文件路径是否安全
    bool ret;
    QString TestName;
    TestName = filename + ".QHY";

    //判断文件是否存在
    if(QFile::exists(TestName))
    {
        QFile::remove(TestName);
    }

    fitHeader_dialog->FITwrite_Common(TestName,10,10,ix.ImgData);

    if(QFile::exists(TestName))
    {
        QFile::remove(TestName);
        ret = true;
    }
    else
    {
        ret = false;
    }

    return ret;
}

/**
 * @brief EZCAP::exitMainWindow
 */
void EZCAP::exitMainWindow()
{
    //点击exit菜单项,关闭主窗体
    this->close();
}

/**
 * @brief EZCAP::closeEvent
 * @param event
 */
void EZCAP::closeEvent( QCloseEvent * event )
{
    //（注意：正常关闭软件时，需要关闭相机，如果当前有拍摄任务，强制停止曝光后，立马CloseQHYCCD(关闭相机)会报错，
    //估计尚未从相机中读完所有图像数据，关闭相机导致句柄异常，
    //  若添加循环等待 则会导致界面卡死，故而暂时采用以下机制。）
    qDebug() << "------------------Close EZCAP------------------";

    int ret;
    bool acceptCloseEvent = true;

    if(ix.cameraState != Camera_Idle)
    {
        QMessageBox::StandardButton choiceBtn;
        choiceBtn = QMessageBox::question(NULL,tr("Close EZCAP"),
                                   tr("Warning: A task is running,Are you sure wanted to exit?"),
                                   QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if(choiceBtn == QMessageBox::Yes)
        {
            acceptCloseEvent = true;
        }
        else
        {
            acceptCloseEvent = false;
        }

    }

    if(acceptCloseEvent)
    {
        isSettleDone = true; //退出可能存在的Dither等待循环

        if(threadTempControl->isRunning()) threadTempControl->stop();

        if(ix.CFWStatus == CFW_Moving)
        {
            ix.CFWStatus = CFW_Idle;  //stop CFW timer
            stopCFWTimer();
        }
        if(ix.plannerState == PlannerStatus_Start)
        {
            ix.plannerState = PlannerStatus_Stop;
        }

        if(camhandle)
        {
            if(ix.camStreamMode == 0)
            {
                if(ix.cameraState != Camera_Idle)
                {
                    ret = libqhyccd->CancelQHYCCDExposingAndReadout(camhandle);
                    if(ret != QHYCCD_SUCCESS)
                    {
                        DBGOPT_WARNING("CancelQHYCCDExposingAndReadout() Failed!");
                    }
                }

                ix.ForceStop = true;  //stop exposing.
                ix.onLiveMode = false;  //stop the exist PreviewLive/FocusLive
                ix.cameraState = Camera_Idle;
            }
            else if(ix.camStreamMode == 1)
            {
                if(threadProcessImage->isRunning())
                    threadProcessImage->stop();
                if(liveCap->isRunning())
                    liveCap->closeThread();

                ret = libqhyccd->StopQHYCCDLive(camhandle);
                if(ret == QHYCCD_ERROR)
                {
                    OutputDebug("EZCAPWARNING | %s | %s | StopQHYCCDLive() Failed!", __FILE__, __FUNCTION__);
                }

                if(updateImgTimer->isActive())
                    updateImgTimer->stop(); //停止图像刷新定时器
                updateFrameTimer->stop();
            }
            else
            {
                OutputDebug("EZCAPERROR | %s | %s | camStreamMode Value Error!", __FILE__, __FUNCTION__);
            }

            ret = libqhyccd->CloseQHYCCD(camhandle);
            if(ret != QHYCCD_SUCCESS)
            {
                OutputDebug("EZCAPWARNING | %s | %s | CloseQHYCCD() Failed!", __FILE__, __FUNCTION__);
            }
        }
        ret = libqhyccd->ReleaseQHYCCDResource();
        if(ret != QHYCCD_SUCCESS)
        {
            OutputDebug("EZCAPWARNING | %s | %s | ReleaseQHYCCDResource() Failed!", __FILE__, __FUNCTION__);
        }

        saveParasAsIni();//保存界面设置参数

        //保存FitHeader列表信息
        QString path_csv = QApplication::applicationDirPath() + "/" + "FITHEADER.csv";
        path_csv = QDir::toNativeSeparators(path_csv);
        fitHeader_dialog->saveAsCSV(path_csv);

        //
        planner_dialog->close();
        fitHeader_dialog->close();
        about_dialog->close();
        favorite_dialog->close();
        tempControl_dialog->close();
        phdLink_dialog->close();
        cfwControl_dialog->close();
        imgAnalyze_dialog->close();

        // ui->widgetFocusAssistant->close();

        event->accept();
    }
    else
    {
        //取消关闭操作
        event->ignore();
    }
}

/**
 * @brief EZCAP::showCameraChooser
 */
void EZCAP::showCameraChooser()
{
    unsigned int ret = QHYCCD_ERROR;
    int num = 0;
    QString tempId = "";

    //判断是单帧模式还是连续模式
    QAction *action = qobject_cast<QAction *>(sender());
    if(action == mainMenuBar->actConnect)
    {
        ix.camStreamMode = 0;
    }
    else if(action == mainMenuBar->actCntLive)
    {
        ix.camStreamMode = 1;
    }
    else
    {
        OutputDebug("EZCAPERROR | %s | %s | Connection Action Error!", __FILE__, __FUNCTION__);
    }

    if(ix.isConnected)
    {
        bool acceptDisEvent = true;
        if((ix.cameraState != Camera_Idle || ix.plannerState != PlannerStatus_Done) && show_disconnect_confirm_box)
        {
            QMessageBox::StandardButton choiceBtn;
            choiceBtn = QMessageBox::question(NULL,tr("Disconnect Camera"),
                                              tr("Warning: A task is running,Are you sure you want to disconnect?"),
                                              QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
            if(choiceBtn == QMessageBox::Yes)
            {
                acceptDisEvent = true;
            }
            else
            {
                acceptDisEvent = false;
            }
        }

        if(acceptDisEvent)
        {
            //reset the camera params, so that it will be set aGain when re-connected.
            ix.BinX_Last                  = 0;
            ix.BinY_Last                  = 0;
            ix.Speed_Last         = 0.0;
            ix.ExpTime_Last               = 0.0;
            ix.Gain_Last                  = 0.0;
            ix.Offset_Last                = 0.0;
            ix.LastMechanicalShutterMode = 0;
            ix.workMode                  = 0;
            //20200302 lyl 大分辨率相机preview后disconnect，重连小分辨率异常问题解决。
            ix.lastWorkMode              = 0;
            ix.isConnected               = false;
            isSettleDone                 = true; //退出可能存在的Dither等待循环

            if(ix.CFWStatus == CFW_Moving)
            {
                stopCFWTimer();
                ix.CFWStatus = CFW_Idle;  //stop CFW timer
            }
            if(ix.plannerState == PlannerStatus_Start)
            {
                ix.plannerState = PlannerStatus_Stop;
            }

            //camera has connected
            if(camhandle)
            {
                if(ix.camStreamMode == 0)
                {
                    if(ix.cameraState != Camera_Idle)
                    {
//                        ret = libqhyccd->CancelQHYCCDExposingAndReadout(camhandle);
//                        OutputDebug("EZCAPDEBUG | %s | %s | CancelQ")
//                        if(ret == QHYCCD_ERROR)
//                        {
//                            OutputDebug("EZCAPWARNING | %s | %s | CancelQHYCCDExposingAndReadout() Failed!", __FILE__, __FUNCTION__);
//                        }

                        while(ix.imageReady == GetSingleFrame_Waiting)
                        {
                            QCoreApplication::processEvents();
                        }

                        ix.imageReady = GetSingleFrame_Failed;
                    }

                    ix.ForceStop = true;  //stop exposing.
                    ix.onLiveMode = false;  //stop the exist PreviewLive/FocusLive
                    ix.cameraState = Camera_Idle;
                }
                else if(ix.camStreamMode == 1)
                {
                    if(threadProcessImage->isRunning())
                    {
                        threadProcessImage->stop();
                    }

                    if(liveCap->isRunning())
                    {
                        liveCap->closeThread();//退出连续拍摄线程

                        ret = libqhyccd->StopQHYCCDLive(camhandle);
                        if(ret != QHYCCD_SUCCESS)
                        {
                            OutputDebug("EZCAPWARNING | %s | %s | StopQHYCCDLive() Failed!", __FILE__, __FUNCTION__);
                        }

                        if(updateImgTimer->isActive())
                            updateImgTimer->stop(); //停止图像刷新定时器
                        updateFrameTimer->stop();
                    }
                }
                else
                {
                    OutputDebug("EZCAPERROR | %s | %s | camStreamMode Value Error!", __FILE__, __FUNCTION__);
                }

                ret = libqhyccd->CloseQHYCCD(camhandle);
                if(ret != QHYCCD_SUCCESS)
                {
                    camhandle = NULL;
                }
                else
                {
                    camhandle = NULL;
                }
            }

            canConnect = false;
            saveParasAsIni();

//            //单帧连续模式切换时需要执行资源释放函数和资源初始化函数，避免个别相机出现内存溢出问题

            emit disconnect_camera();

            // ui->widgetFocusAssistant->close();
        }
    }
    else
    {
        devList.clear();
        num = libqhyccd->ScanQHYCCD();
        if(num >= 0)
        {
            for(int i = 0; i < num; i++)
            {
                ret = libqhyccd->GetQHYCCDId(i, camid);
                if(ret == QHYCCD_SUCCESS)
                {
                    OutputDebug("EZCAP | %s | %s | GetQHYCCDId() Success! | i = %d id = %s", __FILE__, __FUNCTION__, i, camid);

                    tempId = QString(QLatin1String(camid));
                    if(tempId.contains("POLEMASTER") == false)
                    {
                        devList.append(tempId);
                    }
                }
                else
                {
                    OutputDebug("EZCAPWARNING | %s | %s | GetQHYCCDId() Failed! | i = %d", __FILE__, __FUNCTION__, i);
                }
            }
        }
        else
        {
            OutputDebug("EZCAPWARNING | %s | %s | ScanQHYCCD() Failed!", __FILE__, __FUNCTION__);
        }

        if(devList.count() > 0)
        {
            if(devList.count() > 1)//test
            {
                cameraChooser->ui->coBox_cameraChooser->clear();
                cameraChooser->ui->coBox_cameraChooser->addItems(devList);
                cameraChooser->setWindowModality(Qt::ApplicationModal);//设置父窗口不可控
                cameraChooser->show();//show cameraChooser window//cameraChooser->ui->coBox_cameraChooser->focus
            }
            else
            {
                ix.CamID        = devList.at(0);
                ix.CamModel     = ix.CamID.left(ix.CamID.lastIndexOf('-'));

                //根据相机ID获得的相机型号，查看之前连接的相机记录里是否有相同型号，如果有则使用之前保留的参数，若没有则使用SDK默认参数进行设置
                if(SearchCamFromIni(ix.CamModel + "-" + QString::number(ix.camStreamMode)))
                {
                    ix.FoundCam = true;
                    this->loadParasFromIni();
                }

                QByteArray pstr = ix.CamID.toLatin1();
                memset(camid, '\0', 64);
                memcpy(camid, pstr.data(), pstr.size());

                camhandle = libqhyccd->OpenQHYCCD(camid);
                if(camhandle != NULL)
                {
                    //20200220 lyl Add ReadMode Dialog
                    ix.ReadMode_Num = 0;
                    ix.ReadMode_List.clear();//清空列表

                    if(libqhyccd->GetQHYCCDNumberOfReadModes)
                    {
                        ret = libqhyccd->GetQHYCCDNumberOfReadModes(camhandle,&ix.ReadMode_Num);

                        if(iniFileParams.testGuider)
                        {
                            for (int guidCount = 0; guidCount <= 11; ++guidCount)
                            {
                                libqhyccd->ControlQHYCCDGuide(camhandle, guidCount%4, 250);
                                QThread::msleep(200);
                            }
                        }

                        if(ix.ReadMode_Num > 1 && ret == QHYCCD_SUCCESS)//20200318排除只有一种readmode模式
                        {
                            for(int i = 0; i < ix.ReadMode_Num; i ++)
                            {
                                if (libqhyccd->GetQHYCCDReadModeName(camhandle, i, ix.ReadMode_Name) == 0)
                                {
                                    ix.ReadMode_List.append(ix.ReadMode_Name);//列表填充
                                }
                                else
                                {
                                    OutputDebug("EZCAPWARNING | %s | %s | GetQHYCCDReadModeName() Failed!", __FILE__, __FUNCTION__);
                                }
                            }

                            // 验证从 ini 文件读取的 ReadMode 值是否有效
                            if(ix.ReadMode >= ix.ReadMode_Num)
                            {
                                OutputDebug("EZCAPWARNING | %s | %s | ReadMode from ini file (%d) is invalid, reset to 0", __FILE__, __FUNCTION__, ix.ReadMode);
                                ix.ReadMode = 0;
                            }

                            if(!iniFileParams.autoConnectLive)
                            {
                                readMode = new ReadMode(this);
                                readMode->auto_select=iniFileParams.autoConnectLive;

                                readMode->ui->comboBox_readmode->blockSignals(true);
                                readMode->ui->comboBox_readmode->clear();
                                readMode->ui->comboBox_readmode->addItems(ix.ReadMode_List);
                                readMode->ui->comboBox_readmode->setCurrentIndex(ix.ReadMode);
                                readMode->ui->comboBox_readmode->blockSignals(false);
                                readMode->setWindowModality(Qt::ApplicationModal);//设置父窗口不可控
                                readMode->exec();
                                if(!canConnect) return;
                            }
                            else
                            {
                                OutputDebug("EZCAPINFO | %s | %s | autoConnectLive mode, using ReadMode from ini file: %d", __FILE__, __FUNCTION__, ix.ReadMode);
                            }
                        }
                        else if(ix.ReadMode_Num == 1 &&  ret == QHYCCD_SUCCESS)//20200318仅有一种readmode模式
                        {
                            ix.ReadMode = 0;
                        }
                        else
                        {
                            OutputDebug("EZCAPWARNING | %s | %s | GetQHYCCDNumberOfReadModes() Failed!", __FILE__, __FUNCTION__);
                        }
                    }
                    else
                    {
                        OutputDebug("EZCAPWARNING | %s | %s | GetQHYCCDNumberOfReadMode() Has No This Function!", __FILE__, __FUNCTION__);
                    }

                    ret = libqhyccd->SetQHYCCDReadMode(camhandle, ix.ReadMode);
                    if(ret != QHYCCD_SUCCESS)
                    {
                        OutputDebug("EZCAPWARNING | %s | %s | SetQHYCCDReadMode() Failed!", __FILE__, __FUNCTION__);
                    }
                    else
                    {
                        ix.ReadMode_Last = ix.ReadMode;
                    }

                    ret = libqhyccd->SetQHYCCDStreamMode(camhandle, ix.camStreamMode);
                    if(ret != QHYCCD_SUCCESS)
                    {
                        OutputDebug("EZCAPWARNING | %s | %s | SetQHYCCDStreamMode() Failed! | StreamMode = %d", __FILE__, __FUNCTION__, ix.camStreamMode);
                    }
                    else
                    {
                        ix.lastCamStreamMode = ix.camStreamMode;
                    }

                    ret = libqhyccd->InitQHYCCD(camhandle);
                    if(ret != QHYCCD_SUCCESS)
                    {
                        OutputDebug("EZCAPWARNING | %s | %s | InitQHYCCD() Failed!", __FILE__, __FUNCTION__);
                        QMessageBox::critical(this,tr("Error"),tr("Camera initialization failed!"),QMessageBox::Ok);
                    }

                    ix.isConnected = true;
                }
                else
                {
                    OutputDebug("EZCAPWARNING | %s | %s | OpenQHYCCD() Failed!", __FILE__, __FUNCTION__);
                    QMessageBox::critical(this,tr("Warning"),tr("Camera connection failed!"),QMessageBox::Ok);
                }

                if(ix.circle1 == 0 && ix.circle2 == 0)
                {
                    if(ix.CamID.contains("CCM6000"))// || ix.CamID.contains("QHY600"))
                    {
                        ix.circle1 = 5985;
                        ix.circle2 = 6005;
                    }
                    else if(ix.CamID.contains("CCM2600"))// || ix.CamID.contains("QHY268"))
                    {
                        ix.circle1 = 6060;
                        ix.circle2 = 6080;
                    }
                    else
                    {
                        ix.circle1 = 5000;
                        ix.circle2 = 5000;
                    }
                }

                if(ix.CamID.contains("miniCam8"))
                {
                    libqhyccd->SetQHYCCDWriteFPGA(camhandle, 0, 37, ix.CFWCalculateValue);
                }

                ix.zoomMode = Zoom_SpecifyScaling;
                ix.scaleFactor = 1.0;
                mainMenuBar->actFitWindow->trigger();
                scrollArea_ImgShow->setWidgetResizable(true);

                if(iniFileParams.autoConnect){
                    managerMenu->ui->head_capture->click();
                    managerMenu->ui->pBtn_capture->click();
                }

                /*
                 * OSD(20200512lyl)
                 */
                //列表填充
                ix.OSDList.clear();
                ix.OSDList.append("Disable OSD");
                ix.OSDList.append("HardwareFrameCounter");
                ix.OSDList.append("GPS Data");
                favorite_dialog->ui->comboBox_OSD->clear();
                favorite_dialog->ui->comboBox_OSD->addItems(ix.OSDList);

                this->getParamsFromCamera();//设置界面控件初始状态

                // 发送相机连接信号
                emit connect_camera();

                if(ix.camStreamMode == 1)
                {
                    ix.showLabelW = ui->label_ImgShow->width();
                    ix.showLabelH = ui->label_ImgShow->height();

                    setStretchLUT(Live_WPOS, Live_BPOS);
                    managerMenu->ui->hSlider_bPos->setValue(Live_BPOS);
                    managerMenu->ui->hSlider_wPos->setValue(Live_WPOS);

                    ret = libqhyccd->BeginQHYCCDLive(camhandle);
                    if(ret != QHYCCD_SUCCESS)
                    {
                        OutputDebug("EZCAPWARNING | %s | %s | BeginQHYCCDLive() Failed!", __FILE__, __FUNCTION__);
                    }
                    else
                    {
                        OutputDebug("EZCAPWARNING | %s | %s | Start Live Capture Thread", __FILE__, __FUNCTION__);
                        connect(liveCap, SIGNAL(gotFPSData()), this, SLOT(showFPS()));
                        liveCap->start();
                        threadProcessImage->start();
                        updateImgTimer->start(30);
                        updateFrameTimer->start(300);
                    }
                }
            }
        }
        else
        {
#ifdef Q_OS_MAC
            QMessageBox::critical(this,tr("Warning"),tr("Not Found QHYCCD Devices (If hotplug USB,please wait for about 5s to connect)!"),QMessageBox::Ok);
#else
            QMessageBox::critical(this,tr("Warning"),tr("Not Found QHYCCD Devices!"),QMessageBox::Ok);
#endif
        }
    }
}

/**
 * @brief EZCAP::showPlanTable
 */
void EZCAP::showPlanTable()
{
    planner_dialog->show();
}

void EZCAP::imageRotateMirror()
{
    uint32_t ret = QHYCCD_ERROR;
    QAction *action = qobject_cast<QAction *>(sender());
    if(!action->isChecked())
    {
        ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_ImgProc, NOPROC);
        OutputDebug("EZCAPWARNING | %s | %s | Set ImgProc NOPROC", __FILE__, __FUNCTION__);
    }
    else
    {
        if(action == mainMenuBar->actImgRotate180)
        {
            mainMenuBar->actImgRotate90L->setChecked(false);
            mainMenuBar->actImgRotate90R->setChecked(false);
            mainMenuBar->actImgMirrorH->setChecked(false);
            mainMenuBar->actImgMirrorV->setChecked(false);
            ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_ImgProc, ROTATION180);
            OutputDebug("EZCAPWARNING | %s | %s | Set ImgProc ROTATION180", __FILE__, __FUNCTION__);
        }
        else if(action == mainMenuBar->actImgRotate90L)
        {
            mainMenuBar->actImgRotate180->setChecked(false);
            mainMenuBar->actImgRotate90R->setChecked(false);
            mainMenuBar->actImgMirrorH->setChecked(false);
            mainMenuBar->actImgMirrorV->setChecked(false);
            ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_ImgProc, ROTATION90L);
            OutputDebug("EZCAP | %s | %s | Set ImgProc ROTATION90L", __FILE__, __FUNCTION__);
        }
        else if(action == mainMenuBar->actImgRotate90R)
        {
            mainMenuBar->actImgRotate180->setChecked(false);
            mainMenuBar->actImgRotate90L->setChecked(false);
            mainMenuBar->actImgMirrorH->setChecked(false);
            mainMenuBar->actImgMirrorV->setChecked(false);
            ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_ImgProc, ROTATION90R);
            OutputDebug("EZCAP | %s | %s | Set ImgProc ROTATION90R", __FILE__, __FUNCTION__);
        }
        else if(action == mainMenuBar->actImgMirrorH)
        {
            mainMenuBar->actImgRotate180->setChecked(false);
            mainMenuBar->actImgRotate90L->setChecked(false);
            mainMenuBar->actImgRotate90R->setChecked(false);
            mainMenuBar->actImgMirrorV->setChecked(false);
            ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_ImgProc, MIRRORH);
            OutputDebug("EZCAP | %s | %s | Set ImgProc MIRRORH", __FILE__, __FUNCTION__);
        }
        else if(action == mainMenuBar->actImgMirrorV)
        {
            mainMenuBar->actImgRotate180->setChecked(false);
            mainMenuBar->actImgRotate90L->setChecked(false);
            mainMenuBar->actImgRotate90R->setChecked(false);
            mainMenuBar->actImgMirrorH->setChecked(false);
            ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_ImgProc, MIRRORV);
            OutputDebug("EZCAP | %s | %s | Set ImgProc MIRRORV", __FILE__, __FUNCTION__);
        }
        else
        {
            OutputDebug("EZCAPERROR | %s | %s | ImageProcess Action Error!", __FILE__, __FUNCTION__);
        }
    }
}

/**
 * @brief EZCAP::technicalSupport
 */


void EZCAP::technicalSupport()
{
    technicalSupport_dialog->show();
}

void EZCAP::showFavoriteSetting()
{
    favorite_dialog->show();
}

void EZCAP::showGPSTool()
{
    gpsTool_dialog->show();
}

void EZCAP::showToolBurst()
{
    toolBurst_dialog->show();
}

void EZCAP::showToolTrigger()
{
    toolTrigger_dialog->show();
}

/**
 * @brief EZCAP::showTempControl
 */
void EZCAP::showTempControl()
{
    tempControl_dialog->show();
}

/**
 * @brief EZCAP::showCFWControl
 */
void EZCAP::showCFWControl()
{
    cfwControl_dialog->show();
}

void EZCAP::showOtherCameraSetup()
{
    otherCameraSetup_dialog->show();
}

/**
 * @brief EZCAP::showPHDLink
 */
void EZCAP::showPHDLink()
{
    phdLink_dialog->show();
}


void EZCAP::showCaptureDarkFrameTool()
{
    QMessageBox::information(this,tr("Note"),tr("Dark frame required,Please cover the camera OR Chose the right filter wheel hole!"),QMessageBox::Ok);

    if(ix.camStreamMode == 0)
        darkFrameTool_dialog->show();
    else if(ix.camStreamMode == 1)
        frameToolCapCal_dialog->show();
    else
        OutputDebug("EZCAPERROR | %s | %s | camStreamMode Value Error!", __FILE__, __FUNCTION__);
}

void EZCAP::darkFrameCalibration()
{
    frameToolCal_dialog->show();
}

void EZCAP::showToolCorrectCenter()
{
    toolCorrectCenter_dialog->show();
}

void EZCAP::showImgAnalyze()
{
    imgAnalyze_dialog->show();
}

/**
 * @brief EZCAP::showAbout
 */
void EZCAP::showAbout()
{
    about_dialog->show();
}

void EZCAP::showManual()
{
    const QString resPath(":/doc/EZCAP_QT User Manual.pdf");
    QFile resFile(resPath);
    if (!resFile.exists()) {
        QMessageBox::warning(this, tr("提示"), tr("找不到内置说明书资源。"));
        return;
    }

    if (!resFile.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, tr("提示"), tr("无法读取内置说明书资源。"));
        return;
    }

    QString tmpDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    if (tmpDir.isEmpty())
        tmpDir = QDir::tempPath();
    QDir().mkpath(tmpDir);

    QDir manualDir(tmpDir);
    const QStringList staleManuals = manualDir.entryList(QStringList() << "EZCAP_QT User Manual*.pdf", QDir::Files);
    for (const QString &fileName : staleManuals)
    {
        const QString fullPath = manualDir.filePath(fileName);
        if (!manualDir.remove(fileName))
        {
            DBGOPT_WARNING("Failed to remove stale manual %s", qPrintable(fullPath));
        }
    }

    QTemporaryFile tmpFile(QDir(tmpDir).filePath("EZCAP_QT User Manual XXXXXX.pdf"));
    tmpFile.setAutoRemove(false);
    if (!tmpFile.open()) {
        QMessageBox::warning(this, tr("提示"), tr("无法创建临时文件用于存放说明书。"));
        return;
    }

    DBGOPT_INFO("resPath = %s tmpFile = %s", qPrintable(resPath), qPrintable(tmpFile.fileName()));

    const QByteArray manualData = resFile.readAll();
    if (manualData.isEmpty()) {
        QMessageBox::warning(this, tr("提示"), tr("说明书资源为空或无法读取。"));
        return;
    }

    if (tmpFile.write(manualData) != manualData.size() || !tmpFile.flush()) {
        QMessageBox::warning(this, tr("提示"), tr("无法写入临时说明书文件。"));
        return;
    }

    const QString manualPath = tmpFile.fileName();
    tmpFile.close();
    resFile.close();

    const QUrl url = QUrl::fromLocalFile(QDir::toNativeSeparators(manualPath));
    if (!QDesktopServices::openUrl(url)) {
        QMessageBox::warning(this, tr("提示"), tr("系统未能打开 PDF 阅读器。"));
    }
}

/**
 * @brief EZCAP::showFITHeaderEditor
 */
void EZCAP::showFITHeaderEditor()
{
    fitHeader_dialog->show();
}

void EZCAP::ignoreOverScanAreaClicked(bool checked)
{
    ix.IgnoreOverscan = checked;
    iniFileParams.ignoreOverScan = ix.IgnoreOverscan;

    if(ix.workMode == WorkMode_Capture && ix.lastWorkMode == ix.workMode && ix.imageReady == GetSingleFrame_Success)
    {
        displaySingleFrame(ix.FrameW_Last, ix.FrameH_Last, ix.ImgData_Last);

        if(viewBoxCX < viewBoxW / 2) viewBoxCX = viewBoxW / 2;
        if(viewBoxCY < viewBoxH / 2) viewBoxCY = viewBoxH / 2;
        if(viewBoxCX > (managerMenu->ui->img_screenView->width() - viewBoxW / 2 - 1))
            viewBoxCX = managerMenu->ui->img_screenView->width() - viewBoxW / 2 - 1;
        if(viewBoxCY > (managerMenu->ui->img_screenView->height() - viewBoxH / 2 - 1))
            viewBoxCY = managerMenu->ui->img_screenView->height() - viewBoxH / 2 - 1;
        displayScreenViewImage(viewBoxCX, viewBoxCY, viewBoxW, viewBoxH);
    }
}

void EZCAP::calibrateOverScanClicked(bool checked)
{
    ix.CalibrateOverscan = checked;
    iniFileParams.calibrateOverScan = ix.CalibrateOverscan;
}

void EZCAP::enableSaveTHPFile(bool checked)
{
#if THP_File_Saved
    ix.saveTHPFile = checked;

    if(ix.saveTHPFile)
    {
        mainMenuBar->actSaveTHPFile->setChecked(true);

        QString filenameTEMP = QFileDialog::getSaveFileName(this,tr("Save"),"",tr("temp Files(*.txt)")); //选择路径txt   xlsx
        if(filenameTEMP.isEmpty())
        {
            qDebug() << "save Excel Files: fileName is empty, can not save";
        }
        else
        {
            fileTHP=filenameTEMP;
            if(QFile::exists(filenameTEMP))
            {
                qDebug() << "exists  Files: " << filenameTEMP;
                QFile::remove(filenameTEMP);
            }
            qDebug() << "save Excel Files: " << filenameTEMP;
            QFile fileTEMP(filenameTEMP);
            fileTEMP.open(QIODevice::Text | QIODevice::WriteOnly);
            QTextStream out(&fileTEMP);
            out<<qSetFieldWidth(5)<<left<<"\t\t time\t Temp\t Humidity\t Press\t rate\t";
            fileTEMP.close();
        }
    }
    else
    {
        mainMenuBar->actSaveTHPFile->setChecked(false);
    }
#endif
}

/**
 * @brief EZCAP::saveAsFIT
 */
void EZCAP::saveAsFIT()
{
    QString savePath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "/EZCAP Captures";
    QDir saveDir1(savePath);
    if(!saveDir1.exists())
    {
        QDir *folder = new QDir();
        folder->mkdir(savePath);
    }
    savePath += "/Images";
    QDir saveDir2(savePath);
    if(!saveDir2.exists())
    {
        QDir *folder = new QDir();
        folder->mkdir(savePath);
    }

    QString filename = QFileDialog::getSaveFileName(this,
        tr("Save"),
        savePath + "/" + ix.CamID + "-" + QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss-zzz") + ".fit",
        tr("Image Files(*.fit)")); //选择路径

    if(filename.isEmpty())
    {
        return;
    }
    else
    {
        QFileInfo fi(filename);
        lastSavedPath = fi.absolutePath(); //record the iamge saved path

        if(!CheckWritePath(filename))
        {
            QMessageBox::critical(this,tr("Warning"),tr("The selection folder's name has problem and can not be saved there. Please change another folder!"),QMessageBox::Ok);
        }
        else
        {
            //如果是preview模式，则用标准FIT保存函数，忽略掉ignoreOverScanArea的影响
            if(ix.workMode == WorkMode_Preview && ix.lastWorkMode == WorkMode_Preview && ix.imageReady == GetSingleFrame_Success)
            {
                fitHeader_dialog->FITwrite_Common(filename, ix.FrameW, ix.FrameH, ix.ImgData_Save);
            }
            else if(ix.workMode == WorkMode_Focus && ix.lastWorkMode == WorkMode_Focus && ix.imageReady == GetSingleFrame_Success)
            {
                fitHeader_dialog->FITwrite_Common(filename, ix.FrameW, ix.FrameH, ix.ImgData_Save);
            }
            else if(ix.workMode == WorkMode_Capture && ix.lastWorkMode == WorkMode_Capture && ix.imageReady == GetSingleFrame_Success)
            {
                fitHeader_dialog->FITWrite(filename, ix.ImgData_Save);
            }
            mainMenuBar->actOpenFolder->setEnabled(true);
        }
    }
}

/**
 * @brief EZCAP::saveAsBMP
 */
void EZCAP::saveAsBMP()
{
    QString savePath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "/EZCAP Captures";
    QDir saveDir1(savePath);
    if(!saveDir1.exists())
    {
        QDir *folder = new QDir();
        folder->mkdir(savePath);
    }
    savePath += "/Images";
    QDir saveDir2(savePath);
    if(!saveDir2.exists())
    {
        QDir *folder = new QDir();
        folder->mkdir(savePath);
    }

    QString filename = QFileDialog::getSaveFileName(this,
        tr("Save"),
        savePath + "/" + ix.CamID + "-" + QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss-zzz") + ".bmp",
        tr("Image Files(*.bmp)"));
    if(filename.isEmpty())
    {
        return;
    }
    else
    {
        QFileInfo fi(filename);
        lastSavedPath = fi.absolutePath(); //record the iamge saved path

        int type = ix.FrameC_Save == 1 ? CV_8UC1 : CV_8UC3;
        Mat matImgSave(Size(ix.FrameW_Save, ix.FrameH_Save), type);
        if(ix.FrameB_Save == 16)
            for (int i = 0; i < ix.FrameW_Save*ix.FrameH_Save*ix.FrameC_Save; i++)
                matImgSave.data[i] = ix.ImgData_Save[2*i+1];
        else
            matImgSave.data = ix.ImgData_Save;

        if(ix.workMode == WorkMode_Preview)
        {
            //string holds the text converted to Unicode, 支持中文路径
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
            QByteArray encodedString = QStringEncoder("GB18030")(filename);
#else
            QTextCodec *codec = QTextCodec::codecForName("GB18030");
            QByteArray encodedString = codec->fromUnicode(filename);
#endif
            imwrite(encodedString.constData(), matImgSave);
        }
        else if(ix.workMode == WorkMode_Focus)
        {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
            QByteArray encodedString = QStringEncoder("GB18030")(filename);
#else
            QTextCodec *codec = QTextCodec::codecForName("GB18030");
            QByteArray encodedString = codec->fromUnicode(filename);
#endif
            imwrite(encodedString.constData(), matImgSave);
        }
        else if(ix.workMode == WorkMode_Capture)
        {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
            QByteArray encodedString = QStringEncoder("GB18030")(filename);
#else
            QTextCodec *codec = QTextCodec::codecForName("GB18030");
            QByteArray encodedString = codec->fromUnicode(filename);
#endif
            imwrite(encodedString.constData(), matImgSave);
        }
        matImgSave.release();
        mainMenuBar->actOpenFolder->setEnabled(true);
    }
}

/**
 * @brief EZCAP::saveAsJPG
 */
void EZCAP::saveAsJPG()
{
    QString savePath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "/EZCAP Captures";
    QDir saveDir1(savePath);
    if(!saveDir1.exists())
    {
        QDir *folder = new QDir();
        folder->mkdir(savePath);
    }
    savePath += "/Images";
    QDir saveDir2(savePath);
    if(!saveDir2.exists())
    {
        QDir *folder = new QDir();
        folder->mkdir(savePath);
    }

    QString filename = QFileDialog::getSaveFileName(this,
        tr("Save"),
        savePath + "/" + ix.CamID + "-" + QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss-zzz") + ".jpg",
        tr("Image Files(*.jpg)"));
    if(filename.isEmpty())
    {
        qCritical() << "saveAsJPG: fileName is empty,can not save";
        return;
    }
    else
    {
        QFileInfo fi(filename);
        lastSavedPath = fi.absolutePath(); //record the iamge saved path

        int type = ix.FrameC_Save == 1 ? CV_8UC1 : CV_8UC3;
        Mat matImgSave(Size(ix.FrameW_Save, ix.FrameH_Save), type);
        if(ix.FrameB_Save == 16)
            for (int i = 0; i < ix.FrameW_Save*ix.FrameH_Save*ix.FrameC_Save; i++)
                matImgSave.data[i] = ix.ImgData_Save[2*i+1];
        else
            matImgSave.data = ix.ImgData_Save;

        std::vector<int> compression_params;
        compression_params.push_back(IMWRITE_JPEG_QUALITY);  //选择jpg
        compression_params.push_back(100); //在这个填入你要的图片质量
        if(ix.workMode == WorkMode_Preview)
        {
            //string holds the text converted to Unicode, 支持中文路径
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
            QByteArray encodedString = QStringEncoder("GB18030")(filename);
#else
            QTextCodec *codec = QTextCodec::codecForName("GB18030");
            QByteArray encodedString = codec->fromUnicode(filename);
#endif
            imwrite(encodedString.constData(), matImgSave, compression_params);
        }
        else if(ix.workMode == WorkMode_Focus)
        {
            //string holds the text converted to Unicode, 支持中文路径
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
            QByteArray encodedString = QStringEncoder("GB18030")(filename);
#else
            QTextCodec *codec = QTextCodec::codecForName("GB18030");
            QByteArray encodedString = codec->fromUnicode(filename);
#endif
            imwrite(encodedString.constData(), matImgSave, compression_params);
        }
        else if(ix.workMode == WorkMode_Capture)
        {
            //string holds the text converted to Unicode, 支持中文路径
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
            QByteArray encodedString = QStringEncoder("GB18030")(filename);
#else
            QTextCodec *codec = QTextCodec::codecForName("GB18030");
            QByteArray encodedString = codec->fromUnicode(filename);
#endif
            imwrite(encodedString.constData(), matImgSave, compression_params);
        }
        mainMenuBar->actOpenFolder->setEnabled(true);
    }
}

void EZCAP::saveAsTIF()
{
    QString savePath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "/EZCAP Captures";
    QDir saveDir1(savePath);
    if(!saveDir1.exists())
    {
        QDir *folder = new QDir();
        folder->mkdir(savePath);
    }
    savePath += "/Images";
    QDir saveDir2(savePath);
    if(!saveDir2.exists())
    {
        QDir *folder = new QDir();
        folder->mkdir(savePath);
    }

    QString filename = QFileDialog::getSaveFileName(this,
        tr("Save"),
        savePath + "/" + ix.CamID + "-" + QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss-zzz") + ".tif",
        tr("Image Files(*.tif)"));
    if(filename.isEmpty())
    {
        qCritical() << "saveAsTIF: fileName is empty,can not save";
        return;
    }
    else
    {
        QFileInfo fi(filename);
        lastSavedPath = fi.absolutePath(); //record the iamge saved path

        OutputDebug("EZCAP | %s | %s | filename = %s", __FILE__, __FUNCTION__, qPrintable(filename));
        int type;
        if     (ix.FrameB_Save == 8  && ix.FrameC_Save == 1) type = CV_8UC1;
        else if(ix.FrameB_Save == 16 && ix.FrameC_Save == 1) type = CV_16UC1;
        else if(ix.FrameB_Save == 8  && ix.FrameC_Save == 3) type = CV_8UC3;
        else if(ix.FrameB_Save == 16 && ix.FrameC_Save == 3) type = CV_16UC3;
        Mat matImgSave(Size(ix.FrameW_Save, ix.FrameH_Save), type);
        matImgSave.data = ix.ImgData_Save;

        std::vector<int> compression_params;
        compression_params.push_back(IMWRITE_TIFF_COMPRESSION);  //选择tif
        compression_params.push_back(1); //在这个填入你要的图片质量
        if(ix.workMode == WorkMode_Preview)
        {
            //string holds the text converted to Unicode, 支持中文路径
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
            QByteArray encodedString = QStringEncoder("GB18030")(filename);
#else
            QTextCodec *codec = QTextCodec::codecForName("GB18030");
            QByteArray encodedString = codec->fromUnicode(filename);
#endif
            imwrite(encodedString.constData(), matImgSave, compression_params);
        }
        else if(ix.workMode == WorkMode_Focus)
        {
            //string holds the text converted to Unicode, 支持中文路径
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
            QByteArray encodedString = QStringEncoder("GB18030")(filename);
#else
            QTextCodec *codec = QTextCodec::codecForName("GB18030");
            QByteArray encodedString = codec->fromUnicode(filename);
#endif
            imwrite(encodedString.constData(), matImgSave, compression_params);
        }
        else if(ix.workMode == WorkMode_Capture)
        {
            //string holds the text converted to Unicode, 支持中文路径
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
            QByteArray encodedString = QStringEncoder("GB18030")(filename);
#else
            QTextCodec *codec = QTextCodec::codecForName("GB18030");
            QByteArray encodedString = codec->fromUnicode(filename);
#endif
            imwrite(encodedString.constData(), matImgSave, compression_params);
        }
        matImgSave.release();
        mainMenuBar->actOpenFolder->setEnabled(true);
    }
}

void EZCAP::saveAsPNG()
{
    QString savePath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "/EZCAP Captures";
    QDir saveDir1(savePath);
    if(!saveDir1.exists())
    {
        QDir *folder = new QDir();
        folder->mkdir(savePath);
    }
    savePath += "/Images";
    QDir saveDir2(savePath);
    if(!saveDir2.exists())
    {
        QDir *folder = new QDir();
        folder->mkdir(savePath);
    }

    QString filename = QFileDialog::getSaveFileName(this,
        tr("Save"),
        savePath + "/" + ix.CamID + "-" + QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss-zzz") + ".png",
        tr("Image Files(*.png)"));
    if(filename.isEmpty())
    {
        qCritical() << "saveAsTIF: fileName is empty,can not save";
        return;
    }
    else
    {
        QFileInfo fi(filename);
        lastSavedPath = fi.absolutePath(); //record the iamge saved path

        int type = ix.FrameC_Save == 1 ? CV_8UC1 : CV_8UC3;
        Mat matImgSave(Size(ix.FrameW_Save, ix.FrameH_Save), type);
        if(ix.FrameB_Save == 16)
            for (int i = 0; i < ix.FrameW_Save*ix.FrameH_Save*ix.FrameC_Save; i++)
                matImgSave.data[i] = ix.ImgData_Save[2*i+1];
        else
            matImgSave.data = ix.ImgData_Save;

        std::vector<int> compression_params;
        compression_params.push_back(IMWRITE_PNG_COMPRESSION);  //选择png
        compression_params.push_back(0); //在这个填入你要的图片质量
        if(ix.workMode == WorkMode_Preview)
        {
            //string holds the text converted to Unicode, 支持中文路径
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
            QByteArray encodedString = QStringEncoder("GB18030")(filename);
#else
            QTextCodec *codec = QTextCodec::codecForName("GB18030");
            QByteArray encodedString = codec->fromUnicode(filename);
#endif
            imwrite(encodedString.constData(), matImgSave, compression_params);
        }
        else if(ix.workMode == WorkMode_Focus)
        {
            //string holds the text converted to Unicode, 支持中文路径
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
            QByteArray encodedString = QStringEncoder("GB18030")(filename);
#else
            QTextCodec *codec = QTextCodec::codecForName("GB18030");
            QByteArray encodedString = codec->fromUnicode(filename);
#endif
            imwrite(encodedString.constData(), matImgSave, compression_params);
        }
        else if(ix.workMode == WorkMode_Capture)
        {
            //string holds the text converted to Unicode, 支持中文路径
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
            QByteArray encodedString = QStringEncoder("GB18030")(filename);
#else
            QTextCodec *codec = QTextCodec::codecForName("GB18030");
            QByteArray encodedString = codec->fromUnicode(filename);
#endif
            imwrite(encodedString.constData(), matImgSave, compression_params);
        }
        matImgSave.release();
        mainMenuBar->actOpenFolder->setEnabled(true);
    }
}

/**
 * @brief EZCAP::openFolder
 */
void EZCAP::openFolder()
{
    OutputDebug("QHYCCDEZCAP | %s | %s | lastSavePath = %s", __FILE__, __FUNCTION__, qPrintable(lastSavedPath));
    if(lastSavedPath.isEmpty())
    {
        QMessageBox::information(this,tr("Warning"), tr("Has no images or videos be saved!"), QMessageBox::Ok);
    }
    else
    {
        lastSavedPath = QDir::toNativeSeparators(lastSavedPath);
        QUrl url = QUrl::fromLocalFile(lastSavedPath);
        QDesktopServices::openUrl(url); //open folder
    }
}

//-----------------------------------------------------------------

void EZCAP::cfwPositionChanged()
{
    if(!ix.isConnected){
        qDebug() << QString("Trigger cfwPositionChangeed But not connected  (char code) %1").arg(ix.dstCfwPos);
        return;
    }
    qDebug() << QString("move CFW to: (char code) %1").arg(ix.dstCfwPos);
    ix.CFWStatus = CFW_Moving;

    runCFWOrder->start(); //start thread to set the CFW posiion

    this->startCFWTimer();//开启色轮状态查询  定时器
    cfwControl_dialog->startCFWProgressBar(); //开启进度条

    runCFWOrder->wait();
}

/**
 * @brief EZCAP::scaleImage  改变图像显示比例
 * @param factor
 */
void EZCAP::scaleImage(double factor)
{
    if(factor != 0.0)
    {
        scrollArea_ImgShow->setWidgetResizable(false);
        displaySingleFrame(ix.FrameW_Last, ix.FrameH_Last, ix.ImgData_Last);
    }

    viewBoxW = managerMenu->ui->img_screenView->width()  * scrollArea_ImgShow->width()  / ui->label_ImgShow->width();
    viewBoxH = managerMenu->ui->img_screenView->height() * scrollArea_ImgShow->height() / ui->label_ImgShow->height();
    viewBoxCX = managerMenu->ui->img_screenView->width()  * (double)ix.showLabelX / ui->label_ImgShow->width()  + viewBoxW / 2;
    viewBoxCY = managerMenu->ui->img_screenView->height() * (double)ix.showLabelY / ui->label_ImgShow->height() + viewBoxH / 2;

    if(viewBoxCX < viewBoxW / 2) viewBoxCX = viewBoxW / 2;
    if(viewBoxCY < viewBoxH / 2) viewBoxCY = viewBoxH / 2;
    if(viewBoxCX > (managerMenu->ui->img_screenView->width() - viewBoxW / 2 - 1))
        viewBoxCX = managerMenu->ui->img_screenView->width() - viewBoxW / 2 - 1;
    if(viewBoxCY > (managerMenu->ui->img_screenView->height() - viewBoxH / 2 - 1))
        viewBoxCY = managerMenu->ui->img_screenView->height() - viewBoxH / 2 - 1;

    displayScreenViewImage(viewBoxW, viewBoxH, viewBoxCX, viewBoxCY);
}

void EZCAP::zoomFitWindow()
{
    if(ix.zoomMode != Zoom_FitWindow)
    {
        ix.zoomMode = Zoom_FitWindow;

        scrollArea_ImgShow->setWidgetResizable(true);
        if (ix.camStreamMode == 0)
        {
            displayScreenViewImage(viewBoxW, viewBoxH, viewBoxCX, viewBoxCY);
            displaySingleFrame(ix.FrameW_Last, ix.FrameH_Last, ix.ImgData_Last);
        }
        else if (ix.camStreamMode == 1)
        {
            ui->horizontalScrollBar_ImgShow->setMinimum(0);
            ui->horizontalScrollBar_ImgShow->setMaximum(0);
            ui->horizontalScrollBar_ImgShow->setVisible(false);
            ui->verticalScrollBar_ImgShow->setMinimum(0);
            ui->verticalScrollBar_ImgShow->setMaximum(0);
            ui->verticalScrollBar_ImgShow->setVisible(false);
        }
    }
}

/**
 * @brief EZCAP::zoomFitWindow
 */
void EZCAP::zoomFillWindow()
{
    if(ix.zoomMode != Zoom_FillWindow)
    {
        ix.zoomMode = Zoom_FillWindow;

        scrollArea_ImgShow->setWidgetResizable(true);
        if (ix.camStreamMode == 0)
        {
            displayScreenViewImage(viewBoxW, viewBoxH, viewBoxCX, viewBoxCY);
            displaySingleFrame(ix.FrameW_Last, ix.FrameH_Last, ix.ImgData_Last);
        }
        else if (ix.camStreamMode == 1)
        {
            ui->horizontalScrollBar_ImgShow->setMinimum(0);
            ui->horizontalScrollBar_ImgShow->setMaximum(0);
            ui->horizontalScrollBar_ImgShow->setVisible(false);
            ui->verticalScrollBar_ImgShow->setMinimum(0);
            ui->verticalScrollBar_ImgShow->setMaximum(0);
            ui->verticalScrollBar_ImgShow->setVisible(false);
        }
    }
}

/**
 * @brief EZCAP::zoom0_25X
 */
void EZCAP::zoom0_25X()
{
    if(ix.scaleFactor != 0.25 || ix.zoomMode != Zoom_SpecifyScaling)
    {
        ix.scaleFactor = 0.25;
        ix.zoomMode = Zoom_SpecifyScaling;

        if (ix.camStreamMode == 0)
        {
            scaleImage(ix.scaleFactor);
        }
        else if (ix.camStreamMode == 1)
        {
            if((double)ix.RoiW_Last * ix.scaleFactor - ix.showLabelW > 0)
            {
                ui->horizontalScrollBar_ImgShow->setMinimum(0);
                ui->horizontalScrollBar_ImgShow->setMaximum((double)ix.RoiW_Last * ix.scaleFactor - ix.showLabelW);
                ui->horizontalScrollBar_ImgShow->setPageStep(ix.showLabelW);
                ui->horizontalScrollBar_ImgShow->setValue(ix.showLabelX);
                ui->horizontalScrollBar_ImgShow->setVisible(true);
            }
            else
            {
                ui->horizontalScrollBar_ImgShow->setVisible(false);
            }
            if((double)ix.RoiH_Last * ix.scaleFactor - ix.showLabelH > 0)
            {
                ui->verticalScrollBar_ImgShow->setMinimum(0);
                ui->verticalScrollBar_ImgShow->setMaximum((double)ix.RoiH_Last * ix.scaleFactor - ix.showLabelH);
                ui->verticalScrollBar_ImgShow->setPageStep(ix.showLabelH);
                ui->verticalScrollBar_ImgShow->setValue(ix.showLabelY);
                ui->verticalScrollBar_ImgShow->setVisible(true);
            }
            else
            {
                ui->verticalScrollBar_ImgShow->setVisible(false);
            }
        }
    }
}

/**
 * @brief EZCAP::zoom0_5X
 */
void EZCAP::zoom0_5X()
{
    if(ix.scaleFactor != 0.5 || ix.zoomMode != Zoom_SpecifyScaling)
    {
        ix.scaleFactor = 0.5;
        ix.zoomMode = Zoom_SpecifyScaling;

        if (ix.camStreamMode == 0)
        {
            scaleImage(ix.scaleFactor);
        }
        else if (ix.camStreamMode == 1)
        {
            if((double)ix.RoiW_Last * ix.scaleFactor - ix.showLabelW > 0)
            {
                ui->horizontalScrollBar_ImgShow->setMinimum(0);
                ui->horizontalScrollBar_ImgShow->setMaximum((double)ix.RoiW_Last * ix.scaleFactor - ix.showLabelW);
                ui->horizontalScrollBar_ImgShow->setPageStep(ix.showLabelW);
                ui->horizontalScrollBar_ImgShow->setValue(ix.showLabelX);
                ui->horizontalScrollBar_ImgShow->setVisible(true);
            }
            else
            {
                ui->horizontalScrollBar_ImgShow->setVisible(false);
            }
            if((double)ix.RoiH_Last * ix.scaleFactor - ix.showLabelH > 0)
            {
                ui->verticalScrollBar_ImgShow->setMinimum(0);
                ui->verticalScrollBar_ImgShow->setMaximum((double)ix.RoiH_Last * ix.scaleFactor - ix.showLabelH);
                ui->verticalScrollBar_ImgShow->setPageStep(ix.showLabelH);
                ui->verticalScrollBar_ImgShow->setValue(ix.showLabelY);
                ui->verticalScrollBar_ImgShow->setVisible(true);
            }
            else
            {
                ui->verticalScrollBar_ImgShow->setVisible(false);
            }
        }
    }
}

/**
 * @brief EZCAP::zoom0_75X
 */
void EZCAP::zoom0_75X()
{
    if(ix.scaleFactor != 0.75 || ix.zoomMode != Zoom_SpecifyScaling)
    {
        ix.scaleFactor = 0.75;
        ix.zoomMode = Zoom_SpecifyScaling;

        if (ix.camStreamMode == 0)
        {
            scaleImage(ix.scaleFactor);
        }
        else if (ix.camStreamMode == 1)
        {
            if((double)ix.RoiW_Last * ix.scaleFactor - ix.showLabelW > 0)
            {
                ui->horizontalScrollBar_ImgShow->setMinimum(0);
                ui->horizontalScrollBar_ImgShow->setMaximum((double)ix.RoiW_Last * ix.scaleFactor - ix.showLabelW);
                ui->horizontalScrollBar_ImgShow->setPageStep(ix.showLabelW);
                ui->horizontalScrollBar_ImgShow->setValue(ix.showLabelX);
                ui->horizontalScrollBar_ImgShow->setVisible(true);
            }
            else
            {
                ui->horizontalScrollBar_ImgShow->setVisible(false);
            }
            if((double)ix.RoiH_Last * ix.scaleFactor - ix.showLabelH > 0)
            {
                ui->verticalScrollBar_ImgShow->setMinimum(0);
                ui->verticalScrollBar_ImgShow->setMaximum((double)ix.RoiH_Last * ix.scaleFactor - ix.showLabelH);
                ui->verticalScrollBar_ImgShow->setPageStep(ix.showLabelH);
                ui->verticalScrollBar_ImgShow->setValue(ix.showLabelY);
                ui->verticalScrollBar_ImgShow->setVisible(true);
            }
            else
            {
                ui->verticalScrollBar_ImgShow->setVisible(false);
            }
        }
    }
}
/**
 * @brief EZCAP::zoom1X
 */
void EZCAP::zoom1X()
{
    if(ix.scaleFactor != 1.0 || ix.zoomMode != Zoom_SpecifyScaling)
    {
        ix.scaleFactor = 1.0;
        ix.zoomMode = Zoom_SpecifyScaling;

        if (ix.camStreamMode == 0)
        {
            scaleImage(ix.scaleFactor);
        }
        else if (ix.camStreamMode == 1)
        {
            if((double)ix.RoiW_Last * ix.scaleFactor - ix.showLabelW > 0)
            {
                ui->horizontalScrollBar_ImgShow->setMinimum(0);
                ui->horizontalScrollBar_ImgShow->setMaximum((double)ix.RoiW_Last * ix.scaleFactor - ix.showLabelW);
                ui->horizontalScrollBar_ImgShow->setPageStep(ix.showLabelW);
                ui->horizontalScrollBar_ImgShow->setValue(ix.showLabelX);
                ui->horizontalScrollBar_ImgShow->setVisible(true);
            }
            else
            {
                ui->horizontalScrollBar_ImgShow->setVisible(false);
            }
            if((double)ix.RoiH_Last * ix.scaleFactor - ix.showLabelH > 0)
            {
                ui->verticalScrollBar_ImgShow->setMinimum(0);
                ui->verticalScrollBar_ImgShow->setMaximum((double)ix.RoiH_Last * ix.scaleFactor - ix.showLabelH);
                ui->verticalScrollBar_ImgShow->setPageStep(ix.showLabelH);
                ui->verticalScrollBar_ImgShow->setValue(ix.showLabelY);
                ui->verticalScrollBar_ImgShow->setVisible(true);
            }
            else
            {
                ui->verticalScrollBar_ImgShow->setVisible(false);
            }
        }
    }
}

/**
 * @brief EZCAP::zoom1_5X
 */
void EZCAP::zoom1_5X()
{
    if(ix.scaleFactor != 1.5 || ix.zoomMode != Zoom_SpecifyScaling)
    {
        ix.scaleFactor = 1.5;
        ix.zoomMode = Zoom_SpecifyScaling;

        if (ix.camStreamMode == 0)
        {
            scaleImage(ix.scaleFactor);
        }
        else if (ix.camStreamMode == 1)
        {
            if((double)ix.RoiW_Last * ix.scaleFactor - ix.showLabelW > 0)
            {
                ui->horizontalScrollBar_ImgShow->setMinimum(0);
                ui->horizontalScrollBar_ImgShow->setMaximum((double)ix.RoiW_Last * ix.scaleFactor - ix.showLabelW);
                ui->horizontalScrollBar_ImgShow->setPageStep(ix.showLabelW);
                ui->horizontalScrollBar_ImgShow->setValue(ix.showLabelX);
                ui->horizontalScrollBar_ImgShow->setVisible(true);
            }
            else
            {
                ui->horizontalScrollBar_ImgShow->setVisible(false);
            }
            if((double)ix.RoiH_Last * ix.scaleFactor - ix.showLabelH > 0)
            {
                ui->verticalScrollBar_ImgShow->setMinimum(0);
                ui->verticalScrollBar_ImgShow->setMaximum((double)ix.RoiH_Last * ix.scaleFactor - ix.showLabelH);
                ui->verticalScrollBar_ImgShow->setPageStep(ix.showLabelH);
                ui->verticalScrollBar_ImgShow->setValue(ix.showLabelY);
                ui->verticalScrollBar_ImgShow->setVisible(true);
            }
            else
            {
                ui->verticalScrollBar_ImgShow->setVisible(false);
            }
        }
    }
}

/**
 * @brief EZCAP::zoom2X
 */
void EZCAP::zoom2X()
{
    if(ix.scaleFactor != 2.0 || ix.zoomMode != Zoom_SpecifyScaling)
    {
        ix.scaleFactor = 2.0;
        ix.zoomMode = Zoom_SpecifyScaling;

        if (ix.camStreamMode == 0)
        {
            scaleImage(ix.scaleFactor);
        }
        else if (ix.camStreamMode == 1)
        {
            if((double)ix.RoiW_Last * ix.scaleFactor - ix.showLabelW > 0)
            {
                ui->horizontalScrollBar_ImgShow->setMinimum(0);
                ui->horizontalScrollBar_ImgShow->setMaximum((double)ix.RoiW_Last * ix.scaleFactor - ix.showLabelW);
                ui->horizontalScrollBar_ImgShow->setPageStep(ix.showLabelW);
                ui->horizontalScrollBar_ImgShow->setValue(ix.showLabelX);
                ui->horizontalScrollBar_ImgShow->setVisible(true);
            }
            else
            {
                ui->horizontalScrollBar_ImgShow->setVisible(false);
            }
            if((double)ix.RoiH_Last * ix.scaleFactor - ix.showLabelH > 0)
            {
                ui->verticalScrollBar_ImgShow->setMinimum(0);
                ui->verticalScrollBar_ImgShow->setMaximum((double)ix.RoiH_Last * ix.scaleFactor - ix.showLabelH);
                ui->verticalScrollBar_ImgShow->setPageStep(ix.showLabelH);
                ui->verticalScrollBar_ImgShow->setValue(ix.showLabelY);
                ui->verticalScrollBar_ImgShow->setVisible(true);
            }
            else
            {
                ui->verticalScrollBar_ImgShow->setVisible(false);
            }
        }
    }
}

//--------------------------------------------------------------------------------
void EZCAP::favorite_pBtn_calibrateFrame_clicked()
{
    uint32_t ret = QHYCCD_ERROR;

    if(ix.isConnected && ix.workMode == WorkMode_Capture && ix.isCalibrateFrame == false)
    {
        qDebug() << "start calibrate frame...";
        ix.isCalibrateFrame = true;
        favorite_dialog->ui->pBtn_calibrateFrame->setStyleSheet("border-image: url(:/image/buttonDown.bmp);");
        managerMenu->ui->pBtn_capture->setEnabled(false);

        //camera has connected and it worked with capture mode
        ret = libqhyccd->QHYCCDI2CTwoWrite(camhandle,0x30BA,0x000b);
        if(ret == QHYCCD_ERROR)
        {
            OutputDebug("EZCAPWARNING | %s | %s | QHYCCDI2CTwoWrite() Failed!", __FILE__, __FUNCTION__);
        }

        ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_GAIN, 0.0);
        if(ret == QHYCCD_ERROR)
        {
            OutputDebug("EZCAPWARNING | %s | %s | SetQHYCCDParam() CONTROL_Gain Failed!", __FILE__, __FUNCTION__);
        }
        QThread::msleep(10);
        ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_GAIN, 1.0);
        if(ret == QHYCCD_ERROR)
        {
            OutputDebug("EZCAPWARNING | %s | %s | SetQHYCCDParam() CONTROL_Gain Failed!", __FILE__, __FUNCTION__);
        }
        QThread::msleep(10);
        ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_GAIN, 2.0);
        if(ret == QHYCCD_ERROR)
        {
            OutputDebug("EZCAPWARNING | %s | %s | SetQHYCCDParam() CONTROL_Gain Failed!", __FILE__, __FUNCTION__);
        }
        QThread::msleep(10);
        ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_GAIN, 3.0);
        if(ret == QHYCCD_ERROR)
        {
            OutputDebug("EZCAPWARNING | %s | %s | SetQHYCCDParam() CONTROL_Gain Failed!", __FILE__, __FUNCTION__);
        }
        QThread::msleep(10);
        ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_GAIN, 4.0);
        if(ret == QHYCCD_ERROR)
        {
            OutputDebug("EZCAPWARNING | %s | %s | SetQHYCCDParam() CONTROL_Gain Failed!", __FILE__, __FUNCTION__);
        }
        QThread::msleep(10);
        ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_GAIN, ix.Gain);
        if(ret == QHYCCD_ERROR)
        {
            OutputDebug("EZCAPWARNING | %s | %s | SetQHYCCDParam() CONTROL_Gain Failed!", __FILE__, __FUNCTION__);
        }

        //wait for the calibrate frame finished.
        QTime dieTime= QTime::currentTime().addSecs(3);
        while( QTime::currentTime() < dieTime )
        {
            QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
        }

        ret = libqhyccd->QHYCCDI2CTwoWrite(camhandle, 0x30BA, 0x000a);
        if(ret == QHYCCD_ERROR)
        {
            OutputDebug("EZCAPWARNING | %s | %s | QHYCCDI2CTwoWrite() Failed!", __FILE__, __FUNCTION__);
        }

        managerMenu->ui->pBtn_capture->setEnabled(true);
        favorite_dialog->ui->pBtn_calibrateFrame->setStyleSheet("border-image: url(:/image/button.bmp);");
        ix.isCalibrateFrame = false;
        qDebug() << "calibrate frame done.";
    }
}

void EZCAP::favorite_pBtn_getRealTemp_clicked()
{
    unsigned short temp70,temp55,altemp;
    double slope,T0;
    double temp;
    char chartemp[32];
    uint32_t ret = QHYCCD_ERROR;

    ret = libqhyccd->QHYCCDI2CTwoWrite(camhandle,0x30b4,0x11);
    if(ret == QHYCCD_ERROR)
    {
        OutputDebug("EZCAPWARNING | %s | %s | QHYCCDI2CTwoWrite() Failed!", __FILE__, __FUNCTION__);
    }
    temp70 = libqhyccd->QHYCCDI2CTwoRead(camhandle,0x30c6);
    temp55 = libqhyccd->QHYCCDI2CTwoRead(camhandle,0x30c8);
    altemp = libqhyccd->QHYCCDI2CTwoRead(camhandle,0x30b2);
    if(temp70 == temp55)
    {
        favorite_dialog->ui->pBtn_getRealTemp->setText(QString("error"));
    }
    else
    {
        slope = 15.0 / (temp70 - temp55);
        T0 = 70.0 - temp70*slope;

        temp = slope * altemp + T0;
        sprintf(chartemp,"%.2f",temp);
        favorite_dialog->ui->pBtn_getRealTemp->setText(QString(chartemp));
    }
}
//20201127 lyl SensorChamberCyclePUMP
void EZCAP::favorite_pBtn_controlSensorChamberCyclePUMP_clicked()
{
    uint32_t ret = QHYCCD_ERROR;

    if(favorite_dialog->ui->pBtn_controlSensorChamberCyclePUMP->text() == "SensorChamberCyclePUMP ON")
    {
        ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_SensorChamberCycle_PUMP, 1);
        if(ret == QHYCCD_ERROR)
        {
            OutputDebug("EZCAPWARNING | %s | %s | SetQHYCCDParam() CONTROL_SensorChanberCyclePUMB ON Failed!", __FILE__, __FUNCTION__);
        }

        if(favorite_dialog->ui->cBox_AutoTurnPUMBOff->isChecked())
        {
            this->startPumpTimer();
        }

        favorite_dialog->ui->pBtn_controlSensorChamberCyclePUMP->setText("SensorChamberCyclePUMP OFF");
    }
    else
    {
        ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_SensorChamberCycle_PUMP, 0);
        if(ret == QHYCCD_ERROR)
        {
            OutputDebug("EZCAPWARNING | %s | %s | SetQHYCCDParam() CONTROL_SensorChamberCyclePUMB OFF Failed!", __FILE__, __FUNCTION__);
        }

        if(favorite_dialog->ui->cBox_AutoTurnPUMBOff->isChecked())
        {
            this->stopPumpTimer();
        }

        favorite_dialog->ui->pBtn_controlSensorChamberCyclePUMP->setText("SensorChamberCyclePUMP ON");
    }
}

//--------------------------------------------------------------------------------

void EZCAP::setStretchLUT(unsigned short W, unsigned short B)
{
    double ratio;
    uint32_t pixel;

    ratio = double((W - B)) / 256.0;
    if(ratio == 0)
        ratio = 1;

    for(int i=0; i < 65536; i++)
    {
        if(i > B)
        {
            pixel = (i - B) / ratio;
            if(pixel > 255)
                pixel = 255;
        }
        else
        {
            pixel = 0;
        }

        ix.StretchLUT[i] = pixel;
    }

    if(ix.camStreamMode == 1) return;
    if(ix.imageReady != GetSingleFrame_Success) return;
    if(ix.workMode == ix.lastWorkMode && ix.workMode == WorkMode_Focus) return;

    if(!ImgShow.empty()) ImgShow.release();

    int s = 0, k = 0;
    if(!ix.Color_Fun)
    {
        ImgShow.create(cv::Size(ix.FrameW_Last, ix.FrameH_Last), CV_8UC3);
        for(uint32_t i = 0; i < ix.FrameH_Last; i++)
        {
            for(uint32_t j = 0; j < ix.FrameW_Last; j++)
            {
                ImgShow.data[k] = ix.StretchLUT[ix.ImgData_Last[s + 1] * 256 + ix.ImgData_Last[s]];
                ImgShow.data[k + 1] = ix.StretchLUT[ix.ImgData_Last[s + 1] * 256 + ix.ImgData_Last[s]];
                ImgShow.data[k + 2] = ix.StretchLUT[ix.ImgData_Last[s + 1] * 256 + ix.ImgData_Last[s]];
                s += 2;
                k += 3;
            }
            k += ImgShow.cols * (ImgShow.depth()==CV_8U?1:2) * ImgShow.channels() - ImgShow.channels() * ImgShow.cols;
        }
    }
    else if(ix.IsCvtColor)
    {
        ImgShow.create(cv::Size(ix.FrameW_Last, ix.FrameH_Last), CV_8UC1);
        for(uint32_t i = 0; i < ix.FrameW_Last * ix.FrameH_Last; i++)
        {
            ImgShow.data[i] = ix.StretchLUT[ix.ImgData_Last[2 * i + 1] * 256 + ix.ImgData_Last[2 * i]];
        }

        if(ix.Bayer == BAYER_GB)      cv::cvtColor(ImgShow, ImgShow, COLOR_BayerGB2BGR_VNG);
        else if(ix.Bayer == BAYER_GR) cv::cvtColor(ImgShow, ImgShow, COLOR_BayerGR2BGR_VNG);
        else if(ix.Bayer == BAYER_BG) cv::cvtColor(ImgShow, ImgShow, COLOR_BayerBG2BGR_VNG);
        else if(ix.Bayer == BAYER_RG) cv::cvtColor(ImgShow, ImgShow, COLOR_BayerRG2BGR_VNG);
    }
    else if(!ix.IsCvtColor)
    {
        ImgShow.create(cv::Size(ix.FrameW_Last, ix.FrameH_Last), CV_8UC3);

        for(uint32_t j = 0; j < ix.FrameH_Last; j++)
        {
            for(uint32_t i = 0; i < ix.FrameW_Last; i++)
            {
                ImgShow.data[k]     = ix.StretchLUT[ix.ImgData_Last[s + 1] * 256 + ix.ImgData_Last[s]];
                ImgShow.data[k + 1] = ix.StretchLUT[ix.ImgData_Last[s + 1] * 256 + ix.ImgData_Last[s]];
                ImgShow.data[k + 2] = ix.StretchLUT[ix.ImgData_Last[s + 1] * 256 + ix.ImgData_Last[s]];
                s += 2;
                k += 3;
            }
            k += ImgShow.cols * (ImgShow.depth()==CV_8U?1:2) * ImgShow.channels() - ImgShow.channels() * ImgShow.cols;
        }
    }

    if(!ImgView.empty()) ImgView.release();

    if(ix.Color_Fun && !ix.IsCvtColor)
    {
        ImgView.create(ImgShow.size(), CV_8UC1);
        int s = 0, k = 0;
        for(int j = 0; j < ImgShow.rows; j++)
        {
            for(int i = 0; i < ImgShow.cols; i++)
            {
                ImgView.data[s] = ImgShow.data[k];
                s += 1;
                k += 3;
            }
        }

        if(ix.Bayer == BAYER_GB)      cv::cvtColor(ImgView, ImgView, COLOR_BayerGB2GRAY);
        else if(ix.Bayer == BAYER_GR) cv::cvtColor(ImgView, ImgView, COLOR_BayerGR2GRAY);
        else if(ix.Bayer == BAYER_BG) cv::cvtColor(ImgView, ImgView, COLOR_BayerBG2GRAY);
        else if(ix.Bayer == BAYER_RG) cv::cvtColor(ImgView, ImgView, COLOR_BayerRG2GRAY);
        cv::cvtColor(ImgView, ImgView, COLOR_GRAY2RGB);
    }
    else
    {
        ImgView.create(ImgShow.size(), ImgShow.type());
        ImgShow.copyTo(ImgView);
    }

    int w = managerMenu->ui->img_screenView->width(); //249
    int h = managerMenu->ui->img_screenView->height(); //181
    cv::resize(ImgView, ImgView, cv::Size(w, h), 0, 0, CV_INTER_LINEAR);
}

//********************************************************************************
//                              preview tab页操作
//********************************************************************************
/**
 * @brief EZCAP::mgrMenu_pBtn_cross_clicked
 */
void EZCAP::mgrMenu_pBtn_cross_clicked()
{
    if (ix.crossBtnState == Cross_Disabled)
    {
        managerMenu->ui->pBtn_cross->setText("+");
        ix.crossBtnState = Cross_Enabled;
    }
    else if(ix.crossBtnState == Cross_Enabled)
    {
        managerMenu->ui->pBtn_cross->setText(tr("Cross"));
        ix.crossBtnState = Cross_Disabled;
    }

    if(ix.workMode == WorkMode_Preview && ix.lastWorkMode == ix.workMode && ix.imageReady == GetSingleFrame_Success)
    {   //if already had image, redisplay it
        displaySingleFrame(ix.FrameW_Last, ix.FrameH_Last, ix.ImgData_Last);
    }
}

/**
 * @brief EZCAP::mgrMenu_pBtn_grid_clicked
 */
void EZCAP::mgrMenu_pBtn_grid_clicked()
{
    if (ix.gridBtnState == Grid_Disabled)
    {
        managerMenu->ui->pBtn_grid->setText(QString::fromUtf8("▓▓"));
        ix.gridBtnState = Grid_Enabled;
    }
    else if(ix.gridBtnState == Grid_Enabled)
    {
        managerMenu->ui->pBtn_grid->setText(tr("Grid"));
        ix.gridBtnState = Grid_Disabled;
    }

    if(ix.workMode == WorkMode_Preview && ix.lastWorkMode == ix.workMode && ix.imageReady == GetSingleFrame_Success)
    {
        displaySingleFrame(ix.FrameW_Last, ix.FrameH_Last, ix.ImgData_Last);
    }
}

/**
 * @brief EZCAP::mgrMenu_pBtn_circle_clicked
 */
void EZCAP::mgrMenu_pBtn_circle_clicked()
{
    if (ix.circleBtnState == Circle_Disabled)
    {
        managerMenu->ui->pBtn_circle->setText(QString::fromUtf8("◎"));
        ix.circleBtnState = Circle_Enabled;
    }
    else if(ix.circleBtnState == Circle_Enabled)
    {
        managerMenu->ui->pBtn_circle->setText(tr("Circle"));
        ix.circleBtnState = Circle_Disabled;
    }

    if(ix.workMode == WorkMode_Preview && ix.lastWorkMode == ix.workMode && ix.imageReady == GetSingleFrame_Success)
    {
        displaySingleFrame(ix.FrameW_Last, ix.FrameH_Last, ix.ImgData_Last);
    }
}

/**
 * @brief EZCAP::mgrMenu_pBtn_preview_clicked
 */
void EZCAP::mgrMenu_pBtn_preview_clicked()
{
    unsigned int ret;
    QElapsedTimer prev_time;
    int prev_betweenTime;

    managerMenu->ui->pBtn_live_preview->setEnabled(false);
    managerMenu->ui->pBtn_preview->setChecked(true);
    managerMenu->ui->head_capture->setCheckable(false);
    managerMenu->ui->head_focus->setCheckable(false);

    memset(ix.ImgData,          0, ix.ImageW_Max * ix.ImageH_Max * 2);

    if(ix.cameraState == Camera_Idle)
    {
        ix.cameraState = Camera_Waiting;
        ix.imageReady = GetSingleFrame_Waiting;

        //---set params
        ix.onLiveMode = false;

        if(ix.Gain_Fun && managerMenu->ui->hSlider_Gain_preview->value() != ix.Gain_Last)
        {
            ret = libqhyccd->SetQHYCCDParam(camhandle,CONTROL_GAIN, managerMenu->ui->hSlider_Gain_preview->value());
            if(ret == QHYCCD_SUCCESS)
            {
                ix.Gain_Last = managerMenu->ui->hSlider_Gain_preview->value();
            }
            else
            {
                DBGOPT_WARNING("SetQHYCCDParam() CONTROL_Gain Failed! | Gain = %f", ix.Gain);
            }
        }

        if(ix.Offset_Fun && managerMenu->ui->hSlider_Offset_preview->value() != ix.Offset_Last)
        {
            ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_OFFSET, managerMenu->ui->hSlider_Offset_preview->value());
            if(ret == QHYCCD_SUCCESS)
            {
                ix.Offset_Last = managerMenu->ui->hSlider_Offset_preview->value();
            }
            else
            {
                DBGOPT_WARNING("SetQHYCCDParam() CONTROL_Offset Failed! | Offset = %f", ix.Offset);
            }
        }

        if(1000.0 != ix.ExpUnit_Last || managerMenu->ui->hSlider_exposure_preview->value() != ix.ExpTime_Last)
        {
            ret = libqhyccd->SetQHYCCDParam(camhandle,CONTROL_EXPOSURE, managerMenu->ui->hSlider_exposure_preview->value() * 1000.0);
            if(ret == QHYCCD_SUCCESS)
            {
                ix.ExpUnit_Last = 1000.0;
                ix.ExpTime_Last = managerMenu->ui->hSlider_exposure_preview->value();
            }
            else
            {
                DBGOPT_WARNING("SetQHYCCDParam() CONTROL_EXPOSURE Failed! | ExpTime = %f", ix.ExpTime * ix.ExpUnit);
            }
        }

        if(ix.Traffic_Fun && ix.Traffic_Last != ix.Traffic)
        {
            ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_USBTRAFFIC, ix.Traffic);
            if(ret == QHYCCD_SUCCESS)
            {
                ix.Traffic_Last = ix.Traffic;
            }
            else
            {
                DBGOPT_WARNING("SetQHYCCDParam() CONTROL_Traffic Failed! | Traffic = %f", ix.Traffic);
            }
        }

        if(ix.Speed_Fun && ix.Speed_Last != 1)
        {
            ret = libqhyccd->SetQHYCCDParam(camhandle,CONTROL_SPEED, 1.0);
            if(ret == QHYCCD_SUCCESS)
            {
                ix.Speed_Last = 1;
            }
            else
            {
                DBGOPT_WARNING("SetQHYCCDParam() CONTROL_SPEED Failed! | speed = %f", 1.0);
            }
        }

        if(ix.RoiX != ix.RoiX_Last || ix.RoiY != ix.RoiY_Last || ix.RoiW != ix.RoiW_Last || ix.RoiH != ix.RoiH_Last)
        {
            ret = libqhyccd->SetQHYCCDResolution(camhandle, ix.RoiX, ix.RoiY, ix.RoiW, ix.RoiH);
            if(ret != QHYCCD_SUCCESS)
            {
                DBGOPT_WARNING("SetQHYCCDResolution() Failed! | startx = %d starty = %d sizex = %d sizey = %d", ix.RoiX, ix.RoiY, ix.RoiW, ix.RoiH);
            }
            else
            {
                ix.RoiX_Last = ix.RoiX;
                ix.RoiY_Last = ix.RoiY;
                ix.RoiW_Last = ix.RoiW;
                ix.RoiH_Last = ix.RoiH;
            }
        }

        ix.dateOBS = QDateTime::currentDateTime().toString(Qt::ISODate);  //记录当前拍摄时间戳

        //进度条清0
        managerMenu->ui->proBar_preview->setValue(0);
        managerMenu->ui->proBar_previewTime->setValue(0);

        ix.cameraState = Camera_Exposing;
        prev_time.start();

        ret = libqhyccd->ExpQHYCCDSingleFrame(camhandle);
        if(ret == QHYCCD_SUCCESS)
        {
            qDebug() <<"ExpQHYCCDSingleFrame success, wait...A";
            QThread::msleep(200);
        }
        else if(ret == QHYCCD_READ_DIRECTLY)
        {
            qDebug() << "ExpQHYCCDSingleFrame QHYCCD_READ_DIRECTLY!";
        }
        else
        {
            OutputDebug("EZCAPWARNING | %s | %s | ExpQHYCCDSingleFrame() Failed!", __FILE__, __FUNCTION__);
        }

        prev_betweenTime = prev_time.elapsed();//返回从上次start()或restart()开始以来的时间差，单位ms
        while (prev_betweenTime < (ix.ExpTime*ix.ExpUnit/1000))
        {
            if((prev_betweenTime + 1000) >= (ix.ExpTime*ix.ExpUnit/1000))
            {
                break;
            }
            prev_betweenTime = prev_time.elapsed();
            managerMenu->ui->proBar_previewTime->setValue(prev_betweenTime*100/(ix.ExpTime*ix.ExpUnit)*1000);
            QApplication::processEvents();//防止长时间导致界面假死
            QThread::msleep(1);
        }
        managerMenu->ui->proBar_previewTime->setValue(100);

        ix.cameraState = Camera_Reading;
        downloadPre = new DownloadPreThread(this);
        connect(downloadPre, SIGNAL(finished()), downloadPre, SLOT(deleteLater()));
        downloadPre->start();

        if(ix.Cooler_Fun)
        {
//            mainWidget->stopTimerTemp();//停止温控定时器
            threadTempControl->suspend();
        }

        while(ix.imageReady == GetSingleFrame_Waiting)
        {
            //处理下载进度条
            managerMenu->ui->proBar_preview->setValue(libqhyccd->GetQHYCCDReadingProgress(camhandle));
            QThread::msleep(1);
            QApplication::processEvents();//响应界面操作，防止界面假死
        }

        managerMenu->ui->proBar_preview->setValue(100);

        if(ix.Cooler_Fun)
        {
//            mainWidget->startTimerTemp();//开启温控定时器
            threadTempControl->resume();
        }

        if(ix.imageReady == GetSingleFrame_Success)
        {
            emit change_fitHeaderInfo();  //图像拍摄成功，刷新FitHeader信息

            if(ix.workMode != ix.lastWorkMode)
            {
                ix.lastWorkMode = ix.workMode;
            }

            int wpos = 0, bpos = 0;
            if(ix.workMode == WorkMode_Preview)
            {
                wpos = Preview_WPOS;
                bpos = Preview_BPOS;
            }
            else
            {
                wpos = Capture_WPOS;
                bpos = Capture_BPOS;
            }
            managerMenu->ui->hSlider_bPos->setValue(bpos);
            managerMenu->ui->hSlider_wPos->setValue(wpos);
            setStretchLUT(wpos, bpos);  //set the stretch LUT value

            //display preview image
            displaySingleFrame(ix.FrameW_Last, ix.FrameH_Last, ix.ImgData_Last);

            if(ix.zoomMode == Zoom_FitWindow || ix.zoomMode == Zoom_FillWindow)
            {
                viewBoxW = managerMenu->ui->img_screenView->width();
                viewBoxH = managerMenu->ui->img_screenView->height();
            }
            else
            {
                viewBoxW = managerMenu->ui->img_screenView->width()  * scrollArea_ImgShow->width()  / (ix.FrameW_Last * ix.scaleFactor);
                viewBoxH = managerMenu->ui->img_screenView->height() * scrollArea_ImgShow->height() / (ix.FrameH_Last * ix.scaleFactor);
                viewBoxCX = managerMenu->ui->img_screenView->width()  * (double)ix.showLabelX / ix.scaleFactor / ix.FrameW_Last + viewBoxW / 2;
                viewBoxCY = managerMenu->ui->img_screenView->height() * (double)ix.showLabelY / ix.scaleFactor / ix.FrameH_Last + viewBoxH / 2;
            }

            displayScreenViewImage(viewBoxW, viewBoxH, viewBoxCX, viewBoxCY);

            //display the histogram image
            displayHistogramImage(ix.FrameW_Last, ix.FrameH_Last, ix.ImgData_Last);

            //get overscan balck value, used for auto histogram
            getOverScanBlack(ix.ImgData_Last, ix.FrameW_Last, ix.FrameH_Last);

            QString str1 = QString::number(ix.FrameW_Last) + "x" + QString::number(ix.FrameH_Last);
            statusLabel_imgSize->setText(str1);
            showFrameCount();

            noImgInWorkMode = false;
        }

        qDebug() << "----preview end----";
        managerMenu->ui->pBtn_preview->setChecked(false);
        managerMenu->ui->pBtn_live_preview->setEnabled(true);
        managerMenu->ui->head_capture->setCheckable(true);
        managerMenu->ui->head_focus->setCheckable(true);

        ix.cameraState = Camera_Idle;

        mainMenuBar->actOpenFolder->setEnabled(true);
        mainMenuBar->actSaveBMP->setEnabled(true);//设置保存图像菜单项可用
        mainMenuBar->actSaveFIT->setEnabled(true);
        mainMenuBar->actSaveJPG->setEnabled(true);
        mainMenuBar->actSavePNG->setEnabled(true);
        mainMenuBar->actSaveTIF->setEnabled(true);
    }
}

/**
 * @brief EZCAP::mgrMenu_pBtn_live_preview_clicked
 */
void EZCAP::mgrMenu_pBtn_live_preview_clicked()
{
    ix.onLiveMode = !ix.onLiveMode;

    if(ix.cameraState == Camera_Idle && ix.onLiveMode)
    {
        unsigned int ret;
        QElapsedTimer sTime;

        managerMenu->ui->pBtn_live_preview->setChecked(true);//设置live按钮处于按下状态
        managerMenu->ui->pBtn_preview->setEnabled(false);//禁用preview按钮
        managerMenu->ui->head_capture->setCheckable(false);
        managerMenu->ui->head_focus->setCheckable(false);

        memset(ix.ImgData,          0, ix.ImageW_Max * ix.ImageH_Max * 2);

        ix.cameraState = Camera_Waiting;

        while(ix.onLiveMode)
        {
            ix.imageReady = GetSingleFrame_Waiting;

            if(ix.Gain_Fun && managerMenu->ui->hSlider_Gain_preview->value() != ix.Gain_Last)
            {
                ret = libqhyccd->SetQHYCCDParam(camhandle,CONTROL_GAIN, managerMenu->ui->hSlider_Gain_preview->value());
                if(ret == QHYCCD_SUCCESS)
                {
                    ix.Gain_Last = managerMenu->ui->hSlider_Gain_preview->value();
                }
                else
                {
                    DBGOPT_WARNING("SetQHYCCDParam() CONTROL_Gain Failed! | Gain = %f", ix.Gain);
                }
            }

            if(ix.Offset_Fun && managerMenu->ui->hSlider_Offset_preview->value() != ix.Offset_Last)
            {
                ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_OFFSET, managerMenu->ui->hSlider_Offset_preview->value());
                if(ret == QHYCCD_SUCCESS)
                {
                    ix.Offset_Last = managerMenu->ui->hSlider_Offset_preview->value();
                }
                else
                {
                    DBGOPT_WARNING("SetQHYCCDParam() CONTROL_Offset Failed! | Offset = %f", ix.Offset);
                }
            }

            if(1000.0 != ix.ExpUnit_Last || managerMenu->ui->hSlider_exposure_preview->value() != ix.ExpTime_Last)
            {
                ret = libqhyccd->SetQHYCCDParam(camhandle,CONTROL_EXPOSURE, managerMenu->ui->hSlider_exposure_preview->value() * 1000.0);
                if(ret == QHYCCD_SUCCESS)
                {
                    ix.ExpUnit_Last = 1000.0;
                    ix.ExpTime_Last = managerMenu->ui->hSlider_exposure_preview->value();
                }
                else
                {
                    OutputDebug("EZCAPWARNING | %s | %s | SetQHYCCDParam() CONTROL_EXPOSURE Failed! | ExpTime = %f", __FILE__, __FUNCTION__, ix.ExpTime * ix.ExpUnit);
                }
            }

            if(ix.Traffic_Fun && ix.Traffic_Last != ix.Traffic)
            {
                ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_USBTRAFFIC, ix.Traffic);
                if(ret == QHYCCD_SUCCESS)
                {
                    ix.Traffic_Last = ix.Traffic;
                }
                else
                {
                    OutputDebug("EZCAPWARNING | %s | %s | SetQHYCCDParam() CONTROL_Traffic Failed! | traffic = %f", __FILE__, __FUNCTION__, ix.Traffic);
                }
            }

            if(ix.Speed_Fun && ix.Speed_Last != 1.0)
            {
                ret = libqhyccd->SetQHYCCDParam(camhandle,CONTROL_SPEED, 1.0); //set speed
                if(ret == QHYCCD_SUCCESS)
                {
                    ix.Speed_Last = 1.0;
                }
                else
                {
                    OutputDebug("EZCAPWARNING | %s | %s | SetQHYCCDParam() CONTROL_SPEED Failed! | speed = %f", __FILE__, __FUNCTION__, ix.Speed);
                }
            }

            if(ix.RoiX != ix.RoiX_Last || ix.RoiY != ix.RoiY_Last || ix.RoiW != ix.RoiW_Last || ix.RoiH != ix.RoiH_Last)
            {
                ret = libqhyccd->SetQHYCCDResolution(camhandle, ix.RoiX, ix.RoiY, ix.RoiW, ix.RoiH);
                if(ret != QHYCCD_SUCCESS)
                {
                    DBGOPT_WARNING("SetQHYCCDResolution() Failed! | startx = %d starty = %d sizex = %d sizey = %d", ix.RoiX, ix.RoiY, ix.RoiW, ix.RoiH);
                }
                else
                {
                    ix.RoiX_Last = ix.RoiX;
                    ix.RoiY_Last = ix.RoiY;
                    ix.RoiW_Last = ix.RoiW;
                    ix.RoiH_Last = ix.RoiH;
                }
            }

            ix.dateOBS = QDateTime::currentDateTime().toString(Qt::ISODate);  //记录当前拍摄时间戳
            ix.cameraState = Camera_Exposing;
            sTime.start();

            ret = libqhyccd->ExpQHYCCDSingleFrame(camhandle);
            if(ret == QHYCCD_SUCCESS)
            {
                QThread::msleep(200);
            }
            else if(ret == QHYCCD_READ_DIRECTLY)
            {
                qDebug() << "ExpQHYCCDSingleFrame QHYCCD_READ_DIRECTLY!";
            }
            else
            {
                OutputDebug("EZCAPWARNING | %s | %s | ExpQHYCCDSingleFrame() Failed!", __FILE__, __FUNCTION__);
            }

            //进度条复位
            managerMenu->ui->proBar_previewTime->setValue(0);
            managerMenu->ui->proBar_preview->setValue(0);

            int prev_betweenTime = sTime.elapsed();//返回从上次start()或restart()开始以来的时间差，单位ms
            while (prev_betweenTime < (ix.ExpTime*ix.ExpUnit/1000))
            {
                if((prev_betweenTime + 1000) >= (ix.ExpTime*ix.ExpUnit/1000))
                {
                    break;
                }
                prev_betweenTime = sTime.elapsed();
                managerMenu->ui->proBar_previewTime->setValue(prev_betweenTime*100/(ix.ExpTime*ix.ExpUnit)*1000);
                QApplication::processEvents();//防止长时间导致界面假死
                QThread::msleep(1);
            }
            managerMenu->ui->proBar_previewTime->setValue(100);

            //开启线程，获取图像数据
            ix.cameraState = Camera_Reading;
            downloadPre = new DownloadPreThread(this);
            connect(downloadPre, SIGNAL(finished()), downloadPre, SLOT(deleteLater()));
            downloadPre->start();

            if(ix.Cooler_Fun)
            {
                threadTempControl->suspend();
            }

            while(ix.imageReady == GetSingleFrame_Waiting)
            {
                //处理下载进度条
                managerMenu->ui->proBar_preview->setValue(libqhyccd->GetQHYCCDReadingProgress(camhandle));
                QThread::msleep(1);
                QApplication::processEvents();//响应界面操作，防止界面假死
            }
            managerMenu->ui->proBar_preview->setValue(100);

            if(ix.Cooler_Fun)
            {
                threadTempControl->resume();
            }

            if(ix.imageReady == GetSingleFrame_Success)
            {
                emit change_fitHeaderInfo();  //图像拍摄成功，刷新FitHeader信息

                if(ix.workMode != ix.lastWorkMode)
                {
                    ix.lastWorkMode = ix.workMode;
                }

                int wpos = 0, bpos = 0;
                DBGOPT_INFO("Preview WPOS = %d BPOS = %d Capture WPOS = %d BPOS = %d", Preview_WPOS, Preview_BPOS, Capture_WPOS, Capture_BPOS);
                if(ix.workMode == WorkMode_Preview)
                {
                    wpos = Preview_WPOS;
                    bpos = Preview_BPOS;
                }
                else
                {
                    wpos = Capture_WPOS;
                    bpos = Capture_BPOS;
                }
                managerMenu->ui->hSlider_bPos->setValue(bpos);
                managerMenu->ui->hSlider_wPos->setValue(wpos);
                setStretchLUT(wpos, bpos);  //set the stretch LUT value

                //显示图像
                frame_count++;
                mainWidget->statusLabel_frame_status->setText(QString::number(frame_count));

                displaySingleFrame(ix.FrameW_Last, ix.FrameH_Last, ix.ImgData_Last);

                if(ix.zoomMode == Zoom_FitWindow || ix.zoomMode == Zoom_FillWindow)
                {
                    viewBoxW = managerMenu->ui->img_screenView->width();
                    viewBoxH = managerMenu->ui->img_screenView->height();
                }
                else
                {
                    viewBoxW = managerMenu->ui->img_screenView->width()  * scrollArea_ImgShow->width()  / (ix.FrameW_Last * ix.scaleFactor);
                    viewBoxH = managerMenu->ui->img_screenView->height() * scrollArea_ImgShow->height() / (ix.FrameH_Last * ix.scaleFactor);
                    viewBoxCX = managerMenu->ui->img_screenView->width()  * (double)ix.showLabelX / ix.scaleFactor / ix.FrameW_Last + viewBoxW / 2;
                    viewBoxCY = managerMenu->ui->img_screenView->height() * (double)ix.showLabelY / ix.scaleFactor / ix.FrameH_Last + viewBoxH / 2;
                }
                displayScreenViewImage(viewBoxW, viewBoxH, viewBoxCX, viewBoxCY);

                //显示直方图
                displayHistogramImage(ix.FrameW_Last, ix.FrameH_Last, ix.ImgData_Last);

                //get the overscan balck value, used for auto histogram
                getOverScanBlack(ix.ImgData_Last, ix.FrameW_Last, ix.FrameH_Last);

                QString str1 = QString::number(ix.FrameW_Last) + "x" + QString::number(ix.FrameH_Last);
                statusLabel_imgSize->setText(str1);
                showFrameCount();

                noImgInWorkMode = false;
            }

            QApplication::processEvents();//响应界面是否发送了停止信号
        }

        ix.cameraState = Camera_Idle;

        managerMenu->ui->pBtn_live_preview->setChecked(false);
        managerMenu->ui->pBtn_preview->setEnabled(true);
        managerMenu->ui->head_capture->setCheckable(true);
        managerMenu->ui->head_focus->setCheckable(true);

        mainMenuBar->actOpenFolder->setEnabled(true);
        mainMenuBar->actSaveBMP->setEnabled(true);//设置保存图像菜单项可用
        mainMenuBar->actSaveFIT->setEnabled(true);
        mainMenuBar->actSaveJPG->setEnabled(true);
        mainMenuBar->actSavePNG->setEnabled(true);
        mainMenuBar->actSaveTIF->setEnabled(true);
    }
}

/**
 * @brief getOverScanBlack
 * @param Buf
 * @param x
 * @param y
 */
void EZCAP::getOverScanBlack(unsigned char *Buf, int x,int y)
{
    float std,rms;
    double max,min;
    int ret;
    unsigned int pxOB,pyOB,sxOB,syOB;//存放GetOverScanArea获取的数据

    pxOB = pyOB = sxOB = syOB = 0;
    //调用SDK，获取overscan区域
    ret = libqhyccd->GetQHYCCDOverScanArea(camhandle,&pxOB, &pyOB, &sxOB, &syOB);
    if(ret != QHYCCD_SUCCESS)
    {
        DBGOPT_WARNING("GetQHYCCDOverScanArea() Failed!");
    }

    getImageInfo(Buf, x, y, pxOB, pyOB, sxOB, syOB, std, rms, max, min);

    OverScanRMS = rms;
}
//------------------------------------------------------------------------------------

//*********************************************************************************************************
//                             focus tab页操作
//*********************************************************************************************************
/**
 * @brief EZCAP::mgrMenu_pBtn_focus_clicked
 */
void EZCAP::mgrMenu_pBtn_focus_clicked()
{
    unsigned int ret;

    managerMenu->ui->pBtn_focus->setChecked(true);//设置处于按下状态
    managerMenu->ui->pBtn_live_focus->setEnabled(false);//禁用focus tab中live按钮
    managerMenu->ui->head_preview->setCheckable(false);
    managerMenu->ui->head_capture->setCheckable(false);

    memset(ix.ImgData, 0, ix.ImageW_Max * ix.ImageH_Max * 2);

    if(ix.cameraState == Camera_Idle)
    {
        ix.cameraState = Camera_Waiting;
        ix.imageReady = GetSingleFrame_Waiting;

        if(ix.Gain_Fun && managerMenu->ui->hSlider_Gain_focus->value() != ix.Gain_Last)
        {
            ret = libqhyccd->SetQHYCCDParam(camhandle,CONTROL_GAIN, managerMenu->ui->hSlider_Gain_focus->value());
            if(ret == QHYCCD_SUCCESS)
            {
                ix.Gain_Last = managerMenu->ui->hSlider_Gain_focus->value();
            }
            else
            {
                DBGOPT_WARNING("SetQHYCCDParam() CONTROL_GAIN Failed! | gain = %f", managerMenu->ui->hSlider_Gain_focus->value());
            }
        }

        if(ix.Offset_Fun && managerMenu->ui->hSlider_Offset_focus->value() != ix.Offset_Last)
        {
            ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_OFFSET, managerMenu->ui->hSlider_Offset_focus->value());
            if(ret == QHYCCD_SUCCESS)
            {
                ix.Offset_Last = managerMenu->ui->hSlider_Offset_focus->value();
            }
            else
            {
                DBGOPT_WARNING("SetQHYCCDParam() CONTROL_OFFSET Failed! | offset = %d", managerMenu->ui->hSlider_Offset_focus->value());
            }
        }

        if(ix.ExpUnit_Last != 1000.0 || managerMenu->ui->hSlider_exposure_focus->value() != ix.ExpTime_Last)
        {
            ret = libqhyccd->SetQHYCCDParam(camhandle,CONTROL_EXPOSURE, managerMenu->ui->hSlider_exposure_focus->value()*1000.0);
            if(ret == QHYCCD_SUCCESS)
            {
                ix.ExpUnit_Last = 1000.0;
                ix.ExpTime_Last = managerMenu->ui->hSlider_exposure_focus->value();
            }
            else
            {
                DBGOPT_WARNING("SetQHYCCDParam() CONTROL_EXPOSURE Failed! | ExpTime = %f", managerMenu->ui->hSlider_exposure_focus->value()*1000.0);
            }
        }

        if(ix.Traffic_Fun && ix.Traffic_Last != ix.Traffic)
        {
            ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_USBTRAFFIC, ix.Traffic);
            if(ret == QHYCCD_SUCCESS)
            {
                ix.Traffic_Last = ix.Traffic;
            }
            else
            {
                DBGOPT_WARNING("SetQHYCCDParam() CONTROL_USBTRAFFIC Failed! | traffic = %f", ix.Traffic);
            }
        }

        if(ix.Speed_Fun && ix.Speed_Last != 1.0)
        {
            ret = libqhyccd->SetQHYCCDParam(camhandle,CONTROL_SPEED, 1.0);
            if(ret == QHYCCD_SUCCESS)
            {
                ix.Speed_Last = 1.0;
            }
            else
            {
                DBGOPT_WARNING("SetQHYCCDParam() CONTROL_SPEED Failed! | speed = %f", 1.0);
            }
        }

        if(ix.BinX_Last != 1 || ix.BinY_Last != 1)
        {
            ret = libqhyccd->SetQHYCCDBinMode(camhandle, 1, 1);
            if(ret == QHYCCD_SUCCESS)
            {
                ix.BinX_Last = 1;
                ix.BinY_Last = 1;
            }
            else
            {
                DBGOPT_WARNING("SetQHYCCDBinMode() Failed! | binx = %d biny = %d", 1, 1);
            }
        }

        //set focus area position
        focusAreaStartX = 0;
        focusAreaStartY = FocusCenterY_Pre * ix.BinY_Max - 100;
        focusAreaSizeX  = ix.ImageW_Min;
        focusAreaSizeY  = 200;
        if(focusAreaStartX < 0) focusAreaSizeX  = 0;
        if(focusAreaStartY < 0) focusAreaStartY = 0;
        if(focusAreaSizeY  > (int)ix.ImageH_Min ) focusAreaSizeY = (int)ix.ImageH_Min;
        if(focusAreaStartY + focusAreaSizeY > (int)ix.ImageH_Min) focusAreaStartY = (int)ix.ImageH_Min - focusAreaSizeY;
        ix.RoiX = focusAreaStartX;
        ix.RoiY = focusAreaStartY;
        ix.RoiW = focusAreaSizeX;
        ix.RoiH = focusAreaSizeY;
        DBGOPT_INFO("RoiX = %d RoiY = %d RoiW = %d RoiH = %d", ix.RoiX, ix.RoiY, ix.RoiW, ix.RoiH);
        if(ix.RoiX != ix.RoiX_Last || ix.RoiY != ix.RoiY_Last || ix.RoiW != ix.RoiW_Last || ix.RoiH != ix.RoiH_Last)
        {
            ret = libqhyccd->SetQHYCCDResolution(camhandle, ix.RoiX, ix.RoiY, ix.RoiW, ix.RoiH);
            if(ret != QHYCCD_SUCCESS)
            {
                DBGOPT_WARNING("SetQHYCCDResolution() Failed! | startx = %d starty = %d sizex = %d sizey = %d", ix.RoiX, ix.RoiY, ix.RoiW, ix.RoiH);
            }
            else
            {
                ix.RoiX_Last = ix.RoiX;
                ix.RoiY_Last = ix.RoiY;
                ix.RoiW_Last = ix.RoiW;
                ix.RoiH_Last = ix.RoiH;
            }
        }

        ix.dateOBS = QDateTime::currentDateTime().toString(Qt::ISODate);  //记录当前拍摄时间戳

        ix.cameraState = Camera_Exposing;

        qCritical("------------- E3");
        ret = libqhyccd->ExpQHYCCDSingleFrame(camhandle);
        if(ret == QHYCCD_SUCCESS)
        {
            qDebug() <<"ExpQHYCCDSingleFrame success, wait...C";
            QThread::msleep(200);
        }
        else if(ret == QHYCCD_READ_DIRECTLY)
        {
            qDebug() << "ExpQHYCCDSingleFrame QHYCCD_READ_DIRECTLY!";
        }
        else
        {
            OutputDebug("EZCAPWARNING | %s | %s | ExpQHYCCDSingleFrame() Failed!", __FILE__, __FUNCTION__);
        }

        //获取图像
        qDebug() << "donwnloading focus frame";
        ix.cameraState = Camera_Reading;
        downloadFoc = new DownloadFocThread(this);
        connect(downloadFoc, SIGNAL(finished()), downloadFoc, SLOT(deleteLater()));
        downloadFoc->start();

        while(ix.imageReady == GetSingleFrame_Waiting)
        {
            QApplication::processEvents();//响应界面操作，防止界面假死
        }

        if(ix.imageReady == GetSingleFrame_Success)
        {
            emit change_fitHeaderInfo();  //图像拍摄成功，刷新FitHeader信息

            if(ix.workMode != ix.lastWorkMode)
            {
                ix.lastWorkMode = ix.workMode;
            }

            int wpos = 0, bpos = 0;
            if(ix.workMode == WorkMode_Preview)
            {
                wpos = Preview_WPOS;
                bpos = Preview_BPOS;
            }
            else if(ix.workMode == WorkMode_Focus)
            {
                wpos = Focus_WPOS;
                bpos = Focus_BPOS;
            }
            else
            {
                wpos = Capture_WPOS;
                bpos = Capture_BPOS;
            }
            managerMenu->ui->hSlider_bPos->setValue(bpos);
            managerMenu->ui->hSlider_wPos->setValue(wpos);
            setStretchLUT(wpos, bpos);  //set the stretch LUT value

            displayFocusImage_Ex(ix.FrameW_Last, ix.FrameH_Last, ix.ImgData_Last);

            //显示直方图
            displayHistogramImage(ix.FrameW, ix.FrameH, ix.ImgData);

            //获取overscan balck值
            getOverScanBlack(ix.ImgData, ix.FrameW, ix.FrameH);

            noImgInWorkMode = false;
        }

        ix.cameraState = Camera_Idle;

        managerMenu->ui->pBtn_focus->setChecked(false);
        managerMenu->ui->pBtn_live_focus->setEnabled(true);
        managerMenu->ui->head_preview->setCheckable(true);
        managerMenu->ui->head_capture->setCheckable(true);

        mainMenuBar->actOpenFolder->setEnabled(true);
        mainMenuBar->actSaveBMP->setEnabled(true);//设置保存图像菜单项是否可用
        mainMenuBar->actSaveFIT->setEnabled(true);
        mainMenuBar->actSaveJPG->setEnabled(true);
        mainMenuBar->actSavePNG->setEnabled(true);
        mainMenuBar->actSaveTIF->setEnabled(true);
    }
}

/**
 * @brief EZCAP::mgrMenu_pBtn_live_focus_clicked
 */
void EZCAP::mgrMenu_pBtn_live_focus_clicked()
{
    ix.onLiveMode = !ix.onLiveMode;

    if(ix.cameraState == Camera_Idle && ix.onLiveMode)
    {
        unsigned int ret;

        managerMenu->ui->pBtn_live_focus->setChecked(true);//设置live按钮处于按下状态
        managerMenu->ui->pBtn_focus->setEnabled(false);//禁用focus按钮
        managerMenu->ui->head_preview->setCheckable(false);
        managerMenu->ui->head_capture->setCheckable(false);

        memset(ix.ImgData,          0, ix.ImageW_Max * ix.ImageH_Max * 2);

        qDebug() <<"----focus live start----";

        ix.cameraState = Camera_Waiting;

        while(ix.onLiveMode)
        {
            ix.imageReady = GetSingleFrame_Waiting;

            if(ix.Gain_Fun && managerMenu->ui->hSlider_Gain_focus->value() != ix.Gain_Last)
            {
                ret = libqhyccd->SetQHYCCDParam(camhandle,CONTROL_GAIN, managerMenu->ui->hSlider_Gain_focus->value());
                if(ret == QHYCCD_SUCCESS)
                {
                    ix.Gain_Last = managerMenu->ui->hSlider_Gain_focus->value();
                }
                else
                {
                    DBGOPT_WARNING("SetQHYCCDParam() CONTROL_GAIN Failed! | gain = %f", managerMenu->ui->hSlider_Gain_focus->value());
                }
            }

            if(ix.Offset_Fun && managerMenu->ui->hSlider_Offset_focus->value() != ix.Offset_Last)
            {
                ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_OFFSET, managerMenu->ui->hSlider_Offset_focus->value());
                if(ret == QHYCCD_SUCCESS)
                {
                    ix.Offset_Last = managerMenu->ui->hSlider_Offset_focus->value();
                }
                else
                {
                    DBGOPT_WARNING("SetQHYCCDParam() CONTROL_OFFSET Failed! | offset = %d", managerMenu->ui->hSlider_Offset_focus->value());
                }
            }

            if(ix.ExpUnit_Last != 1000.0 || managerMenu->ui->hSlider_exposure_focus->value() != ix.ExpTime_Last)
            {
                ret = libqhyccd->SetQHYCCDParam(camhandle,CONTROL_EXPOSURE, managerMenu->ui->hSlider_exposure_focus->value()*1000.0);
                if(ret == QHYCCD_SUCCESS)
                {
                    ix.ExpUnit_Last = 1000.0;
                    ix.ExpTime_Last = managerMenu->ui->hSlider_exposure_focus->value();
                }
                else
                {
                    DBGOPT_WARNING("SetQHYCCDParam() CONTROL_EXPOSURE Failed! | ExpTime = %f", managerMenu->ui->hSlider_exposure_focus->value()*1000.0);
                }
            }

            if(ix.Traffic_Fun && ix.Traffic_Last != ix.Traffic)
            {
                ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_USBTRAFFIC, ix.Traffic);
                if(ret == QHYCCD_SUCCESS)
                {
                    ix.Traffic_Last = ix.Traffic;
                }
                else
                {
                    OutputDebug("EZCAPWARNING | %s | %s | SetQHYCCDParam() CONTROL_Traffic Failed! | traffic = %f", __FILE__, __FUNCTION__, ix.Traffic);
                }
            }

            if(ix.Speed_Fun && ix.Speed_Last != ix.Speed)
            {
                ret = libqhyccd->SetQHYCCDParam(camhandle,CONTROL_SPEED, ix.Speed);
                if(ret == QHYCCD_SUCCESS)
                {
                    ix.Speed_Last = ix.Speed;
                }
                else
                {
                    DBGOPT_WARNING("SetQHYCCDParam() CONTROL_SPEED Failed! | speed = %f", ix.Speed);
                }
            }

            if(ix.BinX_Last != ix.BinX || ix.BinY_Last != ix.BinY)
            {
                ret = libqhyccd->SetQHYCCDBinMode(camhandle, ix.BinX, ix.BinY);
                if(ret == QHYCCD_SUCCESS)
                {
                    ix.BinX_Last = ix.BinX;
                    ix.BinY_Last = ix.BinY;
                }
                else
                {
                    DBGOPT_WARNING("SetQHYCCDBinMode() Failed! | binx = %d biny = %d", ix.BinX, ix.BinY);
                }

            }

            //set focus area position
            focusAreaStartX = 0;
            focusAreaStartY = FocusCenterY_Pre * ix.BinY_Max - 100;
            focusAreaSizeX  = ix.ImageW_Min;
            focusAreaSizeY  = 200;
            if(focusAreaStartX < 0) focusAreaSizeX  = 0;
            if(focusAreaStartY < 0) focusAreaStartY = 0;
            if(focusAreaSizeY  > (int)ix.ImageH_Min ) focusAreaSizeY = (int)ix.ImageH_Min;
            if(focusAreaStartY + focusAreaSizeY > (int)ix.ImageH_Min) focusAreaStartY = (int)ix.ImageH_Min - focusAreaSizeY;
            ix.RoiX = focusAreaStartX;
            ix.RoiY = focusAreaStartY;
            ix.RoiW = focusAreaSizeX;
            ix.RoiH = focusAreaSizeY;
            if(ix.RoiX != ix.RoiX_Last || ix.RoiY != ix.RoiY_Last || ix.RoiW != ix.RoiW_Last || ix.RoiH != ix.RoiH_Last)
            {
                ret = libqhyccd->SetQHYCCDResolution(camhandle, ix.RoiX, ix.RoiY, ix.RoiW, ix.RoiH);
                if(ret != QHYCCD_SUCCESS)
                {
                    DBGOPT_WARNING("SetQHYCCDResolution() Failed! | startx = %d starty = %d sizex = %d sizey = %d", ix.RoiX, ix.RoiY, ix.RoiW, ix.RoiH);
                }
                else
                {
                    ix.RoiX_Last = ix.RoiX;
                    ix.RoiY_Last = ix.RoiY;
                    ix.RoiW_Last = ix.RoiW;
                    ix.RoiH_Last = ix.RoiH;
                }
            }

            ix.lastWorkMode = ix.workMode;

            ix.dateOBS = QDateTime::currentDateTime().toString(Qt::ISODate);  //记录当前拍摄时间戳

            ix.cameraState = Camera_Exposing;
            qCritical("------------- E4");
            ret = libqhyccd->ExpQHYCCDSingleFrame(camhandle);
            if(ret == QHYCCD_SUCCESS)
            {
                QThread::msleep(200);
            }
            else if(ret == QHYCCD_READ_DIRECTLY)
            {
                DBGOPT_INFO("ExpQHYCCDSingleFrame QHYCCD_READ_DIRECTLY!");
            }
            else
            {
                DBGOPT_WARNING("ExpQHYCCDSingleFrame() Failed!");
            }

            ix.cameraState = Camera_Reading;
            //开启线程，获取图像数据
            downloadFoc = new DownloadFocThread(this);
            connect(downloadFoc, SIGNAL(finished()), downloadFoc, SLOT(deleteLater()));
            downloadFoc->start();

            while(ix.imageReady == GetSingleFrame_Waiting)
            {
                //....
                QApplication::processEvents();//响应界面操作，防止界面假死
            }

            if(ix.imageReady == GetSingleFrame_Success)
            {
                emit change_fitHeaderInfo();  //图像拍摄成功，刷新FitHeader信息

                //显示图像
                displayFocusImage_Ex(ix.FrameW_Last, ix.FrameH_Last, ix.ImgData_Last);

                //显示直方图
                displayHistogramImage(ix.FrameW, ix.FrameH, ix.ImgData);
                //获取overscan balck值
                getOverScanBlack(ix.ImgData, ix.FrameW, ix.FrameH);

                noImgInWorkMode = false;
            }

            QApplication::processEvents();//响应界面是否发送了停止信号

        }
        qDebug() << "----focus live end----";

        ix.cameraState = Camera_Idle;

        //live结束 还原focus和live按钮状态
        managerMenu->ui->pBtn_live_focus->setChecked(false);
        managerMenu->ui->pBtn_focus->setEnabled(true);
        managerMenu->ui->head_preview->setCheckable(true);//设置preivew tab有效
        managerMenu->ui->head_capture->setCheckable(true);//设置capture tab有效

        mainMenuBar->actOpenFolder->setEnabled(true);
        mainMenuBar->actSaveBMP->setEnabled(true);//设置保存图像菜单项可用
        mainMenuBar->actSaveFIT->setEnabled(true);
        mainMenuBar->actSaveJPG->setEnabled(true);
        mainMenuBar->actSavePNG->setEnabled(true);
        mainMenuBar->actSaveTIF->setEnabled(true);
    }
}

/**
 * @brief EZCAP::displayFocusImage
 * @param x
 * @param y
 * @param boxWidth
 * @param boxHeight
 * @param boxX
 * @param boxY
 */
void EZCAP::displayFocusImage(int x, int y)
{
    unsigned char pixel;
    long s, k, n;
    int SubX0;
    Mat srcImg;
    Mat focAreaImg;
    int focusAreaW;
    int focusAreaH;

    if(x < 800)
       focusAreaW = x;
    else
        focusAreaW = 800;
    focusAreaH = 200;

    srcImg.create(Size(x, y), CV_8UC3);
//    srcImg.data = ix.dispIplImgData24;
    focAreaImg.create(Size(focusAreaW, focusAreaH), CV_8UC3);

    //convert 16bit to 8bit
    s = 0;
    k = 0;
    for(int i = 0; i < y; i++)
    {
        for(int j = 0; j < x; j++)
        {
            // look up the Stretch LUT table to convert current 16bit data to 8bit
            pixel = ix.StretchLUT[ix.ImgData_Last[s]+ix.ImgData_Last[s+1]*256];
//            ix.OutputData8[k] = pixel;
            s += 2;
            k += 1;
        }
    }

    //copy 8bit data to opencv 8bit 3channels IplImage
    n=0;
    k=0;
    for(int j=0; j<y; j++)
    {
        for(int i=0; i<x; i++)
        {
//            ix.dispIplImgData24[n] = ix.OutputData8[k];
//            ix.dispIplImgData24[n+1] = ix.OutputData8[k];
//            ix.dispIplImgData24[n+2] = ix.OutputData8[k];
            n += 3;
            k += 1;
        }
        n += srcImg.cols*(srcImg.depth()==CV_8U?1:2)*srcImg.channels() - srcImg.channels()*srcImg.cols;
    }


    SubX0 = FocusCenterX_Pre * ix.BinX_Max - focusAreaW / 2;
    if(SubX0 > x - focusAreaW)
    {
        SubX0 = x - focusAreaW;
    }
    if(SubX0 < 0)
    {
        SubX0 = 0;
    }

    focAreaImg = srcImg(Rect(SubX0, 0, focusAreaW, focusAreaH));

    //qDebug() << "display the focus assistant image";
    displayFocusAssistantImage(focAreaImg);

    //display image size into statusBar
    QString str1 = QString::number(focusAreaW) + "x" + QString::number(focusAreaH);
    statusLabel_imgSize->setText(str1);
    showFrameCount();

    //显示到qlabel上
    cvtColor(focAreaImg, focAreaImg, COLOR_BGR2RGB);

    qImg_focus = this->MatToQImage(focAreaImg);
    if(qImg_focus)
    {
        ui->label_ImgShow->setMaximumHeight(focusAreaH);
        ui->label_ImgShow->setMaximumWidth(focusAreaW);
        scrollArea_ImgShow->setWidgetResizable(false);
        ui->label_ImgShow->setPixmap(QPixmap::fromImage(*qImg_focus));
        ui->label_ImgShow->adjustSize();
    }

    srcImg.release();
    focAreaImg.release();
}

void EZCAP::displayFocusImage_Ex(int x, int y, unsigned char *dataBuf)
{
    unsigned char pixel;
    long s, k, n;
    int SubX0;
    Mat srcImg;
    Mat focAreaImg;
    int focusAreaW;
    int focusAreaH;

    if(x < 800)
       focusAreaW = x;
    else
        focusAreaW = 800;
    focusAreaH = 200;

    srcImg.create(Size(x, y), CV_8UC3);
    focAreaImg.create(Size(focusAreaW, focusAreaH), CV_8UC3);

    //convert 16bit to 8bit
    s = 0;
    k = 0;
    for(int i = 0; i < y; i++)
    {
        for(int j = 0; j < x; j++)
        {
            // look up the Stretch LUT table to convert current 16bit data to 8bit
            pixel = ix.StretchLUT[dataBuf[s]+dataBuf[s+1]*256];
            srcImg.data[k] = pixel;
            srcImg.data[k + 1] = pixel;
            srcImg.data[k + 2] = pixel;
            s += 2;
            k += 3;
        }

        k += srcImg.cols * (srcImg.depth()==CV_8U?1:2) * srcImg.channels() - srcImg.channels() * srcImg.cols;
    }

    SubX0 = FocusCenterX_Pre * ix.BinX_Max - focusAreaW / 2;
    if(SubX0 > x - focusAreaW)
    {
        SubX0 = x - focusAreaW;
    }
    if(SubX0 < 0)
    {
        SubX0 = 0;
    }

    Mat imageROI = srcImg(Rect(SubX0, 0, focusAreaW, focusAreaH));
    imageROI.copyTo(focAreaImg);
    imageROI.release();

    displayFocusAssistantImage(focAreaImg);

    //display image size into statusBar
    QString str1 = QString::number(focusAreaW) + "x" + QString::number(focusAreaH);
    statusLabel_imgSize->setText(str1);
    showFrameCount();

    //显示到qlabel上
    cvtColor(focAreaImg, focAreaImg, COLOR_BGR2RGB);

    qImg_focus = this->MatToQImage(focAreaImg);
    if(qImg_focus)
    {
        ui->label_ImgShow->setMaximumHeight(focusAreaH);
        ui->label_ImgShow->setMaximumWidth(focusAreaW);
        scrollArea_ImgShow->setWidgetResizable(false);
        ui->label_ImgShow->setPixmap(QPixmap::fromImage(*qImg_focus));
        ui->label_ImgShow->adjustSize();
    }

    srcImg.release();
    focAreaImg.release();
}
//----------------------------------------------------------------------------------------------------------
//                                     FocusAssistant
//----------------------------------------------------------------------------------------------------------
/**
 * @brief EZCAP::displayFocusAssistantImage
 * @param image
 */
void EZCAP::displayFocusAssistantImage(Mat image)
{
    int pixel;
    Mat ZoomImg;
    Mat GuideBIGImg;

    ZoomImg.create(Size(249/*200*/, 249/*200*/), CV_8UC3);
    GuideBIGImg.create(Size(249/*200*/, 249/*200*/), CV_8UC1);

    if (ZoomFocus_X < 20)
        ZoomFocus_X = 20;
    if (ZoomFocus_Y < 20)
        ZoomFocus_Y = 20;


    Mat ImgROI = image(Rect(ZoomFocus_X - 20, ZoomFocus_Y - 20, 40, 40));
    if(FocusZoomMode == 0)
        cv::resize(ImgROI, ZoomImg, Size(ZoomImg.cols, ZoomImg.rows), 0, 0, INTER_LINEAR);
    else
        cv::resize(ImgROI, ZoomImg, Size(ZoomImg.cols, ZoomImg.rows), 0, 0, INTER_NEAREST);
    ImgROI.release();

    cvtColor(ZoomImg, GuideBIGImg, COLOR_RGB2GRAY);

    FWHMFocus(GuideBIGImg, FocusInfo);//获取Focus信息

    FalseColorConvert(ZoomImg, ZoomImg);//伪彩色变换

    //draw FWHM curve
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    QImage image2_focusAssistant = ui->image2_focusAssistant->pixmap().toImage(); //image2_foc
#else
    QImage image2_focusAssistant = ui->image2_focusAssistant->pixmap()->toImage(); //image2_foc
#endif
    QPainter painter2(&image2_focusAssistant); //为这个QImage构造一个QPainter
    painter2.setCompositionMode(QPainter::CompositionMode_SourceIn);//设置画刷的组合模式CompositionMode_SourceOut这个模式为目标图像在上。
    QPen pen2 = painter2.pen();
    pen2.setWidth(2);
    pen2.setColor(QColor(255, 0, 0));
    painter2.setPen(pen2);//重设画笔
    painter2.drawLine(fwhm_x, fwhm_y, FocusCurveX, 128 - FocusInfo.FWHM_Result);//画线

    ui->image2_focusAssistant->setPixmap(QPixmap::fromImage(image2_focusAssistant));//刷新显示
    ui->fwhm_value_focusAssistant->setText(QString::number(FocusInfo.FWHM_Result));//显示fwhm值

    //draw PEAK curve
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    QImage image3_focusAssistant = ui->image3_focusAssistant->pixmap().toImage(); //image3_foc
#else
    QImage image3_focusAssistant = ui->image3_focusAssistant->pixmap()->toImage(); //image3_foc
#endif
    QPainter painter3(&image3_focusAssistant);//为这个QImage构造一个QPainter
    painter3.setCompositionMode(QPainter::CompositionMode_SourceIn);//设置画刷的组合模式CompositionMode_SourceOut这个模式为目标图像在上。
    QPen pen3 = painter3.pen();
    pen3.setColor(QColor(0, 0, 255));
    painter3.setPen(pen3);//重设画笔
    painter3.drawLine(peak_x, peak_y, FocusCurveX, (255-FocusInfo.DeltaPixel)/2);//画线

    ui->image3_focusAssistant->setPixmap(QPixmap::fromImage(image3_focusAssistant));//刷新显示
    ui->peak_value_focusAssistant->setText(QString::number(FocusInfo.DeltaPixel));//显示peak值

/*
    char fwhm[64];
    char itsty[64];

    sprintf(fwhm,"FWHM %7d",FocusInfo.FWHM_Result);
    sprintf(itsty,"Intensity %d",FocusInfo.DeltaPixel);

    SendTwoLine2QHYCCDInterCamOled(camhandle,fwhm,itsty);
*/
    //当前终点坐标为下一次的起始点坐标
    fwhm_x = FocusCurveX;
    fwhm_y = 128 - FocusInfo.FWHM_Result;
    peak_x = FocusCurveX;
    peak_y = (255 - FocusInfo.DeltaPixel)/2;

    FocusCurveX++;

    if (FocusCurveX > 300)
    {
        FocusCurveX = 0;
        //初始化表格线
        DrawGridBox(&image2_focusAssistant);
        DrawGridBox(&image3_focusAssistant);
        //初始化起始点坐标
        fwhm_x = 0;
        fwhm_y = 0;
        peak_x = 0;
        peak_y = 0;

        pen2.setColor(QColor(255,0,0));
        pen2.setWidth(2);
        painter2.setPen(pen2);//重设画笔

        pen3.setColor(QColor(0,0,255));
        pen3.setWidth(2);
        painter3.setPen(pen3);
    }

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    QImage screenImg_focus = managerMenu->ui->img_screenView->pixmap().toImage();
#else
    QImage screenImg_focus = managerMenu->ui->img_screenView->pixmap()->toImage();
#endif
    QPainter painter_screenFocus(&screenImg_focus); //为这个QImage构造一个QPainter
    painter_screenFocus.setCompositionMode(QPainter::CompositionMode_SourceIn);//设置画刷的组合模式CompositionMode_SourceOut这个模式为目标图像在上。

    QBrush brush_screenFocus = painter_screenFocus.brush();
    brush_screenFocus.setStyle(Qt::SolidPattern);
    brush_screenFocus.setColor(QColor(0,0,0));
    painter_screenFocus.setBrush(brush_screenFocus);//设置画刷

    QPen pen_screenFocus = painter_screenFocus.pen();
    pen_screenFocus.setColor(QColor(0,255,0));
    painter_screenFocus.setPen(pen_screenFocus);//设置画笔

    painter_screenFocus.drawRect(0,0,249/*200*/,249/*200*/);//绘制矩形

    int x1 = 0;//设置起点坐标
    int y1 = 0;
    //画直线
    for (int i = 0; i < 249/*200*/; i++)
    {
        pixel = FocusInfo.Row[i];
        if (pixel > 249/*192*/)
            pixel = 249/*192*/;

        painter_screenFocus.drawLine(x1,y1,i,pixel);
        //s=s+3;
        x1 = i;
        y1 = pixel;
    }

    brush_screenFocus.setStyle(Qt::SolidPattern);
    brush_screenFocus.setColor(QColor(0,0,0));
    painter_screenFocus.setBrush(brush_screenFocus);//设置画刷
    pen_screenFocus.setColor(QColor(255,0,0));
    painter_screenFocus.setPen(pen_screenFocus);//设置画笔
    //设置起点坐标
    x1 = 0;
    y1 = FocusInfo.MinPixel;
    painter_screenFocus.drawLine(x1,y1,249/*200*/,FocusInfo.MinPixel);//绘制直线

    pen_screenFocus.setColor(QColor(255,255,0));
    painter_screenFocus.setPen(pen_screenFocus);//设置画笔
    //设置起点坐标
    x1 = FocusInfo.CenterX;
    y1 = 0;
    painter_screenFocus.drawLine(x1,y1,FocusInfo.CenterX,249/*192*/);//绘制直线

    pen_screenFocus.setColor(QColor(255,0,0));
    painter_screenFocus.setPen(pen_screenFocus);
    //设置起点坐标
    x1 = FocusInfo.FWHM_X1;
    y1 = 0;
    painter_screenFocus.drawLine(x1,y1,FocusInfo.FWHM_X1,249/*192*/);
    //设置起点坐标
    x1 = FocusInfo.FWHM_X2;
    y1 = 0;
    painter_screenFocus.drawLine(x1,y1,FocusInfo.FWHM_X2,249/*192*/);

    managerMenu->ui->img_screenView->setPixmap(QPixmap::fromImage(screenImg_focus));//刷新screenview图像显示

    //绘制圆圈
    circle(ZoomImg, Point(FocusInfo.CenterX, FocusInfo.CenterY), 5, Scalar(255, 0, 0), 1, LINE_8, 0);

    //显示到qlabel上
    cvtColor(ZoomImg, ZoomImg, COLOR_BGR2RGB);
    qImg_focus = this->MatToQImage(ZoomImg);
    if(qImg_focus)
    {
        ui->image1_focusAssistant->setPixmap(QPixmap::fromImage(*qImg_focus));
    }

    ZoomImg.release();
    GuideBIGImg.release();

}

/**
 * @brief EZCAP::FWHMFocus
 * @param Img
 * @param FocusInfo
 */
void EZCAP::FWHMFocus(Mat Img,FOCUSINFO &FocusInfo)
{
    //input image should be 1 channel image, and 8bit
    Mat MaxColumn, MaxRow, imageROI;

    MaxColumn.create(Size(1, Img.rows), CV_8UC1);
    MaxRow.create(Size(Img.cols, 1), CV_8UC1);

    double minPixel,maxPixel;
    Point  maxLocation;

    minMaxLoc(Img, &minPixel, &maxPixel, 0, &maxLocation);

    FocusInfo.MaxPixel = maxPixel;
    FocusInfo.MinPixel = minPixel;
    FocusInfo.DeltaPixel = maxPixel - minPixel;

    FocusInfo.width = Img.cols;
    FocusInfo.height = Img.rows;

    FocusInfo.CenterX = maxLocation.x;
    FocusInfo.CenterY = maxLocation.y;

    imageROI = Img(Rect(maxLocation.x, 0, 1, Img.rows));
    imageROI.copyTo(MaxColumn);
    imageROI.release();

    imageROI = Img(Rect(0, maxLocation.y, Img.cols,1));
    imageROI.copyTo(MaxRow);
    imageROI.release();

    for(int i=0; i < Img.rows; i++)
    {
        FocusInfo.Column[i] = MaxColumn.data[i*4];
    }

    for(int i=0; i < Img.rows; i++)
    {
        FocusInfo.Row[i] = MaxRow.data[i];
    }

    int FWHM_Level;    //FWHM is the half intensity
    FWHM_Level = (maxPixel-minPixel)/2 + minPixel;

    int FWHM;

    FWHM = maxLocation.y;
    while(FocusInfo.Column[FWHM] > FWHM_Level)
    {
        FWHM--;
        if(FWHM < 0)
            return;
    }
    FocusInfo.FWHM_Y1 = FWHM;

    FWHM = maxLocation.y;
    while(FocusInfo.Column[FWHM] > FWHM_Level)
    {
        FWHM++;
        if(FWHM > Img.rows/*Img->height*/)
            return;
    }
    FocusInfo.FWHM_Y2 = FWHM;

    FocusInfo.FWHM_ResultY = FocusInfo.FWHM_Y2 - FocusInfo.FWHM_Y1 ;

    FWHM = maxLocation.x;
    while(FocusInfo.Row[FWHM] > FWHM_Level)
    {
        FWHM--;
        if(FWHM < 0)
            return;
    }
    FocusInfo.FWHM_X1 = FWHM;

    FWHM = maxLocation.x;
    while(FocusInfo.Row[FWHM] > FWHM_Level)
    {
        FWHM++;
        if(FWHM > Img.cols/*Img->width*/)
            return;
    }
    FocusInfo.FWHM_X2 = FWHM;

    FocusInfo.FWHM_ResultX = FocusInfo.FWHM_X2 - FocusInfo.FWHM_X1 ;

    double R,R1,R2;

    R1 = FocusInfo.FWHM_ResultX ;
    R2 = FocusInfo.FWHM_ResultY ;
    R = sqrt(R1*R1+R2*R2)/1.414 ;
    FocusInfo.FWHM_Result = (unsigned char)R;

    MaxRow.release();
    MaxColumn.release();
}

/**
 * @brief EZCAP::DrawGridBox
 * @param img
 */
void EZCAP::DrawGridBox(QImage *img)
{
    int x,y;
    x = img->width();
    y = img->height();

    QPainter painter(img); //create a painter for the QImage
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);//设置画刷的组合模式CompositionMode_SourceOut这个模式为目标图像在上。

    //改变画刷
    QBrush brush = painter.brush();
    brush.setStyle(Qt::SolidPattern);
    brush.setColor(QColor(10,10,10));
    painter.setBrush(brush);
    //改变画笔
    QPen pen = painter.pen();
    pen.setStyle(Qt::SolidLine);
    pen.setWidth(4);
    pen.setColor(QColor(128,128,128));
    painter.setPen(pen);
    painter.drawRect(0,0,x,y);//绘制矩形

    pen.setWidth(1);
    pen.setColor(QColor(64,64,64));
    painter.setPen(pen);
    //画细分线
    for(int i=0; i<y; i=i+y/4)
    {
        painter.drawLine(0,i,x,i);
    }

    for(int i=0; i<x; i=i+x/6)
    {
        painter.drawLine(i,0,i,y);
    }

}

/**
 * @brief EZCAP::FalseColorConvert
 * @param inputImg
 * @param outputImg
 */
void EZCAP::FalseColorConvert(Mat inputImg, Mat outputImg)
{
    // 使用三通道LUT对图像进行伪彩色处理
    //需要先定义全局变量  unsigned char LUT1[256][3];
    //必须保证输入和输出图像尺寸相同
    int height, width;
    height = inputImg.rows;
    width = inputImg.cols;

    if(inputImg.channels() == 1)
    {
        //输入为单色，输出为彩色
        for(int y=0; y<height; y++)
        {
            unsigned char *ptrInput = inputImg.data + y * inputImg.cols * (inputImg.depth()==CV_8U?1:2) * inputImg.channels();
            unsigned char *ptrOutput = outputImg.data + y * outputImg.cols * (outputImg.depth()==CV_8U?1:2) * outputImg.channels();
            for(int x=0; x<width; x++)
            {
                ptrOutput[3*x] = ix.LUT_1[ptrInput[x]][0];
                ptrOutput[3*x+1] = ix.LUT_1[ptrInput[x]][1];
                ptrOutput[3*x+2] = ix.LUT_1[ptrInput[x]][2];
            }
        }
    }
    else
    {
        for(int y=0; y<height; y++)
        {
            unsigned char *ptrInput = inputImg.data + y * inputImg.cols * (inputImg.depth()==CV_8U?1:2) * inputImg.channels();
            unsigned char *ptrOutput = outputImg.data + y * outputImg.cols * (outputImg.depth()==CV_8U?1:2) * outputImg.channels();
            for(int x=0; x<width; x++)
            {
                ptrOutput[3*x] = ix.LUT_1[ptrInput[3*x]][0];
                ptrOutput[3*x+1] = ix.LUT_1[ptrInput[3*x+1]][1];
                ptrOutput[3*x+2] = ix.LUT_1[ptrInput[3*x+2]][2];
            }
        }
    }
}

/**
 * @brief EZCAP::LoadFalseColor
 * @param LUTMAP
 */
void EZCAP::LoadFalseColor(QString LUTMAP)
{
    int i;
    long vr,vg,vb;
    double vcolor;

    if (LUTMAP == "Linear")
    {
        ui->falseColor_focusAssistant->setPixmap(QPixmap(":image/Linear.bmp"));
        for (i = 0; i<256; i++)
        {
            ix.LUT_1[i][0]=i;
            ix.LUT_1[i][1]=i;
            ix.LUT_1[i][2]=i;
        }
    }
    else
    {
        QPixmap pixmap_falseColor(":/image/"+LUTMAP);
        ui->falseColor_focusAssistant->setPixmap(pixmap_falseColor);//reflash the falsecolor image

        for (i = 0; i<256; i++)
        {
            //qpixmap convert to qimage, and convert an RGB to BGR
            QImage imgTemp = pixmap_falseColor.toImage().rgbSwapped();

            vcolor = imgTemp.pixel(i,3);

            vr = fmod(fmod(vcolor,65536),256);
            vg = fmod((vcolor-vr)/256,256);
            vb = fmod((vcolor-vr-256*vg)/65536,256);

            ix.LUT_1[i][0]=vb;
            ix.LUT_1[i][1]=vg;
            ix.LUT_1[i][2]=vr;
        }
    }
}

/**
 * @brief EZCAP::on_pBtn_linear_clicked
 */
void EZCAP::on_pBtn_linear_clicked()
{
    LoadFalseColor("Linear.bmp");
}

/**
 * @brief EZCAP::on_pBtn_thermal_clicked
 */
void EZCAP::on_pBtn_thermal_clicked()
{
    LoadFalseColor("ThermalMAP.bmp");
}

/**
 * @brief EZCAP::on_pBtn_false_clicked
 */
void EZCAP::on_pBtn_false_clicked()
{
    LoadFalseColor("FalseColor.bmp");
}

/**
 * @brief EZCAP::on_pBtn_invert_clicked
 */
void EZCAP::on_pBtn_invert_clicked()
{
    LoadFalseColor("NegativeFilm.bmp");
}
//--------------------------------------------------------------------------------------

//***************************************************************************************
//                                  capture tab页操作
//***************************************************************************************

/**
 * @brief EZCAP::mgrMenu_pBtn_stop_clicked
 */
void EZCAP::mgrMenu_pBtn_stop_clicked()
{
    //传输数据过程中 stop无效 仍需等待数据传输完成
    if(ix.cameraState != Camera_Idle)
    {
        isSettleDone = true; //停止可能存在的Dither 等待循环

        //停止曝光
        uint32_t ret = libqhyccd->CancelQHYCCDExposingAndReadout(camhandle);
        if(ret != QHYCCD_SUCCESS)
        {
            DBGOPT_ERROR("CancelQHYCCDExposingAndReadout failed");
        }

        ix.ForceStop = true;//曝光停止，退出循环
        ix.cameraState = Camera_Idle;
    }
}

/**
 * @brief EZCAP::mgrMenu_pBtn_capture_clicked
 */
void EZCAP::mgrMenu_pBtn_capture_clicked()
{
    unsigned int ret = QHYCCD_ERROR;

    QElapsedTimer sTime;
    int betweenTime;

    // 如果当前相机还处在曝光/下载/重传接收等非空闲状态，不能再开启新的 Capture。
    // 特别是重传超时场景下，主界面虽然已经返回，但后台 DownloadCapThread 可能仍阻塞在
    // GetQHYCCDSingleFrame() 中等待 SDK 返回。此时再次点击 Capture 会再创建一个读图线程，
    // 两个线程同时访问同一个 camhandle/USB 读图接口，容易导致 SDK 卡死。
    // 因此这里把 Capture 按钮状态恢复，并等待前一个流程真正结束后再允许下一次拍摄。
    if(ix.cameraState != Camera_Idle)
    {
        managerMenu->ui->pBtn_capture->setChecked(false);
        statusLabel_msg->setText(tr("Downloading..."));
        return;
    }

    managerMenu->ui->pBtn_capture->setChecked(true);
    managerMenu->ui->head_focus->setCheckable(false);//禁用focus tab
    managerMenu->ui->head_preview->setCheckable(false);//禁用preview tab

    memset(ix.ImgData,          0, ix.ImageW_Max * ix.ImageH_Max * 2);

    if(ix.cameraState == Camera_Idle)
    {
        ix.cameraState = Camera_Waiting;
        ix.imageReady = GetSingleFrame_Waiting;
        ix.onLiveMode = false;

        if (ix.CamID.contains("QHY411M_1") || ix.CamID.contains("QHY411MERIS_1") ||
            ix.CamID.contains("QHY411M_2") || ix.CamID.contains("QHY411MERIS_2") ||
            ix.CamID.contains("QHY411M_3") || ix.CamID.contains("QHY411MERIS_3") ||
            ix.CamID.contains("QHY411M_4") || ix.CamID.contains("QHY411MERIS_4"))
            libqhyccd->SetQHYCCDWriteFPGA(camhandle, 0, 34, 1);

        //set camera parameters...
        //------------------------
        if(ix.canMechanicalShutter && (ix.MechanicalShutterMode != ix.LastMechanicalShutterMode))
        {
            ret = libqhyccd->ControlQHYCCDShutter(camhandle, ix.MechanicalShutterMode);
            if(ret == QHYCCD_SUCCESS)
            {
                ix.LastMechanicalShutterMode = ix.MechanicalShutterMode;
            }
            else
            {
                DBGOPT_WARNING("ControlQHYCCDShutter() Failed! | shuttermode = %d", ix.MechanicalShutterMode);
            }
        }

        if(managerMenu->ui->comboBox_color_capture->currentText() == "ON")
            ix.IsCvtColor = true;
        else
            ix.IsCvtColor = false;

        ix.Gain = managerMenu->ui->hSlider_Gain_capture->value();
        if(ix.Gain_Fun && ix.Gain != ix.Gain_Last)
        {
            ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_GAIN, ix.Gain);
            if(ret == QHYCCD_SUCCESS) ix.Gain_Last = ix.Gain;
            else DBGOPT_INFO("SetQHYCCDParam() CONTROL_Gain Failed! | Gain = %d", ix.Gain);
        }

        ix.Offset = managerMenu->ui->hSlider_Offset_capture->value();
        if(ix.Offset_Fun && ix.Offset != ix.Offset_Last)
        {
            ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_OFFSET, ix.Offset);
            if(ret == QHYCCD_SUCCESS) ix.Offset_Last = ix.Offset;
            else DBGOPT_INFO("SetQHYCCDParam() CONTROL_Offset Failed! | Offset = %d", ix.Offset);
        }

        ix.ExpTime = managerMenu->ui->hSlider_exposure_capture->value();
        if(managerMenu->ui->comBoxSingleUnit->currentIndex() == 0) ix.ExpUnit = 1.0;
        if(managerMenu->ui->comBoxSingleUnit->currentIndex() == 1) ix.ExpUnit = 1000.0;
        if(managerMenu->ui->comBoxSingleUnit->currentIndex() == 2) ix.ExpUnit = 1000000.0;
        if(ix.ExpUnit != ix.ExpUnit_Last || ix.ExpTime != ix.ExpTime_Last)
        {
            ret = libqhyccd->SetQHYCCDParam(camhandle,CONTROL_EXPOSURE, ix.ExpTime*ix.ExpUnit);
            if(ret == QHYCCD_SUCCESS)
            {
                ix.ExpUnit_Last = ix.ExpUnit;
                ix.ExpTime_Last = ix.ExpTime;
            }
            else
            {
                DBGOPT_WARNING("SetQHYCCDParam() CONTROL_EXPOSURE Failed! | ExpTime = %f", ix.ExpTime * ix.ExpUnit);
            }
        }

        if(ix.Traffic_Fun && ix.Traffic_Last != ix.Traffic)
        {
            ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_USBTRAFFIC, ix.Traffic);
            if(ret == QHYCCD_SUCCESS)
            {
                ix.Traffic_Last = ix.Traffic;
            }
            else
            {
                DBGOPT_WARNING("SetQHYCCDParam() CONTROL_Traffic Failed! | traffic = %f", ix.Traffic);
            }
        }

        if(ix.Speed_Fun && managerMenu->ui->checkBox_highSpeed->isChecked())
            ix.Speed = 1;
        else
            ix.Speed = 0;
        if(ix.Speed_Fun && ix.Speed != ix.Speed_Last)
        {
            ret = libqhyccd->SetQHYCCDParam(camhandle,CONTROL_SPEED, ix.Speed);
            if(ret == QHYCCD_SUCCESS)
            {
                ix.Speed_Last = ix.Speed;
            }
            else
            {
                DBGOPT_WARNING("SetQHYCCDParam() CONTROL_SPEED Failed! | speed = %f", ix.Speed);
            }
        }

        if(ix.RoiX != ix.RoiX_Last || ix.RoiY != ix.RoiY_Last || ix.RoiW != ix.RoiW_Last || ix.RoiH != ix.RoiH_Last)
        {
            ret = libqhyccd->SetQHYCCDResolution(camhandle, ix.RoiX, ix.RoiY, ix.RoiW, ix.RoiH);
            if(ret != QHYCCD_SUCCESS)
            {
                DBGOPT_WARNING("SetQHYCCDResolution() Failed! | startx = %d starty = %d sizex = %d sizey = %d", ix.RoiX, ix.RoiY, ix.RoiW, ix.RoiH);
            }
            else
            {
                ix.RoiX_Last = ix.RoiX;
                ix.RoiY_Last = ix.RoiY;
                ix.RoiW_Last = ix.RoiW;
                ix.RoiH_Last = ix.RoiH;
            }
        }

        if (ix.CamID.contains("QHY411M_1") || ix.CamID.contains("QHY411MERIS_1") ||
            ix.CamID.contains("QHY411M_2") || ix.CamID.contains("QHY411MERIS_2") ||
            ix.CamID.contains("QHY411M_3") || ix.CamID.contains("QHY411MERIS_3") ||
            ix.CamID.contains("QHY411M_4") || ix.CamID.contains("QHY411MERIS_4"))
            libqhyccd->SetQHYCCDWriteFPGA(camhandle, 0, 34, 0);

        //fit header info...
        ix.dateOBS = QDateTime::currentDateTime().toString(Qt::ISODate);  //记录当前拍摄时间戳
        //ix.dateOBS = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);

        statusLabel_msg->setText(tr("Exposuring..."));
        //sign up camera is exposuring...
        ix.cameraState = Camera_Exposing;
        sTime.start();
        ix.GPS_LocalTime = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss-zzz");
        ret = libqhyccd->ExpQHYCCDSingleFrame(camhandle);
        if(ret == QHYCCD_ERROR)
        {
            DBGOPT_ERROR("ExpQHYCCDSingleFrame() Failed!");
        }
        else if(ret == QHYCCD_READ_DIRECTLY)
        {
            DBGOPT_INFO("ExpQHYCCDSingleFrame QHYCCD_READ_DIRECTLY!");
        }
        else
        {
            DBGOPT_INFO("ExpQHYCCDSingleFrame success, wait...");
            QThread::msleep(200);
        }

        //---progress for Exposing
        managerMenu->ui->proBar_captureTime->setValue(0);
        managerMenu->ui->proBar_capture->setValue(0);

        betweenTime = sTime.elapsed();//返回从上次start()或restart()开始以来的时间差，单位ms
        while (betweenTime < (ix.ExpTime*ix.ExpUnit/1000) && !ix.ForceStop)
        {
            if((betweenTime + 1000) >= (ix.ExpTime*ix.ExpUnit / 1000))
            {
                break;
            }
            betweenTime = sTime.elapsed();
            managerMenu->ui->proBar_captureTime->setValue(betweenTime*100/(ix.ExpTime*ix.ExpUnit)*1000);
            QThread::msleep(1);
            QApplication::processEvents();//防止长时间导致界面假死
        }
        managerMenu->ui->proBar_captureTime->setValue(100);

        if(ix.Cooler_Fun)
        {
            threadTempControl->suspend();
        }

        if(!ix.ForceStop)
        {
            //开启线程，从usb donload图像数据
            qDebug() << "downloading captured frame";
            ix.cameraState = Camera_Reading;

            downloadCap = new DownloadCapThread(this);
            connect(downloadCap, SIGNAL(updateGPSInfo()), gpsTool_dialog, SLOT(updateGPSInfo()));
            connect(downloadCap, SIGNAL(finished()), downloadCap, SLOT(deleteLater()));
            downloadCap->start();

            //while(ix.cameraState == Camera_Reading)
            while(ix.imageReady == GetSingleFrame_Waiting && ix.cameraState != Camera_Idle)
            {
                //处理下载进度条
                //....
                managerMenu->ui->proBar_capture->setValue(libqhyccd->GetQHYCCDReadingProgress(camhandle));
                QThread::msleep(1);

                statusLabel_msg->setText(tr("Downloading..."));
                QApplication::processEvents();//响应界面操作，防止界面假死
            }
            managerMenu->ui->proBar_capture->setValue(100);
        }
        else
        {
            ix.imageReady = GetSingleFrame_Failed;
        }
        ix.ForceStop = false; //reset ForceStop flag after GetQHYCCDSingleFrame returned, otherwise can not wait in next time
#if 0
//miniCam5系列看帧序号的东西
        unsigned char value[8];
        value[0] = (ix.ImgData[0] & 0xf0) >> 4;
        value[1] = (ix.ImgData[1] & 0xf0) >> 4;
        value[2] = (ix.ImgData[2] & 0xf0) >> 4;
        value[3] = (ix.ImgData[3] & 0xf0) >> 4;
        value[4] = (ix.ImgData[4] & 0xf0) >> 4;
        value[5] = (ix.ImgData[5] & 0xf0) >> 4;
        value[6] = (ix.ImgData[6] & 0xf0) >> 4;
        value[7] = (ix.ImgData[7] & 0xf0) >> 4;
        char str[16];
        sprintf(str,"%x%x%x%x%x%x%x%x",value[6],value[7],value[4],value[5],value[2],value[3],value[0],value[1]);
        this->setWindowTitle(tr("FrameNum=") + QString(str));
#endif

        if(ix.Cooler_Fun)
        {
            threadTempControl->resume();
        }

        statusLabel_msg->setText(tr("IDLE"));
        ix.cameraState = Camera_Idle;

        if(ix.imageReady == GetSingleFrame_Success)
        {
            emit change_fitHeaderInfo();  //图像拍摄成功，刷新FitHeader信息

            if(ix.workMode != ix.lastWorkMode)
            {
                ix.lastWorkMode = ix.workMode;
            }

            int wpos = 0, bpos = 0;
            if(ix.workMode == WorkMode_Preview)
            {
                wpos = Preview_WPOS;
                bpos = Preview_BPOS;
            }
            else
            {
                wpos = Capture_WPOS;
                bpos = Capture_BPOS;
            }
            managerMenu->ui->hSlider_bPos->setValue(bpos);
            managerMenu->ui->hSlider_wPos->setValue(wpos);
            setStretchLUT(wpos, bpos);  //set the stretch LUT value

            //由于可能进行黑电平校正，校正后数据存储在ImgData_Last中，所以这里传入ImgData_Last数据用于显示
            displaySingleFrame(ix.FrameW_Last, ix.FrameH_Last, ix.ImgData_Last);

            if(ix.zoomMode == Zoom_FitWindow || ix.zoomMode == Zoom_FillWindow)
            {
                viewBoxW = managerMenu->ui->img_screenView->width();
                viewBoxH = managerMenu->ui->img_screenView->height();
            }
            else
            {
                viewBoxW = managerMenu->ui->img_screenView->width()  * scrollArea_ImgShow->width()  / (ix.FrameW_Last * ix.scaleFactor);
                viewBoxH = managerMenu->ui->img_screenView->height() * scrollArea_ImgShow->height() / (ix.FrameH_Last * ix.scaleFactor);
                viewBoxCX = managerMenu->ui->img_screenView->width()  * (double)ix.showLabelX / ix.scaleFactor / ix.FrameW_Last + viewBoxW / 2;
                viewBoxCY = managerMenu->ui->img_screenView->height() * (double)ix.showLabelY / ix.scaleFactor / ix.FrameH_Last + viewBoxH / 2;
            }

            if(viewBoxCX < viewBoxW / 2) viewBoxCX = viewBoxW / 2;
            if(viewBoxCY < viewBoxH / 2) viewBoxCY = viewBoxH / 2;
            if(viewBoxCX > (managerMenu->ui->img_screenView->width() - viewBoxW / 2 - 1))
                viewBoxCX = managerMenu->ui->img_screenView->width() - viewBoxW / 2 - 1;
            if(viewBoxCY > (managerMenu->ui->img_screenView->height() - viewBoxH / 2 - 1))
                viewBoxCY = managerMenu->ui->img_screenView->height() - viewBoxH / 2 - 1;
            displayScreenViewImage(viewBoxW, viewBoxH, viewBoxCX, viewBoxCY);

            displayHistogramImage(ix.FrameW_Last, ix.FrameH_Last, ix.ImgData_Last);

            //获取OB level
            getOverScanBlack(ix.ImgData_Last, ix.FrameW_Last, ix.FrameH_Last);

            //状态栏显示图像分辨率
            QString str1 = QString::number(ix.FrameW_Last) + "x" + QString::number(ix.FrameH_Last);
            statusLabel_imgSize->setText(str1);
            showFrameCount();

            noImgInWorkMode = false;
        }

        //------------------------------------------------------------------
        managerMenu->ui->pBtn_capture->setChecked(false);
        managerMenu->ui->head_focus->setCheckable(true);
        managerMenu->ui->head_preview->setCheckable(true);

        mainMenuBar->actOpenFolder->setEnabled(true);
        mainMenuBar->actSaveBMP->setEnabled(true);
        mainMenuBar->actSaveFIT->setEnabled(true);
        mainMenuBar->actSaveJPG->setEnabled(true);
        mainMenuBar->actSavePNG->setEnabled(true);
        mainMenuBar->actSaveTIF->setEnabled(true);
        if(iniFileParams.autoConnect){
            managerMenu->ui->pBtn_auto_histogram->click();
        }
    }

}

bool EZCAP::triggerRetransferAndReceiveFrame()
{
    // FPGA 寄存器 69 是当前用于触发重传的控制位。
    // 和 SdkDemo08 中的测试按钮保持一致：先写 1，保持 100ms，再写回 0，形成一个脉冲。
    uint32_t ret1 = libqhyccd->SetQHYCCDWriteFPGA(camhandle, 0, 69, 1);
    QThread::msleep(100);
    uint32_t ret2 = libqhyccd->SetQHYCCDWriteFPGA(camhandle, 0, 69, 0);

    // 如果任意一次写 FPGA 失败，说明重传触发指令没有可靠发出，直接返回失败。
    if(ret1 != QHYCCD_SUCCESS || ret2 != QHYCCD_SUCCESS){
        return false;
    }

    // 连续模式下，EZCAP 已经启动了 BeginQHYCCDLive() 和 LiveCapThread，
    // LiveCapThread 会持续调用 GetQHYCCDLiveFrame() 接收图像。
    // 因此这里不能再启动单帧接收，否则可能和 live 线程同时读 USB 图像数据。
    //
    // 如果相机当前不是空闲状态，也说明已有曝光/下载/其它读取流程正在进行；
    // 此时同样只发送重传脉冲，不额外插入一次 GetQHYCCDSingleFrame()。
    if(ix.camStreamMode == 1 || ix.cameraState != Camera_Idle){
        return true;
    }

    // 单帧模式空闲时，上位机默认没有持续调用 GetQHYCCDSingleFrame()。
    // 只发送 FPGA[69] 的重传脉冲，硬件即使吐出图像数据，软件侧也没有接收者。
    // 所以这里主动进入一次单帧下载流程，用 DownloadCapThread 调用 GetQHYCCDSingleFrame()。
    ix.imageReady = GetSingleFrame_Waiting;
    ix.ForceStop = false;
    ix.cameraState = Camera_Reading;
    statusLabel_msg->setText(tr("Downloading..."));
    managerMenu->ui->proBar_capture->setValue(0);

    downloadCap = new DownloadCapThread(this);
    connect(downloadCap, SIGNAL(updateGPSInfo()), gpsTool_dialog, SLOT(updateGPSInfo()));
    connect(downloadCap, SIGNAL(finished()), this, SLOT(retransferDownloadFinished()));
    connect(downloadCap, SIGNAL(finished()), downloadCap, SLOT(deleteLater()));
    downloadCap->start();
    DownloadCapThread *startedDownloadCap = downloadCap;
    QPointer<DownloadCapThread> retransferDownloadCap = downloadCap;

    // 等待下载线程完成。DownloadCapThread 成功时会把 ix.imageReady 置为
    // GetSingleFrame_Success，失败时置为 GetSingleFrame_Failed。
    // 这里保持事件处理，避免界面在等待图像期间完全卡死。
    QElapsedTimer retransferTimer;
    retransferTimer.start();
    bool retransferTimeout = false;
    while(ix.imageReady == GetSingleFrame_Waiting && ix.cameraState != Camera_Idle)
    {
        // SDK 和相机没有“是否支持重传”的查询接口，因此用超时作为兜底。
        // 超时后把本次下载标记为丢弃，即使下载线程之后才返回成功，也不会增加 Count。
        if(retransferTimer.elapsed() > 5000)
        {
            retransferTimeout = true;
            ix.ForceStop = true;
            ix.imageReady = GetSingleFrame_Failed;

            // 标记当前 DownloadCapThread 即使之后才拿到数据，也要丢弃这一帧。
            // 这样可以保证“超时失败”的重传不会增加 Count，也不会把迟到的图像当作有效结果显示。
            if(downloadCap){
                downloadCap->discardFrame = true;
            }

            // 超时说明 SDK 的 GetQHYCCDSingleFrame() 还没有返回。
            // 这里主动通知 SDK 取消当前曝光/读出，促使后台 DownloadCapThread 尽快从阻塞调用中退出。
            libqhyccd->CancelQHYCCDExposingAndReadout(camhandle);
            break;
        }
        managerMenu->ui->proBar_capture->setValue(libqhyccd->GetQHYCCDReadingProgress(camhandle));
        QThread::msleep(1);
        QApplication::processEvents();
    }

    if(retransferTimeout && retransferDownloadCap && retransferDownloadCap->isRunning())
    {
        // CancelQHYCCDExposingAndReadout() 只是发出取消请求，不代表后台线程已经立刻退出。
        // 给 DownloadCapThread 一个短暂的收尾窗口，让它有机会从 GetQHYCCDSingleFrame() 返回，
        // 并通过 finished 信号进入 retransferDownloadFinished() 做统一清理。
        QElapsedTimer cancelTimer;
        cancelTimer.start();
        while(retransferDownloadCap->isRunning() && cancelTimer.elapsed() < 3000)
        {
            QThread::msleep(1);
            QApplication::processEvents();
        }
    }

    managerMenu->ui->proBar_capture->setValue(100);

    if(retransferTimeout && retransferDownloadCap && retransferDownloadCap->isRunning())
    {
        // 如果等待取消后线程仍然没退出，不能把 cameraState 改回 Camera_Idle。
        // 否则用户马上点击 Capture 时会启动第二个 GetQHYCCDSingleFrame()，和未结束的重传下载线程抢资源。
        // 保持非空闲状态，让 Capture 入口的保护逻辑挡住下一次拍摄；等线程真正 finished 后再恢复 Idle。
        statusLabel_msg->setText(tr("Retransfer Timeout"));
        return false;
    }

    // 走到这里说明没有超时，或者超时后的下载线程已经退出，可以安全恢复相机状态。
    statusLabel_msg->setText(tr("IDLE"));
    ix.cameraState = Camera_Idle;
    ix.ForceStop = false;
    if(downloadCap == startedDownloadCap)
    {
        downloadCap = NULL;
    }

    // 重传触发成功但没有取回图像时，仍视为本次完整流程失败，方便 Favorite 显示 failed。
    if(ix.imageReady != GetSingleFrame_Success){
        return false;
    }

    // 以下逻辑和普通单帧 Capture 成功后的显示流程保持一致：
    // 更新当前工作模式、灰度拉伸参数、主图显示、ScreenView、直方图、状态栏尺寸和 Count。
    if(ix.workMode != ix.lastWorkMode)
    {
        ix.lastWorkMode = ix.workMode;
    }

    int wpos = 0, bpos = 0;
    if(ix.workMode == WorkMode_Preview)
    {
        wpos = Preview_WPOS;
        bpos = Preview_BPOS;
    }
    else
    {
        wpos = Capture_WPOS;
        bpos = Capture_BPOS;
    }
    managerMenu->ui->hSlider_bPos->setValue(bpos);
    managerMenu->ui->hSlider_wPos->setValue(wpos);
    setStretchLUT(wpos, bpos);

    displaySingleFrame(ix.FrameW_Last, ix.FrameH_Last, ix.ImgData_Last);

    if(ix.zoomMode == Zoom_FitWindow || ix.zoomMode == Zoom_FillWindow)
    {
        viewBoxW = managerMenu->ui->img_screenView->width();
        viewBoxH = managerMenu->ui->img_screenView->height();
    }
    else
    {
        viewBoxW = managerMenu->ui->img_screenView->width()  * scrollArea_ImgShow->width()  / (ix.FrameW_Last * ix.scaleFactor);
        viewBoxH = managerMenu->ui->img_screenView->height() * scrollArea_ImgShow->height() / (ix.FrameH_Last * ix.scaleFactor);
        viewBoxCX = managerMenu->ui->img_screenView->width()  * (double)ix.showLabelX / ix.scaleFactor / ix.FrameW_Last + viewBoxW / 2;
        viewBoxCY = managerMenu->ui->img_screenView->height() * (double)ix.showLabelY / ix.scaleFactor / ix.FrameH_Last + viewBoxH / 2;
    }

    if(viewBoxCX < viewBoxW / 2) viewBoxCX = viewBoxW / 2;
    if(viewBoxCY < viewBoxH / 2) viewBoxCY = viewBoxH / 2;
    if(viewBoxCX > (managerMenu->ui->img_screenView->width() - viewBoxW / 2 - 1))
        viewBoxCX = managerMenu->ui->img_screenView->width() - viewBoxW / 2 - 1;
    if(viewBoxCY > (managerMenu->ui->img_screenView->height() - viewBoxH / 2 - 1))
        viewBoxCY = managerMenu->ui->img_screenView->height() - viewBoxH / 2 - 1;
    displayScreenViewImage(viewBoxW, viewBoxH, viewBoxCX, viewBoxCY);

    displayHistogramImage(ix.FrameW_Last, ix.FrameH_Last, ix.ImgData_Last);
    getOverScanBlack(ix.ImgData_Last, ix.FrameW_Last, ix.FrameH_Last);

    QString str1 = QString::number(ix.FrameW_Last) + "x" + QString::number(ix.FrameH_Last);
    statusLabel_imgSize->setText(str1);
    showFrameCount();
    noImgInWorkMode = false;

    mainMenuBar->actOpenFolder->setEnabled(true);
    mainMenuBar->actSaveBMP->setEnabled(true);
    mainMenuBar->actSaveFIT->setEnabled(true);
    mainMenuBar->actSaveJPG->setEnabled(true);
    mainMenuBar->actSavePNG->setEnabled(true);
    mainMenuBar->actSaveTIF->setEnabled(true);

    return true;
}

void EZCAP::retransferDownloadFinished()
{
    // 只有当前 finished 信号来自仍被 EZCAP 记录的 downloadCap 时，才清空指针。
    // 这样可以避免后续代码误以为还有一个正在运行的下载线程。
    if(sender() == downloadCap)
    {
        downloadCap = NULL;
    }

    // 重传超时后，主流程会设置 ForceStop=true 并尝试取消 SDK 读出。
    // 如果后台 DownloadCapThread 是在主流程返回之后才真正结束，就由这个槽补做最终收尾：
    // 清掉 ForceStop，把 cameraState 恢复为 Idle，并更新界面进度。
    // 这样用户等线程退出后再次点击 Capture 时，就不会被前一次超时状态卡住。
    if(ix.imageReady == GetSingleFrame_Failed && ix.ForceStop)
    {
        ix.ForceStop = false;
        ix.cameraState = Camera_Idle;
        statusLabel_msg->setText(tr("IDLE"));
        managerMenu->ui->proBar_capture->setValue(100);
    }
}

void EZCAP::displaySingleFrame(uint32_t imgw, uint32_t imgh, unsigned char *imgdata)
{
    if(ix.imageReady != GetSingleFrame_Success) return;

    cv::Mat ImgShowTemp(ImgShow.size(), ImgShow.type());
    ImgShow.copyTo(ImgShowTemp);

    if(ix.workMode == WorkMode_Preview && ix.workMode == ix.lastWorkMode)
    {
        //add image shape
        rectangle(ImgShowTemp, Point(FocusCenterX_Pre-200/2, FocusCenterY_Pre-100/2), Point(FocusCenterX_Pre+200/2, FocusCenterY_Pre+100/2), Scalar(0, 0, 255), 1, LINE_8, 0);
        line(ImgShowTemp, Point(FocusCenterX_Pre-3, FocusCenterY_Pre), Point(FocusCenterX_Pre+3, FocusCenterY_Pre), Scalar(0, 0, 255), 1, LINE_8, 0);
        line(ImgShowTemp, Point(FocusCenterX_Pre, FocusCenterY_Pre-3), Point(FocusCenterX_Pre, FocusCenterY_Pre+3), Scalar(0, 0, 255), 1, LINE_8, 0);
        putText(ImgShowTemp, "Focus Area", Point(FocusCenterX_Pre-200/2, FocusCenterY_Pre-100/2-3), FONT_HERSHEY_COMPLEX, 0.5, Scalar(0, 0, 255), 1, LINE_8, false);

        if (managerMenu->ui->pBtn_cross->text() == "+")
        {
            line(ImgShowTemp, Point(0,                ImgShowTemp.rows/2), Point(ImgShowTemp.cols/2-4, ImgShowTemp.rows/2), Scalar(0, 255, 0), 1, LINE_8, 0);
            line(ImgShowTemp, Point(ImgShowTemp.cols/2+4, ImgShowTemp.rows/2), Point(ImgShowTemp.cols,     ImgShowTemp.rows/2), Scalar(0, 255, 0), 1, LINE_8, 0);
            line(ImgShowTemp, Point(ImgShowTemp.cols/2,                0), Point(ImgShowTemp.cols/2, ImgShowTemp.rows/2-4), Scalar(0, 255, 0), 1, LINE_8, 0);
            line(ImgShowTemp, Point(ImgShowTemp.cols/2, ImgShowTemp.rows/2+4), Point(ImgShowTemp.cols/2,     ImgShowTemp.rows), Scalar(0, 255, 0), 1, LINE_8, 0);
        }

        if (managerMenu->ui->pBtn_grid->text() == QString::fromUtf8("▓▓"))
        {
            line(ImgShowTemp, Point(0,   ImgShowTemp.rows/4), Point(ImgShowTemp.cols,   ImgShowTemp.rows/4), Scalar(0, 255, 0), 1, LINE_8, 0);
            line(ImgShowTemp, Point(0,   ImgShowTemp.rows/2), Point(ImgShowTemp.cols,   ImgShowTemp.rows/2), Scalar(0, 255, 0), 1, LINE_8, 0);
            line(ImgShowTemp, Point(0, ImgShowTemp.rows*3/4), Point(ImgShowTemp.cols, ImgShowTemp.rows*3/4), Scalar(0, 255, 0), 1, LINE_8, 0);
            line(ImgShowTemp, Point(ImgShowTemp.cols/4,   0), Point(ImgShowTemp.cols/4,   ImgShowTemp.rows), Scalar(0, 255, 0), 1, LINE_8, 0);
            line(ImgShowTemp, Point(ImgShowTemp.cols/2,   0), Point(ImgShowTemp.cols/2,   ImgShowTemp.rows), Scalar(0, 255, 0), 1, LINE_8, 0);
            line(ImgShowTemp, Point(ImgShowTemp.cols*3/4, 0), Point(ImgShowTemp.cols*3/4, ImgShowTemp.rows), Scalar(0, 255, 0), 1, LINE_8, 0);
        }

        if (managerMenu->ui->pBtn_circle->text() == QString::fromUtf8("◎"))
        {
            circle(ImgShowTemp, Point(ImgShowTemp.cols/2, ImgShowTemp.rows/2), ImgShowTemp.rows/2, Scalar(0, 255, 255), 1, LINE_8, 0);
            circle(ImgShowTemp, Point(ImgShowTemp.cols/2, ImgShowTemp.rows/2), ImgShowTemp.rows/6, Scalar(0, 255, 255), 1, LINE_8, 0);
            circle(ImgShowTemp, Point(ImgShowTemp.cols/2, ImgShowTemp.rows/2),         10, Scalar(0, 200, 200), 1, LINE_8, 0);
        }
    }
    else if(ix.workMode == WorkMode_Capture && ix.workMode == ix.lastWorkMode)
    {
        if(ix.IgnoreOverscan)
        {
            rectangle(ImgShowTemp, Point(ix.EffectiveX, ix.EffectiveY), Point(ix.EffectiveW-1+ix.EffectiveX, ix.EffectiveH-1+ix.EffectiveY), Scalar(0, 255, 0), 1, LINE_8, 0);
        }

        //显示图像噪声分析
        if(mainMenuBar->actNoiseAnalyse->isChecked())
        {
            ImageAnalyze(ImgShowTemp, imgw, imgh);
        }
    }

    ui->label_ImgShow->setMaximumHeight(16777215);
    ui->label_ImgShow->setMaximumWidth(16777215);

    if(ix.zoomMode == Zoom_FitWindow)
    {
        scrollArea_ImgShow->setWidgetResizable(true);

        double rate_label = (double)ui->label_ImgShow->width() / (double)ui->label_ImgShow->height();
        double rate_image = (double)ImgShowTemp.cols / (double)ImgShowTemp.rows;
        if(rate_label > rate_image)
            cv::copyMakeBorder(ImgShowTemp, ImgShowTemp, 0, 0, 0, ImgShowTemp.rows * rate_label - ImgShowTemp.cols, BORDER_CONSTANT, cv::Scalar(100, 100, 115));
        else
            cv::copyMakeBorder(ImgShowTemp, ImgShowTemp, 0, ImgShowTemp.cols / rate_label - ImgShowTemp.rows, 0, 0, BORDER_CONSTANT, cv::Scalar(100, 100, 115));
        cv::resize(ImgShowTemp, ImgShowTemp, cv::Size(ui->label_ImgShow->width(), ui->label_ImgShow->height()), 0, 0, CV_INTER_LINEAR);
    }
    else if(ix.zoomMode == Zoom_FillWindow) scrollArea_ImgShow->setWidgetResizable(true);
    else
    {
        ui->label_ImgShow->adjustSize();
        ui->label_ImgShow->resize(ix.FrameW_Last*ix.scaleFactor, ix.FrameH_Last*ix.scaleFactor);
    }

    qImg_show = MatToQImage(ImgShowTemp);
    if(qImg_show) ui->label_ImgShow->setPixmap(QPixmap::fromImage(*qImg_show));

    if(updateHoverLabelPosFromCursor())
    {
        updateHoverPixelStatus(hoverLabelPos, ImgShow);
        updatePixelMagnifier(hoverLabelPos);
    }

    ImgShowTemp.release();
}

//******************************************************************************
//                         histogram页中操作
//******************************************************************************
/**
 * @brief EZCAP::mgrMenu_hSlider_bPos_sliderReleased
 */
void EZCAP::mgrMenu_hSlider_bPos_sliderReleased()
{
    if(ix.camStreamMode == 0)
    {
        if(ix.workMode == WorkMode_Preview)
        {
            Preview_BPOS = managerMenu->ui->hSlider_bPos->value();
            iniFileParams.bPos_Preview = Preview_BPOS;
            setStretchLUT(Preview_WPOS, Preview_BPOS);  //set the stretch LUT value

            if(ix.lastWorkMode == ix.workMode && noImgInWorkMode == false)
            {
                displaySingleFrame(ix.FrameW_Last, ix.FrameH_Last, ix.ImgData_Last);

                displayScreenViewImage(viewBoxW, viewBoxH, viewBoxCX, viewBoxCY);
            }
        }
        else if(ix.workMode == WorkMode_Focus)
        {
            Focus_BPOS = managerMenu->ui->hSlider_bPos->value();
            iniFileParams.bPos_Focus = Focus_BPOS;
            setStretchLUT(Focus_WPOS, Focus_BPOS);  //set the stretch LUT value

            if(ix.lastWorkMode == ix.workMode && noImgInWorkMode == false)
            {
                displayFocusImage_Ex(ix.FrameW_Last, ix.FrameH_Last, ix.ImgData_Last);
            }
        }
        else if(ix.workMode == WorkMode_Capture)
        {
            Capture_BPOS = managerMenu->ui->hSlider_bPos->value();
            iniFileParams.bPos_Capture = Capture_BPOS;
            setStretchLUT(Capture_WPOS, Capture_BPOS);  //set the stretch LUT value

            if(ix.lastWorkMode == ix.workMode && noImgInWorkMode == false)
            {
                displaySingleFrame(ix.FrameW_Last, ix.FrameH_Last, ix.ImgData_Last);

                displayScreenViewImage(viewBoxW, viewBoxH, viewBoxCX, viewBoxCY);
            }
        }
    }
    else
    {
        showMutex.lock();
        Live_BPOS = managerMenu->ui->hSlider_bPos->value();
        setStretchLUT(Live_WPOS, Live_BPOS);
        showMutex.unlock();
    }
}

/**
 * @brief EZCAP::mgrMenu_hSlider_wPos_sliderReleased
 */
void EZCAP::mgrMenu_hSlider_wPos_sliderReleased()
{
    if(ix.camStreamMode == 0)
    {
        if(ix.workMode == WorkMode_Preview)
        {
            Preview_WPOS = managerMenu->ui->hSlider_wPos->value();
            iniFileParams.wPos_Preview = Preview_WPOS;
            setStretchLUT(Preview_WPOS, Preview_BPOS);  //set the stretch LUT value

            if(ix.lastWorkMode == ix.workMode && noImgInWorkMode == false)
            {
                displaySingleFrame(ix.FrameW_Last, ix.FrameH_Last, ix.ImgData_Last);

                displayScreenViewImage(viewBoxW, viewBoxH, viewBoxCX, viewBoxCY);
            }
        }
        else if(ix.workMode == WorkMode_Focus)
        {
            Focus_WPOS = managerMenu->ui->hSlider_wPos->value();
            iniFileParams.wPos_Focus = Focus_WPOS;
            setStretchLUT(Focus_WPOS, Focus_BPOS);  //set the stretch LUT value

            if(ix.lastWorkMode == ix.workMode && noImgInWorkMode == false)
            {
                displayFocusImage_Ex(ix.FrameW_Last, ix.FrameH_Last, ix.ImgData_Last);
            }
        }
        else if(ix.workMode == WorkMode_Capture)
        {
            Capture_WPOS = managerMenu->ui->hSlider_wPos->value();
            iniFileParams.wPos_Capture = Capture_WPOS;
            setStretchLUT(Capture_WPOS, Capture_BPOS);  //set the stretch LUT value

            if(ix.lastWorkMode == ix.workMode && noImgInWorkMode == false)
            {
                displaySingleFrame(ix.FrameW_Last, ix.FrameH_Last, ix.ImgData_Last);

                displayScreenViewImage(viewBoxW, viewBoxH, viewBoxCX, viewBoxCY);
            }
        }
    }
    else
    {
        showMutex.lock();
        Live_WPOS = managerMenu->ui->hSlider_wPos->value();
        setStretchLUT(Live_WPOS, Live_BPOS);
        showMutex.unlock();
    }
}

/**
 * @brief EZCAP::mgrMenu_pBtn_stretchMinusB_clicked
 */
void EZCAP::mgrMenu_pBtn_stretchMinusB_clicked()
{
    int curPos = managerMenu->ui->hSlider_bPos->value();
    managerMenu->ui->hSlider_bPos->setValue(curPos - ix.StretchStep);

    mgrMenu_hSlider_bPos_sliderReleased();
}
/**
 * @brief EZCAP::mgrMenu_pBtn_stretchPlusB_clicked
 */
void EZCAP::mgrMenu_pBtn_stretchPlusB_clicked()
{
    managerMenu->ui->hSlider_bPos->setValue(managerMenu->ui->hSlider_bPos->value() + ix.StretchStep);
    mgrMenu_hSlider_bPos_sliderReleased();
}
/**
 * @brief EZCAP::mgrMenu_pBtn_stretchMinusW_clicked
 */
void EZCAP::mgrMenu_pBtn_stretchMinusW_clicked()
{
    managerMenu->ui->hSlider_wPos->setValue(managerMenu->ui->hSlider_wPos->value() - ix.StretchStep);
    mgrMenu_hSlider_wPos_sliderReleased();
}
/**
 * @brief EZCAP::mgrMenu_pBtn_stretchPlusW_clicked
 */
void EZCAP::mgrMenu_pBtn_stretchPlusW_clicked()
{
    managerMenu->ui->hSlider_wPos->setValue(managerMenu->ui->hSlider_wPos->value() + ix.StretchStep);
    mgrMenu_hSlider_wPos_sliderReleased();
}

void HistInfo(uint32_t x,uint32_t y,uint8_t *InBuf,uint8_t *outBuf)
{
    uint32_t s,k;
    uint32_t i;
    uint32_t Histogram[256];
    uint32_t pixel;
    uint32_t maxHist;

    Mat histImg, histResizeImg;

    histImg.create(Size(256, 100), CV_8UC3);
    histResizeImg.create(Size(managerMenu->ui->img_hist->width(), managerMenu->ui->img_hist->height()), CV_8UC3);

    histImg.setTo(Scalar(0, 0, 0));
    s=x * y ;
    for (i=0;i<256;i++) {Histogram[i]=0;}
    k=1;

    while(s)
    {
        pixel=InBuf[k];
        Histogram[pixel]++;
        k=k+2;
        s--;
    }

    maxHist=Histogram[0];
    for (uint32_t i=1; i < 255; i++)
    {
        if (Histogram[i]>maxHist)
        {
            maxHist=Histogram[i];
        }
    }

    if (maxHist==0)   maxHist=1;

    for (i=0;i<256;i++)
    {
        line(histImg, Point(i, 100), Point(i, 100-Histogram[i]*256/maxHist), Scalar(255, 0, 0), 1, LINE_8, 0);
    }

    cv::resize(histImg, histResizeImg, Size(histResizeImg.cols, histResizeImg.rows), 0, 0, INTER_CUBIC);

    int copyLen = histResizeImg.cols * histResizeImg.rows * (histResizeImg.depth()==CV_8U?1:2) * histResizeImg.channels();
    memcpy(outBuf, histResizeImg.data, copyLen);

    histImg.release();
    histResizeImg.release();
}

/**
 * @brief EZCAP::displayHistogramImage
 * @param x
 * @param y
 * @param buf
 */
void EZCAP::displayHistogramImage(int x, int y, unsigned char *buf)
{
//    unsigned int pixel;
//    long s,k;

//    Mat histImg(Size(249, 141), CV_8UC3);
//    unsigned char *outBuf = (unsigned char*)malloc(35000000);
//    if(outBuf)
//    {
//        //获取直方图数据
//        HistInfo(x,y,buf,outBuf);
//        histImg.data = outBuf;

//        //根据buf中数据，保存相应数据到histogram中，用于之后的自动拉伸功能
//        s = x * y ;
//        for(int i=0; i<256; i++)
//        {
//            ix.Histogram[i] = 0;//初始化Histogram数组
//        }
//        k=1;
//        //给histogram数组赋值
//        while(s)
//        {
//            pixel = buf[k];
//            ix.Histogram[pixel]++;
//            k = k+2;
//            s--;
//        }

//        //opencv默认彩色模式BRG,  QImage默认为RGB,故先进行颜色转换，防止IplImage转QImage后红色变蓝色，蓝色变红色
//        QImage *histgramQImg = MatToQImage(histImg);

//        //显示histogram
//        managerMenu->ui->img_hist->setPixmap(QPixmap::fromImage(*histgramQImg));

//        free(outBuf);
//        outBuf = NULL;

//        histImg.release();
//    }

    if(ImgHist.empty()) return;

    QImage *histQImg = MatToQImage(ImgHist);
    managerMenu->ui->img_hist->clear();
    managerMenu->ui->img_hist->setPixmap(QPixmap::fromImage(*histQImg));
}

void EZCAP::displayHistogramImageLive()
{
    cv::Mat histImg(Size(256, 100), CV_8UC3);

    histImg.setTo(Scalar(0, 0, 0));

    uint32_t Histogram[256] = { 0 };
    long k = 0, s = ix.FrameW_Last * ix.FrameH_Last;
    uint32_t maxHist;
    int index = 0;

    while(s)
    {
        if(ix.FrameB_Last == 8 && ix.FrameC_Last == 1)
            index = ix.ImgData_Last[k];
        else if(ix.FrameB_Last == 16 && ix.FrameC_Last == 1)
            index = ix.ImgData_Last[2 * k + 1];
        else if(ix.FrameB_Last == 8 && ix.FrameC_Last == 3)
            index = (ix.ImgData_Last[2 * k + 5] * 30 + ix.ImgData_Last[2 * k + 3] * 150 + ix.ImgData_Last[2 * k + 1] * 76) >> 8;
        Histogram[index]++;
        k = k + 1;
        s--;
    }

    maxHist = Histogram[0];
    for(int i = 1; i < 255; i++)
    {
        if(Histogram[i] > maxHist)
        {
            maxHist = Histogram[i];
        }
    }
    if(maxHist==0) maxHist=1;

    for(int i = 0; i < 256; i++)
    {
        line(histImg, Point(i, 100), Point(i,100-Histogram[i]*256/maxHist), Scalar(255, 0, 0), 1, LINE_8, 0);
    }

    cv::resize(histImg, ImgHist, ImgHist.size(), 0, 0, INTER_CUBIC);

    histImg.release();

    QImage *histQImg = MatToQImage(ImgHist);

    managerMenu->ui->img_hist->clear();
    managerMenu->ui->img_hist->setPixmap(QPixmap::fromImage(*histQImg));
}

/**
 * @brief EZCAP::mgrMenu_pBtn_auto_histogram_clicked
 */
void EZCAP::mgrMenu_pBtn_auto_histogram_clicked()
{
    //输入HIST信息，然后进行分析，获得自动stretch的数据，进行autostretch
    unsigned long maxHist;
    unsigned char maxHistIndex;
    int W_POS = 1, B_POS = 0;

    //求最大值
    maxHist = ix.Histogram[0];
    maxHistIndex = 0;
    for(int i = 1; i < 255; i++)
    {
        if(ix.Histogram[i]>maxHist)
        {
            maxHist = ix.Histogram[i];
            //ix.maxHistPosition = i;
            maxHistIndex = i;
        }
    }

    //设置自动拉伸的W B值
    switch(ix.autoStretchMode)
    {
        case StretchMode_NoiseFloor:
        {
            if(maxHistIndex < 1)
                maxHistIndex = 1;
            else if(maxHistIndex > 254)
                maxHistIndex = 254;

            B_POS = 256 * (maxHistIndex - 1);
            W_POS = 256 * (maxHistIndex + 1);
        }
        break;
        case StretchMode_BackGroundLevel:
        {
            if(maxHistIndex < 4)
                maxHistIndex = 4;
            else if(maxHistIndex > 245)
                maxHistIndex = 245;

            B_POS = 256 * (maxHistIndex - 3);
            W_POS = 256 * (maxHistIndex + 10);
        }
        break;
        case StretchMode_3timesBackGround:
        {
            if(maxHistIndex < 11)
                maxHistIndex = 11;
            else if(maxHistIndex > 215)
                maxHistIndex = 215;

            B_POS = 256 * (maxHistIndex - 10);
            W_POS = 256 * (maxHistIndex + 30);
        }
        break;
        case StretchMode_10timesBackGround:
        {
            if(maxHistIndex < 31)
                maxHistIndex = 31;
            else if(maxHistIndex > 200)
                maxHistIndex = 200;

            B_POS= 256 * (maxHistIndex - 30);
            W_POS= 256 * (maxHistIndex + 50);
        }
        break;
        case StretchMode_MaxRange:
        {
            B_POS = 0;
            W_POS = 65535;
        }
        break;
        case StretchMode_OverScanX256:
        {
            B_POS = OverScanRMS;
            W_POS = OverScanRMS + 256;
        }
        break;
        case StretchMode_OverScanX128:
        {
            B_POS = OverScanRMS;
            W_POS = OverScanRMS + 512;
        }
        break;
        case StretchMode_OverScanX64:
        {
            B_POS = OverScanRMS;
            W_POS = OverScanRMS + 1028;
        }
        break;
        case StretchMode_OverScanX32:
        {
            B_POS = OverScanRMS;
            W_POS = OverScanRMS + 2048;
        }
        break;
        case StretchMode_OverScanX16:
        {
            B_POS = OverScanRMS;
            W_POS = OverScanRMS + 4096;
        }
        break;
        case StretchMode_OverScanX8:
        {
            B_POS = OverScanRMS;
            W_POS = OverScanRMS + 8192;
        }
        break;

    }
    //越界处理
    if(W_POS > 65535)
        W_POS = 65535;
    if(W_POS < 0)
        W_POS = 0;
    if(B_POS > 65535)
        B_POS = 65535;
    if(B_POS < 0)
        B_POS = 0;

    managerMenu->ui->hSlider_bPos->setValue(B_POS);
    managerMenu->ui->hSlider_wPos->setValue(W_POS);
    mgrMenu_hSlider_bPos_sliderReleased();
    mgrMenu_hSlider_wPos_sliderReleased();

}
//------------------------------------------------------------------------------

//******************************************************************************
//                         screenview页中操作
//******************************************************************************
/**
 * @brief EZCAP::displayScreenViewImage
 * @param boxWidth
 * @param boxHeight
 * @param boxX
 * @param boxY
 */
void EZCAP::displayScreenViewImage(int boxWidth, int boxHeight, int boxX, int boxY)
{
    if(ix.camStreamMode == 0 && ix.imageReady != GetSingleFrame_Success) return;

    int w = managerMenu->ui->img_screenView->width(); //249
    int h = managerMenu->ui->img_screenView->height(); //181

    cv::Mat ImgViewTemp(ImgView.size(), ImgView.type());

    if(ix.camStreamMode == 0)
    {
        ImgView.copyTo(ImgViewTemp);

        if(ix.zoomMode == Zoom_SpecifyScaling && (boxWidth < w || boxHeight < h) && (boxX < w || boxY < h))
        {
            rectangle(ImgViewTemp, Point(boxX-boxWidth/2, boxY-boxHeight/2), Point(boxX+boxWidth/2, boxY+boxHeight/2), Scalar(255, 0, 0), 1, LINE_8, 0);
        }
    }
    else
    {
        memset(ImgViewTemp.data, 0, w * h * 3);

        if((boxWidth < w || boxHeight < h) && (boxX < w || boxY < h))
        {
            rectangle(ImgViewTemp, Point(boxX-boxWidth/2, boxY-boxHeight/2), Point(boxX+boxWidth/2, boxY+boxHeight/2), Scalar(255, 0, 0), 1, LINE_8, 0);
        }
    }

    QImage *screenQImg = MatToQImage(ImgViewTemp);

    managerMenu->ui->img_screenView->setPixmap(QPixmap::fromImage(*screenQImg));

    ImgViewTemp.release();
}
//------------------------------------------------------------------------------

//*************************************************************************************
//            事件过滤器以及相关事件
//*************************************************************************************
/**
 * @brief EZCAP::displayedImageMouseDown
 * @param posX
 * @param posY
 */
void EZCAP::displayedImageMouseDown(int posX, int posY)
{
    if(ix.workMode == WorkMode_Preview && ix.lastWorkMode == WorkMode_Preview && ix.imageReady == GetSingleFrame_Success)
    {
        if (ix.FrameW_Last > 0)
        {
            bool canDraw = true;
            if(ix.zoomMode == Zoom_FitWindow)
            {
                double rate_label = (double)ui->label_ImgShow->width() / (double)ui->label_ImgShow->height();
                double rate_image = (double)ix.FrameW_Last / (double)ix.FrameH_Last;
                int limited_w = ui->label_ImgShow->width(), limited_h = ui->label_ImgShow->height();

                if(rate_label > rate_image)
                {
                    limited_w = ui->label_ImgShow->height() * rate_image;
                    FocusCenterX_Pre = posX * (int)ix.FrameW_Last / limited_w;
                    FocusCenterY_Pre = posY * (int)ix.FrameH_Last / ui->label_ImgShow->height();
                }
                else
                {
                    limited_h = ui->label_ImgShow->width() / rate_image;
                    FocusCenterX_Pre = posX * (int)ix.FrameW_Last / ui->label_ImgShow->width();
                    FocusCenterY_Pre = posY * (int)ix.FrameH_Last / limited_h;
                }
                if(posX >= limited_w || posY >= limited_h) canDraw = false;
            }
            else if(ix.zoomMode == Zoom_FillWindow)
            {
                FocusCenterX_Pre = posX * (int)ix.FrameW_Last / ui->label_ImgShow->width();
                FocusCenterY_Pre = posY * (int)ix.FrameH_Last / ui->label_ImgShow->height();
            }
            else
            {
                FocusCenterX_Pre = (posX + ix.showLabelX) / ix.scaleFactor;
                FocusCenterY_Pre = (posY + ix.showLabelY) / ix.scaleFactor;
            }

            if(canDraw) displaySingleFrame(ix.FrameW_Last, ix.FrameH_Last, ix.ImgData_Last);
        }
    }
    else if(ix.workMode == WorkMode_Focus && ix.lastWorkMode == WorkMode_Focus && ix.imageReady == GetSingleFrame_Success)
    {
#if 0
        if (ix.FrameW != 0)
        {
            if (posX < 800 && posY < 200)
            {
                ZoomFocus_X = posX;
                ZoomFocus_Y = posY;
            }

            this->displayFocusImage(ix.FrameW, ix.FrameH);
        }
#else
        if (ix.FrameW_Last > 0)
        {
            if (posX < 800 && posY < 200)
            {
                ZoomFocus_X = posX;
                ZoomFocus_Y = posY;

                displayFocusImage_Ex(ix.FrameW_Last, ix.FrameH_Last, ix.ImgData_Last);
            }
        }
#endif
    }
}

/**
 * @brief EZCAP::screenViewAreaMouseDown
 * @param posX
 * @param posY
 */
void EZCAP::screenViewAreaMouseDown(int posX, int posY)
{
    if((ix.workMode == WorkMode_Capture || ix.workMode == WorkMode_Preview) && ix.zoomMode != Zoom_FillWindow)
    {
        viewBoxCX = posX;
        viewBoxCY = posY;

        screenViewBoxResize();
    }
    else if(ix.camStreamMode == 1)
    {
        viewBoxCX = posX;
        viewBoxCY = posY;

        screenViewBoxResize();
    }
}

void EZCAP::screenViewBoxResize()
{
    viewBoxW = managerMenu->ui->img_screenView->width()  * scrollArea_ImgShow->width()  / (ix.FrameW_Last * ix.scaleFactor);
    viewBoxH = managerMenu->ui->img_screenView->height() * scrollArea_ImgShow->height() / (ix.FrameH_Last * ix.scaleFactor);
    ix.showLabelX = ((double)viewBoxCX - (double)viewBoxW / 2) / managerMenu->ui->img_screenView->width() * ui->label_ImgShow->width();
    if((int)ix.showLabelX < 0) ix.showLabelX = 0;
    if((int)ix.showLabelX > scrollArea_ImgShow->horizontalScrollBar()->maximum()) ix.showLabelX = scrollArea_ImgShow->horizontalScrollBar()->maximum();
    ix.showLabelY = ((double)viewBoxCY - (double)viewBoxH / 2) / managerMenu->ui->img_screenView->height() * ui->label_ImgShow->height();
    if((int)ix.showLabelY < 0) ix.showLabelY = 0;
    if((int)ix.showLabelY > scrollArea_ImgShow->verticalScrollBar()->maximum()) ix.showLabelY = scrollArea_ImgShow->verticalScrollBar()->maximum();

    scrollArea_ImgShow->horizontalScrollBar()->setValue(ix.showLabelX);
    scrollArea_ImgShow->verticalScrollBar()->setValue(ix.showLabelY);

    if(viewBoxCX < viewBoxW / 2) viewBoxCX = viewBoxW / 2;
    if(viewBoxCY < viewBoxH / 2) viewBoxCY = viewBoxH / 2;
    if(viewBoxCX > (managerMenu->ui->img_screenView->width() - viewBoxW / 2 - 1))
        viewBoxCX = managerMenu->ui->img_screenView->width() - viewBoxW / 2 - 1;
    if(viewBoxCY > (managerMenu->ui->img_screenView->height() - viewBoxH / 2 - 1))
        viewBoxCY = managerMenu->ui->img_screenView->height() - viewBoxH / 2 - 1;
    displayScreenViewImage(viewBoxW, viewBoxH, viewBoxCX, viewBoxCY);
    if(ix.zoomMode == Zoom_FitWindow && ix.imageReady == GetSingleFrame_Success)
    {
        displaySingleFrame(ix.FrameW_Last, ix.FrameH_Last, ix.ImgData_Last);
    }
}

void EZCAP::resizeEvent(QResizeEvent *)
{
    screenViewBoxResize(); // window resize, then screenview box redraw...
}

/**
 * @brief EZCAP::focusAssistantImageDblClick
 */
void EZCAP::focusAssistantImageDblClick()
{
    FocusZoomMode = !FocusZoomMode;
    displayFocusImage_Ex(ix.FrameW_Last, ix.FrameH_Last, ix.ImgData_Last);
}

bool EZCAP::mapHoverPointToImage(const QPoint &labelPos, const cv::Mat &image, QPoint &imagePos, QPoint &matPos)
{
    int labelW = ui->label_ImgShow->width();
    int labelH = ui->label_ImgShow->height();
    int frameW = (int)ix.FrameW_Last;
    int frameH = (int)ix.FrameH_Last;

    if(labelW <= 0 || labelH <= 0 || frameW <= 0 || frameH <= 0)
        return false;
    if(labelPos.x() < 0 || labelPos.y() < 0 || labelPos.x() >= labelW || labelPos.y() >= labelH)
        return false;

    int x = 0, y = 0;
    bool canShow = true;

    if(ix.zoomMode == Zoom_FitWindow)
    {
        double rateLabel = (double)labelW / (double)labelH;
        double rateImage = (double)frameW / (double)frameH;
        int limitedW = labelW;
        int limitedH = labelH;

        if(rateLabel > rateImage)
        {
            limitedW = labelH * rateImage;
            if(limitedW <= 0) limitedW = 1;
            x = labelPos.x() * frameW / limitedW;
            y = labelPos.y() * frameH / labelH;
        }
        else
        {
            limitedH = labelW / rateImage;
            if(limitedH <= 0) limitedH = 1;
            x = labelPos.x() * frameW / labelW;
            y = labelPos.y() * frameH / limitedH;
        }

        if(labelPos.x() >= limitedW || labelPos.y() >= limitedH)
            canShow = false;
    }
    else if(ix.zoomMode == Zoom_FillWindow)
    {
        x = labelPos.x() * frameW / labelW;
        y = labelPos.y() * frameH / labelH;
    }
    else
    {
        if(ix.camStreamMode == 1)
        {
            x = ((double)ix.showLabelX + labelPos.x()) / ix.scaleFactor;
            y = ((double)ix.showLabelY + labelPos.y()) / ix.scaleFactor;
        }
        else
        {
            x = labelPos.x() / ix.scaleFactor;
            y = labelPos.y() / ix.scaleFactor;
        }
    }

    if(!canShow || x < 0 || y < 0 || x >= frameW || y >= frameH)
        return false;

    imagePos = QPoint(x, y);
    matPos = imagePos;

    if(!image.empty())
    {
        if(ix.camStreamMode == 1 || image.cols != frameW || image.rows != frameH)
        {
            int matX = labelPos.x() * image.cols / labelW;
            int matY = labelPos.y() * image.rows / labelH;
            matPos = QPoint(matX, matY);
        }

        if(matPos.x() < 0 || matPos.y() < 0 || matPos.x() >= image.cols || matPos.y() >= image.rows)
            return false;
    }

    return true;
}

bool EZCAP::updateHoverLabelPosFromCursor()
{
    QPoint labelPos = ui->label_ImgShow->mapFromGlobal(QCursor::pos());
    if(labelPos.x() < 0 || labelPos.y() < 0 ||
       labelPos.x() >= ui->label_ImgShow->width() ||
       labelPos.y() >= ui->label_ImgShow->height())
    {
        return hoverLabelPosValid;
    }

    hoverLabelPos = labelPos;
    hoverLabelPosValid = true;
    return true;
}

bool EZCAP::readRawGrayPixel(const QPoint &imagePos, int &gray)
{
    if(ix.ImgData_Last == NULL)
        return false;
    if(ix.FrameW_Last == 0 || ix.FrameH_Last == 0 || ix.FrameB_Last == 0 || ix.FrameC_Last == 0)
        return false;
    if(imagePos.x() < 0 || imagePos.y() < 0 || imagePos.x() >= (int)ix.FrameW_Last || imagePos.y() >= (int)ix.FrameH_Last)
        return false;

    uint32_t channelCount = ix.FrameC_Last;
    uint32_t bytesPerChannel = ix.FrameB_Last / 8;
    if(bytesPerChannel == 0)
        return false;

    uint32_t pixelIndex = (imagePos.y() * ix.FrameW_Last + imagePos.x()) * channelCount;
    uint32_t byteIndex = pixelIndex * bytesPerChannel;

    if(ix.FrameB_Last == 16)
    {
        gray = ix.ImgData_Last[byteIndex] + ix.ImgData_Last[byteIndex + 1] * 256;
        return true;
    }

    if(ix.FrameB_Last == 8)
    {
        gray = ix.ImgData_Last[byteIndex];
        return true;
    }

    return false;
}

void EZCAP::updateHoverPixelStatus(const QPoint &labelPos, const cv::Mat &image)
{
    QPoint imagePos;
    QPoint matPos;

    if(!mapHoverPointToImage(labelPos, image, imagePos, matPos))
    {
        clearHoverPixelStatus();
        return;
    }

    statusLabel_mousePos->setText("(" + QString::number(imagePos.x()) + "," + QString::number(imagePos.y()) + ")");

    if(image.empty())
        return;

    int r = 0, g = 0, b = 0;
    if(image.channels() == 1)
    {
        r = g = b = image.at<uchar>(matPos.y(), matPos.x());
    }
    else
    {
        cv::Vec3b p = image.at<cv::Vec3b>(matPos.y(), matPos.x());
        r = p[0];
        g = p[1];
        b = p[2];
    }

    bool showColorChannels = ix.Color_Fun && (ix.Color || ix.IsCvtColor);
    if(showColorChannels && image.channels() >= 3)
    {
        QString rgbStr = tr("r:") + QString::number(r) + "," +
                         tr("g:") + QString::number(g) + "," +
                         tr("b:") + QString::number(b);
        statusLabel_rgb->setText(rgbStr);
    }
    else
    {
        int gray = 0;
        if(!readRawGrayPixel(imagePos, gray))
            gray = (b + g + r) / 3;
        statusLabel_rgb->setText(tr("gray:") + QString::number(gray));
    }
}

void EZCAP::clearHoverPixelStatus()
{
    hoverLabelPosValid = false;
    statusLabel_mousePos->setText("");
    statusLabel_rgb->setText("");
    hidePixelMagnifier();
}

/**
 * @brief EZCAP::hidePixelMagnifier 隐藏像素放大镜
 */
void EZCAP::hidePixelMagnifier()
{
    if(pixelMagnifier && pixelMagnifier->isVisible())
        pixelMagnifier->hide();
}

/**
 * @brief EZCAP::updatePixelMagnifier
 * 鼠标悬停时更新像素放大镜：显示光标周围 NxN 区域的放大图、
 * 光标所在像素的数值以及区域统计信息（Min/Max/Mean/StdDev）。
 * @param labelPos 光标在 label_ImgShow 中的坐标
 */
void EZCAP::updatePixelMagnifier(const QPoint &labelPos)
{
    if(pixelMagnifier == NULL)
        return;

    //功能开关（Zoom 菜单中的 Pixel Magnifier）
    if(mainMenuBar->actPixelMagnifier == NULL || !mainMenuBar->actPixelMagnifier->isChecked())
    {
        hidePixelMagnifier();
        return;
    }

    if(ui->label_ImgShow->pixmap() == NULL || ix.FrameW_Last == 0 || ix.FrameH_Last == 0)
    {
        hidePixelMagnifier();
        return;
    }

    QPoint imagePos;
    QPoint matPos;
    cv::Mat emptyImage;
    if(!mapHoverPointToImage(labelPos, emptyImage, imagePos, matPos))
    {
        hidePixelMagnifier();
        return;
    }

    const int regionSize = 15;//统计区域边长（奇数，中心为光标像素）

    QImage region = sampleMagnifierRegion(imagePos, regionSize);

    QString pixelText;
    QStringList statLines;
    computeRegionStats(imagePos, region, pixelText, statLines);

    pixelMagnifier->setContent(region, pixelText, statLines);
    QPoint viewportPos = ui->label_ImgShow->mapTo(scrollArea_ImgShow->viewport(), labelPos);
    pixelMagnifier->placeNear(viewportPos, scrollArea_ImgShow->viewport()->size());
    if(!pixelMagnifier->isVisible())
        pixelMagnifier->show();
    pixelMagnifier->raise();
}

/**
 * @brief EZCAP::sampleMagnifierRegion
 * 从 label_ImgShow 当前显示内容中，按图像像素逐一采样，生成以 imagePos 为中心的
 * NxN 区域图（所见即所得：包含拉伸、伪彩等显示效果；图像范围外的像素填黑）。
 * @param imagePos   光标所在的图像像素坐标
 * @param regionSize 区域边长（奇数）
 * @return NxN 的 QImage
 */
QImage EZCAP::sampleMagnifierRegion(const QPoint &imagePos, int regionSize)
{
    QImage region(regionSize, regionSize, QImage::Format_RGB32);
    region.fill(QColor(0, 0, 0));

    const QPixmap *pm = ui->label_ImgShow->pixmap();
    if(pm == NULL || pm->isNull() || ix.FrameW_Last == 0 || ix.FrameH_Last == 0)
        return region;

    QImage shown = pm->toImage();
    int labelW = ui->label_ImgShow->width();
    int labelH = ui->label_ImgShow->height();
    if(shown.isNull() || labelW <= 0 || labelH <= 0)
        return region;

    //图像像素 -> label 像素 的映射（与 mapHoverPointToImage 互逆）
    double zx = 1.0, zy = 1.0, ox = 0.0, oy = 0.0;
    if(ix.zoomMode == Zoom_FitWindow)
    {
        zx = zy = qMin((double)labelW / (double)ix.FrameW_Last, (double)labelH / (double)ix.FrameH_Last);
    }
    else if(ix.zoomMode == Zoom_FillWindow)
    {
        zx = (double)labelW / (double)ix.FrameW_Last;
        zy = (double)labelH / (double)ix.FrameH_Last;
    }
    else
    {
        zx = zy = ix.scaleFactor;
        if(ix.camStreamMode == 1)
        {
            ox = -(double)ix.showLabelX;
            oy = -(double)ix.showLabelY;
        }
    }
    if(zx <= 0.0 || zy <= 0.0)
        return region;

    //pixmap 实际分辨率与 label 尺寸的比例（scaledContents 拉伸时进行归一化）
    const double rx = (double)shown.width()  / (double)labelW;
    const double ry = (double)shown.height() / (double)labelH;
    const int half = regionSize / 2;

    for(int j = 0; j < regionSize; ++j)
    {
        for(int i = 0; i < regionSize; ++i)
        {
            int px = imagePos.x() + i - half;
            int py = imagePos.y() + j - half;
            if(px < 0 || py < 0 || px >= (int)ix.FrameW_Last || py >= (int)ix.FrameH_Last)
                continue;//图像范围外保持黑色

            int lx = (int)((px + 0.5) * zx + ox);
            int ly = (int)((py + 0.5) * zy + oy);
            int sx = qBound(0, (int)(lx * rx), shown.width()  - 1);
            int sy = qBound(0, (int)(ly * ry), shown.height() - 1);
            region.setPixel(i, j, shown.pixel(sx, sy));
        }
    }

    return region;
}

/**
 * @brief EZCAP::computeRegionStats
 * 计算光标像素值与 NxN 区域统计信息。
 * 单色模式优先使用原始图像数据（8/16bit 精确值）；
 * 彩色模式或无原始数据时使用显示亮度（8bit）。
 * @param imagePos   光标所在的图像像素坐标
 * @param region     sampleMagnifierRegion 生成的 NxN 区域图
 * @param pixelText  输出：光标像素信息文本
 * @param statLines  输出：统计信息文本行（Min/Max/Mean/Std）
 * @return 是否成功计算
 */
bool EZCAP::computeRegionStats(const QPoint &imagePos, const QImage &region, QString &pixelText, QStringList &statLines)
{
    statLines.clear();

    const int n    = region.width();
    const int half = n / 2;

    bool colorActive = ix.Color_Fun && (ix.Color || ix.IsCvtColor);
    bool rawOk = !colorActive && ix.ImgData_Last != NULL &&
                 (ix.FrameB_Last == 8 || ix.FrameB_Last == 16) &&
                 ix.FrameC_Last == 1 &&
                 ix.FrameW_Last > 0 && ix.FrameH_Last > 0;

    double minV = 1e30, maxV = -1e30, sum = 0.0, sum2 = 0.0;
    int count = 0;

    if(rawOk)
    {
        const unsigned char *data = ix.ImgData_Last;
        const uint32_t w   = ix.FrameW_Last;
        const uint32_t h   = ix.FrameH_Last;
        const int      bpc = ix.FrameB_Last / 8;

        //光标所在像素的原始值
        size_t pidx = ((size_t)imagePos.y() * w + imagePos.x()) * bpc;
        unsigned int pv = (bpc == 2) ? (unsigned int)(data[pidx] | (data[pidx + 1] << 8))
                                     : (unsigned int)data[pidx];
        pixelText = "(" + QString::number(imagePos.x()) + "," + QString::number(imagePos.y()) + ")  " +
                    QString::number(pv);

        //区域统计（边界处裁剪）
        int x0 = qBound(0, imagePos.x() - half, (int)w - 1);
        int y0 = qBound(0, imagePos.y() - half, (int)h - 1);
        int x1 = qBound(0, imagePos.x() + half, (int)w - 1);
        int y1 = qBound(0, imagePos.y() + half, (int)h - 1);

        for(int yy = y0; yy <= y1; ++yy)
        {
            const unsigned char *row = data + (size_t)yy * w * bpc;
            for(int xx = x0; xx <= x1; ++xx)
            {
                size_t i = (size_t)xx * bpc;
                double v = (bpc == 2) ? (double)(row[i] | (row[i + 1] << 8))
                                      : (double)row[i];
                if(v < minV) minV = v;
                if(v > maxV) maxV = v;
                sum  += v;
                sum2 += v * v;
                ++count;
            }
        }
    }
    else
    {
        //光标像素显示颜色（彩色模式）
        QRgb center = region.pixel(half, half);
        pixelText = "(" + QString::number(imagePos.x()) + "," + QString::number(imagePos.y()) + ")  " +
                    "R:" + QString::number(qRed(center)) +
                    " G:" + QString::number(qGreen(center)) +
                    " B:" + QString::number(qBlue(center));

        //基于显示亮度的区域统计
        for(int j = 0; j < n; ++j)
        {
            for(int i = 0; i < n; ++i)
            {
                int px = imagePos.x() + i - half;
                int py = imagePos.y() + j - half;
                if(px < 0 || py < 0 || px >= (int)ix.FrameW_Last || py >= (int)ix.FrameH_Last)
                    continue;

                QRgb c = region.pixel(i, j);
                double v = (qRed(c) + qGreen(c) + qBlue(c)) / 3.0;
                if(v < minV) minV = v;
                if(v > maxV) maxV = v;
                sum  += v;
                sum2 += v * v;
                ++count;
            }
        }
    }

    if(count <= 0)
        return false;

    double mean = sum / count;
    double var  = sum2 / count - mean * mean;
    if(var < 0.0) var = 0.0;
    double stddev = qSqrt(var);

    int precision = rawOk ? 0 : 1;
    statLines << (tr("Min: ") + QString::number(minV, 'f', precision) +
                  "   " + tr("Max: ") + QString::number(maxV, 'f', precision));
    statLines << (tr("Mean: ") + QString::number(mean, 'f', 1) +
                  "   " + tr("Std: ") + QString::number(stddev, 'f', 2));

    return true;
}


/**
 * @brief EZCAP::eventFilter
 * @param target
 * @param event
 * @return
 */
bool EZCAP::eventFilter(QObject *target, QEvent *event)
{
    if (target == ui->label_ImgShow)
    {
        if(event->type() == QEvent::Resize && ix.camStreamMode == 1)
        {
            ix.showLabelW = ui->label_ImgShow->width();
            ix.showLabelH = ui->label_ImgShow->height();
            if((double)ix.RoiW_Last * ix.scaleFactor <= ix.showLabelW || ix.zoomMode == Zoom_FitWindow || ix.zoomMode == Zoom_FillWindow)
            {
                ui->horizontalScrollBar_ImgShow->setMinimum(0);
                ui->horizontalScrollBar_ImgShow->setMaximum(0);
            }
            else
            {
                ui->horizontalScrollBar_ImgShow->setMaximum((double)ix.RoiW_Last * ix.scaleFactor - ix.showLabelW);
                ui->horizontalScrollBar_ImgShow->setPageStep(ix.showLabelW);
                ui->horizontalScrollBar_ImgShow->setValue(ix.showLabelX);
            }
            if((double)ix.RoiH_Last * ix.scaleFactor <= ix.showLabelH || ix.zoomMode == Zoom_FitWindow || ix.zoomMode == Zoom_FillWindow)
            {
                ui->verticalScrollBar_ImgShow->setMinimum(0);
                ui->verticalScrollBar_ImgShow->setMaximum(0);
            }
            else
            {
                ui->verticalScrollBar_ImgShow->setMaximum((double)ix.RoiH_Last * ix.scaleFactor - ix.showLabelH);
                ui->verticalScrollBar_ImgShow->setPageStep(ix.showLabelH);
                ui->verticalScrollBar_ImgShow->setValue(ix.showLabelY);
            }

//            if(ix.isConnected && !ImgBase.empty())
//            {
//                displaySingleFrame(ix.FrameW_Last, ix.FrameH_Last, ix.ImgData_Last);
//            }
        }

        //鼠标移动进入label_ImgShow区域，设置鼠标跟踪有效
        if(event->type() == QEvent::Enter)
        {
            ui->label_ImgShow->setMouseTracking(true);
            scrollArea_ImgShow->setMouseTracking(true);
            ui->centralWidget->setMouseTracking(true);
            this->setMouseTracking(true);
        }
        //鼠标离开进入label_ImgShow区域，设置鼠标跟踪无效
        if(event->type() == QEvent::Leave)
        {
            ui->label_ImgShow->setMouseTracking(false);
            scrollArea_ImgShow->setMouseTracking(false);
            ui->centralWidget->setMouseTracking(false);
            this->setMouseTracking(false);

            clearHoverPixelStatus();
        }
        if(event->type() == QEvent::MouseMove)
        {
            if(ix.camStreamMode == 1 || (ix.lastWorkMode == ix.workMode && ix.imageReady == GetSingleFrame_Success))
            {
                QMouseEvent *e = (QMouseEvent *)event;
                hoverLabelPos = e->pos();
                hoverLabelPosValid = true;

                if(ix.camStreamMode == 1)
                {
                    QPoint imagePos;
                    QPoint matPos;
                    cv::Mat emptyImage;
                    if(mapHoverPointToImage(hoverLabelPos, emptyImage, imagePos, matPos))
                        statusLabel_mousePos->setText("(" + QString::number(imagePos.x()) + "," + QString::number(imagePos.y()) + ")");
                    else
                        clearHoverPixelStatus();
                }
                else
                {
                int x,y;//当前焦点位于图像上的坐标
                updateHoverPixelStatus(hoverLabelPos, ImgShow);
                bool canShow = false;

                if(ix.zoomMode == Zoom_FitWindow)
                {
                    double rate_label = (double)ui->label_ImgShow->width() / (double)ui->label_ImgShow->height();
                    double rate_image = (double)ix.FrameW_Last / (double)ix.FrameH_Last;
                    int limited_w = ui->label_ImgShow->width(), limited_h = ui->label_ImgShow->height();

                    if(rate_label > rate_image)
                    {
                        limited_w = ui->label_ImgShow->height() * rate_image;
                        x = e->x() * (int)ix.FrameW_Last / limited_w;
                        y = e->y() * (int)ix.FrameH_Last / ui->label_ImgShow->height();
                    }
                    else
                    {
                        limited_h = ui->label_ImgShow->width() / rate_image;
                        x = e->x() * (int)ix.FrameW_Last / ui->label_ImgShow->width();
                        y = e->y() * (int)ix.FrameH_Last / limited_h;
                    }

                    if(e->x() >= limited_w || e->y() >= limited_h) canShow = false;
                }
                else if(ix.zoomMode == Zoom_FillWindow)//mainMenuBar->actFitWindow->isChecked())
                {
                    x = e->x() * ix.FrameW_Last / ui->label_ImgShow->width();
                    y = e->y() * ix.FrameH_Last / ui->label_ImgShow->height();
                }
                else
                {
                    x = e->x() / ix.scaleFactor;
                    y = e->y() / ix.scaleFactor;
                }

                //显示坐标
                if(canShow)
                {
                    QString str2 = "("+QString::number(x)+","+QString::number(y)+")";
                    this->statusLabel_mousePos->setText(str2);

                    //计算灰度值或RGB值，把存储的8为图像数据转成opencv RGB彩色,在转成Qimage图像，最后根据QImage计算
                    QString rgbStr = "";

                    int i = 0, r = 0, g = 0, b = 0;
                    cv::Vec3b p = ImgShow.at<Vec3b>(y, x);
                    r = p[0];
                    g = p[1];
                    b = p[2];
                    i = (b + g + r) / 3;

                    rgbStr = tr("i:") + QString::number(i) + "  " +
                             tr("r:") + QString::number(r) + "," +
                             tr("g:") + QString::number(g) + "," +
                             tr("b:") + QString::number(b);

                    //显示RGB值
                    this->statusLabel_rgb->setText(rgbStr);
                }
                else
                {
                    // Status is already updated by updateHoverPixelStatus().
                }
                }

                //更新像素放大镜（区域放大图、光标像素值、区域统计信息）
                updatePixelMagnifier(hoverLabelPos);
            }
        }
        //鼠标点击label_ImgShow区域
        if(event->type() == QEvent::MouseButtonPress)
        {
            QMouseEvent *pMouseEvent = static_cast<QMouseEvent *>(event);
            // check whether it's left button pressed
            if (pMouseEvent->button() == Qt::LeftButton/* || pMouseEvent->button() == Qt::RightButton*/)
            {
                displayedImageMouseDown(pMouseEvent->x(),pMouseEvent->y());//适用preview和focus模式

                return true;
            }
        }

        // 支持在 Capture 模式或 Live 模式下双击全屏
        if(event->type() == QEvent::MouseButtonDblClick &&
           ((ix.workMode == WorkMode_Capture && ix.workMode == ix.lastWorkMode) || ix.camStreamMode == 1))
        {
            QMouseEvent *pMouseEvent = static_cast<QMouseEvent *>(event);
            // check whether it's left button pressed
            if (pMouseEvent->button() == Qt::RightButton) return true;

            DBGOPT_INFO("Double click detected, workMode=%d, camStreamMode=%d", ix.workMode, ix.camStreamMode);
            doubleClick ++;

            if(doubleClick % 2 == 1)
            {
                DBGOPT_INFO("Entering fullscreen mode");
                mainLayout->removeWidget(ui->scrollArea_manager);
                mainLayout->removeWidget(mainMenuBar);//test menubar in Mac os
                mainLayout->removeWidget(ui->statusBar);
                mainMenuBar->setVisible(false);
                ui->statusBar->setVisible(false);
                ui->horizontalScrollBar_ImgShow->setVisible(false);
                ui->verticalScrollBar_ImgShow->setVisible(false);

                if(ix.zoomMode != Zoom_FitWindow && ix.zoomMode != Zoom_FillWindow)  scrollArea_ImgShow->setWidgetResizable(true);
                ix.lastZoomMode = ix.zoomMode;
                ix.zoomMode = Zoom_FillWindow;

                if(ix.camStreamMode == 0) this->displaySingleFrame(ix.FrameW_Last, ix.FrameH_Last, ix.ImgData_Last);

                this->showFullScreen();
            }
            else
            {
                DBGOPT_INFO("Exiting fullscreen mode");
                mainLayout->addWidget(ui->scrollArea_manager, BorderLayout::West);
                mainLayout->addWidget(mainMenuBar, BorderLayout::North);//test menubar in Mac os
                mainLayout->addWidget(ui->statusBar, BorderLayout::South);
                mainMenuBar->setVisible(true);
                ui->statusBar->setVisible(true);

                this->showNormal();
                ix.zoomMode = ix.lastZoomMode;

                if(ix.zoomMode == Zoom_FitWindow || ix.zoomMode == Zoom_FillWindow)
                {
                    scrollArea_ImgShow->setWidgetResizable(true);
                    if(ix.camStreamMode == 0) this->displaySingleFrame(ix.FrameW_Last, ix.FrameH_Last, ix.ImgData_Last);
                }
                else
                {
                    if(ix.camStreamMode == 0) scaleImage(ix.scaleFactor);
                    else adjustScrollBar(ix.FrameW_Last, ix.FrameH_Last);
                }
            }
        }
    }

    if(target == managerMenu->ui->img_Roi)
    {
        if(event->type() == QEvent::MouseButtonPress)
        {
            QMouseEvent *pMouseEvent = static_cast<QMouseEvent *>(event);
            if (pMouseEvent->button() == Qt::LeftButton)
            {
                if(ix.RoiW < ix.ImageW_Min / ix.BinX && ix.RoiH < ix.ImageH_Min / ix.BinY)
                {
                    cv::Mat ImgRoiTemp = cv::Mat::zeros(cv::Size(managerMenu->ui->img_Roi->width(), managerMenu->ui->img_Roi->height()), CV_8UC3);
                    double posX = pMouseEvent->x() - (double)ix.RoiW / 2 / ix.ImageW_Min * ix.BinX * managerMenu->ui->img_Roi->width();
                    double posY = pMouseEvent->y() - (double)ix.RoiH / 2 / ix.ImageH_Min * ix.BinY * managerMenu->ui->img_Roi->height();
                    if(posX < 0) posX = 0;
                    if(posY < 0) posY = 0;
                    if(posX > managerMenu->ui->img_Roi->width() - (double)ix.RoiW / ix.ImageW_Min * ix.BinX * managerMenu->ui->img_Roi->width())
                        posX = managerMenu->ui->img_Roi->width() - (double)ix.RoiW / ix.ImageW_Min * ix.BinX * managerMenu->ui->img_Roi->width();
                    if(posY > managerMenu->ui->img_Roi->height() - (double)ix.RoiH / ix.ImageH_Min * ix.BinY * managerMenu->ui->img_Roi->height())
                        posY = managerMenu->ui->img_Roi->height() - (double)ix.RoiH / ix.ImageH_Min * ix.BinY * managerMenu->ui->img_Roi->height();

                    rectangle(ImgRoiTemp,
                              Point((int)posX, (int)posY),
                              Point((int)posX + (double)ix.RoiW / ix.ImageW_Min * ix.BinX * managerMenu->ui->img_Roi->width(),
                                    (int)posY + (double)ix.RoiH / ix.ImageH_Min * ix.BinY * managerMenu->ui->img_Roi->height()),
                              Scalar(255, 0, 0), 1, LINE_8, 0);
                    QImage *RoiQImg = MatToQImage(ImgRoiTemp);
                    managerMenu->ui->img_Roi->setPixmap(QPixmap::fromImage(*RoiQImg));
                    ix.RoiX = (double)posX / managerMenu->ui->img_Roi->width() * ix.ImageW_Min / ix.BinX;
                    ix.RoiY = (double)posY / managerMenu->ui->img_Roi->height() * ix.ImageH_Min / ix.BinY;
                    ImgRoiTemp.release();

                    if(ix.camStreamMode == 1 && (ix.RoiX_Last != ix.RoiX || ix.RoiY_Last != ix.RoiY))
                    {
                        uint32_t ret = libqhyccd->SetQHYCCDResolution(camhandle, ix.RoiX, ix.RoiY, ix.RoiW, ix.RoiH);
                        if(ret == QHYCCD_SUCCESS)
                        {
                            ix.RoiX_Last = ix.RoiX;
                            ix.RoiY_Last = ix.RoiY;
                        }
                    }
                }
            }
        }
    }

    if(target == managerMenu->ui->img_screenView)
    {   //点击screenvieww图像区域
        if(event->type() == QEvent::MouseButtonPress)
        {
            QMouseEvent *pMouseEvent = static_cast<QMouseEvent *>(event);
            // check whether it's left button pressed
            if (pMouseEvent->button() == Qt::LeftButton)
            {
                //响应screenview图像被点击操作
                screenViewAreaMouseDown(pMouseEvent->x(), pMouseEvent->y());

                return true; //一定要返回true，如果 不想别的object也能接收到这个event
            }
        }
    }

    if(target == ui->image1_focusAssistant)
    {
        //双击focusassistant区域图像
        if(event->type() == QEvent::MouseButtonDblClick)
        {
            focusAssistantImageDblClick();

            return true;
        }
    }

    return QMainWindow::eventFilter(target,event);
}

QImage *EZCAP::MatToQImage(const Mat imageMat)
{
    QImage *image = NULL;
    uchar *imgData = imageMat.data;

    if(imageMat.channels() == 1)
        image = new QImage(imgData, imageMat.cols, imageMat.rows, imageMat.cols*(imageMat.depth()==CV_8U?1:2)*imageMat.channels(), QImage::Format_Indexed8);
    else if(imageMat.channels() == 3)
        image = new QImage(imgData, imageMat.cols, imageMat.rows, imageMat.cols*(imageMat.depth()==CV_8U?1:2)*imageMat.channels(), QImage::Format_RGB888);
    else
        qDebug() << " ";

    return image;
}

Mat EZCAP::QImageToMat(const QImage *qImage)
{
    int width = qImage->width();
    int height = qImage->height();
    Mat matBuffer;

    matBuffer.create(Size(width, height), CV_16U);
    uchar *ucharTemp = matBuffer.data;
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            int index = y * width + x;
            ucharTemp[index] = (unsigned char) qGray(qImage->pixel(x, y));
        }
    }

    return matBuffer;
}
//-------------------------------------------------------------------------------------

//*************************************************************************************
//  函数 currentWorkingModeChanged       改变tabWidget的当前活动页
//*************************************************************************************
/**
 * @brief EZCAP::currentWorkingModeChanged
 * @param index
 */
void EZCAP::currentWorkingModeChanged(int workmode)
{
    uint32_t ret = QHYCCD_ERROR;

    switch(workmode)
    {
    case WorkMode_Preview:
        {
            ix.IsCvtColor = false;

            ui->widgetFocusAssistant->setVisible(false);

            mainMenuBar->menuImageProcess->setEnabled(false);
            mainMenuBar->menuZoom->setEnabled(true);
            mainMenuBar->actIgnoreOverScanArea->setEnabled(false);
            mainMenuBar->actCalibrateOverScan->setEnabled(false);
            mainMenuBar->actOpenFolder->setEnabled(false);
            mainMenuBar->actSaveBMP->setEnabled(false);
            mainMenuBar->actSaveFIT->setEnabled(false);
            mainMenuBar->actSaveJPG->setEnabled(false);
            mainMenuBar->actSavePNG->setEnabled(false);
            mainMenuBar->actSaveTIF->setEnabled(false);

            ix.BinX = ix.BinX_Max;
            ix.BinY = ix.BinY_Max;
            if(ix.BinX_Last != ix.BinX || ix.BinY_Last != ix.BinY)
            {
                ret = libqhyccd->SetQHYCCDBinMode(camhandle, ix.BinX_Max, ix.BinY_Max);
                if(ret == QHYCCD_SUCCESS)
                {
                    ix.BinX_Last = ix.BinX;
                    ix.BinY_Last = ix.BinY;
                }
                else
                {
                    DBGOPT_WARNING("SetQHYCCDBinMode() Failed! | binx = %d biny = %d", ix.BinX, ix.BinY);
                }
            }

            ix.RoiX = 0;
            ix.RoiY = 0;
            ix.RoiW  = ix.ImageW_Min / ix.BinX;
            ix.RoiH  = ix.ImageH_Min / ix.BinY;

            if(ix.RoiX != ix.RoiX_Last || ix.RoiY != ix.RoiY_Last || ix.RoiW != ix.RoiW_Last || ix.RoiH != ix.RoiH_Last)
            {
                ret = libqhyccd->SetQHYCCDResolution(camhandle, ix.RoiX, ix.RoiY, ix.RoiW, ix.RoiH);
                if(ret != QHYCCD_SUCCESS)
                {
                    DBGOPT_WARNING("SetQHYCCDResolution() Failed! | startx = %d starty = %d sizex = %d sizey = %d", ix.RoiX, ix.RoiY, ix.RoiW, ix.RoiH);
                }
                else
                {
                    ix.RoiX_Last = ix.RoiX;
                    ix.RoiY_Last = ix.RoiY;
                    ix.RoiW_Last = ix.RoiW;
                    ix.RoiH_Last = ix.RoiH;
                }
            }

            managerMenu->ui->grpBox_Roi->setVisible(true);
            cv::Mat ImgRoiTemp = cv::Mat::zeros(cv::Size(managerMenu->ui->img_Roi->width(), managerMenu->ui->img_Roi->height()), CV_8UC3);
            QImage *RoiQImg = MatToQImage(ImgRoiTemp);
            managerMenu->ui->img_Roi->setPixmap(QPixmap::fromImage(*RoiQImg));
            ImgRoiTemp.release();

            managerMenu->ui->comboBox_RoiSaved->blockSignals(true);
            managerMenu->ui->comboBox_RoiSaved->clear();
            managerMenu->ui->comboBox_RoiSaved->addItem(QString::number(ix.RoiW) + " x " + QString::number(ix.RoiH));
            managerMenu->ui->comboBox_RoiSaved->addItem(QString::number(ix.RoiW * 8 / 10 / 2 * 2) + " x " + QString::number(ix.RoiH * 8 / 10 / 2 * 2));
            managerMenu->ui->comboBox_RoiSaved->addItem(QString::number(ix.RoiW / 2 / 2 * 2) + " x " + QString::number(ix.RoiH / 2 / 2 * 2));
            managerMenu->ui->comboBox_RoiSaved->addItem(QString::number(ix.RoiW * 3 / 10 / 2 * 2) + " x " + QString::number(ix.RoiH * 3 / 10 / 2 * 2));
            managerMenu->ui->comboBox_RoiSaved->addItem(QString::number(ix.RoiW / 10 / 2 * 2) + " x " + QString::number(ix.RoiH / 10 / 2 * 2));
            QString CustomedROI = "";
            mainWidget->loadParamFromIni(ix.CamModel + "-" + QString::number(ix.camStreamMode),
                                         "CustomedROI_Bin" + QString::number(ix.BinX) + "x" + QString::number(ix.BinY),
                                         &CustomedROI, "");
            if(!CustomedROI.isEmpty())
            {
                QStringList CustomedROIList = CustomedROI.split(";");
                for(int i = 0; i < CustomedROIList.count(); i++)
                {
                    if(!CustomedROIList.at(i).isEmpty())
                        managerMenu->ui->comboBox_RoiSaved->addItem(CustomedROIList.at(i));
                }
            }
            managerMenu->ui->comboBox_RoiSaved->addItem("Customed");
            managerMenu->ui->comboBox_RoiSaved->blockSignals(false);
        }
        break;
    case WorkMode_Focus:
        {
            ix.IsCvtColor = false;

            ui->widgetFocusAssistant->setVisible(true);

            mainMenuBar->menuImageProcess->setEnabled(false);
            mainMenuBar->menuZoom->setEnabled(false);
            mainMenuBar->actIgnoreOverScanArea->setEnabled(false);
            mainMenuBar->actCalibrateOverScan->setEnabled(false);
            mainMenuBar->actOpenFolder->setEnabled(false);
            mainMenuBar->actSaveBMP->setEnabled(false);
            mainMenuBar->actSaveFIT->setEnabled(false);
            mainMenuBar->actSaveJPG->setEnabled(false);
            mainMenuBar->actSavePNG->setEnabled(false);
            mainMenuBar->actSaveTIF->setEnabled(false);

            managerMenu->ui->grpBox_Roi->setVisible(false);
        }
        break;
    case WorkMode_Capture:
        {
            if(managerMenu->ui->comboBox_color_capture->currentText() == "ON")
                ix.IsCvtColor = true;
            else
                ix.IsCvtColor = false;

            ui->widgetFocusAssistant->setVisible(false);

            mainMenuBar->menuImageProcess->setEnabled(true);
            mainMenuBar->menuZoom->setEnabled(true);
            mainMenuBar->actIgnoreOverScanArea->setEnabled(true);
            mainMenuBar->actIgnoreOverScanArea->setChecked(ix.IgnoreOverscan);
            mainMenuBar->actCalibrateOverScan->setEnabled(true);
            mainMenuBar->actCalibrateOverScan->setChecked(ix.CalibrateOverscan);
            mainMenuBar->actOpenFolder->setEnabled(false);
            mainMenuBar->actSaveBMP->setEnabled(false);
            mainMenuBar->actSaveFIT->setEnabled(false);
            mainMenuBar->actSaveJPG->setEnabled(false);
            mainMenuBar->actSavePNG->setEnabled(false);
            mainMenuBar->actSaveTIF->setEnabled(false);

            if(ix.Bin88_Fun && managerMenu->ui->bin8x8->isChecked())
            {
                ix.BinX = 8;
                ix.BinY = 8;
            }
            else if(ix.Bin66_Fun && managerMenu->ui->bin6x6->isChecked())
            {
                ix.BinX = 6;
                ix.BinY = 6;
            }
            else if(ix.Bin44_Fun && managerMenu->ui->bin4x4->isChecked())
            {
                ix.BinX = 4;
                ix.BinY = 4;
            }
            else if(ix.Bin33_Fun && managerMenu->ui->bin3x3->isChecked())
            {
                ix.BinX = 3;
                ix.BinY = 3;
            }
            else if(ix.Bin22_Fun && managerMenu->ui->bin2x2->isChecked())
            {
                ix.BinX = 2;
                ix.BinY = 2;
            }
            else
            {
                ix.BinX = 1;
                ix.BinY = 1;
            }

            if(ix.BinX_Last != ix.BinX || ix.BinY_Last != ix.BinY)
            {
                ret = libqhyccd->SetQHYCCDBinMode(camhandle, ix.BinX, ix.BinY);
                if(ret == QHYCCD_SUCCESS)
                {
                    ix.BinX_Last = ix.BinX;
                    ix.BinY_Last = ix.BinY;
                }
                else
                {
                    DBGOPT_WARNING("SetQHYCCDBinMode() Failed! | binx = %d biny = %d", ix.BinX, ix.BinY);
                }
            }

            ix.RoiX = 0;
            ix.RoiY = 0;
            ix.RoiW = ix.ImageW_Min / ix.BinX;
            ix.RoiH = ix.ImageH_Min / ix.BinY;
            if(ix.CamID.contains("QHY992_") && ix.BinX == 3 && ix.BinY == 3)
            {
                ix.RoiW = (ix.ImageW_Min + 5) / 6 * 6 / ix.BinX;
                ix.RoiH = (ix.ImageH_Min + 5) / 6 * 6 / ix.BinY;
            }

            if(ix.RoiX != ix.RoiX_Last || ix.RoiY != ix.RoiY_Last || ix.RoiW != ix.RoiW_Last || ix.RoiH != ix.RoiH_Last)
            {
                ret = libqhyccd->SetQHYCCDResolution(camhandle, ix.RoiX, ix.RoiY, ix.RoiW, ix.RoiH);
                if(ret != QHYCCD_SUCCESS)
                {
                    DBGOPT_WARNING("SetQHYCCDResolution() Failed! | startx = %d starty = %d sizex = %d sizey = %d", ix.RoiX, ix.RoiY, ix.RoiW, ix.RoiH);
                }
                else
                {
                    ix.RoiX_Last = ix.RoiX;
                    ix.RoiY_Last = ix.RoiY;
                    ix.RoiW_Last = ix.RoiW;
                    ix.RoiH_Last = ix.RoiH;
                }
            }

            managerMenu->ui->grpBox_Roi->setVisible(true);
            cv::Mat ImgRoiTemp = cv::Mat::zeros(cv::Size(managerMenu->ui->img_Roi->width(), managerMenu->ui->img_Roi->height()), CV_8UC3);
            QImage *RoiQImg = MatToQImage(ImgRoiTemp);
            managerMenu->ui->img_Roi->setPixmap(QPixmap::fromImage(*RoiQImg));
            ImgRoiTemp.release();

            managerMenu->ui->comboBox_RoiSaved->blockSignals(true);
            managerMenu->ui->comboBox_RoiSaved->clear();
            managerMenu->ui->comboBox_RoiSaved->addItem(QString::number(ix.RoiW) + " x " + QString::number(ix.RoiH));
            managerMenu->ui->comboBox_RoiSaved->addItem(QString::number(ix.RoiW * 8 / 10 / 2 * 2) + " x " + QString::number(ix.RoiH * 8 / 10 / 2 * 2));
            managerMenu->ui->comboBox_RoiSaved->addItem(QString::number(ix.RoiW / 2 / 2 * 2) + " x " + QString::number(ix.RoiH / 2 / 2 * 2));
            managerMenu->ui->comboBox_RoiSaved->addItem(QString::number(ix.RoiW * 3 / 10 / 2 * 2) + " x " + QString::number(ix.RoiH * 3 / 10 / 2 * 2));
            managerMenu->ui->comboBox_RoiSaved->addItem(QString::number(ix.RoiW / 10 / 2 * 2) + " x " + QString::number(ix.RoiH / 10 / 2 * 2));
            QString CustomedROI = "";
            mainWidget->loadParamFromIni(ix.CamModel + "-" + QString::number(ix.camStreamMode),
                                         "CustomedROI_Bin" + QString::number(ix.BinX) + "x" + QString::number(ix.BinY),
                                         &CustomedROI, "");
            if(!CustomedROI.isEmpty())
            {
                QStringList CustomedROIList = CustomedROI.split(";");
                for(int i = 0; i < CustomedROIList.count(); i++)
                {
                    if(!CustomedROIList.at(i).isEmpty())
                        managerMenu->ui->comboBox_RoiSaved->addItem(CustomedROIList.at(i));
                }
            }
            managerMenu->ui->comboBox_RoiSaved->addItem("Customed");
            managerMenu->ui->comboBox_RoiSaved->blockSignals(false);
        }
        break;
    }
}

//-------------------------------------------------------------------------------------

//*************************************************************************************
//         图像显示区域操作
//*************************************************************************************
/**
 * @brief EZCAP::on_label_ImgShow_customContextMenuRequested
 * @param pos
 */
void EZCAP::on_label_ImgShow_customContextMenuRequested(const QPoint &pos)
{
    if(ix.lastWorkMode == ix.workMode && ix.imageReady)
    {
        if(cmenu_imgArea)//保证同时只存在一个menu，及时释放内存
        {
            delete cmenu_imgArea;
            cmenu_imgArea = NULL;
        }
        cmenu_imgArea = new QMenu(ui->label_ImgShow);  //popMenu

        QAction *actGoFocusCenter = cmenu_imgArea->addAction(tr("Go Focus Center"));
        QAction *actSaveFit = cmenu_imgArea->addAction(tr("Save FIT"));
        QAction *actSaveBmp = cmenu_imgArea->addAction(tr("Save BMP"));
        QAction *actSaveJpg = cmenu_imgArea->addAction(tr("Save JPG"));
        QAction *actSavePng = cmenu_imgArea->addAction(tr("Save PNG"));
        QAction *actSaveTif = cmenu_imgArea->addAction(tr("Save TIF"));
        QAction *actOpenSavedPath = cmenu_imgArea->addAction(tr("Open Last Saved Folder"));

        connect(actGoFocusCenter, SIGNAL(triggered()), this, SLOT(goFocusCenter()));
        connect(actSaveFit, SIGNAL(triggered()), this, SLOT(saveAsFIT()));
        connect(actSaveBmp, SIGNAL(triggered()), this, SLOT(saveAsBMP()));
        connect(actSaveJpg, SIGNAL(triggered()), this, SLOT(saveAsJPG()));
        connect(actSavePng, SIGNAL(triggered()), this, SLOT(saveAsPNG()));
        connect(actSaveTif, SIGNAL(triggered()), this, SLOT(saveAsTIF()));
        connect(actOpenSavedPath, SIGNAL(triggered()), this, SLOT(openFolder()));

        qDebug() << "cursor" << pos.x() << pos.y();
        cmenu_imgArea->exec(QCursor::pos());//在当前鼠标位置显示
    }
}

/**
 * @brief EZCAP::goFocusCenter
 */
void EZCAP::goFocusCenter()
{
    //跳转到focus页
    managerMenu->ui->head_focus->click();
}

//-------------------------------------------------------------------------------------

/**
 * @brief EZCAP::getImageInfo
 * @param Buf
 * @param ImageWidth
 * @param ImageHeight
 * @param startX
 * @param startY
 * @param sizeX
 * @param sizeY
 * @param std
 * @param rms
 * @param max
 * @param min
 */
void EZCAP::getImageInfo(unsigned char *Buf,int ImageWidth,int ImageHeight, int startX,int startY,int sizeX,int sizeY, float &std,float &rms,double &max,double &min)
{
    //输入16位图像尺寸和图像缓冲区，以及局部图像开始位置和长宽，输出方差，平均值，最大值，最小值
    Scalar RMS,STD;
    Mat FitImg;

    if(sizeX > 0 && sizeY > 0)
    {
        FitImg.create(Size(ImageWidth, ImageHeight), CV_16UC1);
        FitImg.data = Buf;

        if(startX + sizeX > ImageWidth || startY + sizeY > ImageHeight)
        {
            rms = 0;
            std = 0;
        }
        else
        {
            Mat ROI;
            ROI = FitImg(Rect(startX, startY, sizeX, sizeY));
            meanStdDev(ROI, RMS, STD);
            minMaxLoc(ROI, &min, &max);
            ROI.release();

            rms=RMS.val[0];
            std=STD.val[0];
        }

        FitImg.release();
    }
    else
    {
        rms = 0;
        std = 0;
        min = 0;
        max = 0;
    }
}

/**
 * @brief EZCAP::ImageAnalyze
 * @param Img
 * @param x
 * @param y
 */
void EZCAP::ImageAnalyze(Mat Img,int x,int y)
{
    //从IMGDATA区获得原始数据进行分析。函数输入的IMG仅用于画框和显示
    int OX,Y;
    float std,rms;
    double max,min;

    OX=x;
    Y=y;

    QString std2str,rms2str,max2str,min2str;

    for (int i=1; i < 9; i=i+3)
    {
        for (int j = 1; j < 9; j=j+3)
        {
            getImageInfo(ix.ImgData_Last,x,y,OX*i/9,Y*j/9,OX/9,Y/9,std,rms,max,min);

            rectangle(Img, Point(OX*i/9,Y*j/9), Point(OX*i/9+OX/9,Y*j/9+Y/9), Scalar(0, 0, 255), 1, LINE_8, 0);

            putText(Img, (QString("STD:")+std2str.setNum(std,'f',1)).toStdString().c_str(), Point(OX*i/9,Y*j/9+Y/9+20), FONT_HERSHEY_COMPLEX, 0.5, Scalar(0, 0, 255), 1, LINE_8, false);
            putText(Img, (QString("RMS:")+rms2str.setNum(rms,'f',1)).toStdString().c_str(), Point(OX*i/9,Y*j/9+Y/9+35), FONT_HERSHEY_COMPLEX, 0.5, Scalar(0, 0, 255), 1, LINE_8, false);
            putText(Img, (QString("MAX:")+max2str.setNum(max,'f',1)).toStdString().c_str(), Point(OX*i/9,Y*j/9+Y/9+50), FONT_HERSHEY_COMPLEX, 0.5, Scalar(0, 0, 255), 1, LINE_8, false);
            putText(Img, (QString("MIN:")+min2str.setNum(min,'f',1)).toStdString().c_str(), Point(OX*i/9,Y*j/9+Y/9+65), FONT_HERSHEY_COMPLEX, 0.5, Scalar(0, 0, 255), 1, LINE_8, false);
        }
    }

    getImageInfo(ix.ImgData_Last, x, y, ix.OverscanX, ix.OverscanY, ix.OverscanW, ix.OverscanH, std, rms, max, min);

    rectangle(Img, Point(ix.OverscanX, ix.OverscanY), Point(ix.OverscanX+ix.OverscanW, ix.OverscanY+ix.OverscanH), Scalar(0, 255, 0), 1, LINE_8, 0);

    putText(Img, (QString("OverScan Area")).toStdString().c_str(), Point(ix.OverscanX-150,ix.OverscanY+5), FONT_HERSHEY_COMPLEX, 0.5, Scalar(0, 255, 0), 1, LINE_8, false);
    putText(Img, (QString("STD:")+std2str.setNum(std,'f',1)).toStdString().c_str(), Point(ix.OverscanX-150,ix.OverscanY+20), FONT_HERSHEY_COMPLEX, 0.5, Scalar(0, 255, 0), 1, LINE_8, false);
    putText(Img, (QString("RMS:")+rms2str.setNum(rms,'f',1)).toStdString().c_str(), Point(ix.OverscanX-150,ix.OverscanY+35), FONT_HERSHEY_COMPLEX, 0.5, Scalar(0, 255, 0), 1, LINE_8, false);
    putText(Img, (QString("MAX:")+max2str.setNum(max,'f',1)).toStdString().c_str(), Point(ix.OverscanX-150,ix.OverscanY+50), FONT_HERSHEY_COMPLEX, 0.5, Scalar(0, 255, 0), 1, LINE_8, false);
    putText(Img, (QString("MIN:")+min2str.setNum(min,'f',1)).toStdString().c_str(), Point(ix.OverscanX-150,ix.OverscanY+65), FONT_HERSHEY_COMPLEX, 0.5, Scalar(0, 255, 0), 1, LINE_8, false);

    OverScanRMS = rms;
}

/**
 * @brief EZCAP::AutoFileName
 * @return
 */
QString EZCAP::AutoFileName()
{
    int H,M,S,MS;
    QString TimeStamp;

    QTime timeForStamp = QTime::currentTime();
    H = timeForStamp.hour();//获取小时
    M = timeForStamp.minute();//获取分钟
    S = timeForStamp.second();//获取秒
    MS = timeForStamp.msec();//获取毫秒
    TimeStamp = QString::number(H)+"-"+ QString::number(M)+"-"+QString::number(S)+"-"+QString::number(MS);

    return TimeStamp;
}
//-------------------------------------------------------------------------------------

//******************************************************************************************************************************
// *     线程ExecutePlanTable          执行计划任务表
// *****************************************************************************************************************************
IplImage *imgsum,*imgcur,*imgavg,*imgresult,*imgblack;
bool darkframeflag = false;

void ExecutePlanTable::run()
{
    //-----------------处理连续拍摄事件---------------
    if(ix.plannerState == PlannerStatus_Start)
    {
        planner_dialog->ui->status1_planner->setText(tr("Capture..."));

        //---highspeed setting
        if(ix.Speed_Fun)
        {
            managerMenu->ui->checkBox_highSpeed->setChecked(planner_dialog->ui->cBox_highReadSpeed_planner->isChecked());
        }

        //读取Script.ini中的内容
        QString path_script;
        bool c_enable;//该项任务是否执行
        int c_exp;    //该项任务设置的曝光时间
        int c_cfwPos;  //   色轮位置
        int c_repeat; //   重复次数
        int c_bin;    //   bin模式
        int c_delay;  //   等待时间
        QString namez;
        int c_loop;  //任务循环次数
        int c_total; //计划表中任务总条目数
        int c_Gain;//增益 注意这个增益是百分比
        bool c_avg_enable;//该项任务执行后，是否平均输出（仅仅输出平均后的项目）
        bool c_subblack_enable;//是否减去暗场，有个前提，不含FPN噪声的暗场必须存在
        bool c_subbias_enable;//是否减去bias场

        QString tempStr = "";

        char biasfilename[1024];
        char blackfilename[1024];

        path_script = QCoreApplication::applicationDirPath() + "/" + "Script.ini";
        path_script = QDir::toNativeSeparators(path_script); //converted '/'. on windows return \   else return /

        QSettings *configIniRead = new QSettings(path_script, QSettings::IniFormat);

        //将读取到的ini文件保存在QString中，先取值，然后通过toString()函数转换成QString类型
        configIniRead->beginGroup("Total");
        c_total = configIniRead->value("TaskNum", 0).toInt();
        configIniRead->endGroup();

        configIniRead->beginGroup("Loop");
        c_loop = configIniRead->value("LoopNum", 0).toInt();
        configIniRead->endGroup();

        for(int LP = 0; LP < c_loop; LP++)
        {
            for(int iTask = 1; iTask <= c_total; iTask++)
            {
                configIniRead->beginGroup("CAP" + QString::number(iTask));
                c_enable = configIniRead->value("Enabled", false).toBool();
                c_bin = configIniRead->value("BIN", 1).toInt();
                c_exp = configIniRead->value("EXP", 1).toInt();
                c_repeat = configIniRead->value("Repeat", 1).toInt();
                c_cfwPos = configIniRead->value("CFW", 0).toInt();
                c_delay = configIniRead->value("Delay", 0).toInt();
                c_Gain = configIniRead->value("Gain", 0).toInt();
                c_avg_enable = configIniRead->value("AVGEnabled", false).toBool();
                c_subblack_enable = configIniRead->value("SubBlackEnabled", false).toBool();
                c_subbias_enable = configIniRead->value("SubBiasEnabled", false).toBool();
                configIniRead->endGroup();

                memset(biasfilename,'\0',1024);
                sprintf(biasfilename,"./Bias/%s.fit",camid);

                memset(blackfilename,'\0',1024);
                sprintf(blackfilename,"./Black/%s-BIN%d-EXP%d-Gain%d.fit",camid,c_bin,c_exp,c_Gain);

                if(c_enable)
                {
                    //修改任务表格中iTask任务背景色，以表示其正在执行
                    emit changeRowColor(iTask - 1, QColor(100,100,0));

                    //-----------------------------color filter wheel setting-----------------------------
                    if(c_cfwPos != 0)
                    {
                        if(ix.canFilterWheel)
                        {
                            emit changeCurCFWPos(c_cfwPos - 1);
                        }
                        else
                        {
                            //send signal to show the error info
                            emit showErrorInfo(tr("Forced To Stop Planner"),tr("Camera not support CFW, cannot run CFW to ")+ QString::number(c_cfwPos));

                            //reset the row color
                            emit changeRowColor(iTask - 1, QColor(80, 50, 0));

                            //set the value to the loop end
                            LP = c_loop;
                            iTask = c_total + 1;

                            break;
                        }
                    }

                    //-----------------------------Gain setting---------------------------------
                    if(ix.Gain_Fun)
                        managerMenu->ui->hSlider_Gain_capture->setValue(c_Gain);

                    //-----------------------------bin mode setting-----------------------------
                    if(c_bin == 1 && ix.Bin11_Fun)
                        managerMenu->ui->bin1x1->setChecked(true);
                    else if(c_bin == 2 && ix.Bin22_Fun)
                        managerMenu->ui->bin2x2->setChecked(true);
                    else if(c_bin == 3 && ix.Bin33_Fun)
                        managerMenu->ui->bin3x3->setChecked(true);
                    else if(c_bin == 4 && ix.Bin44_Fun)
                        managerMenu->ui->bin4x4->setChecked(true);
                    else if(c_bin == 6 && ix.Bin66_Fun)
                        managerMenu->ui->bin4x4->setChecked(true);
                    else if(c_bin == 8 && ix.Bin88_Fun)
                        managerMenu->ui->bin4x4->setChecked(true);
                    else
                    {   //send signal to show the error info
                        emit showErrorInfo(tr("Forced To Stop Planner"),tr("Camera Not Support Binning ") + QString::number(c_bin)+"*"+QString::number(c_bin) + tr(", Planner will be Forced To Stop!"));

                        //reset the row color
                        emit changeRowColor(iTask - 1, QColor(80, 50, 0));

                        //set the value to the loop end
                        LP = c_loop;
                        iTask = c_total + 1;

                        break;
                    }

                    //-----------------------------exposure time setting-----------------------------
                    if (c_exp >= 1000)
                    {
                        managerMenu->ui->comBoxSingleUnit->setCurrentText("1~1200 s");
                        managerMenu->ui->hSlider_exposure_capture->setValue(c_exp/1000);
                    }
                    else
                    {
                        managerMenu->ui->comBoxSingleUnit->setCurrentText("1~1000 ms");
                        managerMenu->ui->hSlider_exposure_capture->setValue(c_exp);
                    }

                    //-----------------------------start repeat capture and save image-----------------------------
                    for (int iRP = 0; iRP < c_repeat; iRP++)
                    {
                        tempStr = "CAP" + QString::number(iTask) + " " + QString::number(iRP + 1) + " of " + QString::number(c_repeat) + " Loop:" + QString::number(LP+1);
                        planner_dialog->ui->status1_planner->setText(tempStr);

#ifdef Q_OS_WIN32
                        QString savePath;
                        if(planner_dialog->ui->pBtn_folder_planner->text().compare("Folder") == 0 )
                        {
                            savePath = QCoreApplication::applicationDirPath();
                        }
                        else
                        {
                            savePath = planner_dialog->ui->pBtn_folder_planner->text();
                        }
                        DiskTools *dt = new DiskTools;
                        quint64 freeSpace = dt->getDiskFreeSpace(savePath);
                        //qDebug() << "Free:" << freeSpace << "MB";
                        planner_dialog->ui->status2_planner->setText(tr("Free:")+QString::number(freeSpace)+"MB");
#endif

                        //发送doCapture信号，执行Capture 获取图像
                        emit startCaptureImage();

                        if (c_loop > 1)
                        {
                            QString unit = "";
                            if(managerMenu->ui->comBoxSingleUnit->currentText() == "1~1000 us") unit = "us";
                            if(managerMenu->ui->comBoxSingleUnit->currentText() == "1~1000 ms") unit = "ms";
                            if(managerMenu->ui->comBoxSingleUnit->currentText() == "1~1200 s")  unit = "s";
                            if(managerMenu->ui->comBoxSingleUnit->currentText() == "20~60 min") unit = "min";
                            namez = configIniRead->value("/ScriptName/FileName").toString().trimmed() + "-" + QString::number(iTask) + "-" +
                                    QString::number(managerMenu->ui->hSlider_exposure_capture->value()) + unit +
                                    "-" + QString::number(iRP + 1) + "-CFW" + QString::number(c_cfwPos) + "-Loop" + QString::number(LP) + ".fit";
                        }
                        else
                        {
                            QString unit = "";
                            if(managerMenu->ui->comBoxSingleUnit->currentText() == "1~1000 us") unit = "us";
                            if(managerMenu->ui->comBoxSingleUnit->currentText() == "1~1000 ms") unit = "ms";
                            if(managerMenu->ui->comBoxSingleUnit->currentText() == "1~1200 s")  unit = "s";
                            if(managerMenu->ui->comBoxSingleUnit->currentText() == "20~60 min") unit = "min";
                            namez = configIniRead->value("/ScriptName/FileName").toString().trimmed() + "-" + QString::number(iTask) + "-" +
                                    QString::number(managerMenu->ui->hSlider_exposure_capture->value()) + unit +
                                    "-" + QString::number(iRP + 1) + "-CFW" + QString::number(c_cfwPos) + ".fit";
                        }


                        if(QFile::exists(biasfilename) && c_subbias_enable)
                        {
                            if(imgavg == NULL)
                            {
                                imgavg = cvCreateImage(cvSize(ix.FrameW,ix.FrameH),IPL_DEPTH_16U,1);
                                cvZero(imgavg);
                                fitHeader_dialog->FITRead(biasfilename,ix.FrameW * ix.FrameH,(unsigned char*)imgavg->imageData);
                            }

                            if(imgcur == NULL)
                            {
                                imgcur = cvCreateImage(cvSize(ix.FrameW,ix.FrameH),IPL_DEPTH_16U,1);
                                cvZero(imgcur);
                            }

                            if(imgresult == NULL)
                            {
                                imgresult = cvCreateImage(cvSize(ix.FrameW,ix.FrameH),IPL_DEPTH_16U,1);
                                cvZero(imgresult);
                            }

                            memcpy(imgcur->imageData,ix.ImgData,imgcur->imageSize);
                            cvSub(imgcur,imgavg,imgresult,NULL);
                            memcpy(ix.ImgData,imgresult->imageData,imgresult->imageSize);
                        }

                        if(QFile::exists(blackfilename) && c_subblack_enable)
                        {
                            qDebug() << "enable sub balck frame function" << QT_ENDL;

                            if(imgblack == NULL)
                            {
                                imgblack = cvCreateImage(cvSize(ix.FrameW,ix.FrameH),IPL_DEPTH_16U,1);
                                cvZero(imgblack);
                            }

                            if(imgcur == NULL)
                            {
                                imgcur = cvCreateImage(cvSize(ix.FrameW,ix.FrameH),IPL_DEPTH_16U,1);
                                cvZero(imgcur);
                            }

                            if(imgresult == NULL)
                            {
                                imgresult = cvCreateImage(cvSize(ix.FrameW,ix.FrameH),IPL_DEPTH_16U,1);
                                cvZero(imgresult);
                            }

                            fitHeader_dialog->FITRead(blackfilename,ix.FrameW * ix.FrameH,(unsigned char*)imgblack->imageData);

                            memcpy(imgcur->imageData,ix.ImgData,imgcur->imageSize);
                            cvSub(imgcur,imgblack,imgresult,NULL);
                            memcpy(ix.ImgData,imgresult->imageData,imgresult->imageSize);
                        }


                        if(!planner_dialog->ui->checkBox_disableAutoSave->isChecked())
                        {
                            //this will need a flag in panel for turning on OR turning off this function
                            if(c_avg_enable)
                            {
                                qDebug() << "enable average function" << QT_ENDL;
                                if(imgcur == NULL)
                                {
                                    imgcur = cvCreateImage(cvSize(ix.FrameW,ix.FrameH),IPL_DEPTH_16U,1);
                                    cvZero(imgcur);
                                }

                                if(imgsum == NULL)
                                {
                                    imgsum = cvCreateImage(cvSize(ix.FrameW,ix.FrameH),IPL_DEPTH_32F,1);
                                    cvZero(imgsum);
                                }

                                if(imgavg == NULL)
                                {
                                    imgavg = cvCreateImage(cvSize(ix.FrameW,ix.FrameH),IPL_DEPTH_16U,1);
                                    cvZero(imgavg);
                                }

                                memcpy(imgcur->imageData,ix.ImgData,imgcur->imageSize);

                                cvAcc(imgcur,imgsum);

                                if((iRP + 1) == c_repeat)
                                {
                                    cvConvertScale(imgsum,imgavg,1.0/c_repeat);

                                    if(iTask == 1)
                                    {
                                        fitHeader_dialog->FITWrite(biasfilename, (unsigned char *)imgavg->imageData);
                                    }
                                    else if(iTask > 1 && darkframeflag)
                                    {
                                        fitHeader_dialog->FITWrite(blackfilename, (unsigned char *)imgavg->imageData);
                                    }
                                    else
                                    {
                                        //sava Image as fit
                                        fitHeader_dialog->FITWrite(namez, ix.ImgData);
                                    }
                                }
                            }
                            else
                            {
                                //sava Image as fit
                                fitHeader_dialog->FITWrite(namez, ix.ImgData);
                            }
                        }

                        //---------------if click ForceStop------------------
                        if(ix.plannerState == PlannerStatus_Stop)
                        {   //ForceSTOP,  break out repeat capture loop
                            iRP = c_repeat;
                            c_delay = 0;
                            qDebug() << "force stop planner...";
                        }

                        //----------------Delay setting----------------------
                        if(c_delay > 0)
                            QThread::msleep(c_delay*1000);

                    }

                    //task row1 finished, reset the row color
                    emit changeRowColor(iTask - 1, QColor(80, 50, 0));

                    if(ix.plannerState == PlannerStatus_Stop)
                    {   //ForceSTOP,  break out tasks loop
                        iTask = c_total + 1;
                        LP = c_loop;
                    }

                }

                if(planner_dialog->firstRowIsBais && iTask == 1)
                {
                    emit finish_baisImages();
                }

            }
        }

        //读入完成后删除指针
        delete configIniRead;

        ix.plannerState = PlannerStatus_Done;
        planner_dialog->ui->status1_planner->setText(tr("Done"));
        darkframeflag = false;
        qDebug() << "Plane Done";

    }
}
//-------------------------------------------------------------------------------------

/**
 * @brief ExecuteCFWOrder::run
 */
void ExecuteCFWOrder::run()
{
    int ret = libqhyccd->SendOrder2QHYCCDCFW(camhandle, &ix.dstCfwPos, 1);
    if(ret != QHYCCD_SUCCESS)
        qCritical() << "SendOrder2QHYCCDCFW failure";
    else
        qDebug() << "SendOrder2QHYCCDCFW success";
}

//-------------------------------------------------------------------------------------------------------------------
//                      温度控制
//-------------------------------------------------------------------------------------------------------------------
double DegreeToR(double degree)
{

#define SQR3(x) ((x)*(x)*(x))
#define SQRT3(x) (exp(log(x)/3))

        if (degree<-50) degree=-50;
        if (degree>50)  degree=50;

        double x,y;
        double R;
        double T;

        double A=0.002679;
        double B=0.000291;
        double C=4.28e-7;

        T=degree+273.15;


        y=(A-1/T)/C;
        x=sqrt( SQR3(B/(3*C))+(y*y)/4);
        R=exp(  SQRT3(x-y/2)-SQRT3(x+y/2));

        return R;
}


double RToDegree(double R)
{
        double 	T;
        double LNR;

        if (R>400) R=400;
        if (R<1) R=1;

        LNR=log(R);
        T=1 / (0.002679+0.000291*LNR + LNR*LNR*LNR*4.28e-7);
        T=T-273.15;
        return T;
}

double DegreeTomV(double degree)
{
        double V;
        double R;

        R=DegreeToR(degree);
        V=33000/(R+10)-1625;

        return V;
}

double mVToDegree(double V)
{
        double R;
        double T;

        R=33/(V/1000+1.625)-10;

        T=RToDegree(R);

        return T;
}

//----------------色轮状态查询定时器响应--------------------------
void EZCAP::cfwTimer_timeout()
{
    if(ix.isConnected)
    {
        for(int i=0; i< 64; i++)
        {
            ix.curCfwPos[i] = 0;
        }
        ix.curCfwPos[0] = '0';
        int ret = libqhyccd->GetQHYCCDCFWStatus(camhandle, ix.curCfwPos);
        OutputDebug("QHYCCDEZCAP | ezCap.cpp | cfwTimer | GetCFWStatus GetQHYCCDCFWStatus()");
        if(ret == QHYCCD_SUCCESS)
        {
            if(ix.dstCfwPos == ix.curCfwPos[0])
            {
                ix.CFWStatus = CFW_Idle;//CFW运转到位
                stopCFWTimer();
                qDebug() << "CFW moved Done!";
            }
            else
            {
                qDebug() << QString("CFW is moving...target hole %1, current hole %2").arg(QString(ix.dstCfwPos)).arg(QString(ix.curCfwPos[0]));
            }
        }
        QCoreApplication::processEvents();
    }
}

void EZCAP::startCFWTimer()
{
    cfwTimer->start();
}

void EZCAP::stopCFWTimer()
{
    cfwTimer->stop();
}

//-------------------PHD2 Dither功能 状态查询定时器-----------------
void EZCAP::ditherTimer_timeout()
{
    if(phdLink_dialog->IsDitherEnabled())
    {
        char str[1024];
        int ret;
        QString recStr;
        ret = libqhyccd->CheckPHD2Status(str);
        if(ret == 0)
        {
            recStr = QString(QLatin1String(str));
            if(recStr.compare(QString("SettleDone"), recStr) == 0)
            {
                isSettleDone = true;
            }
            qDebug() <<"PHD2 Status:" << recStr;
        }
    }
    else
    {
        isSettleDone = true;  //如果断开PHD2连接，则退出等待循环
    }

    QApplication::processEvents();

}
void EZCAP::startDitherTimer()
{
    ditherTimer->start();
}
void EZCAP::stopDitherTimer()
{
    ditherTimer->stop();
}
void EZCAP::PumpTimer_timeout()
{
    pumpcount+=1;
    if(pumpcount>=5*60)//5*
    {
        stopPumpTimer();
    }
    QApplication::processEvents();
}
void EZCAP::startPumpTimer()
{
    PumpTimer->start();
    pumpcount=0;
    ix.cyclePUMBStatus = true;
}
void EZCAP::stopPumpTimer()
{
    PumpTimer->stop();

    libqhyccd->SetQHYCCDParam(camhandle,CONTROL_SensorChamberCycle_PUMP,0);
    favorite_dialog->ui->pBtn_controlSensorChamberCyclePUMP->setText("SensorChamberCyclePUMP ON");
    favorite_dialog->ui->pBtn_controlSensorChamberCyclePUMP->setChecked(false);

    ix.cyclePUMBStatus = false;
}

void EZCAP::PumpV2CycleTimer_timeout()
{
    libqhyccd->SetQHYCCDParam(camhandle, CONTROL_OUTSIDE_PUMP_V2, 1.0);
    QApplication::processEvents();
}

void EZCAP::PumpV2CycleSecondTimer_timeout()
{
    libqhyccd->SetQHYCCDParam(camhandle, CONTROL_OUTSIDE_PUMP_V2, 3.0);
    QApplication::processEvents();
}

static QUdpSocket *clientUDPSocket;
static QHostAddress clientReadIP;
static quint16 clientReadPort;
static MsgClient* msgClient;

void EZCAP::UDPServerScan(){
    DBGOPT_INFO("=== UDPServerScan() called ===");
    DBGOPT_INFO("enableMsgClient: %d", iniFileParams.enableMsgClient);
    DBGOPT_INFO("msgClientName: %s", qPrintable(iniFileParams.msgClientName));

    QJsonObject commandDiscovery;
    commandDiscovery.insert("cmd_code",cmd_code_discovery);
    commandDiscovery.insert("cmd_name",cmd_name_discovery);
    commandDiscovery.insert("cmd_status",cmd_status_send);

    QJsonDocument doc(commandDiscovery);
    QByteArray byteArray = doc.toJson();

    DBGOPT_INFO("UDP Discovery JSON: %s", byteArray.data());

    clientUDPSocket = new QUdpSocket();
//    connect(clientUDPSocket,SIGNAL(readyRead()),this,SLOT(clientReadMessage()));
    connect(clientUDPSocket,&QUdpSocket::readyRead,this,&EZCAP::clientReadMessage);

    qint64 udpReturn = clientUDPSocket->writeDatagram(byteArray,byteArray.length(),QHostAddress::Broadcast,udp_port);
    DBGOPT_INFO("UDP Broadcast to port: %d", udp_port);
    DBGOPT_INFO("UDP return bytes: %lld", udpReturn);
    this->statusLabel_SDKmsg->setText(tr("UDP return: %1").arg(udpReturn));

    if(udpReturn == -1) {
        DBGOPT_ERROR("UDP Send Error: %s", qPrintable(clientUDPSocket->errorString()));
        this->statusLabel_SDKmsg->setText(tr("UDP Error: %1").arg(clientUDPSocket->errorString()));
    }
}

void EZCAP::clientReadMessage(){
        DBGOPT_INFO("=== clientReadMessage() called ===");

        QByteArray data;
        data.resize(clientUDPSocket->pendingDatagramSize());
        clientUDPSocket->readDatagram(data.data(),data.size(),&clientReadIP, &clientReadPort);
        DBGOPT_INFO("Received UDP data from: %s:%d", qPrintable(clientReadIP.toString()), clientReadPort);
        DBGOPT_INFO("Data: %s", data.data());

        QString msg = QString::fromStdString(data.toStdString());
        DBGOPT_INFO("msg: %s", qPrintable(msg));
        QString		json = msg;
        QJsonParseError error;
        QJsonDocument	jsonDocument = QJsonDocument::fromJson( json.toUtf8(), &error );
        if ( error.error == QJsonParseError::NoError )
        {
            if ( jsonDocument.isObject() )
            {
                QVariantMap result = jsonDocument.toVariant().toMap();
                DBGOPT_INFO("JSON parsed successfully");
                DBGOPT_INFO("cmd_status: %d", result["cmd_status"].toInt());
                DBGOPT_INFO("cmd_code: %d", result["cmd_code"].toInt());

                if(result["cmd_status"] == cmd_status_end_success &&
                       result["cmd_code"]== cmd_code_discovery ){

                    DBGOPT_INFO("Server discovery response received!");
                    this->statusLabel_SDKmsg->setText(tr("Client Find Server : %1  %2").arg(clientReadIP.toString()).arg(QString::number(clientReadPort)));
                    bool conversionOK = false;
                    QHostAddress ip4Address(clientReadIP.toIPv4Address(&conversionOK));
                    if(conversionOK){
                        DBGOPT_INFO("IPv4 conversion OK: %s", qPrintable(ip4Address.toString()));
                        this->statusLabel_SDKmsg->setText(tr("Ip V4 convert ok  %1").arg(ip4Address.toString()));
                        QString wsUrl = tr("ws://%1:%2").arg(ip4Address.toString()).arg(websocket_port);
                        DBGOPT_INFO("Creating WebSocket client to: %s", qPrintable(wsUrl));
                        DBGOPT_INFO("Client name: %s", qPrintable(iniFileParams.msgClientName));

                        msgClient = new MsgClient(QUrl(wsUrl),iniFileParams.msgClientName,false,this);

                        connect(msgClient,&MsgClient::setAVI,this,&EZCAP::on_avi_action);
                        connect(msgClient,&MsgClient::setWindow,this,&EZCAP::on_windows_resize_action);
                        connect(msgClient,&MsgClient::netWorkError,this,&EZCAP::on_network_error);
                        DBGOPT_INFO("Opening WebSocket connection...");
                        msgClient->OpenConnection();
                    }else{
                        DBGOPT_ERROR("IPv4 conversion failed");
                        this->statusLabel_SDKmsg->setText(tr("Ip V4 convert error  %1").arg(ip4Address.toString()));
                    }
                } else {
                    DBGOPT_INFO("Response does not match discovery criteria");
                }

            }else{
                DBGOPT_ERROR("JSON is not an object");
                this->statusLabel_SDKmsg->setText("Json Error");
            }
        } else {
            DBGOPT_ERROR("JSON parse error: %s", qPrintable(error.errorString()));
            this->statusLabel_SDKmsg->setText(tr("Client Error msg: %1").arg(msg));
        }

}


void EZCAP::on_avi_action(bool startOrStop){
    this->statusLabel_SDKmsg->setText(tr("env: %1  %2").arg(cmd_name_client_ezcap_save_avi).arg(startOrStop));
}

void EZCAP::on_windows_resize_action(bool maxOrNormal){
    this->statusLabel_SDKmsg->setText(tr("env: %1  %2").arg(cmd_name_client_ezcap_max_window).arg(maxOrNormal));
    eventFilter(this->ui->label_ImgShow, new  QEvent(QEvent::MouseButtonDblClick));
}

void EZCAP::on_network_error(QString errorMessage){
    this->statusLabel_SDKmsg->setText(errorMessage);
}

//for single
void EZCAP::hScrollBarValueChanged(int value)
{
    if(value < 0) value = 0;
    ix.showLabelX = value;

    if(ix.camStreamMode == 0 && ix.imageReady == GetSingleFrame_Success)
    {
        scaleImage(0.0);
    }
}

//for single
void EZCAP::vScrollBarValueChanged(int value)
{
    if(value < 0) value = 0;
    ix.showLabelY = value;

    if(ix.camStreamMode == 0 && ix.imageReady == GetSingleFrame_Success)
    {
        scaleImage(0.0);
    }

}

//for live
void EZCAP::on_horizontalScrollBar_ImgShow_valueChanged(int value)
{
    if(value < 0) value = 0;
    ix.showLabelX = value;
}

//for live
void EZCAP::on_verticalScrollBar_ImgShow_valueChanged(int value)
{
    if(value < 0) value = 0;
    ix.showLabelY = value;
}
