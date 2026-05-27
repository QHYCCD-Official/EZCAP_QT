#include "include/cameraChooser.h"
#include "ui_cameraChooser.h"
#include "ezCap.h"
#include "mainMenu.h"
#include "managementMenu.h"
#include "ui_managementMenu.h"
//20200220 lyl Add ReadMode Dialog
//#include "readmode.h"
//#include "ui_readmode.h"
#include "outputdebug.h"
#include <QException>

#include "favorite.h"
#include "ui_favorite.h"

#include "liveCapThread.h"
#include "threadProcessImage.h"
#include "gpsTool.h"
//#include "qhyccdStatus.h"
#include "include/dllqhyccd.h"
#include <QDebug>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <QtCore/QtGlobal>
#include <QScrollArea>
#include <QTimer>

extern struct IX ix;

extern qhyccd_handle *camhandle;

CameraChooser *cameraChooser;

//char camid[64];
extern char camid[64];

CameraChooser::CameraChooser(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CameraChooser)
{
    ui->setupUi(this);

    this->setFixedSize(this->width(), this->height());//设置窗口不可改变大小
    this->setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);// hide the "?" help button ont the title bar

    mainWidget = (EZCAP*)this->parentWidget();//获取父窗口指针
}

CameraChooser::~CameraChooser()
{
    delete ui;
}

/**
 * @brief CameraChooser::on_coBox_cameraChooser_currentIndexChanged        
 */
void CameraChooser::on_coBox_cameraChooser_currentIndexChanged(const QString &arg1)
{
    if(arg1 == "")//No Camera
    {
        ui->okBtn_cameraChooser->setEnabled(false);
    }
    else
    {
        ui->okBtn_cameraChooser->setEnabled(true);

        ix.CamID        = ui->coBox_cameraChooser->currentText();
        ix.CamModel     = ix.CamID.left(ix.CamID.lastIndexOf('-'));

        //根据相机ID获得的相机型号，查看之前连接的相机记录里是否有相同型号，如果有则使用之前保留的参数，若没有则使用SDK默认参数进行设置
        if(mainWidget->SearchCamFromIni(ix.CamModel + "-" + QString::number(ix.camStreamMode)))
        {
            ix.FoundCam = true;
            mainWidget->loadParasFromIni();
        }

        uint32_t ret    = QHYCCD_ERROR;
        QByteArray pstr = ix.CamID.toLatin1();

        memset(camid, '\0', 64);
        memcpy(camid, pstr.data(),pstr.size());

        camhandle = libqhyccd->OpenQHYCCD(camid);
        if(camhandle != NULL)
        {
            ix.ReadMode_Num = 0;
            ix.ReadMode_List.clear();//清空列表

            if(libqhyccd->GetQHYCCDNumberOfReadModes)
            {
                ret = libqhyccd->GetQHYCCDNumberOfReadModes(camhandle,&ix.ReadMode_Num);

                if(ix.ReadMode_Num >=1 && ret == QHYCCD_SUCCESS)
                {
                    for(int i = 0; i < ix.ReadMode_Num; i++)
                    {
                        if (libqhyccd->GetQHYCCDReadModeName(camhandle, i, ix.ReadMode_Name) == QHYCCD_SUCCESS)
                        {
                            ix.ReadMode_List.append(ix.ReadMode_Name);//列表填充
                        }
                    }

                    ui->comboBox_readmode->blockSignals(true);
                    ui->comboBox_readmode->clear();
                    ui->comboBox_readmode->addItems(ix.ReadMode_List);

                    if(ix.ReadMode_Num == 1)
                    {
                        OutputDebug("EZCAP | %s | %s | camReadMode = 0", __FILE__, __FUNCTION__);
                        ix.ReadMode = 0;
                        ui->comboBox_readmode->setEnabled(false);
                    }
                    else
                    {
                        if(!ix.FoundCam)
                        {
                            ix.ReadMode = 0;
                        }
                        ui->comboBox_readmode->setEnabled(true);
                    }
                    ui->comboBox_readmode->setCurrentIndex(ix.ReadMode);
                    ui->comboBox_readmode->blockSignals(false);
                }
                else
                {
                    OutputDebug("EZCAPWARNING | %s | %s | GetQHYCCDNumberOfReadModes() Failed!", __FILE__, __FUNCTION__);
                }
            }
            else
            {
                OutputDebug("EZCAPWARNING | %s | %s | GetQHYCCDNumberOfReadModes() Has No This Function!", __FILE__, __FUNCTION__);
            }

            ret = libqhyccd->CloseQHYCCD(camhandle);
            if(ret == QHYCCD_ERROR)
            {
                OutputDebug("EZCAPWARNING | %s | %s | CloseQHYCCD() Failed!", __FILE__, __FUNCTION__);
            }
        }
    }
}

/**
 * @brief CameraChooser::on_cancelBtn_cameraChooser_clicked       
 */
void CameraChooser::on_cancelBtn_cameraChooser_clicked()
{
    this->close();
}

/**
 * @brief CameraChooser::on_okBtn_cameraChooser_clicked 
 */        
void CameraChooser::on_okBtn_cameraChooser_clicked()
{
    uint32_t ret    = QHYCCD_ERROR;
    QByteArray pstr = ix.CamID.toLatin1();

    memset(camid, '\0', 64);
    memcpy(camid, pstr.data(),pstr.size());

    camhandle = libqhyccd->OpenQHYCCD(camid);
    if(camhandle != NULL)
    {
        ix.isConnected = true;
        mainWidget->resetFrameCount();

        OutputDebug("EZCAP | %s | %s | camReadMode = %d", __FILE__, __FUNCTION__, ix.ReadMode);
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
            QMessageBox::critical(this,tr("Error"),tr("Camera initialization failed!"),QMessageBox::Ok);
        }
        else
        {
            ix.lastCamStreamMode = ix.camStreamMode;
        }

        ret =libqhyccd-> InitQHYCCD(camhandle);
        if(ret != QHYCCD_SUCCESS)
        {
            qCritical("InitQHYCCD: failed");
            QMessageBox::critical(this,tr("Error"),tr("Camera can not connect as init failure!"),QMessageBox::Ok);
        }

        ix.zoomMode = Zoom_SpecifyScaling;
        ix.scaleFactor = 1.0;
        mainMenuBar->actFitWindow->trigger();
        mainWidget->scrollArea_ImgShow->setWidgetResizable(true);

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

        mainWidget->getParamsFromCamera();//设置界面控件初始状态

        // 发送相机连接信号
        emit connect_camera();

        if(ix.camStreamMode == 1)
        {
            mainWidget->setStretchLUT(mainWidget->Live_WPOS, mainWidget->Live_BPOS);
            managerMenu->ui->hSlider_bPos->setValue(mainWidget->Live_BPOS);
            managerMenu->ui->hSlider_wPos->setValue(mainWidget->Live_WPOS);

            ret = libqhyccd->BeginQHYCCDLive(camhandle);
            if(ret != QHYCCD_SUCCESS)
            {
                OutputDebug("EZCAPWARNING | %s | %s | BeginQHYCCDLive() Failed!", __FILE__, __FUNCTION__);
            }
            else
            {
                connect(mainWidget->liveCap, SIGNAL(gotFPSData()), mainWidget, SLOT(showFPS()));
                mainWidget->liveCap->start();
                mainWidget->threadProcessImage->start();

                mainWidget->updateImgTimer->start(30);
                mainWidget->updateFrameTimer->start(300);
            }
        }
    }
    else
    {
        qCritical("OpenQHYCCD: failure");
        QMessageBox::critical(this,tr("Warning"),tr("Camera Connect Failure!"),QMessageBox::Ok);
    }
}

/**
 * @brief CameraChooser::on_logo_cameraChooser_clicked      
 */
void CameraChooser::on_logo_cameraChooser_clicked()
{
    QDesktopServices::openUrl(QUrl("http://www.qhyccd.com"));
}

void CameraChooser::resetUI()
{
    ui->retranslateUi(this);
}

void CameraChooser::on_comboBox_readmode_currentIndexChanged(int index)
{
    OutputDebug("EZCAP | %s | %s | index = %d", __FILE__, __FUNCTION__, index);

    if(index >= 0)
    {
        ix.ReadMode = index;
    }
    else
    {
        OutputDebug("EZCAPERROR | %s | %s | Index Value Error!", __FILE__, __FUNCTION__);
    }
}
