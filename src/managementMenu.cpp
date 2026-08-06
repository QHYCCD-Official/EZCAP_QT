#include "managementMenu.h"
#include "ui_managementMenu.h"
#include "ezCap.h"
#include "planner.h"
#include "favorite.h"
#include "cameraChooser.h"
#include "outputdebug.h"
#include "liveCapThread.h"
#include "threadProcessImage.h"
#include "gpsTool.h"

//#include "qhyccdStatus.h"
#include "include/dllqhyccd.h"
#include <QGridLayout>
#include <QVBoxLayout>
#include <QListView>
#include <QDebug>
#include <QMessageBox>
#include <QTimer>
#include <QMutex>
#include <QQueue>

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#else
#include <QRegExp>
#include <QRegExpValidator>
#endif

//extern QQueue<unsigned char*> frameQueue;
//extern QQueue<unsigned char*> histQueue;

ManagementMenu *managerMenu;
extern struct IX ix;
extern qhyccd_handle *camhandle;
extern QMutex capImgMutex;

ManagementMenu::ManagementMenu(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ManagementMenu)
{
    ui->setupUi(this);

    //-------------------preview layout---------------------------
    QGridLayout *widgetPreviewLayout = new QGridLayout;
    widgetPreviewLayout->setContentsMargins(8,5,8,5); //设置四周间隔
    widgetPreviewLayout->setHorizontalSpacing(5);     //设置水平间隔
    widgetPreviewLayout->setVerticalSpacing(5);       //设置垂直间隔

    widgetPreviewLayout->addWidget(ui->label_Gain_preview,        0, 0, 1, 1);
    widgetPreviewLayout->addWidget(ui->hSlider_Gain_preview,      0, 1, 1, 3);
    widgetPreviewLayout->addWidget(ui->lineEdit_Gain_preview,     0, 4, 1, 1);
    widgetPreviewLayout->addWidget(ui->label_Offset_preview,      1, 0, 1, 1);
    widgetPreviewLayout->addWidget(ui->hSlider_Offset_preview,    1, 1, 1, 3);
    widgetPreviewLayout->addWidget(ui->lineEdit_Offset_preview,   1, 4, 1, 1);
    widgetPreviewLayout->addWidget(ui->label_expunit_preview,     2, 0, 1, 1);
    widgetPreviewLayout->addWidget(ui->lineEdit_exposure_preview, 2, 1, 1, 4);
    widgetPreviewLayout->addWidget(ui->label_exposure_preview,    3, 0, 1, 1);
    widgetPreviewLayout->addWidget(ui->hSlider_exposure_preview,  3, 1, 1, 4);
    widgetPreviewLayout->addWidget(ui->pBtn_cross,                4, 1, 1, 1);
    widgetPreviewLayout->addWidget(ui->pBtn_grid,                 4, 2, 1, 1);
    widgetPreviewLayout->addWidget(ui->pBtn_circle,               4, 3, 1, 1);
    widgetPreviewLayout->addWidget(ui->proBar_previewTime,        5, 0, 1, 5);
    widgetPreviewLayout->addWidget(ui->proBar_preview,            6, 0, 1, 5);
    widgetPreviewLayout->addWidget(ui->pBtn_preview,              7, 0, 1, 2);
    widgetPreviewLayout->addWidget(ui->pBtn_live_preview,         7, 3, 1, 2);
    ui->widget_preview->setLayout(widgetPreviewLayout);

    QGridLayout *grpBoxPreviewLayout = new QGridLayout;
    grpBoxPreviewLayout->setContentsMargins(0, 0, 0, 0);
    grpBoxPreviewLayout->setSpacing(0);//设置间隔为0
    grpBoxPreviewLayout->addWidget(ui->head_preview,   0, 0, 1, 1);
    grpBoxPreviewLayout->addWidget(ui->widget_preview, 1, 0, 1, 1);
    ui->grpBox_preview->setLayout(grpBoxPreviewLayout);
    ui->grpBox_preview->setFixedWidth(261);

    //----------------focus layout---------------------------------

    QGridLayout *widgetFocusLayout = new QGridLayout;
    widgetFocusLayout->setContentsMargins(8,5,8,5);
    widgetFocusLayout->setHorizontalSpacing(5);
    widgetFocusLayout->setVerticalSpacing(5);

    widgetFocusLayout->addWidget(ui->label_Gain_focus,        0, 0, 1, 1);
    widgetFocusLayout->addWidget(ui->hSlider_Gain_focus,      0, 1, 1, 3);
    widgetFocusLayout->addWidget(ui->lineEdit_Gain_focus,     0, 4, 1, 1);
    widgetFocusLayout->addWidget(ui->label_Offset_focus,      1, 0, 1, 1);
    widgetFocusLayout->addWidget(ui->hSlider_Offset_focus,    1, 1, 1, 3);
    widgetFocusLayout->addWidget(ui->lineEdit_Offset_focus,   1, 4, 1, 1);
    widgetFocusLayout->addWidget(ui->label_expunit_focus,     2, 0, 1, 1);
    widgetFocusLayout->addWidget(ui->lineEdit_exposure_focus, 2, 1, 1, 4);
    widgetFocusLayout->addWidget(ui->label_exposure_focus,    3, 0, 1, 1);
    widgetFocusLayout->addWidget(ui->hSlider_exposure_focus,  3, 1, 1, 4);
    widgetFocusLayout->addWidget(ui->pBtn_focus,              4, 0, 1, 2);
    widgetFocusLayout->addWidget(ui->pBtn_live_focus,         4, 3, 1, 2);
    ui->widget_focus->setLayout(widgetFocusLayout);

    QGridLayout *grpBoxFocusLayout = new QGridLayout;
    grpBoxFocusLayout->setContentsMargins(0, 0, 0, 0);
    grpBoxFocusLayout->setSpacing(0);
    grpBoxFocusLayout->addWidget(ui->head_focus,   0, 0, 1, 1);
    grpBoxFocusLayout->addWidget(ui->widget_focus, 1, 0, 1, 1);
    ui->grpBox_focus->setLayout(grpBoxFocusLayout);
    ui->grpBox_focus->setFixedWidth(261);

    //-----------------capture layout------------------------------
    QGridLayout *widgetCapLayout = new QGridLayout;
    widgetCapLayout->setContentsMargins(8,5,8,5);
    widgetCapLayout->setHorizontalSpacing(5);
    widgetCapLayout->setVerticalSpacing(5);

    widgetCapLayout->addWidget(ui->label_readmode_capture,    0, 0, 1, 1);
    widgetCapLayout->addWidget(ui->comboBox_readmode_capture, 0, 1, 1, 4);
    widgetCapLayout->addWidget(ui->label_color_capture,       1, 0, 1, 1);
    widgetCapLayout->addWidget(ui->comboBox_color_capture,    1, 1, 1, 4);
    widgetCapLayout->addWidget(ui->label_Gain_capture,        2, 0, 1, 1);
    widgetCapLayout->addWidget(ui->hSlider_Gain_capture,      2, 1, 1, 3);
    widgetCapLayout->addWidget(ui->lineEdit_Gain_capture,     2, 4, 1, 1);
    widgetCapLayout->addWidget(ui->label_Offset_capture,      3, 0, 1, 1);
    widgetCapLayout->addWidget(ui->hSlider_Offset_capture,    3, 1, 1, 3);
    widgetCapLayout->addWidget(ui->lineEdit_Offset_capture,   3, 4, 1, 1);
    widgetCapLayout->addWidget(ui->lineEdit_exposure_capture, 4, 1, 1, 2);
    widgetCapLayout->addWidget(ui->comBoxSingleUnit,          4, 3, 1, 2);
    widgetCapLayout->addWidget(ui->label_exposure_capture,    5, 0, 1, 1);
    widgetCapLayout->addWidget(ui->hSlider_exposure_capture,  5, 1, 1, 4);
    widgetCapLayout->addWidget(ui->proBar_captureTime,        6, 0, 1, 5);
    widgetCapLayout->addWidget(ui->proBar_capture,            7, 0, 1, 5);
    widgetCapLayout->addWidget(ui->bin1x1,                    8, 0, 1, 2);
    widgetCapLayout->addWidget(ui->bin2x2,                    8, 3, 1, 2);
    widgetCapLayout->addWidget(ui->bin3x3,                    9, 0, 1, 2);
    widgetCapLayout->addWidget(ui->bin4x4,                    9, 3, 1, 2);
    widgetCapLayout->addWidget(ui->bin6x6,                   10, 0, 1, 2);
    widgetCapLayout->addWidget(ui->bin8x8,                   10, 3, 1, 2);
    widgetCapLayout->addWidget(ui->checkBox_highSpeed,       11, 0, 1, 5);
    widgetCapLayout->addWidget(ui->pBtn_capture,             12, 0, 1, 2);
    widgetCapLayout->addWidget(ui->pBtn_stop,                12, 3, 1, 2);
    ui->widget_capture->setLayout(widgetCapLayout);

    QGridLayout *grpBoxCaptureLayout = new QGridLayout;
    grpBoxCaptureLayout->setContentsMargins(0, 0, 0, 0);
    grpBoxCaptureLayout->setSpacing(0);
    grpBoxCaptureLayout->addWidget(ui->head_capture,   0, 0, 1, 1);
    grpBoxCaptureLayout->addWidget(ui->widget_capture, 1, 0, 1, 1);
    ui->grpBox_capture->setLayout(grpBoxCaptureLayout);
    ui->grpBox_capture->setFixedWidth(261);

    //------------save --------------------------------------
    QGridLayout *saveLayout = new QGridLayout;
    saveLayout->setContentsMargins(5, 1, 5, 1);
    saveLayout->setSpacing(5);

    saveLayout->addWidget(ui->btnOpenSaveFolder,    0, 0, 1, 1);
    saveLayout->addWidget(ui->btnSnapShot,          1, 0, 1, 1);
    saveLayout->addWidget(ui->comBoxSnapshotFormat, 1, 1, 1, 1);
    saveLayout->addWidget(ui->btnStartSaveCap,      2, 0, 1, 1);
    saveLayout->addWidget(ui->comBoxCapFormat,      2, 1, 1, 1);
    ui->widget_save->setLayout(saveLayout);

    QGridLayout *grpBoxSaveLayout = new QGridLayout;
    grpBoxSaveLayout->setContentsMargins(0, 0, 0, 0);
    grpBoxSaveLayout->setSpacing(0);
    grpBoxSaveLayout->addWidget(ui->head_save,  0, 0, 1, 1);
    grpBoxSaveLayout->addWidget(ui->widget_save, 1, 0, 1, 1);
    ui->grpBox_save->setLayout(grpBoxSaveLayout);
    ui->grpBox_save->setFixedWidth(261);
    ui->comBoxCapFormat->clear();
#ifdef Q_OS_MAC
    ui->comBoxCapFormat->addItem("MOV");
#else
    ui->comBoxCapFormat->addItem("AVI");
#endif

    //-----------------image format------------------------------
    QGridLayout *widgetLiveImageFormat = new QGridLayout;
    widgetLiveImageFormat->setContentsMargins(8, 5, 8, 5);
    widgetLiveImageFormat->setHorizontalSpacing(5);
    widgetLiveImageFormat->setVerticalSpacing(5);

    widgetLiveImageFormat->addWidget(ui->labelLiveReadMode,  0, 0, 1, 1);
    widgetLiveImageFormat->addWidget(ui->comBoxLiveReadMode, 0, 1, 1, 1);
    widgetLiveImageFormat->addWidget(ui->labelLiveBin,       1, 0, 1, 1);
    widgetLiveImageFormat->addWidget(ui->comBoxLiveBin,      1, 1, 1, 1);
    widgetLiveImageFormat->addWidget(ui->labelLiveBits,      2, 0, 1, 1);
    widgetLiveImageFormat->addWidget(ui->comBoxLiveBits,     2, 1, 1, 1);
    widgetLiveImageFormat->addWidget(ui->labelLiveColor,     3, 0, 1, 1);
    widgetLiveImageFormat->addWidget(ui->comBoxLiveColor,    3, 1, 1, 1);
    ui->widget_liveimageformat->setLayout(widgetLiveImageFormat);

    QGridLayout *grpBoxLiveImageFormat = new QGridLayout;
    grpBoxLiveImageFormat->setContentsMargins(0, 0, 0, 0);
    grpBoxLiveImageFormat->setSpacing(0);
    grpBoxLiveImageFormat->addWidget(ui->head_liveimageformat,   0, 0, 1, 1);
    grpBoxLiveImageFormat->addWidget(ui->widget_liveimageformat, 1, 0, 1, 1);
    ui->grpBox_liveImageFormat->setLayout(grpBoxLiveImageFormat);
    ui->grpBox_liveImageFormat->setFixedWidth(261);

    //-----------------camera setup------------------------------
    QGridLayout *widgetLiveCameraSetup = new QGridLayout;
    widgetLiveCameraSetup->setContentsMargins(8, 5, 8, 5);
    widgetLiveCameraSetup->setHorizontalSpacing(5);
    widgetLiveCameraSetup->setVerticalSpacing(5);

    widgetLiveCameraSetup->addWidget(ui->lineEditLiveExp,     0, 1, 1, 2);
    widgetLiveCameraSetup->addWidget(ui->comBoxLiveUnit,      0, 3, 1, 2);
    widgetLiveCameraSetup->addWidget(ui->labelLiveExposure,   1, 0, 1, 1);
    widgetLiveCameraSetup->addWidget(ui->sliderLiveExposure,  1, 1, 1, 4);
    widgetLiveCameraSetup->addWidget(ui->labelLiveGain,       2, 0, 1, 1);
    widgetLiveCameraSetup->addWidget(ui->sliderLiveGain,      2, 1, 1, 3);
    widgetLiveCameraSetup->addWidget(ui->lineEditLiveGain,    2, 4, 1, 1);
    widgetLiveCameraSetup->addWidget(ui->labelLiveOffset,     3, 0, 1, 1);
    widgetLiveCameraSetup->addWidget(ui->sliderLiveOffset,    3, 1, 1, 3);
    widgetLiveCameraSetup->addWidget(ui->lineEditLiveOffset,  3, 4, 1, 1);
    widgetLiveCameraSetup->addWidget(ui->labelLiveTraffic,    4, 0, 1, 1);
    widgetLiveCameraSetup->addWidget(ui->sliderLiveTraffic,   4, 1, 1, 3);
    widgetLiveCameraSetup->addWidget(ui->lineEditLiveTraffic, 4, 4, 1, 1);
    widgetLiveCameraSetup->addWidget(ui->labelLiveSpeed,      5, 0, 1, 1);
    widgetLiveCameraSetup->addWidget(ui->sliderLiveSpeed,     5, 1, 1, 3);
    widgetLiveCameraSetup->addWidget(ui->lineEditLiveSpeed,   5, 4, 1, 1);
    widgetLiveCameraSetup->addWidget(ui->labelLiveDDR,        6, 0, 1, 1);
    widgetLiveCameraSetup->addWidget(ui->comBoxLiveDDR,       6, 1, 1, 4);
    widgetLiveCameraSetup->addWidget(ui->labelLiveAMPV,       7, 0, 1, 1);
    widgetLiveCameraSetup->addWidget(ui->comBoxLiveAMPV,      7, 1, 1, 4);
    ui->widget_livecamerasetup->setLayout(widgetLiveCameraSetup);

    QGridLayout *grpBoxLiveCameraSetup = new QGridLayout;
    grpBoxLiveCameraSetup->setContentsMargins(0, 0, 0, 0);
    grpBoxLiveCameraSetup->setSpacing(0);
    grpBoxLiveCameraSetup->addWidget(ui->head_livecamerasetup,    0, 0, 1, 1);
    grpBoxLiveCameraSetup->addWidget(ui->widget_livecamerasetup, 1, 0, 1, 1);
    ui->grpBox_liveCameraSetup->setLayout(grpBoxLiveCameraSetup);
    ui->grpBox_liveCameraSetup->setFixedWidth(261);

    //-----------------image setup------------------------------
    QGridLayout *widgetLiveImageSetup = new QGridLayout;
    widgetLiveImageSetup->setContentsMargins(8,5,8,5);
    widgetLiveImageSetup->setHorizontalSpacing(5);
    widgetLiveImageSetup->setVerticalSpacing(5);
    QLabel *tmpLabelImageSetup = new QLabel;
    tmpLabelImageSetup->setFixedHeight(5);

    widgetLiveImageSetup->addWidget(ui->labelLiveWBR,           0, 0, 1, 1);
    widgetLiveImageSetup->addWidget(ui->sliderLiveWBR,          0, 1, 1, 1);
    widgetLiveImageSetup->addWidget(ui->lineEditLiveWBR,        0, 2, 1, 1);
    widgetLiveImageSetup->addWidget(ui->labelLiveWBG,           1, 0, 1, 1);
    widgetLiveImageSetup->addWidget(ui->sliderLiveWBG,          1, 1, 1, 1);
    widgetLiveImageSetup->addWidget(ui->lineEditLiveWBG,        1, 2, 1, 1);
    widgetLiveImageSetup->addWidget(ui->labelLiveWBB,           2, 0, 1, 1);
    widgetLiveImageSetup->addWidget(ui->sliderLiveWBB,          2, 1, 1, 1);
    widgetLiveImageSetup->addWidget(ui->lineEditLiveWBB,        2, 2, 1, 1);
    widgetLiveImageSetup->addWidget(tmpLabelImageSetup,         3, 0, 1, 3);
    widgetLiveImageSetup->addWidget(ui->labelLiveBrightness,    4, 0, 1, 1);
    widgetLiveImageSetup->addWidget(ui->sliderLiveBrightness,   4, 1, 1, 1);
    widgetLiveImageSetup->addWidget(ui->lineEditLiveBrightness, 4, 2, 1, 1);
    widgetLiveImageSetup->addWidget(ui->labelLiveContrast,      5, 0, 1, 1);
    widgetLiveImageSetup->addWidget(ui->sliderLiveContrast,     5, 1, 1, 1);
    widgetLiveImageSetup->addWidget(ui->lineEditLiveContrast,   5, 2, 1, 1);
    widgetLiveImageSetup->addWidget(ui->labelLiveGamma,         6, 0, 1, 1);
    widgetLiveImageSetup->addWidget(ui->sliderLiveGamma,        6, 1, 1, 1);
    widgetLiveImageSetup->addWidget(ui->lineEditLiveGamma,      6, 2, 1, 1);
    ui->widget_liveimagesetup->setLayout(widgetLiveImageSetup);

    QGridLayout *grpBoxLiveImageSetup = new QGridLayout;
    grpBoxLiveImageSetup->setContentsMargins(0, 0, 0, 0);
    grpBoxLiveImageSetup->setSpacing(0);
    grpBoxLiveImageSetup->addWidget(ui->head_liveimagesetup,   0, 0, 1, 1);
    grpBoxLiveImageSetup->addWidget(ui->widget_liveimagesetup, 1, 0, 1, 1);
    ui->grpBox_liveImageSetup->setLayout(grpBoxLiveImageSetup);
    ui->grpBox_liveImageSetup->setFixedWidth(261);

    //-----------------Histogram layout--------------------------------
    ui->pBtn_stretchMinusB->setFixedHeight(16);
    ui->pBtn_stretchPlusB->setFixedHeight(16);
    ui->pBtn_coarse->setFixedSize(38,16);
    ui->pBtn_stretchMinusW->setFixedHeight(16);
    ui->pBtn_stretchPlusW->setFixedHeight(16);
    ui->pBtn_auto_histogram->setFixedSize(45,22);
    ui->cBox_autoStretchList->setFixedSize(131,22);
    ui->hSlider_bPos->setFixedHeight(24);
    ui->hSlider_wPos->setFixedHeight(24);

    QHBoxLayout *histBtnLayout = new QHBoxLayout;
    histBtnLayout->setContentsMargins(3, 3, 3, 3);
    histBtnLayout->setSpacing(3);
    QLabel *labelHist = new QLabel;
    labelHist->setFixedWidth(15);
    QLabel *labelHist2 = new QLabel;
    labelHist2->setFixedWidth(15);

    histBtnLayout->setSizeConstraint(QLayout::SetFixedSize);
    histBtnLayout->addWidget(labelHist);
    histBtnLayout->addWidget(ui->pBtn_stretchMinusB);
    histBtnLayout->addWidget(ui->pBtn_stretchPlusB);
    histBtnLayout->addWidget(ui->pBtn_coarse);
    histBtnLayout->addWidget(ui->pBtn_stretchMinusW);
    histBtnLayout->addWidget(ui->pBtn_stretchPlusW);
    histBtnLayout->addWidget(labelHist2);

    QVBoxLayout *hSliderWBLayout = new QVBoxLayout;
    hSliderWBLayout->setContentsMargins(0, 0, 0, 0);
    hSliderWBLayout->setSpacing(0);
    hSliderWBLayout->addWidget(ui->hSlider_wPos);
    hSliderWBLayout->addWidget(ui->hSlider_bPos);

    QHBoxLayout *autoBtnsLayout = new QHBoxLayout;
    autoBtnsLayout->setContentsMargins(0, 0, 0, 0);
    autoBtnsLayout->setSpacing(10);
    autoBtnsLayout->addWidget(ui->pBtn_auto_histogram);
    autoBtnsLayout->addWidget(ui->cBox_autoStretchList);

    QVBoxLayout *widgetHistLayout = new QVBoxLayout;
    widgetHistLayout->setSpacing(0);
    widgetHistLayout->setContentsMargins(4, 5, 4, 5);
    widgetHistLayout->addWidget(ui->img_hist);
    widgetHistLayout->addLayout(histBtnLayout);
    widgetHistLayout->addLayout(hSliderWBLayout);
    widgetHistLayout->addLayout(autoBtnsLayout);
    ui->widget_hist->setLayout(widgetHistLayout);

    QGridLayout *grpBoxHistLayout = new QGridLayout;
    grpBoxHistLayout->setContentsMargins(0, 0, 0, 0);
    grpBoxHistLayout->setSpacing(0);
    grpBoxHistLayout->addWidget(ui->head_hist,   0, 0, 1, 1);
    grpBoxHistLayout->addWidget(ui->widget_hist, 1, 0, 1, 1);
    ui->grpBox_hist->setLayout(grpBoxHistLayout);
    ui->grpBox_hist->setFixedWidth(261);

    //------------Region Of Interest-------------------------------------
    ui->img_Roi->setFixedSize(249,181);
    QGridLayout *widgetRoiLayout = new QGridLayout;
    widgetRoiLayout->setContentsMargins(5, 1, 5, 1);//left:3,top:3,right:3,bottom:3
    widgetRoiLayout->setSpacing(5);

    widgetRoiLayout->addWidget(ui->img_Roi,                    0, 0, 1, 4);
    widgetRoiLayout->addWidget(ui->comboBox_RoiSaved,          1, 0, 1, 4);
    widgetRoiLayout->addWidget(ui->label_sizex_Roi,            2, 0, 1, 1);
    widgetRoiLayout->addWidget(ui->lineEdit_sizex_Roi,         2, 1, 1, 1);
    widgetRoiLayout->addWidget(ui->label_sizey_Roi,            2, 2, 1, 1);
    widgetRoiLayout->addWidget(ui->lineEdit_sizey_Roi,         2, 3, 1, 1);
    widgetRoiLayout->addWidget(ui->pushButton_SetROI,          3, 0, 1, 2);
    widgetRoiLayout->addWidget(ui->pushButton_SaveROI,         3, 2, 1, 2);
    ui->widget_Roi->setLayout(widgetRoiLayout);

    QGridLayout *grpBoxRoiLayout = new QGridLayout;
    grpBoxRoiLayout->setContentsMargins(0, 0, 0, 0);
    grpBoxRoiLayout->setSpacing(0);
    grpBoxRoiLayout->addWidget(ui->head_Roi,   0, 0, 1, 1);
    grpBoxRoiLayout->addWidget(ui->widget_Roi, 1, 0, 1, 1);
    ui->grpBox_Roi->setLayout(grpBoxRoiLayout);
    ui->grpBox_Roi->setFixedWidth(261);


    //------------screenView layout--------------------------------------
    ui->img_screenView->setFixedSize(249,181);

    QGridLayout *widgetScreenLayout = new QGridLayout;
    widgetScreenLayout->setContentsMargins(5, 1, 5, 1);//left:3,top:3,right:3,bottom:3
    widgetScreenLayout->setSpacing(5);

    widgetScreenLayout->addWidget(ui->img_screenView,             0, 0, 1, 4);
    ui->widget_screenView->setLayout(widgetScreenLayout);

    QGridLayout *grpBoxScreenViewLayout = new QGridLayout;
    grpBoxScreenViewLayout->setContentsMargins(0, 0, 0, 0);
    grpBoxScreenViewLayout->setSpacing(0);
    grpBoxScreenViewLayout->addWidget(ui->head_screenView,   0, 0, 1, 1);
    grpBoxScreenViewLayout->addWidget(ui->widget_screenView, 1, 0, 1, 1);
    ui->grpBox_screenView->setLayout(grpBoxScreenViewLayout);
    ui->grpBox_screenView->setFixedWidth(261);

    //---------------------整体布局----------------------------------------
    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->setContentsMargins(5, 0, 5, 5);
    mainLayout->setSpacing(0);
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);
    mainLayout->addWidget(ui->grpBox_preview,         0, 0, 1, 2);
    mainLayout->addWidget(ui->grpBox_focus,           1, 0, 1, 2);
    mainLayout->addWidget(ui->grpBox_capture,         2, 0, 1, 2);
    mainLayout->addWidget(ui->grpBox_save,            3, 0, 1, 2);
    mainLayout->addWidget(ui->grpBox_liveImageFormat, 4, 0, 1, 2);
    mainLayout->addWidget(ui->grpBox_liveCameraSetup, 5, 0, 1, 2);
    mainLayout->addWidget(ui->grpBox_liveImageSetup,  6, 0, 1, 2);
    mainLayout->addWidget(ui->grpBox_Roi,             7, 0, 1, 2);
    mainLayout->addWidget(ui->grpBox_screenView,      8, 0, 1, 2);
    mainLayout->addWidget(ui->grpBox_hist,            9, 0, 1, 2);
    setLayout(mainLayout);
    setMinimumWidth(287);

    //----------------------init members and controls----------------------
    cmenu_captureExp = NULL;

    ui->cBox_autoStretchList->clear();
    ui->cBox_autoStretchList->addItem(QString("Noise Floor"));
    ui->cBox_autoStretchList->addItem(QString("BackGround Level"));
    ui->cBox_autoStretchList->addItem(QString("3times BackGround"));
    ui->cBox_autoStretchList->addItem(QString("10times BackGround"));
    ui->cBox_autoStretchList->addItem(QString("Max Range"));
    ui->cBox_autoStretchList->addItem(QString("OverScan X256"));
    ui->cBox_autoStretchList->addItem(QString("OverScan X128"));
    ui->cBox_autoStretchList->addItem(QString("OverScan X64"));
    ui->cBox_autoStretchList->addItem(QString("OverScan X32"));
    ui->cBox_autoStretchList->addItem(QString("OverScan X16"));
    ui->cBox_autoStretchList->addItem(QString("OverScan X8"));


    //-----------------初始设置管理菜单中子项不显示----------------------
    ui->head_preview->setChecked(false);
    ui->widget_preview->setVisible(false);
    ui->head_focus->setChecked(false);
    ui->widget_focus->setVisible(false);
    ui->head_capture->setChecked(false);
    ui->widget_capture->setVisible(false);
    ui->head_save->setChecked(false);
    ui->widget_save->setVisible(false);
    ui->head_liveimageformat->setChecked(false);
    ui->widget_liveimageformat->setVisible(false);
    ui->head_livecamerasetup->setChecked(false);
    ui->widget_livecamerasetup->setVisible(false);
    ui->head_liveimagesetup->setChecked(false);
    ui->widget_liveimagesetup->setVisible(false);
    ui->head_Roi->setChecked(false);
    ui->widget_Roi->setVisible(false);
    ui->head_screenView->setChecked(false);
    ui->widget_screenView->setVisible(false);
    ui->head_hist->setChecked(false);
    ui->widget_hist->setVisible(false);

    ui->cBox_autoStretchList->setView(new QListView());  //结合style.qss中设置item高度

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    QRegularExpression rxInt("[0-9]+$");
    QValidator *regInt = new QRegularExpressionValidator(rxInt, this);
    QRegularExpression rxExposure("^(?:[0-9]+(?:\\.[0-9]*)?|\\.[0-9]+)$");
    QValidator *regExposure = new QRegularExpressionValidator(rxExposure, this);
#else
    QRegExp rxInt("[0-9]+$");
    QValidator *regInt = new QRegExpValidator(rxInt, this);
    QRegExp rxExposure("^(?:[0-9]+(?:\\.[0-9]*)?|\\.[0-9]+)$");
    QValidator *regExposure = new QRegExpValidator(rxExposure, this);
#endif
    ui->lineEdit_Gain_preview->setValidator(regInt);
    ui->lineEdit_Offset_preview->setValidator(regInt);
    ui->lineEdit_exposure_preview->setValidator(regInt);
    ui->lineEdit_Gain_focus->setValidator(regInt);
    ui->lineEdit_Offset_focus->setValidator(regInt);
    ui->lineEdit_exposure_focus->setValidator(regInt);
    ui->lineEdit_Gain_capture->setValidator(regInt);
    ui->lineEdit_Offset_capture->setValidator(regInt);
    ui->lineEdit_exposure_capture->setValidator(regExposure);
    ui->lineEditLiveExp->setValidator(regInt);
    ui->lineEditLiveGain->setValidator(regInt);
    ui->lineEditLiveOffset->setValidator(regInt);
    ui->lineEditLiveTraffic->setValidator(regInt);
    ui->lineEditLiveWBR->setValidator(regInt);
    ui->lineEditLiveWBG->setValidator(regInt);
    ui->lineEditLiveWBB->setValidator(regInt);
    ui->lineEdit_sizex_Roi->setValidator(regInt);
    ui->lineEdit_sizey_Roi->setValidator(regInt);

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    QRegularExpression rxDouble("^(-?[0]|-?[0-9]{1,2})(?:\\.\\d{1,1})?$|(^\\t?$)");
    QValidator *regDouble = new QRegularExpressionValidator(rxDouble, this);
#else
    QRegExp rxDouble("^(-?[0]|-?[0-9]{1,2})(?:\\.\\d{1,1})?$|(^\\t?$)");
    QValidator *regDouble = new QRegExpValidator(rxDouble, this);
#endif
    ui->lineEditLiveBrightness->setValidator(regDouble);
    ui->lineEditLiveContrast->setValidator(regDouble);
    ui->lineEditLiveGamma->setValidator(regDouble);
}

ManagementMenu::~ManagementMenu()
{
    delete ui;
}

void ManagementMenu::resetUI()
{
    ui->retranslateUi(this);
}

void ManagementMenu::camera_connected()
{
    uint32_t ret = QHYCCD_ERROR;

    //Offset init
    if(ix.camStreamMode == 0)
    {
        ///
        /// 设置控件及参数
        ///
        OutputDebug("EZCAP | %s | %s | number = %d camReadMode = %d", __FILE__, __FUNCTION__, ix.ReadMode_List.count(), ix.ReadMode);
        //Read Mode
        if(ix.ReadMode_Num == 1)
        {
            ui->label_readmode_capture->setVisible(false);
            ui->comboBox_readmode_capture->setVisible(false);
        }
        else
        {
            ui->label_readmode_capture->setVisible(false);
            ui->comboBox_readmode_capture->setVisible(false);
        }

        //Color
        if(!ix.Color_Fun)
        {
            ui->label_color_capture->setVisible(false);
            ui->comboBox_color_capture->setVisible(false);
        }
        else
        {
            ui->label_color_capture->setVisible(true);
            ui->comboBox_color_capture->setVisible(true);

            ui->comboBox_color_capture->blockSignals(true);
            ui->comboBox_color_capture->clear();
            ui->comboBox_color_capture->addItem("ON");
            ui->comboBox_color_capture->addItem("OFF");
            ui->comboBox_color_capture->addItem("Bayer GBBR");
            ui->comboBox_color_capture->addItem("Bayer GRBG");
            ui->comboBox_color_capture->addItem("Bayer BGGR");
            ui->comboBox_color_capture->addItem("Bayer RGGB");

            if(ix.IsCvtColor)
            {
                ui->comboBox_color_capture->setCurrentText("ON");
                ui->comboBox_color_capture->setEnabled(true);

                if(ix.BinX != 1 && ix.BinY != 1)
                {
                    ui->comboBox_color_capture->setCurrentText("OFF");
                    ui->comboBox_color_capture->setEnabled(false);
                }
                lastBayer = "ON";
            }
            else
            {
                ui->comboBox_color_capture->setCurrentText("OFF");
                lastBayer = "OFF";
            }
            ui->comboBox_color_capture->blockSignals(false);

            ix.Bayer = ix.CamBayer;

            ret = libqhyccd->SetQHYCCDDebayerOnOff(camhandle, ix.Color);
            ix.Color_Last = ix.Color;
        }

        //Bits
        ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_TRANSFERBIT, ix.Bits);
        ix.Bits_Last = ix.Bits;

        //binning init
        ui->bin1x1->setVisible(ix.Bin11_Fun);
        ui->bin2x2->setVisible(ix.Bin22_Fun);
        ui->bin3x3->setVisible(ix.Bin33_Fun);
        ui->bin4x4->setVisible(ix.Bin44_Fun);
        ui->bin6x6->setVisible(ix.Bin66_Fun);
        ui->bin8x8->setVisible(ix.Bin88_Fun);

        ui->bin1x1->blockSignals(true);
        ui->bin1x1->setChecked(true);
        ui->bin1x1->blockSignals(false);

        ix.BinX = 1;
        ix.BinY = 1;
        ix.RoiX = 0;
        ix.RoiY = 0;
        ix.RoiW  = ix.ImageW_Min / ix.BinX;
        ix.RoiH  = ix.ImageH_Min / ix.BinY;
//        mainWidget->adjustScrollBar(ix.FrameW_Last, ix.FrameH_Last);

        ui->img_Roi->setFixedSize(ui->img_Roi->width(), ui->img_Roi->width() * ix.RoiH / ix.RoiW);

        ui->comboBox_RoiSaved->setVisible(true);
        ui->label_sizex_Roi->setVisible(true);
        ui->label_sizey_Roi->setVisible(true);
        ui->lineEdit_sizex_Roi->setVisible(true);
        ui->lineEdit_sizey_Roi->setVisible(true);
        ui->pushButton_SetROI->setVisible(true);
        ui->pushButton_SaveROI->setVisible(true);
        ui->comboBox_RoiSaved->blockSignals(true);
        ui->comboBox_RoiSaved->clear();
        ui->comboBox_RoiSaved->addItem(QString::number(ix.RoiW) + " x " + QString::number(ix.RoiH));
        ui->comboBox_RoiSaved->addItem(QString::number(ix.RoiW * 8 / 10 / 2 * 2) + " x " + QString::number(ix.RoiH * 8 / 10 / 2 * 2));
        ui->comboBox_RoiSaved->addItem(QString::number(ix.RoiW / 2 / 2 * 2) + " x " + QString::number(ix.RoiH / 2 / 2 * 2));
        ui->comboBox_RoiSaved->addItem(QString::number(ix.RoiW * 3 / 10 / 2 * 2) + " x " + QString::number(ix.RoiH * 3 / 10 / 2 * 2));
        ui->comboBox_RoiSaved->addItem(QString::number(ix.RoiW / 10 / 2 * 2) + " x " + QString::number(ix.RoiH / 10 / 2 * 2));
        QString CustomedROI = "";
        mainWidget->loadParamFromIni(ix.CamModel + "-" + QString::number(ix.camStreamMode), "CustomedROI_Bin1x1", &CustomedROI, "");
        if(!CustomedROI.isEmpty())
        {
            QStringList CustomedROIList = CustomedROI.split(";");
            for(int i = 0; i < CustomedROIList.count(); i++)
            {
                if(!CustomedROIList.at(i).isEmpty())
                    ui->comboBox_RoiSaved->addItem(CustomedROIList.at(i));
            }
        }
        ui->comboBox_RoiSaved->addItem("Customed");
        ui->comboBox_RoiSaved->blockSignals(false);
        ui->lineEdit_sizex_Roi->setText(QString::number(ix.RoiW));
        ui->lineEdit_sizey_Roi->setText(QString::number(ix.RoiH));

        ui->label_sizex_Roi->setVisible(false);
        ui->label_sizey_Roi->setVisible(false);
        ui->lineEdit_sizex_Roi->setVisible(false);
        ui->lineEdit_sizey_Roi->setVisible(false);
        ui->pushButton_SetROI->setVisible(false);
        ui->pushButton_SaveROI->setVisible(false);

        ret = libqhyccd->SetQHYCCDBinMode(camhandle, ix.BinX, ix.BinY);
        ret = libqhyccd->SetQHYCCDResolution(camhandle, ix.RoiX, ix.RoiY, ix.RoiW, ix.RoiH);
        ix.BinX_Last = ix.BinX;
        ix.BinY_Last = ix.BinY;
        ix.RoiX_Last = ix.RoiX;
        ix.RoiY_Last = ix.RoiY;
        ix.RoiW_Last  = ix.RoiW;
        ix.RoiH_Last  = ix.RoiH;

        //Gain init
        ui->label_Gain_preview->setVisible(ix.Gain_Fun);
        ui->hSlider_Gain_preview->setVisible(ix.Gain_Fun);
        ui->lineEdit_Gain_preview->setVisible(ix.Gain_Fun);
        ui->label_Gain_focus->setVisible(ix.Gain_Fun);
        ui->hSlider_Gain_focus->setVisible(ix.Gain_Fun);
        ui->lineEdit_Gain_focus->setVisible(ix.Gain_Fun);
        ui->label_Gain_capture->setVisible(ix.Gain_Fun);
        ui->hSlider_Gain_capture->setVisible(ix.Gain_Fun);
        ui->lineEdit_Gain_capture->setVisible(ix.Gain_Fun);
        if(ix.Gain_Fun)
        {
            ui->hSlider_Gain_preview->blockSignals(true);
            ui->lineEdit_Gain_preview->blockSignals(true);
            ui->hSlider_Gain_focus->blockSignals(true);
            ui->lineEdit_Gain_focus->blockSignals(true);
            ui->hSlider_Gain_capture->blockSignals(true);
            ui->lineEdit_Gain_capture->blockSignals(true);

            ui->hSlider_Gain_preview->setMaximum((int)ix.Gain_Max);
            ui->hSlider_Gain_preview->setMinimum((int)ix.Gain_Min);
            ui->hSlider_Gain_preview->setSingleStep((int)ix.Gain_Step);
            ui->hSlider_Gain_preview->setValue((int)ix.Gain);
            ui->lineEdit_Gain_preview->setText(QString::number((int)ix.Gain));

            ui->hSlider_Gain_focus->setMaximum((int)ix.Gain_Max);
            ui->hSlider_Gain_focus->setMinimum((int)ix.Gain_Min);
            ui->hSlider_Gain_focus->setSingleStep((int)ix.Gain_Step);
            ui->hSlider_Gain_focus->setValue((int)ix.Gain);
            ui->lineEdit_Gain_focus->setText(QString::number((int)ix.Gain));

            ui->hSlider_Gain_capture->setMaximum((int)ix.Gain_Max);
            ui->hSlider_Gain_capture->setMinimum((int)ix.Gain_Min);
            ui->hSlider_Gain_capture->setSingleStep((int)ix.Gain_Step);
            ui->hSlider_Gain_capture->setValue((int)ix.Gain);
            ui->lineEdit_Gain_capture->setText(QString::number((int)ix.Gain));

            ui->hSlider_Gain_preview->blockSignals(false);
            ui->lineEdit_Gain_preview->blockSignals(false);
            ui->hSlider_Gain_focus->blockSignals(false);
            ui->lineEdit_Gain_focus->blockSignals(false);
            ui->hSlider_Gain_capture->blockSignals(false);
            ui->lineEdit_Gain_capture->blockSignals(false);

            ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_GAIN, ix.Gain);
            ix.Gain_Last = ix.Gain;
        }

        //Offset init
        ui->label_Offset_preview->setVisible(ix.Offset_Fun);
        ui->hSlider_Offset_preview->setVisible(ix.Offset_Fun);
        ui->lineEdit_Offset_preview->setVisible(ix.Offset_Fun);
        ui->label_Offset_focus->setVisible(ix.Offset_Fun);
        ui->hSlider_Offset_focus->setVisible(ix.Offset_Fun);
        ui->lineEdit_Offset_focus->setVisible(ix.Offset_Fun);
        ui->label_Offset_capture->setVisible(ix.Offset_Fun);
        ui->hSlider_Offset_capture->setVisible(ix.Offset_Fun);
        ui->lineEdit_Offset_capture->setVisible(ix.Offset_Fun);
        if(ix.Offset_Fun)
        {
            ui->hSlider_Offset_preview->blockSignals(true);
            ui->lineEdit_Offset_preview->blockSignals(true);
            ui->hSlider_Offset_focus->blockSignals(true);
            ui->lineEdit_Offset_focus->blockSignals(true);
            ui->hSlider_Offset_capture->blockSignals(true);
            ui->lineEdit_Offset_capture->blockSignals(true);

            ui->hSlider_Offset_preview->setMaximum((int)ix.Offset_Max);
            ui->hSlider_Offset_preview->setMinimum((int)ix.Offset_Min);
            ui->hSlider_Offset_preview->setSingleStep((int)ix.Offset_Step);
            ui->hSlider_Offset_preview->setValue((int)ix.Offset);
            ui->lineEdit_Offset_preview->setText(QString::number((int)ix.Offset));

            ui->hSlider_Offset_focus->setMaximum((int)ix.Offset_Max);
            ui->hSlider_Offset_focus->setMinimum((int)ix.Offset_Min);
            ui->hSlider_Offset_focus->setSingleStep((int)ix.Offset_Step);
            ui->hSlider_Offset_focus->setValue((int)ix.Offset);
            ui->lineEdit_Offset_focus->setText(QString::number((int)ix.Offset));

            ui->hSlider_Offset_capture->setMaximum((int)ix.Offset_Max);
            ui->hSlider_Offset_capture->setMinimum((int)ix.Offset_Min);
            ui->hSlider_Offset_capture->setSingleStep((int)ix.Offset_Step);
            ui->hSlider_Offset_capture->setValue((int)ix.Offset);
            ui->lineEdit_Offset_capture->setText(QString::number((int)ix.Offset));

            ui->hSlider_Offset_preview->blockSignals(false);
            ui->lineEdit_Offset_preview->blockSignals(false);
            ui->hSlider_Offset_focus->blockSignals(false);
            ui->lineEdit_Offset_focus->blockSignals(false);
            ui->hSlider_Offset_capture->blockSignals(false);
            ui->lineEdit_Offset_capture->blockSignals(false);

            ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_OFFSET, ix.Offset);
            ix.Offset_Last = ix.Offset;
        }

        //expose init
        ui->label_expunit_preview->setVisible(true);
        ui->lineEdit_exposure_preview->setVisible(true);
        ui->label_exposure_preview->setVisible(true);
        ui->hSlider_exposure_preview->setVisible(true);
        ui->label_expunit_focus->setVisible(true);
        ui->lineEdit_exposure_focus->setVisible(true);
        ui->label_exposure_focus->setVisible(true);
        ui->hSlider_exposure_focus->setVisible(true);
        ui->lineEdit_exposure_capture->setVisible(true);
        ui->comBoxSingleUnit->setVisible(true);
        ui->label_exposure_capture->setVisible(true);
        ui->hSlider_exposure_capture->setVisible(true);
//        if(ix.ExpTime_Fun)
//        {
            ui->lineEdit_exposure_preview->blockSignals(true);
            ui->hSlider_exposure_preview->blockSignals(true);
            ui->lineEdit_exposure_focus->blockSignals(true);
            ui->hSlider_exposure_focus->blockSignals(true);
            ui->lineEdit_exposure_capture->blockSignals(true);
            ui->comBoxSingleUnit->blockSignals(true);
            ui->hSlider_exposure_capture->blockSignals(true);

            if(ix.ExpTime < 0) ix.ExpTime = 0;
            ui->hSlider_exposure_preview->setRange(1, 10000);
            ui->hSlider_exposure_focus->setRange(1, 10000);
            if(ix.ExpUnit == 1000000.0)
            {
                ui->hSlider_exposure_capture->setRange(1, 1200);
            }
            else if(ix.ExpUnit == 60000000.0)
            {
                ui->hSlider_exposure_capture->setRange(20, 60);
            }
            else
            {
                ui->hSlider_exposure_capture->setRange(1, 1000);
            }
            if(ix.ExpUnit == 1000000.0)
            {
                if(ix.ExpTime < 1)    ix.ExpTime = 1;
                if(ix.ExpTime > 1200) ix.ExpTime = 1200;
            }
            if(ix.ExpUnit == 60000000.0)
            {
                if(ix.ExpTime < 20) ix.ExpTime = 20;
                if(ix.ExpTime > 60) ix.ExpTime = 60;
            }
            else
            {
                if(ix.ExpTime < 1)    ix.ExpTime = 1;
                if(ix.ExpTime > 1000) ix.ExpTime = 1000;
            }
            ui->hSlider_exposure_preview->setValue(ix.ExpTime);
            ui->lineEdit_exposure_preview->setText(QString::number(ix.ExpTime));
            ui->hSlider_exposure_focus->setValue(ix.ExpTime);
            ui->lineEdit_exposure_focus->setText(QString::number(ix.ExpTime));
            if(ix.ExpUnit == 1.0) ui->comBoxSingleUnit->setCurrentText("1~1000 us");
            if(ix.ExpUnit == 1000.0) ui->comBoxSingleUnit->setCurrentText("1~1000 ms");
            if(ix.ExpUnit == 1000000.0) ui->comBoxSingleUnit->setCurrentText("1~1200 s");
            if(ix.ExpUnit == 60000000.0) ui->comBoxSingleUnit->setCurrentText("20~60 min");
            ui->hSlider_exposure_capture->setValue(ix.ExpTime);
            ui->lineEdit_exposure_capture->setText(QString::number(ix.ExpTime));

            ui->lineEdit_exposure_preview->blockSignals(false);
            ui->hSlider_exposure_preview->blockSignals(false);
            ui->lineEdit_exposure_focus->blockSignals(false);
            ui->hSlider_exposure_focus->blockSignals(false);
            ui->lineEdit_exposure_capture->blockSignals(false);
            ui->comBoxSingleUnit->blockSignals(false);
            ui->hSlider_exposure_capture->blockSignals(false);

            ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_EXPOSURE, ix.ExpTime * ix.ExpUnit);
            ix.ExpTime_Last = ix.ExpTime;
            ix.ExpUnit_Last = ix.ExpUnit;
//        }

        //high speed
        ui->checkBox_highSpeed->setVisible(ix.Speed_Fun);
        if(ix.Speed_Fun)
        {
            ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_SPEED, ix.Speed);
            ix.Speed_Last = ix.Speed;
        }

        //-----------init controls status, after checked params from camera.
        /// NOTE:
        /// when managerMenu change to Preview mode, The Gain, Offset, Exp controls will be setted using ix.Gain, ix.Offset, ix.exp
        ///

        ui->img_screenView->setFixedSize(ui->img_screenView->width(), ui->img_screenView->width() * ix.RoiH / ix.RoiW);

        // 显示单帧模式控件，隐藏连续模式控件
        ui->grpBox_save->setVisible(false);
        ui->grpBox_liveImageFormat->setVisible(false);
        ui->grpBox_liveCameraSetup->setVisible(false);
        ui->grpBox_liveImageSetup->setVisible(false);

        ui->grpBox_preview->setVisible(true);
        ui->grpBox_focus->setVisible(true);
        ui->grpBox_capture->setVisible(true);

        ui->head_preview->setCheckable(true);
        ui->head_focus->setCheckable(true);
        ui->head_capture->setCheckable(true);

        ui->head_Roi->setCheckable(true);
        ui->head_screenView->setCheckable(true);
        ui->head_hist->setCheckable(true);

        //goto Preview mode in default.
        ui->head_preview->click();
        ui->head_screenView->click();
        ui->head_hist->click();
    }
    else if(ix.camStreamMode == 1)
    {
        ///
        /// 设置控件及参数
        ///
        //设置读出模式控件，若只有一个读出模式则隐藏显示
        ui->comBoxLiveReadMode->blockSignals(true);
        if(ix.ReadMode_Num == 1)
        {
            ui->labelLiveReadMode->setVisible(false);
            ui->comBoxLiveReadMode->setVisible(false);
        }
        else
        {
            ui->labelLiveReadMode->setVisible(false);
            ui->comBoxLiveReadMode->setVisible(false);
        }
        ui->comBoxLiveReadMode->blockSignals(false);


        //设置BIN控件
        ui->comBoxLiveBin->blockSignals(true);
        ui->comBoxLiveBin->clear();
        if(ix.Bin11_Fun == true) ui->comBoxLiveBin->addItem("1X1");
        if(ix.Bin22_Fun == true) ui->comBoxLiveBin->addItem("2X2");
        if(ix.Bin33_Fun == true) ui->comBoxLiveBin->addItem("3X3");
        if(ix.Bin44_Fun == true) ui->comBoxLiveBin->addItem("4X4");
        if(ix.Bin66_Fun == true) ui->comBoxLiveBin->addItem("6X6");
        if(ix.Bin88_Fun == true) ui->comBoxLiveBin->addItem("8X8");
        ui->comBoxLiveBin->setCurrentText("1X1");

        ix.BinX = 1;
        ix.BinY = 1;
        ix.RoiX = 0;
        ix.RoiY = 0;
        ix.RoiW  = ix.ImageW_Min / ix.BinX;
        ix.RoiH  = ix.ImageH_Min / ix.BinY;

        ui->img_Roi->setFixedSize(ui->img_Roi->width(), ui->img_Roi->width() * ix.RoiH / ix.RoiW);

        ui->comboBox_RoiSaved->setVisible(true);
        ui->label_sizex_Roi->setVisible(true);
        ui->label_sizey_Roi->setVisible(true);
        ui->lineEdit_sizex_Roi->setVisible(true);
        ui->lineEdit_sizey_Roi->setVisible(true);
        ui->pushButton_SetROI->setVisible(true);
        ui->pushButton_SaveROI->setVisible(true);
        ui->comboBox_RoiSaved->blockSignals(true);
        ui->comboBox_RoiSaved->clear();
        ui->comboBox_RoiSaved->addItem(QString::number(ix.RoiW) + " x " + QString::number(ix.RoiH));
        ui->comboBox_RoiSaved->addItem(QString::number(ix.RoiW * 8 / 10 / 2 * 2) + " x " + QString::number(ix.RoiH * 8 / 10 / 2 * 2));
        ui->comboBox_RoiSaved->addItem(QString::number(ix.RoiW / 2 / 2 * 2) + " x " + QString::number(ix.RoiH / 2 / 2 * 2));
        ui->comboBox_RoiSaved->addItem(QString::number(ix.RoiW * 3 / 10 / 2 * 2) + " x " + QString::number(ix.RoiH * 3 / 10 / 2 * 2));
        ui->comboBox_RoiSaved->addItem(QString::number(ix.RoiW / 10 / 2 * 2) + " x " + QString::number(ix.RoiH / 10 / 2 * 2));
        QString CustomedROI = "";
        mainWidget->loadParamFromIni(ix.CamModel + "-" + QString::number(ix.camStreamMode), "CustomedROI_Bin1x1", &CustomedROI, "");
        if(!CustomedROI.isEmpty())
        {
            QStringList CustomedROIList = CustomedROI.split(";");
            for(int i = 0; i < CustomedROIList.count(); i++)
            {
                if(!CustomedROIList.at(i).isEmpty())
                    ui->comboBox_RoiSaved->addItem(CustomedROIList.at(i));
            }
        }
        ui->comboBox_RoiSaved->addItem("Customed");
        ui->comboBox_RoiSaved->blockSignals(false);
        ui->lineEdit_sizex_Roi->setText(QString::number(ix.RoiW));
        ui->lineEdit_sizey_Roi->setText(QString::number(ix.RoiH));

        ui->label_sizex_Roi->setVisible(false);
        ui->label_sizey_Roi->setVisible(false);
        ui->lineEdit_sizex_Roi->setVisible(false);
        ui->lineEdit_sizey_Roi->setVisible(false);
        ui->pushButton_SetROI->setVisible(false);
        ui->pushButton_SaveROI->setVisible(false);

        mainWidget->adjustScrollBar(ix.FrameW_Last, ix.FrameH_Last);
        mainWidget->screenViewBoxResize();

        ret = libqhyccd->SetQHYCCDBinMode(camhandle, ix.BinX, ix.BinY);
        if(ret == QHYCCD_SUCCESS)
        {
            ix.BinX_Last  = ix.BinX;
            ix.BinY_Last  = ix.BinY;

#if CALAB_YAU_PLANETARIUM
            ret = libqhyccd->SetQHYCCDResolution(camhandle, 64, 484, 1920, 1080);
#else
            ret = libqhyccd->SetQHYCCDResolution(camhandle, ix.RoiX, ix.RoiY, ix.RoiW, ix.RoiH);
#endif
            if(ret != QHYCCD_SUCCESS)
            {
                OutputDebug("EZCAPWARNING | %s | %s | SetQHYCCDResolution() Failed! | %d %d %d %d", __FILE__, __FUNCTION__, ix.RoiX, ix.RoiY, ix.RoiW, ix.RoiH);
            }
            else
            {
                ix.RoiX_Last = ix.RoiX;
                ix.RoiY_Last = ix.RoiY;
                ix.RoiW_Last = ix.RoiW;
                ix.RoiH_Last = ix.RoiH;
            }
        }
        else
        {
            OutputDebug("EZCAPWARNING | %s | %s | SetQHYCCDBinMode() Failed! | %d %d", __FILE__, __FUNCTION__, ix.BinX, ix.BinY);
        }
        ui->comBoxLiveBin->blockSignals(false);

        //设置图像位数控件
        ui->comBoxLiveBits->blockSignals(true);
        ui->comBoxLiveBits->clear();
        if(ix.Color_Fun)
        {
            ui->comBoxLiveBits->addItem("RAW8");
            ui->comBoxLiveBits->addItem("RAW16");
            ui->comBoxLiveBits->addItem("RGB24");
        }
        else
        {
            ui->comBoxLiveBits->addItem("MONO8");
            ui->comBoxLiveBits->addItem("MONO16");
        }

        OutputDebug("EZCAP | %s | %s | Bits = %d Color = %d", __FILE__, __FUNCTION__, ix.Bits, ix.Color);
        if(ix.Bits == 8  && ix.Color == false)
        {
            if(ix.Color_Fun) ui->comBoxLiveBits->setCurrentText("RAW8");
            else            ui->comBoxLiveBits->setCurrentText("MONO8");
            ui->comBoxLiveColor->setVisible(true);
            ui->labelLiveColor->setVisible(true);
        }
        if(ix.Bits == 16 && ix.Color == false)
        {
            if(ix.Color_Fun) ui->comBoxLiveBits->setCurrentText("RAW16");
            else            ui->comBoxLiveBits->setCurrentText("MONO16");
            ui->comBoxLiveColor->setVisible(true);
            ui->labelLiveColor->setVisible(true);
        }
        if(ix.Bits == 8  && ix.Color == true)
        {
            ui->comBoxLiveBits->setCurrentText("RGB24");
            ui->comBoxLiveColor->setVisible(false);
            ui->labelLiveColor->setVisible(false);
        }
        ret = libqhyccd->SetQHYCCDDebayerOnOff(camhandle, ix.Color);
        OutputDebug("EZCAPDEBUG | %s | %s | SetQHYCCDDebayerOnOff() ret = %d", __FILE__, __FUNCTION__, ret);
        ix.Color_Last = ix.Color;
        ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_TRANSFERBIT, ix.Bits);
        OutputDebug("EZCAPDEBUG | %s | %s | SetQHYCCDParam() ret = %d", __FILE__, __FUNCTION__, ret);
        ix.Bits_Last = ix.Bits;
        ui->comBoxLiveBits->blockSignals(false);

        //设置彩色开关控件
        if(ix.Color_Fun)
        {
            ui->comBoxLiveColor->blockSignals(true);
            ui->comBoxLiveColor->clear();
            ui->comBoxLiveColor->addItem("ON");
            ui->comBoxLiveColor->addItem("OFF");
            ui->comBoxLiveColor->addItem("Bayer GBBR");
            ui->comBoxLiveColor->addItem("Bayer GRBG");
            ui->comBoxLiveColor->addItem("Bayer BGGR");
            ui->comBoxLiveColor->addItem("Bayer RGGB");
            ui->comBoxLiveColor->setCurrentText("OFF");
            ix.IsCvtColor = false;

            if(ix.BinX == 1 && ix.BinY == 1)
            {
                ui->labelLiveColor->setVisible(true);
                ui->comBoxLiveColor->setVisible(true);
            }
            else
            {
                ui->labelLiveColor->setVisible(false);
                ui->comBoxLiveColor->setVisible(false);
            }

            ui->comBoxLiveColor->blockSignals(false);
        }
        else
        {
            ui->labelLiveColor->setVisible(false);
            ui->comBoxLiveColor->setVisible(false);
        }

        //设置曝光时间控件
//        if(ix.ExpTime_Fun == true)
//        {
            ui->labelLiveExposure->setVisible(true);
            ui->lineEditLiveExp->setVisible(true);
            ui->comBoxLiveUnit->setVisible(true);
            ui->sliderLiveExposure->setVisible(true);

            ui->lineEditLiveExp->blockSignals(true);
            ui->comBoxLiveUnit->blockSignals(true);
            ui->sliderLiveExposure->blockSignals(true);

//            if(ix.ExpTime >= 1000.0 && ix.ExpTime < 1000000.0) //1~1000 ms
//            {
//                ui->comBoxLiveUnit->setCurrentIndex(0);
//                ui->sliderLiveExposure->setRange(1, 1000);
//                ui->sliderLiveExposure->setValue((uint32_t)ix.ExpTime / 1000);
//                ui->lineEditLiveExp->setText(QString::number((uint32_t)ix.ExpTime / 1000));
//            }
//            else if(ix.ExpTime >= 1000000.0 && ix.ExpTime < 1200000000.0) //1~1200 s
//            {
//                ui->comBoxLiveUnit->setCurrentIndex(1);
//                ui->sliderLiveExposure->setRange(1, 1000);
//                ui->sliderLiveExposure->setValue((int)(ix.ExpTime / 1000000.0));
//                ui->lineEditLiveExp->setText(QString::number((int)(ix.ExpTime / 1000000.0)));
//            }
//            else
//            {
//                ui->comBoxLiveUnit->setCurrentIndex(2);
//                ui->sliderLiveExposure->setRange(20, 60);
//                ui->sliderLiveExposure->setValue((int)(ix.ExpTime / 60000000.0));
//                ui->lineEditLiveExp->setText(QString::number((int)(ix.ExpTime / 60000000)));
//            }

            if(ix.ExpTime < 0) ix.ExpTime = 0;
            if(ix.ExpUnit == 1000000.0)
            {
                ui->sliderLiveExposure->setRange(1, 1200);
            }
            else if(ix.ExpUnit == 60000000.0)
            {
                ui->sliderLiveExposure->setRange(20, 60);
            }
            else
            {
                ui->sliderLiveExposure->setRange(1, 1000);
            }
            if(ix.ExpUnit == 1000000.0)
            {
                if(ix.ExpTime < 1)    ix.ExpTime = 1;
                if(ix.ExpTime > 1200) ix.ExpTime = 1200;
            }
            else if(ix.ExpUnit == 60000000.0)
            {
                if(ix.ExpTime < 20) ix.ExpTime = 20;
                if(ix.ExpTime > 60) ix.ExpTime = 60;
            }
            else
            {
                if(ix.ExpTime < 1)    ix.ExpTime = 1;
                if(ix.ExpTime > 1000) ix.ExpTime = 1000;
            }
            if(ix.ExpUnit == 1.0) ui->comBoxLiveUnit->setCurrentText("1~1000 us");
            if(ix.ExpUnit == 1000.0) ui->comBoxLiveUnit->setCurrentText("1~1000 ms");
            if(ix.ExpUnit == 1000000.0) ui->comBoxLiveUnit->setCurrentText("1~1200 s");
            if(ix.ExpUnit == 60000000.0) ui->comBoxLiveUnit->setCurrentText("20~60 min");
            ui->sliderLiveExposure->setValue(ix.ExpTime);
            ui->lineEditLiveExp->setText(QString::number(ix.ExpTime));

            ui->lineEditLiveExp->blockSignals(false);
            ui->comBoxLiveUnit->blockSignals(false);
            ui->sliderLiveExposure->blockSignals(false);

            ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_EXPOSURE, ix.ExpTime * ix.ExpUnit);
//        }
//        else
//        {
//            ui->labelLiveExposure->setVisible(false);
//            ui->lineEditLiveExp->setVisible(false);
//            ui->comBoxLiveUnit->setVisible(false);
//            ui->sliderLiveExposure->setVisible(false);
//        }

        //设置增益控件
        if(ix.Gain_Fun == true)
        {
            ui->labelLiveGain->setVisible(true);
            ui->sliderLiveGain->setVisible(true);
            ui->lineEditLiveGain->setVisible(true);

            ui->sliderLiveGain->blockSignals(true);
            ui->lineEditLiveGain->blockSignals(true);

            ui->sliderLiveGain->setRange((int)ix.Gain_Min, (int)ix.Gain_Max);
            ui->sliderLiveGain->setSingleStep((int)ix.Gain_Step);
            ui->sliderLiveGain->setValue((int)ix.Gain);
            ui->lineEditLiveGain->setText(QString::number((int)ix.Gain));

            ui->sliderLiveGain->blockSignals(false);
            ui->lineEditLiveGain->blockSignals(false);

            ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_GAIN, ix.Gain);
        }
        else
        {
            ui->labelLiveGain->setVisible(false);
            ui->sliderLiveGain->setVisible(false);
            ui->lineEditLiveGain->setVisible(false);
        }

        //设置Offset控件
        if(ix.Offset_Fun == true)
        {
            ui->labelLiveOffset->setVisible(true);
            ui->sliderLiveOffset->setVisible(true);
            ui->lineEditLiveOffset->setVisible(true);

            ui->sliderLiveOffset->blockSignals(true);
            ui->lineEditLiveOffset->blockSignals(true);

            ui->sliderLiveOffset->setRange((int)ix.Offset_Min, (int)ix.Offset_Max);
            ui->sliderLiveOffset->setSingleStep((int)ix.Offset_Step);
            ui->sliderLiveOffset->setValue((int)ix.Offset);
            ui->lineEditLiveOffset->setText(QString::number((uint32_t)ix.Offset));

            ui->sliderLiveOffset->blockSignals(false);
            ui->lineEditLiveOffset->blockSignals(false);

            ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_OFFSET, ix.Offset);
        }
        else
        {
            ui->labelLiveOffset->setVisible(false);
            ui->sliderLiveOffset->setVisible(false);
            ui->lineEditLiveOffset->setVisible(false);
        }

        //设置Traffic控件
        if(ix.Traffic_Fun == true)
        {
            ui->labelLiveTraffic->setVisible(true);
            ui->sliderLiveTraffic->setVisible(true);
            ui->lineEditLiveTraffic->setVisible(true);

            ui->sliderLiveTraffic->blockSignals(true);
            ui->lineEditLiveOffset->blockSignals(true);

            ui->sliderLiveTraffic->setRange((int)ix.Traffic_Min, (int)ix.Traffic_Max);
            ui->sliderLiveTraffic->setSingleStep((int)ix.Traffic_Step);
            ui->sliderLiveTraffic->setValue((int)ix.Traffic);
            ui->lineEditLiveTraffic->setText(QString::number(ix.Traffic));

            ui->sliderLiveTraffic->blockSignals(false);
            ui->lineEditLiveOffset->blockSignals(false);

            ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_USBTRAFFIC, ix.Traffic);
        }
        else
        {
            ui->labelLiveTraffic->setVisible(false);
            ui->sliderLiveTraffic->setVisible(false);
            ui->lineEditLiveTraffic->setVisible(false);
        }

        if(ix.Speed_Fun)
        {
            ui->labelLiveSpeed->setVisible(true);
            ui->sliderLiveSpeed->setVisible(true);
            ui->lineEditLiveSpeed->setVisible(true);

            ui->sliderLiveSpeed->blockSignals(true);
            ui->lineEditLiveSpeed->blockSignals(true);

            ui->sliderLiveSpeed->setRange((int)ix.Speed_Min, (int)ix.Speed_Max);
            ui->sliderLiveSpeed->setSingleStep((int)ix.Speed_Step);
            ui->sliderLiveSpeed->setPageStep((int)ix.Speed_Step);
            ui->sliderLiveSpeed->setValue((int)ix.Speed);
            ui->lineEditLiveSpeed->setText(QString::number(ix.Speed));

            ui->sliderLiveSpeed->blockSignals(false);
            ui->lineEditLiveSpeed->blockSignals(false);

            ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_SPEED, ix.Speed);
        }
        else
        {
            ui->labelLiveSpeed->setVisible(false);
            ui->sliderLiveSpeed->setVisible(false);
            ui->lineEditLiveSpeed->setVisible(false);
        }

        //设置DDR控件
        if(ix.DDR_Fun == true)
        {
            ui->labelLiveDDR->setVisible(true);
            ui->comBoxLiveDDR->setVisible(true);

            ui->comBoxLiveDDR->blockSignals(true);

            ui->comBoxLiveDDR->addItem("ON");
            ui->comBoxLiveDDR->addItem("OFF");
            if(ix.DDR)
                ui->comBoxLiveDDR->setCurrentText("ON");
            else
                ui->comBoxLiveDDR->setCurrentText("OFF");

            ui->comBoxLiveDDR->blockSignals(false);

            ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_DDR, ix.DDR);
        }
        else
        {
            ui->labelLiveDDR->setVisible(false);
            ui->comBoxLiveDDR->setVisible(false);
        }

        //设置AMPV控件
        if(ix.AMPV_Fun == true)
        {
            ui->labelLiveAMPV->setVisible(true);
            ui->comBoxLiveAMPV->setVisible(true);

            ui->comBoxLiveAMPV->blockSignals(true);

            ui->comBoxLiveAMPV->addItem("ON");
            ui->comBoxLiveAMPV->addItem("OFF");
            if(ix.AMPV)
                ui->comBoxLiveAMPV->setCurrentText("ON");
            else
                ui->comBoxLiveAMPV->setCurrentText("OFF");

            ui->comBoxLiveAMPV->blockSignals(false);

            ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_AMPV, ix.AMPV);
        }
        else
        {
            ui->labelLiveAMPV->setVisible(false);
            ui->comBoxLiveAMPV->setVisible(false);
        }

        //设置R Gain控件
        if(ix.WBR_Fun == true)
        {
            ui->labelLiveWBR->setVisible(true);
            ui->sliderLiveWBR->setVisible(true);
            ui->lineEditLiveWBR->setVisible(true);

            ui->sliderLiveWBR->blockSignals(true);
            ui->lineEditLiveWBR->blockSignals(true);

            ui->sliderLiveWBR->setRange((int)ix.WBR_Min, (int)ix.WBR_Max);
            ui->sliderLiveWBR->setSingleStep((int)ix.WBR_Step);
            ui->sliderLiveWBR->setValue((int)ix.WBR);
            ui->lineEditLiveWBR->setText(QString::number((int)ix.WBR));

            ui->sliderLiveWBR->blockSignals(false);
            ui->lineEditLiveWBR->blockSignals(false);

            ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_WBR, ix.WBR);
        }
        else
        {
            ui->labelLiveWBR->setVisible(false);
            ui->sliderLiveWBR->setVisible(false);
            ui->lineEditLiveWBR->setVisible(false);
        }

        //设置G Gain控件
        if(ix.WBG_Fun == true)
        {
            ui->labelLiveWBG->setVisible(true);
            ui->sliderLiveWBG->setVisible(true);
            ui->lineEditLiveWBG->setVisible(true);

            ui->sliderLiveWBG->blockSignals(true);
            ui->lineEditLiveWBG->blockSignals(true);

            ui->sliderLiveWBG->setRange((int)ix.WBG_Min, (int)ix.WBG_Max);
            ui->sliderLiveWBG->setSingleStep((int)ix.WBG_Step);
            ui->sliderLiveWBG->setValue((int)ix.WBG);
            ui->lineEditLiveWBG->setText(QString::number((int)ix.WBG));

            ui->sliderLiveWBG->blockSignals(false);
            ui->lineEditLiveWBG->blockSignals(false);

            ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_WBG, ix.WBG);
        }
        else
        {
            ui->labelLiveWBG->setVisible(false);
            ui->sliderLiveWBG->setVisible(false);
            ui->lineEditLiveWBG->setVisible(false);
        }

        //设置B Gain控件
        if(ix.WBB_Fun == true)
        {
            ui->labelLiveWBB->setVisible(true);
            ui->sliderLiveWBB->setVisible(true);
            ui->lineEditLiveWBB->setVisible(true);

            ui->sliderLiveWBB->blockSignals(true);
            ui->lineEditLiveWBB->blockSignals(true);

            ui->sliderLiveWBB->setRange((int)ix.WBB_Min, (int)ix.WBB_Max);
            ui->sliderLiveWBB->setSingleStep((int)ix.WBB_Step);
            ui->sliderLiveWBB->setValue((int)ix.WBB);
            ui->lineEditLiveWBB->setText(QString::number((int)ix.WBB));

            ui->sliderLiveWBB->blockSignals(false);
            ui->lineEditLiveWBB->blockSignals(false);

            ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_WBB, ix.WBB);
        }
        else
        {
            ui->labelLiveWBB->setVisible(false);
            ui->sliderLiveWBB->setVisible(false);
            ui->lineEditLiveWBB->setVisible(false);
        }

        //设置亮度控件
        if(ix.Brightness_Fun == true)
        {
            ui->labelLiveBrightness->setVisible(true);
            ui->sliderLiveBrightness->setVisible(true);
            ui->lineEditLiveBrightness->setVisible(true);

            ui->sliderLiveBrightness->blockSignals(true);
            ui->lineEditLiveBrightness->blockSignals(true);

            ui->sliderLiveBrightness->setRange((int)(ix.Brightness_Min*10), (int)(ix.Brightness_Max*10));
            ui->sliderLiveBrightness->setSingleStep((int)(ix.Brightness_Step*10));
            ui->sliderLiveBrightness->setValue((int)(ix.Brightness*10));
            ui->lineEditLiveBrightness->setText(QString::number(ix.Brightness, 10, 1));

            ui->sliderLiveBrightness->blockSignals(false);
            ui->lineEditLiveBrightness->blockSignals(false);

            ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_BRIGHTNESS, ix.Brightness);
        }
        else
        {
            ui->labelLiveBrightness->setVisible(false);
            ui->sliderLiveBrightness->setVisible(false);
            ui->lineEditLiveBrightness->setVisible(false);
        }

        //设置对比度控件
        if(ix.Contrast_Fun == true)
        {
            ui->labelLiveContrast->setVisible(true);
            ui->sliderLiveContrast->setVisible(true);
            ui->lineEditLiveContrast->setVisible(true);

            ui->sliderLiveContrast->blockSignals(true);
            ui->lineEditLiveContrast->blockSignals(true);

            ui->sliderLiveContrast->setRange((int)(ix.Contrast_Min*10), (int)(ix.Contrast_Max*10));
            ui->sliderLiveContrast->setSingleStep((int)(ix.Contrast_Step*10));
            ui->sliderLiveContrast->setValue((int)(ix.Contrast*10));
            ui->lineEditLiveContrast->setText(QString::number(ix.Contrast, 10, 1));

            ui->sliderLiveContrast->blockSignals(false);
            ui->lineEditLiveContrast->blockSignals(false);

            ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_CONTRAST, ix.Contrast);
        }
        else
        {
            ui->labelLiveContrast->setVisible(false);
            ui->sliderLiveContrast->setVisible(false);
            ui->lineEditLiveContrast->setVisible(false);
        }

        //设置Gamma控件
        OutputDebug("EZCAPDEBUG | %s | %s | Gamma_Fun = %d gamma = %f min = %f max = %f, step = %f", __FILE__, __FUNCTION__, ix.Gamma_Fun, ix.Gamma, ix.Gamma_Min, ix.Gamma_Max, ix.Gamma_Step);
//        printf("manager menu connect %f\n", ix.Gamma);
        if(ix.Gamma_Fun == true)
        {
            ui->labelLiveGamma->setVisible(true);
            ui->sliderLiveGamma->setVisible(true);
            ui->lineEditLiveGamma->setVisible(true);

            ui->sliderLiveGamma->blockSignals(true);
            ui->lineEditLiveGamma->blockSignals(true);

            ui->sliderLiveGamma->setRange((int)(ix.Gamma_Min*10), (int)(ix.Gamma_Max*10));
            ui->sliderLiveGamma->setSingleStep((int)(ix.Gamma_Step*10));
            ui->sliderLiveGamma->setValue((int)(ix.Gamma*10));
            ui->lineEditLiveGamma->setText(QString::number(ix.Gamma, 10, 1));

            ui->sliderLiveGamma->blockSignals(false);
            ui->lineEditLiveGamma->blockSignals(false);

            ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_GAMMA, ix.Gamma);
        }
        else
        {
            ui->labelLiveGamma->setVisible(false);
            ui->sliderLiveGamma->setVisible(false);
            ui->lineEditLiveGamma->setVisible(false);
        }

        ///
        /// 显示连续模式控件，隐藏单帧模式控件
        ///
        ui->grpBox_preview->setVisible(false);
        ui->grpBox_focus->setVisible(false);
        ui->grpBox_capture->setVisible(false);
        ui->grpBox_screenView->setVisible(false);

        ui->grpBox_save->setVisible(true);
        ui->grpBox_liveImageFormat->setVisible(true);
        ui->grpBox_liveCameraSetup->setVisible(true);
        ui->grpBox_liveImageSetup->setVisible(true);

        ui->head_save->setCheckable(true);
        ui->head_liveimageformat->setCheckable(true);
        ui->head_livecamerasetup->setCheckable(true);
        ui->head_liveimagesetup->setCheckable(true);

        ui->head_Roi->setCheckable(true);
//        ui->head_screenView->setCheckable(true);
        ui->head_hist->setCheckable(true);

        ui->head_liveimageformat->click();
        ui->head_livecamerasetup->click();
        ui->head_liveimagesetup->click();

        ui->head_hist->click();
    }
}

void ManagementMenu::camera_disconnected()
{
    if(ui->head_preview->isChecked())
    {
        ui->head_preview->click();
    }
    if(ui->head_focus->isChecked())
    {
        ui->head_focus->click();
    }
    if(ui->head_capture->isChecked())
    {
        ui->head_capture->click();
    }

    if(ui->head_save->isChecked())
    {
        ui->head_save->click();
    }

    if(ui->head_liveimageformat->isChecked())
    {
        ui->head_liveimageformat->click();
    }
    if(ui->head_livecamerasetup->isChecked())
    {
        ui->head_livecamerasetup->click();
    }
    if(ui->head_liveimagesetup->isChecked())
    {
        ui->head_liveimagesetup->click();
    }
    if(ui->head_Roi->isChecked())
    {
        ui->head_Roi->click();
    }
    if(ui->head_screenView->isChecked())
    {
        ui->head_screenView->click();
    }
    if(ui->head_hist->isChecked())
    {
        ui->head_hist->click();
    }

    ui->grpBox_preview->setVisible(true);
    ui->grpBox_focus->setVisible(true);
    ui->grpBox_capture->setVisible(true);

    ui->grpBox_liveImageFormat->setVisible(true);
    ui->grpBox_liveCameraSetup->setVisible(true);
    ui->grpBox_liveImageSetup->setVisible(true);

    ui->grpBox_screenView->setVisible(true);
}

int ManagementMenu::DoubleToInt(double d)
{
    double intPart = floor(d);//向下取整
    if ((d - intPart) >= (double)0.5){
        return (intPart + 1);
    }else{
        return intPart;
    }
}


void ManagementMenu::CollapseSettingPanels()
{
    // Fold management accordion panels while camera settings are applied,
    // matching original SwitchReadmodeBinFormat UI behavior.
    if(ix.camStreamMode == 0)
    {
        if(ui->head_capture->isChecked())
            ui->head_capture->click();
        ui->head_capture->setCheckable(false);
    }
    else if(ix.camStreamMode == 1)
    {
        if(ui->head_liveimageformat->isChecked())
            ui->head_liveimageformat->click();
        ui->head_liveimageformat->setCheckable(false);

        if(ui->head_save->isChecked())
        {
            saveStatus = true;
            ui->head_save->click();
        }
        else
        {
            saveStatus = false;
        }
        if(ui->head_livecamerasetup->isChecked())
        {
            cameraSetupStatus = true;
            ui->head_livecamerasetup->click();
        }
        else
        {
            cameraSetupStatus = false;
        }
        if(ui->head_liveimagesetup->isChecked())
        {
            imageSetupStatus  = true;
            ui->head_liveimagesetup->click();
        }
        else
        {
            imageSetupStatus  = false;
        }

        ui->head_save->setCheckable(false);
        ui->head_livecamerasetup->setCheckable(false);
        ui->head_liveimagesetup->setCheckable(false);
    }

    if(ui->head_Roi->isChecked())
    {
        RoiStatus = true;
        ui->head_Roi->click();
    }
    else
    {
        RoiStatus = false;
    }
    if(ui->head_screenView->isChecked())
    {
        screenViewStatus = true;
        ui->head_screenView->click();
    }
    else
    {
        screenViewStatus = false;
    }
    if(ui->head_hist->isChecked())
    {
        histStatus = true;
        ui->head_hist->click();
    }
    else
    {
        histStatus = false;
    }
    ui->head_Roi->setCheckable(false);
    ui->head_screenView->setCheckable(false);
    ui->head_hist->setCheckable(false);

    // Paint folded UI before long SDK work
    for(int i = 0; i < 5; i++)
    {
        QThread::msleep(1);
        QApplication::processEvents();
    }
}

void ManagementMenu::RestoreSettingPanels()
{
    if(ix.camStreamMode == 0)
    {
        ui->head_capture->setCheckable(true);
        if(!ui->head_capture->isChecked())
            ui->head_capture->click();
    }
    else if(ix.camStreamMode == 1)
    {
        ui->head_save->setCheckable(true);
        ui->head_liveimageformat->setCheckable(true);
        ui->head_livecamerasetup->setCheckable(true);
        ui->head_liveimagesetup->setCheckable(true);

        if(!ui->head_liveimageformat->isChecked())
            ui->head_liveimageformat->click();
        if(saveStatus && !ui->head_save->isChecked())
            ui->head_save->click();
        if(cameraSetupStatus && !ui->head_livecamerasetup->isChecked())
            ui->head_livecamerasetup->click();
        if(imageSetupStatus && !ui->head_liveimagesetup->isChecked())
            ui->head_liveimagesetup->click();
    }

    ui->head_Roi->setCheckable(true);
    ui->head_screenView->setCheckable(true);
    ui->head_hist->setCheckable(true);
    if(screenViewStatus && !ui->head_screenView->isChecked())
        ui->head_screenView->click();
    if(histStatus && !ui->head_hist->isChecked())
        ui->head_hist->click();
    if(RoiStatus && !ui->head_Roi->isChecked())
        ui->head_Roi->click();

    QApplication::processEvents();
}

int ManagementMenu::SwitchReadmodeBinFormat()
{
    uint32_t ret = QHYCCD_ERROR;

    if(ix.camStreamMode != 0 && ix.camStreamMode != 1)
    {
        OutputDebug("EZCAPERROR | %s | %s | Stream Mode Value Error!", __FILE__, __FUNCTION__);
        return 1;
    }

    CollapseSettingPanels();

    //关闭定时器
    if(ix.Cooler_Fun)
    {
//        mainWidget->stopTimerTemp();
        mainWidget->threadTempControl->stop();
    }

    if(ix.canFilterWheel && ix.CFWStatus == CFW_Moving)
    {
        mainWidget->stopCFWTimer();
        ix.CFWStatus = CFW_Idle;
    }

    if(ix.canContolSensorChamberCyclePUMP && ix.cyclePUMBStatus)
    {
        mainWidget->stopPumpTimer();
    }

    //---------------------------------------------------------
    ret = libqhyccd->CloseQHYCCD(camhandle);
    if(ret == QHYCCD_ERROR)
    {
        OutputDebug("EZCAPWARNING | %s | %s | CloseQHYCCD() Failed!", __FILE__, __FUNCTION__);
        RestoreSettingPanels();
        return 1;
    }

    // ret = libqhyccd->ReleaseQHYCCDResource();
    // if(ret == QHYCCD_ERROR)
    // {
    //     OutputDebug("EZCAPWARNING | %s | %s | ReleaseQHYCCDResource() Failed!", __FILE__, __FUNCTION__);
    //     return 1;
    // }

    camhandle = libqhyccd->OpenQHYCCD(ix.CamID.toLocal8Bit().data());
    if(camhandle != NULL)
    {
        ret = libqhyccd->SetQHYCCDReadMode(camhandle, ix.ReadMode);
        if(ret != QHYCCD_SUCCESS)
        {
            OutputDebug("EZCAPWARNING | %s | %s | SetQHYCCDReadMode() Failed!", __FILE__, __FUNCTION__);
            RestoreSettingPanels();
            return 1;
        }
        ix.ReadMode_Last = ix.ReadMode;

        ret = libqhyccd->SetQHYCCDStreamMode(camhandle, ix.camStreamMode);
        if(ret != QHYCCD_SUCCESS)
        {
            OutputDebug("EZCAPWARNING | %s | %s | SetQHYCCDStreamMode() Failed!", __FILE__, __FUNCTION__);
            RestoreSettingPanels();
            return 1;
        }

        ret = libqhyccd->InitQHYCCD(camhandle);
        if(ret != QHYCCD_SUCCESS)
        {
            OutputDebug("EZCAPWARNING | %s | %s | InitQHYCCD() Failed!", __FILE__, __FUNCTION__);
            RestoreSettingPanels();
            return 1;
        }

        ret = libqhyccd->GetQHYCCDChipInfo(camhandle, &ix.CCD_ChipW, &ix.CCD_ChipH, &ix.CCD_ImageW, &ix.CCD_ImageH, &ix.CCD_PixelW, &ix.CCD_PixelH, &ix.CCD_ImageB);

        if(ret == QHYCCD_SUCCESS && ix.CCD_ImageW != 0 && ix.CCD_ImageH != 0)
        {
            ix.ImageW_Min = ix.CCD_ImageW;
            ix.ImageH_Min = ix.CCD_ImageH;
            ix.Resolution_Min = ix.CCD_ImageW * ix.CCD_ImageH;
        }
        else
        {
            OutputDebug("EZCAPWARNING | %s | %s | GetQHYCCDChipInfo() Failed!", __FILE__, __FUNCTION__);
            RestoreSettingPanels();
            return 1;
        }

        ret = libqhyccd->GetQHYCCDReadModeResolution(camhandle, ix.ReadMode, &ix.ReadMode_ImageW, &ix.ReadMode_ImageH);
        if(ret == QHYCCD_SUCCESS && ix.ReadMode_ImageW != 0 && ix.ReadMode_ImageH != 0)
        {
            if(ix.Resolution_Min > ix.ReadMode_ImageW * ix.ReadMode_ImageH)
            {
                ix.ImageW_Min = ix.ReadMode_ImageW;
                ix.ImageH_Min = ix.ReadMode_ImageH;
                ix.Resolution_Min = ix.ReadMode_ImageW * ix.ReadMode_ImageH;
            }
        }
        else
        {
            OutputDebug("EZCAPWARNING | %s | %s | GetQHYCCDReadModeResolution() Failed!", __FILE__, __FUNCTION__);
            RestoreSettingPanels();
            return 1;
        }

        if(ix.CCD_ImageW != ix.ReadMode_ImageW || ix.CCD_ImageH != ix.ReadMode_ImageH)
        {
            OutputDebug("EZCAPWARNING | %s | %s | ChipInfo Resolution != Read Mode Resolution", __FILE__, __FUNCTION__);
            RestoreSettingPanels();
            return 1;
        }

        ret = libqhyccd->SetQHYCCDDebayerOnOff(camhandle, ix.Color);
        if(ret != QHYCCD_SUCCESS)
        {
            OutputDebug("EZCAPWARNING | %s | %s | SetQHYCCDDebayerOnOff() Failed!", __FILE__, __FUNCTION__);
            // keep going; Debayer failure is non-fatal here (original behavior)
        }

        ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_TRANSFERBIT, ix.Bits);
        if(ret != QHYCCD_SUCCESS)
        {
            OutputDebug("EZCAPWARNING | %s | %s | SetQHYCCDParam() CONTROL_TRANSFERBIT Failed!", __FILE__, __FUNCTION__);
            RestoreSettingPanels();
            return 1;
        }

        if(ix.ReadMode_Last != ix.ReadMode) //切换读出模式之后，默认设置为1X1 BIN 全分辨率
        {
            ix.ReadMode_Last = ix.ReadMode;
            ix.BinX_Last  = ix.BinX = 1;
            ix.BinY_Last  = ix.BinY = 1;
            ix.RoiX = 0;
            ix.RoiY = 0;
            ix.RoiW  = ix.ImageW_Min;
            ix.RoiH  = ix.ImageH_Min;

            ui->comBoxLiveBin->blockSignals(true);
            ui->comBoxLiveBin->setCurrentText("1X1");
            ui->comBoxLiveBin->blockSignals(false);

            cv::Mat ImgRoiTemp = cv::Mat::zeros(cv::Size(ui->img_Roi->width(), ui->img_Roi->height()), CV_8UC3);
            QImage *RoiQImg = mainWidget->MatToQImage(ImgRoiTemp);
            ui->img_Roi->setPixmap(QPixmap::fromImage(*RoiQImg));
            ImgRoiTemp.release();
        }

        if(ix.BinX_Last != ix.BinX || ix.BinY_Last != ix.BinY) //切换BIN的时候以全分辨率为设置基准
        {
            ix.RoiX = 0;
            ix.RoiY = 0;
            ix.RoiW  = ix.ImageW_Min / ix.BinX;
            ix.RoiH  = ix.ImageH_Min / ix.BinY;

            cv::Mat ImgRoiTemp = cv::Mat::zeros(cv::Size(ui->img_Roi->width(), ui->img_Roi->height()), CV_8UC3);
            QImage *RoiQImg = mainWidget->MatToQImage(ImgRoiTemp);
            ui->img_Roi->setPixmap(QPixmap::fromImage(*RoiQImg));
            ImgRoiTemp.release();
        }

        ret = libqhyccd->SetQHYCCDBinMode(camhandle, ix.BinX, ix.BinY);
        if(ret != QHYCCD_SUCCESS)
        {
            OutputDebug("EZCAPWARNING | %s | %s | SetQHYCCDBinMode() Failed!", __FILE__, __FUNCTION__);
            RestoreSettingPanels();
            return 1;
        }
        else
        {
            ix.BinX_Last = ix.BinX;
            ix.BinY_Last = ix.BinY;
        }

#if CALAB_YAU_PLANETARIUM
        ret = libqhyccd->SetQHYCCDResolution(camhandle, 64, 484, 1920, 1080);
#else
        ret = libqhyccd->SetQHYCCDResolution(camhandle, ix.RoiX, ix.RoiY, ix.RoiW, ix.RoiH);
#endif
        if(ret != QHYCCD_SUCCESS)
        {
            OutputDebug("EZCAPWARNING | %s | %s | SetQHYCCDResolution() Failed!", __FILE__, __FUNCTION__);
            RestoreSettingPanels();
            return 1;
        }
        else
        {
            ix.RoiX_Last = ix.RoiX;
            ix.RoiY_Last = ix.RoiY;
            ix.RoiW_Last = ix.RoiW;
            ix.RoiH_Last = ix.RoiH;
        }

        ret = libqhyccd->GetQHYCCDEffectiveArea(camhandle, &ix.EffectiveX, &ix.EffectiveY, &ix.EffectiveW, &ix.EffectiveH);
        if(ret != QHYCCD_SUCCESS || (ix.EffectiveX+ix.EffectiveW) > ix.ImageW_Min || (ix.EffectiveY+ix.EffectiveH) > ix.ImageH_Min)
        {
            OutputDebug("EZCAPWARNING | %s | %s | GetQHYCCDEffectiveArea() Failed! %d %d %d %d", __FILE__, __FUNCTION__, ix.EffectiveX, ix.EffectiveY, ix.EffectiveW, ix.EffectiveH);
            QMessageBox::warning(this, tr("Warning"), tr("GetQHYCCDEffectiveArea() Failed(%1 %2 %3 %4 %5 %6)!").arg(ix.EffectiveX).arg(ix.EffectiveW).arg(ix.ImageW_Min).arg(ix.EffectiveY).arg(ix.EffectiveH).arg(ix.ImageH_Min), QMessageBox::Ok);
        }

        ret = libqhyccd->GetQHYCCDOverScanArea(camhandle, &ix.OverscanX, &ix.OverscanY, &ix.OverscanW, &ix.OverscanH);
        if(ret != QHYCCD_SUCCESS || (ix.OverscanX+ix.OverscanW) > ix.ImageW_Min || (ix.OverscanY+ix.OverscanH) > ix.ImageH_Min)
        {
            OutputDebug("EZCAPWARNING | %s | %s | GetQHYCCDOverscanArea() Failed! %d %d %d %d", __FILE__, __FUNCTION__, ix.OverscanX, ix.OverscanY, ix.OverscanW, ix.OverscanH);
            QMessageBox::warning(this, tr("Warning"), tr("GetQHYCCDOverScanArea() Failed(%1 %2 %3 %4 %5 %6)!").arg(ix.OverscanX).arg(ix.OverscanW).arg(ix.ImageW_Min).arg(ix.OverscanY).arg(ix.OverscanH).arg(ix.ImageH_Min), QMessageBox::Ok);
        }
    }
    else
    {
        OutputDebug("EZCAPWARNING | %s | %s | OpenQHYCCD() Failed!", __FILE__, __FUNCTION__);
    }

    RestoreSettingPanels();

    //打开制冷定时器
    if(ix.Cooler_Fun)
    {
//        mainWidget->startTimerTemp();
        mainWidget->threadTempControl->start();
    }

    return 0;
}
int ManagementMenu::ResetParameters()
{
    uint32_t ret = QHYCCD_ERROR;

//    if(ix.ExpTime_Fun)
//    {
        ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_EXPOSURE, ix.ExpTime * ix.ExpUnit);
        if(ret != QHYCCD_SUCCESS)
        {
            OutputDebug("QEZCAPWARNING | %s | %s | SetQHYCCDParam() CONTROL_EXPOSURE Failed!", __FILE__, __FUNCTION__);
            return 1;
        }
//    }

    if(ix.Gain_Fun)
    {
        ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_GAIN, ix.Gain);
        if(ret != QHYCCD_SUCCESS)
        {
            OutputDebug("EZCAPWARNING | %s | %s | SetQHYCCDParam() CONTROL_Gain Failed!", __FILE__, __FUNCTION__);
            return 1;
        }
    }

    if(ix.Offset_Fun)
    {
        ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_OFFSET, ix.Offset);
        if(ret != QHYCCD_SUCCESS)
        {
            OutputDebug("EZCAPWARNING | %s | %s | SetQHYCCDParam() CONTROL_Offset Failed!", __FILE__, __FUNCTION__);
            return 1;
        }
    }

    if(ix.Traffic_Fun)
    {
        ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_USBTRAFFIC, ix.Traffic);
        if(ret != QHYCCD_SUCCESS)
        {
            OutputDebug("EZCAPWARNING | %s | %s | SetQHYCCDParam() CONTROL_Traffic Failed!", __FILE__, __FUNCTION__);
            return 1;
        }
    }

    if(ix.Speed_Fun)
    {
        ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_SPEED, ix.Speed);
        if(ret != QHYCCD_SUCCESS)
        {
            OutputDebug("EZCAPWARNING | %s | %s | SetQHYCCDParam() CONTROL_SPEED Failed!", __FILE__, __FUNCTION__);
            return 1;
        }
    }

    if(ix.camStreamMode == 1)
    {
        if(ix.DDR_Fun)
        {
            ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_DDR, ix.DDR);
            if(ret != QHYCCD_SUCCESS)
            {
                OutputDebug("EZCAPWARNING | %s | %s | SetQHYCCDParam CONTROL_DDR Failed!", __FILE__, __FUNCTION__);
                return 1;
            }
        }

        if(ix.AMPV_Fun)
        {
            ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_AMPV, ix.AMPV);
            if(ret != QHYCCD_SUCCESS)
            {
                OutputDebug("EZCAPWARNING | %s | %s | SetQHYCCDParam() CONTROL_AMPV Failed!", __FILE__, __FUNCTION__);
                return 1;
            }
        }

        if(ix.WBR_Fun)
        {
            ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_WBR, ix.WBR);
            if(ret != QHYCCD_SUCCESS)
            {
                OutputDebug("EZCAPWARNING | %s | %s | SetQHYCCDParam() CONTROL_WBR Failed!", __FILE__, __FUNCTION__);
                return 1;
            }
        }

        if(ix.WBG_Fun)
        {
            ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_WBG, ix.WBG);
            if(ret != QHYCCD_SUCCESS)
            {
                OutputDebug("EZCAPWARNING | %s | %s | SetQHYCCDParam() CONTROL_WBG Failed!", __FILE__, __FUNCTION__);
                return 1;
            }
        }

        if(ix.WBB_Fun)
        {
            ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_WBB, ix.WBB);
            if(ret != QHYCCD_SUCCESS)
            {
                OutputDebug("EZCAPWARNING | %s | %s | SetQHYCCDParam() CONTROL_WBB Failed!", __FILE__, __FUNCTION__);
                return 1;
            }
        }

        if(ix.Brightness_Fun)
        {
            ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_BRIGHTNESS, ix.Brightness);
            if(ret != QHYCCD_SUCCESS)
            {
                OutputDebug("EZCAPWARNING | %s | %s | SetQHYCCDParam() CONTROL_BRIGHTNESS Failed!", __FILE__, __FUNCTION__);
                return 1;
            }
        }

        if(ix.Contrast_Fun)
        {
            ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_CONTRAST, ix.Contrast);
            if(ret != QHYCCD_SUCCESS)
            {
                OutputDebug("EZCAPWARNING | %s | %s | SetQHYCCDParam() CONTROL_CONTRAST Failed!", __FILE__, __FUNCTION__);
                return 1;
            }
        }

        if(ix.Gamma_Fun)
        {
            OutputDebug("EZCAPDEBUG | %s | %s | SetQHYCCDParam() CONTROL_GAMMA | ix.Gamma = %f", __FILE__, __FUNCTION__, ix.Gamma);
            ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_GAMMA, ix.Gamma);
            if(ret != QHYCCD_SUCCESS)
            {
                OutputDebug("EZCAPWARNING | %s | %s | SetQHYCCDParam() CONTROL_GAMMA Failed!", __FILE__, __FUNCTION__);
                return 1;
            }
        }
    }

    return 0;
}

int ManagementMenu::CloseCamera()
{
    uint32_t ret = QHYCCD_ERROR;

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
    mainWidget->isSettleDone     = true; //退出可能存在的Dither等待循环

    if(ix.CFWStatus == CFW_Moving)
    {
        mainWidget->stopCFWTimer();
        ix.CFWStatus = CFW_Idle;  //stop CFW timer
    }
    if(ix.plannerState == PlannerStatus_Start)
    {
        ix.plannerState = PlannerStatus_Stop;
    }

    //camera has connected
    if(camhandle)
    {
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

    mainWidget->saveParasAsIni();
}

//---------------------preview groupBox--------------------------------------------------
void ManagementMenu::on_head_preview_clicked(bool checked)
{
    ui->head_preview->setChecked(checked);
    ui->widget_preview->setVisible(checked);
    if(checked)
    {
//#ifdef Q_OS_MAC
//        ui->head_preview->setFixedHeight(18);
//        ui->head_capture->setFixedHeight(27);
//        ui->head_focus->setFixedHeight(27);
//#endif

        ui->head_focus->setChecked(false);
        ui->widget_focus->setVisible(false);
        ui->head_capture->setChecked(false);
        ui->widget_capture->setVisible(false);

        if(ix.workMode != WorkMode_Preview)
        {
            ix.workMode = WorkMode_Preview;

            emit switchWorkMode(ix.workMode);//emit signal to change the layout
        }
    }
    else
    {
//#ifdef Q_OS_MAC
//        ui->head_preview->setFixedHeight(27);
//#endif
    }
}
void ManagementMenu::on_hSlider_Gain_preview_valueChanged(int value)
{
    ui->lineEdit_Gain_preview->blockSignals(true);
    ui->lineEdit_Gain_preview->setText(QString::number(value));
    ui->lineEdit_Gain_preview->blockSignals(false);

//    Preview_Gain = (double)value;
//    ix.Gain = (double)value;
//    if(ix.Gain_Last != ix.Gain)
//    {
//        unsigned int ret = QHYCCD_ERROR;
//        ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_Gain, ix.Gain);
//        if(ret == QHYCCD_SUCCESS)
//        {
//            ix.Gain_Last = ix.Gain;
//        }
//        else
//        {
//            qCritical() << "SetQHYCCDParam CONTROL_Gain failure";
//        }
//    }
}
//void ManagementMenu::on_lineEdit_Gain_preview_textChanged(const QString &arg1)
//{

//}
void ManagementMenu::on_lineEdit_Gain_preview_textChanged(const QString &arg1)
{
    if(arg1 == "") return;

    if(ui->lineEdit_Gain_preview->text().toInt() > ui->hSlider_Gain_preview->maximum())
        ui->lineEdit_Gain_preview->setText(QString::number(ui->hSlider_Gain_preview->maximum()));
    if(ui->lineEdit_Gain_preview->text().toInt() < ui->hSlider_Gain_preview->minimum())
        ui->lineEdit_Gain_preview->setText(QString::number(ui->hSlider_Gain_preview->minimum()));

    ui->hSlider_Gain_preview->blockSignals(true);
    ui->hSlider_Gain_preview->setValue(ui->lineEdit_Gain_preview->text().toInt());
    ui->hSlider_Gain_preview->blockSignals(false);

//    ix.Gain = ui->lineEdit_Gain_preview->text().toDouble();
//    if(ix.Gain_Last != ix.Gain)
//    {
//        unsigned int ret = QHYCCD_ERROR;
//        ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_Gain, ix.Gain);
//        if(ret == QHYCCD_SUCCESS)
//        {
//            ix.Gain_Last = ix.Gain;
//        }
//        else
//        {
//            qCritical() << "SetQHYCCDParam CONTROL_Gain failure";
//        }
//    }
}

void ManagementMenu::on_hSlider_Offset_preview_valueChanged(int value)
{
    ui->lineEdit_Offset_preview->blockSignals(true);
    ui->lineEdit_Offset_preview->setText(QString::number(value));
    ui->lineEdit_Offset_preview->blockSignals(false);

//    ix.Offset = (double)value;
//    if(ix.Offset_Last != ix.Offset)
//    {
//        unsigned int ret = QHYCCD_ERROR;
//        ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_Offset, ix.Offset);
//        if(ret == QHYCCD_SUCCESS)
//        {
//            ix.Offset_Last = ix.Offset;
//        }
//        else
//        {
//            qCritical() << "SetQHYCCDParam CONTROL_Offset failure";
//        }
//    }
}

void ManagementMenu::on_lineEdit_Offset_preview_textChanged(const QString &arg1)
{
    if(arg1 == "") return;

    if(ui->lineEdit_Offset_preview->text().toInt() > ui->hSlider_Offset_preview->maximum())
        ui->lineEdit_Offset_preview->setText(QString::number(ui->hSlider_Offset_preview->maximum()));
    if(ui->lineEdit_Offset_preview->text().toInt() < ui->hSlider_Offset_preview->minimum())
        ui->lineEdit_Offset_preview->setText(QString::number(ui->hSlider_Offset_preview->minimum()));

    ui->hSlider_Offset_preview->blockSignals(true);
    ui->hSlider_Offset_preview->setValue(ui->lineEdit_Offset_preview->text().toInt());
    ui->hSlider_Offset_preview->blockSignals(false);

//    ix.Offset = ui->lineEdit_Offset_preview->text().toDouble();
//    if(ix.Offset_Last != ix.Offset)
//    {
//        unsigned int ret = QHYCCD_ERROR;
//        ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_Offset, ix.Offset);
//        if(ret == QHYCCD_SUCCESS)
//        {
//            ix.Offset_Last = ix.Offset;
//        }
//        else
//        {
//            qCritical() << "SetQHYCCDParam CONTROL_Offset failure";
//        }
//    }
}

void ManagementMenu::on_comBoxPreviewUnit_currentTextChanged()
{

}

void ManagementMenu::on_hSlider_exposure_preview_valueChanged(int value)
{
    uint32_t ret = QHYCCD_ERROR;

    ui->lineEdit_exposure_preview->blockSignals(true);
    ui->lineEdit_exposure_preview->setText(QString::number(value));
    ui->lineEdit_exposure_preview->blockSignals(false);

//    ix.ExpTime = (double)value;
//    ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_EXPOSURE, ix.ExpTime * 1000.0);
//    ix.ExpTime_Last = ix.ExpTime;
}

void ManagementMenu::on_lineEdit_exposure_preview_textChanged(const QString &arg1)
{
    if(arg1 == "") return;

    if(ui->lineEdit_exposure_preview->text().toInt() > ui->hSlider_exposure_preview->maximum())
        ui->lineEdit_exposure_preview->setText(QString::number(ui->hSlider_exposure_preview->maximum()));
    if(ui->lineEdit_exposure_preview->text().toInt() < ui->hSlider_exposure_preview->minimum())
        ui->lineEdit_exposure_preview->setText(QString::number(ui->hSlider_exposure_preview->minimum()));

    ui->hSlider_exposure_preview->blockSignals(true);
    ui->hSlider_exposure_preview->setValue(ui->lineEdit_exposure_preview->text().toInt());
    ui->hSlider_exposure_preview->blockSignals(false);

//    ix.ExpTime = ui->lineEdit_exposure_preview->text().toDouble();
}

//---------------------focus groupBox----------------------------------------------------
void ManagementMenu::on_head_focus_clicked(bool checked)
{
    ui->head_focus->setChecked(checked);
    ui->widget_focus->setVisible(checked);
    if(checked)
    {
//#ifdef Q_OS_MAC
//        ui->head_focus->setFixedHeight(18);
//        ui->head_capture->setFixedHeight(27);
//        ui->head_preview->setFixedHeight(27);
//#endif

        ui->head_preview->setChecked(false);
        ui->widget_preview->setVisible(false);
        ui->head_capture->setChecked(false);
        ui->widget_capture->setVisible(false);

        if(ix.workMode != WorkMode_Focus)
        {
            ix.workMode = WorkMode_Focus;

            emit switchWorkMode(ix.workMode);//emit signal to change the layout
        }
    }
    else
    {
//#ifdef Q_OS_MAC
//        ui->head_focus->setFixedHeight(27);
//#endif
    }
}

void ManagementMenu::on_hSlider_Gain_focus_valueChanged(int value)
{
    ui->lineEdit_Gain_focus->blockSignals(true);
    ui->lineEdit_Gain_focus->setText(QString::number(value));
    ui->lineEdit_Gain_focus->blockSignals(false);

    ix.Gain = (double)value;
    if(ix.Gain_Last != ix.Gain)
    {
        uint32_t ret = QHYCCD_ERROR;
        ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_GAIN, ix.Gain);
        if(ret == QHYCCD_SUCCESS)
        {
            ix.Gain_Last = ix.Gain;
        }
        else
        {
            qCritical() << "SetQHYCCDParam CONTROL_Gain failure";
        }
    }
}

void ManagementMenu::on_lineEdit_Gain_focus_textChanged(const QString &arg1)
{
    if(arg1 == "") return;

    if(ui->lineEdit_Gain_focus->text().toInt() > ui->hSlider_Gain_focus->maximum())
        ui->lineEdit_Gain_focus->setText(QString::number(ui->hSlider_Gain_focus->maximum()));
    if(ui->lineEdit_Gain_focus->text().toInt() < ui->hSlider_Gain_focus->minimum())
        ui->lineEdit_Gain_focus->setText(QString::number(ui->hSlider_Gain_focus->minimum()));

    ui->hSlider_Gain_focus->blockSignals(true);
    ui->hSlider_Gain_focus->setValue(ui->lineEdit_Gain_focus->text().toInt());
    ui->hSlider_Gain_focus->blockSignals(false);

    ix.Gain = ui->lineEdit_Gain_focus->text().toDouble();

    if(ix.Gain_Last != ix.Gain)
    {
        uint32_t ret = QHYCCD_ERROR;
        ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_GAIN, ix.Gain);
        if(ret == QHYCCD_SUCCESS)
        {
            ix.Gain_Last = ix.Gain;
        }
        else
        {
            qCritical() << "SetQHYCCDParam CONTROL_Gain failure";
        }
    }
}

void ManagementMenu::on_hSlider_Offset_focus_valueChanged(int value)
{
    ui->lineEdit_Offset_focus->blockSignals(true);
    ui->lineEdit_Offset_focus->setText(QString::number(value));
    ui->lineEdit_Offset_focus->blockSignals(false);

    ix.Offset = (double)value;
    if(ix.Offset_Last != ix.Offset)
    {
        unsigned int ret = QHYCCD_ERROR;
        ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_OFFSET, ix.Offset);
        if(ret == QHYCCD_SUCCESS)
        {
            ix.Offset_Last = ix.Offset;
        }
        else
        {
            qCritical() << "SetQHYCCDParam CONTROL_Offset failure";
        }
    }
}

void ManagementMenu::on_lineEdit_Offset_focus_textChanged(const QString &arg1)
{
    if(arg1 == "") return;

    if(ui->lineEdit_Offset_focus->text().toInt() > ui->hSlider_Offset_focus->maximum())
        ui->lineEdit_Offset_focus->setText(QString::number(ui->hSlider_Offset_focus->maximum()));
    if(ui->lineEdit_Offset_focus->text().toInt() < ui->hSlider_Offset_focus->minimum())
        ui->lineEdit_Offset_focus->setText(QString::number(ui->hSlider_Offset_focus->minimum()));

    ui->hSlider_Offset_focus->blockSignals(true);
    ui->hSlider_Offset_focus->setValue(ui->lineEdit_Offset_focus->text().toInt());
    ui->hSlider_Offset_focus->blockSignals(false);

    ix.Offset = ui->lineEdit_Offset_focus->text().toDouble();
    if(ix.Offset_Last != ix.Offset)
    {
        unsigned int ret = QHYCCD_ERROR;
        ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_OFFSET, ix.Offset);
        if(ret == QHYCCD_SUCCESS)
        {
            ix.Offset_Last = ix.Offset;
        }
        else
        {
            qCritical() << "SetQHYCCDParam CONTROL_Offset failure";
        }
    }
}

void ManagementMenu::on_comBoxFocusUnit_currentTextChanged()
{

}

void ManagementMenu::on_hSlider_exposure_focus_valueChanged(int value)
{
    uint32_t ret = QHYCCD_ERROR;

    ui->lineEdit_exposure_focus->blockSignals(true);
    ui->lineEdit_exposure_focus->setText(QString::number(value));
    ui->lineEdit_exposure_focus->blockSignals(false);

    ix.ExpTime = (double)value;
    ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_EXPOSURE, ix.ExpTime * 1000.0);
    ix.ExpTime_Last = ix.ExpTime;
}

void ManagementMenu::on_lineEdit_exposure_focus_textChanged(const QString &arg1)
{
    if(arg1 == "") return;

    if(ui->lineEdit_exposure_focus->text().toInt() > ui->hSlider_exposure_focus->maximum())
        ui->lineEdit_exposure_focus->setText(QString::number(ui->hSlider_exposure_focus->maximum()));
    if(ui->lineEdit_exposure_focus->text().toInt() < ui->hSlider_exposure_focus->minimum())
        ui->lineEdit_exposure_focus->setText(QString::number(ui->hSlider_exposure_focus->minimum()));

    ui->hSlider_exposure_focus->blockSignals(true);
    ui->hSlider_exposure_focus->setValue(ui->lineEdit_exposure_focus->text().toInt());
    ui->hSlider_exposure_focus->blockSignals(false);

    ix.ExpTime = ui->lineEdit_exposure_focus->text().toDouble();
}

//---------------------capture groupBox----------------------------------------------------
void ManagementMenu::on_head_capture_clicked(bool checked)
{
    ui->head_capture->setChecked(checked);
    ui->widget_capture->setVisible(checked);
    if(checked)
    {
//#ifdef Q_OS_MAC
//        ui->head_capture->setFixedHeight(18);
//        ui->head_focus->setFixedHeight(27);
//        ui->head_preview->setFixedHeight(27);
//#endif

        ui->head_preview->setChecked(false);
        ui->widget_preview->setVisible(false);
        ui->head_focus->setChecked(false);
        ui->widget_focus->setVisible(false);

        if(ix.workMode != WorkMode_Capture)
        {
            ix.workMode = WorkMode_Capture;

            emit switchWorkMode(ix.workMode);//emit signal to change the layout
        }
    }
    else
    {
//#ifdef Q_OS_MAC
//        ui->head_capture->setFixedHeight(27);
//#endif
    }
}

void ManagementMenu::on_comboBox_readmode_capture_currentIndexChanged(int index)
{
    uint32_t ret = QHYCCD_ERROR;
    bool acceptDisEvent = true;

    //-------------------------结束拍摄任务---------------------------
    if(ix.cameraState != Camera_Idle)//如果正在拍摄，提示是否继续执行
    {
        QMessageBox::StandardButton choiceBtn;
        choiceBtn = QMessageBox::question(NULL,tr("Close EZCAP"),
                                   tr("Warning: A task is running,Are you sure wanted to exit?"),
                                   QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if(choiceBtn == QMessageBox::Yes)
            acceptDisEvent = true;
        else
            acceptDisEvent = false;

        ix.ForceStop = true;  //stop exposing.
    }

    if(acceptDisEvent)
    {
        ret = libqhyccd->CancelQHYCCDExposingAndReadout(camhandle);
        if(ret == QHYCCD_ERROR)
        {
            OutputDebug("EZCAPWARNING | %s | %s | CancelQHYCCDExposingAndReadout() Failed!", __FILE__, __FUNCTION__);
            return;
        }

        ix.cameraState = Camera_Idle;
    }
    else
    {
        return;
    }

    ix.ReadMode = index;

    if(SwitchReadmodeBinFormat())
    {
        OutputDebug("EZCAPERROR | %s | %s | Switch Readmode Bin Format failed", __FILE__, __FUNCTION__);
        QMessageBox::critical(this,tr("Error"),tr("Camera switch read/bin/format mode failed!"), QMessageBox::Ok);

        return;
    }

    if(ResetParameters())
    {
        OutputDebug("EZCAPERROR | %s | %s | Reset Parameters failed", __FILE__, __FUNCTION__);
        QMessageBox::critical(this,tr("Error"),tr("Camera switch reset parameters failed!"), QMessageBox::Ok);

        return;
    }
}

void ManagementMenu::on_comboBox_color_capture_currentIndexChanged(const QString &arg1)
{
    lastBayer = arg1;

    if(arg1 == "ON")
    {
        ix.IsCvtColor = true;
        ix.Bayer = ix.CamBayer;
    }
    else if(arg1 == "OFF")
    {
        ix.IsCvtColor = false;
        ix.Bayer = ix.CamBayer;
    }
    else if(arg1 == "Bayer GBBR")
    {
        ix.IsCvtColor = true;
        ix.Bayer = BAYER_GB;//1;
    }
    else if(arg1 == "Bayer GRBG")
    {
        ix.IsCvtColor = true;
        ix.Bayer = BAYER_GR;//2;
    }
    else if(arg1 == "Bayer BGGR")
    {
        ix.IsCvtColor = true;
        ix.Bayer = BAYER_BG;//3;
    }
    else if(arg1 == "Bayer RGGB")
    {
        ix.IsCvtColor = true;
        ix.Bayer = BAYER_RG;//4;
    }
}

void ManagementMenu::on_hSlider_Gain_capture_valueChanged(int value)
{
    ui->lineEdit_Gain_capture->blockSignals(true);
    ui->lineEdit_Gain_capture->setText(QString::number(value));
    ui->lineEdit_Gain_capture->blockSignals(false);

//    ix.Gain = value;
//    if(ix.Gain_Last != ix.Gain)
//    {
//        unsigned int ret = QHYCCD_ERROR;
//        ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_Gain, (double)ix.Gain);
//        if(ret == QHYCCD_SUCCESS)
//        {
//            ix.Gain_Last = ix.Gain;
//        }
//        else
//        {
//            OutputDebug("EZCAPWARNING | %s | %s | SetQHYCCDParam() CONTROL_Gain Failed!", __FILE__, __FUNCTION__);
//        }
//    }
}

void ManagementMenu::on_lineEdit_Gain_capture_textChanged(const QString &arg1)
{
    OutputDebug("EZCAPWARNING | %s | %s | arg1 = %s", __FILE__, __FUNCTION__, qPrintable(arg1));
    if(arg1 == "") return;

    if(ui->lineEdit_Gain_capture->text().toInt() > ui->hSlider_Gain_capture->maximum())
        ui->lineEdit_Gain_capture->setText(QString::number(ui->hSlider_Gain_capture->maximum()));
    if(ui->lineEdit_Gain_capture->text().toInt() < ui->hSlider_Gain_capture->minimum())
        ui->lineEdit_Gain_capture->setText(QString::number(ui->hSlider_Gain_capture->minimum()));

    ui->hSlider_Gain_capture->blockSignals(true);
    ui->hSlider_Gain_capture->setValue(ui->lineEdit_Gain_capture->text().toInt());
    ui->hSlider_Gain_capture->blockSignals(false);

//    ix.Gain = ui->lineEdit_Gain_capture->text().toDouble();
//    if(ix.Gain_Last != ix.Gain)
//    {
//        uint32_t ret = QHYCCD_ERROR;
//        ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_Gain, ix.Gain);
//        if(ret == QHYCCD_SUCCESS)
//        {
//            ix.Gain_Last = ix.Gain;
//        }
//        else
//        {
//            OutputDebug("EZCAPWARNING | %s | %s | SetQHYCCDParam() CONTROL_Gain Failed!", __FILE__, __FUNCTION__);
//        }
//    }
}

void ManagementMenu::on_hSlider_Offset_capture_valueChanged(int value)
{
    ui->lineEdit_Offset_capture->blockSignals(true);
    ui->lineEdit_Offset_capture->setText(QString::number(value));
    ui->lineEdit_Offset_capture->blockSignals(false);

//    ix.Offset = value;
//    if(ix.Offset_Last != ix.Offset)
//    {
//        uint32_t ret = QHYCCD_ERROR;
//        ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_Offset, (double)ix.Offset);
//        if(ret == QHYCCD_SUCCESS)
//        {
//            ix.Offset_Last = ix.Offset;
//        }
//        else
//        {
//            OutputDebug("EZCAPWARNING | %s | %s | SetQHYCCDParam() CONTROL_Offset Failed!", __FILE__, __FUNCTION__);
//        }
//    }
}

void ManagementMenu::on_lineEdit_Offset_capture_textChanged(const QString &arg1)
{
    if(arg1 == "") return;

    if(ui->lineEdit_Offset_capture->text().toInt() > ui->hSlider_Offset_capture->maximum())
        ui->lineEdit_Offset_capture->setText(QString::number(ui->hSlider_Offset_capture->maximum()));
    if(ui->lineEdit_Offset_capture->text().toInt() < ui->hSlider_Offset_capture->minimum())
        ui->lineEdit_Offset_capture->setText(QString::number(ui->hSlider_Offset_capture->minimum()));

    ui->hSlider_Offset_capture->blockSignals(true);
    ui->hSlider_Offset_capture->setValue(ui->lineEdit_Offset_capture->text().toInt());
    ui->hSlider_Offset_capture->blockSignals(false);

//    ix.Offset = ui->lineEdit_Offset_capture->text().toDouble();
//    if(ix.Offset_Last != ix.Offset)
//    {
//        uint32_t ret = QHYCCD_ERROR;
//        ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_Offset, ix.Offset);
//        if(ret == QHYCCD_SUCCESS)
//        {
//            ix.Offset_Last = ix.Offset;
//        }
//        else
//        {
//            OutputDebug("EZCAPWARNING | %s | %s | SetQHYCCDParam() CONTROL_Offset Failed!", __FILE__, __FUNCTION__);
//        }
//    }
}

void ManagementMenu::on_hSlider_exposure_capture_valueChanged(int value)
{
//    ix.ExpTime = (double)value;

    ui->lineEdit_exposure_capture->blockSignals(true);
    ui->lineEdit_exposure_capture->setText(QString::number(value));
    ui->lineEdit_exposure_capture->blockSignals(false);
}

void ManagementMenu::on_lineEdit_exposure_capture_textChanged(const QString &arg1)
{
    if(arg1 == "") return;

    bool ok = false;
    double exposure = arg1.toDouble(&ok);
    if(!ok) return;

    if(exposure > ui->hSlider_exposure_capture->maximum())
    {
        exposure = ui->hSlider_exposure_capture->maximum();
        ui->lineEdit_exposure_capture->setText(QString::number(ui->hSlider_exposure_capture->maximum()));
    }
    if(exposure < ui->hSlider_exposure_capture->minimum())
    {
        exposure = ui->hSlider_exposure_capture->minimum();
        ui->lineEdit_exposure_capture->setText(QString::number(ui->hSlider_exposure_capture->minimum()));
    }

    // The slider remains integer-based, but the line edit is the authoritative
    // value so a fractional exposure is not lost when Capture is pressed.
    ui->hSlider_exposure_capture->blockSignals(true);
    ui->hSlider_exposure_capture->setValue(qRound(exposure));
    ui->hSlider_exposure_capture->blockSignals(false);

    ix.ExpTime = exposure;
}

void ManagementMenu::on_comBoxSingleUnit_currentTextChanged()
{
    if(ui->comBoxSingleUnit->currentText() == "1~1000 us")
        ix.ExpUnit = 1.0;
    else if(ui->comBoxSingleUnit->currentText() == "1~1000 ms")
        ix.ExpUnit = 1000.0;
    else if(ui->comBoxSingleUnit->currentText() == "1~1200 s")
        ix.ExpUnit = 1000000.0;
    else if(ui->comBoxSingleUnit->currentText() == "20~60 min")
        ix.ExpUnit = 60 * 1000000.0;

}

void ManagementMenu::on_bin1x1_toggled(bool checked)
{
    if(checked)
    {
        uint32_t ret = QHYCCD_ERROR;

        ix.BinX = 1;
        ix.BinY = 1;

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

            ix.RoiX = 0;
            ix.RoiY = 0;
            ix.RoiW = ix.ImageW_Min / ix.BinX;
            ix.RoiH = ix.ImageH_Min / ix.BinY;
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

        cv::Mat ImgRoiTemp = cv::Mat::zeros(cv::Size(ui->img_Roi->width(), ui->img_Roi->height()), CV_8UC3);
        QImage *RoiQImg = mainWidget->MatToQImage(ImgRoiTemp);
        ui->img_Roi->setPixmap(QPixmap::fromImage(*RoiQImg));
        ImgRoiTemp.release();

        ui->comboBox_RoiSaved->blockSignals(true);
        ui->comboBox_RoiSaved->clear();
        ui->comboBox_RoiSaved->addItem(QString::number(ix.RoiW) + " x " + QString::number(ix.RoiH));
        ui->comboBox_RoiSaved->addItem(QString::number(ix.RoiW * 8 / 10 / 2 * 2) + " x " + QString::number(ix.RoiH * 8 / 10 / 2 * 2));
        ui->comboBox_RoiSaved->addItem(QString::number(ix.RoiW / 2 / 2 * 2) + " x " + QString::number(ix.RoiH / 2 / 2 * 2));
        ui->comboBox_RoiSaved->addItem(QString::number(ix.RoiW * 3 / 10 / 2 * 2) + " x " + QString::number(ix.RoiH * 3 / 10 / 2 * 2));
        ui->comboBox_RoiSaved->addItem(QString::number(ix.RoiW / 10 / 2 * 2) + " x " + QString::number(ix.RoiH / 10 / 2 * 2));
        QString CustomedROI = "";
        mainWidget->loadParamFromIni(ix.CamModel + "-" + QString::number(ix.camStreamMode), "CustomedROI_Bin1x1", &CustomedROI, "");
        if(!CustomedROI.isEmpty())
        {
            QStringList CustomedROIList = CustomedROI.split(";");
            for(int i = 0; i < CustomedROIList.count(); i++)
            {
                if(!CustomedROIList.at(i).isEmpty())
                    ui->comboBox_RoiSaved->addItem(CustomedROIList.at(i));
            }
        }
        ui->comboBox_RoiSaved->addItem("Customed");
        ui->comboBox_RoiSaved->blockSignals(false);
    }
}

void ManagementMenu::on_bin2x2_toggled(bool checked)
{
    if(checked)
    {
        uint32_t ret = QHYCCD_ERROR;

        ix.BinX = 2;
        ix.BinY = 2;

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

            ix.RoiX = 0;
            ix.RoiY = 0;
            ix.RoiW = ix.ImageW_Min / ix.BinX;
            ix.RoiH = ix.ImageH_Min / ix.BinY;
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

        cv::Mat ImgRoiTemp = cv::Mat::zeros(cv::Size(ui->img_Roi->width(), ui->img_Roi->height()), CV_8UC3);
        QImage *RoiQImg = mainWidget->MatToQImage(ImgRoiTemp);
        ui->img_Roi->setPixmap(QPixmap::fromImage(*RoiQImg));
        ImgRoiTemp.release();

        ui->comboBox_RoiSaved->blockSignals(true);
        ui->comboBox_RoiSaved->clear();
        ui->comboBox_RoiSaved->addItem(QString::number(ix.RoiW) + " x " + QString::number(ix.RoiH));
        ui->comboBox_RoiSaved->addItem(QString::number(ix.RoiW * 8 / 10 / 2 * 2) + " x " + QString::number(ix.RoiH * 8 / 10 / 2 * 2));
        ui->comboBox_RoiSaved->addItem(QString::number(ix.RoiW / 2 / 2 * 2) + " x " + QString::number(ix.RoiH / 2 / 2 * 2));
        ui->comboBox_RoiSaved->addItem(QString::number(ix.RoiW * 3 / 10 / 2 * 2) + " x " + QString::number(ix.RoiH * 3 / 10 / 2 * 2));
        ui->comboBox_RoiSaved->addItem(QString::number(ix.RoiW / 10 / 2 * 2) + " x " + QString::number(ix.RoiH / 10 / 2 * 2));
        QString CustomedROI = "";
        mainWidget->loadParamFromIni(ix.CamModel + "-" + QString::number(ix.camStreamMode), "CustomedROI_Bin2x2", &CustomedROI, "");
        if(!CustomedROI.isEmpty())
        {
            QStringList CustomedROIList = CustomedROI.split(";");
            for(int i = 0; i < CustomedROIList.count(); i++)
            {
                if(!CustomedROIList.at(i).isEmpty())
                    ui->comboBox_RoiSaved->addItem(CustomedROIList.at(i));
            }
        }
        ui->comboBox_RoiSaved->addItem("Customed");
        ui->comboBox_RoiSaved->blockSignals(false);
    }
}

void ManagementMenu::on_bin3x3_toggled(bool checked)
{
    if(checked)
    {
        uint32_t ret = QHYCCD_ERROR;

        ix.BinX = 3;
        ix.BinY = 3;

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

            ix.RoiX = 0;
            ix.RoiY = 0;
            ix.RoiW = ix.ImageW_Min / ix.BinX;
            ix.RoiH = ix.ImageH_Min / ix.BinY;
            if(ix.CamID.contains("QHY992_"))
            {
                ix.RoiW = (ix.ImageW_Min + 5) / 6 * 6 / ix.BinX;
                ix.RoiH = (ix.ImageH_Min + 5) / 6 * 6 / ix.BinY;
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

        cv::Mat ImgRoiTemp = cv::Mat::zeros(cv::Size(ui->img_Roi->width(), ui->img_Roi->height()), CV_8UC3);
        QImage *RoiQImg = mainWidget->MatToQImage(ImgRoiTemp);
        ui->img_Roi->setPixmap(QPixmap::fromImage(*RoiQImg));
        ImgRoiTemp.release();

        ui->comboBox_RoiSaved->blockSignals(true);
        ui->comboBox_RoiSaved->clear();
        ui->comboBox_RoiSaved->addItem(QString::number(ix.RoiW) + " x " + QString::number(ix.RoiH));
        ui->comboBox_RoiSaved->addItem(QString::number(ix.RoiW * 8 / 10 / 2 * 2) + " x " + QString::number(ix.RoiH * 8 / 10 / 2 * 2));
        ui->comboBox_RoiSaved->addItem(QString::number(ix.RoiW / 2 / 2 * 2) + " x " + QString::number(ix.RoiH / 2 / 2 * 2));
        ui->comboBox_RoiSaved->addItem(QString::number(ix.RoiW * 3 / 10 / 2 * 2) + " x " + QString::number(ix.RoiH * 3 / 10 / 2 * 2));
        ui->comboBox_RoiSaved->addItem(QString::number(ix.RoiW / 10 / 2 * 2) + " x " + QString::number(ix.RoiH / 10 / 2 * 2));
        QString CustomedROI = "";
        mainWidget->loadParamFromIni(ix.CamModel + "-" + QString::number(ix.camStreamMode), "CustomedROI_Bin3x3", &CustomedROI, "");
        if(!CustomedROI.isEmpty())
        {
            QStringList CustomedROIList = CustomedROI.split(";");
            for(int i = 0; i < CustomedROIList.count(); i++)
            {
                if(!CustomedROIList.at(i).isEmpty())
                    ui->comboBox_RoiSaved->addItem(CustomedROIList.at(i));
            }
        }
        ui->comboBox_RoiSaved->addItem("Customed");
        ui->comboBox_RoiSaved->blockSignals(false);
    }
}

void ManagementMenu::on_bin4x4_toggled(bool checked)
{
    if(checked)
    {
        uint32_t ret = QHYCCD_ERROR;

        ix.BinX = 4;
        ix.BinY = 4;

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

            ix.RoiX = 0;
            ix.RoiY = 0;
            ix.RoiW = ix.ImageW_Min / ix.BinX;
            ix.RoiH = ix.ImageH_Min / ix.BinY;
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

        cv::Mat ImgRoiTemp = cv::Mat::zeros(cv::Size(ui->img_Roi->width(), ui->img_Roi->height()), CV_8UC3);
        QImage *RoiQImg = mainWidget->MatToQImage(ImgRoiTemp);
        ui->img_Roi->setPixmap(QPixmap::fromImage(*RoiQImg));
        ImgRoiTemp.release();

        ui->comboBox_RoiSaved->blockSignals(true);
        ui->comboBox_RoiSaved->clear();
        ui->comboBox_RoiSaved->addItem(QString::number(ix.RoiW) + " x " + QString::number(ix.RoiH));
        ui->comboBox_RoiSaved->addItem(QString::number(ix.RoiW * 8 / 10 / 2 * 2) + " x " + QString::number(ix.RoiH * 8 / 10 / 2 * 2));
        ui->comboBox_RoiSaved->addItem(QString::number(ix.RoiW / 2 / 2 * 2) + " x " + QString::number(ix.RoiH / 2 / 2 * 2));
        ui->comboBox_RoiSaved->addItem(QString::number(ix.RoiW * 3 / 10 / 2 * 2) + " x " + QString::number(ix.RoiH * 3 / 10 / 2 * 2));
        ui->comboBox_RoiSaved->addItem(QString::number(ix.RoiW / 10 / 2 * 2) + " x " + QString::number(ix.RoiH / 10 / 2 * 2));
        QString CustomedROI = "";
        mainWidget->loadParamFromIni(ix.CamModel + "-" + QString::number(ix.camStreamMode), "CustomedROI_Bin4x4", &CustomedROI, "");
        if(!CustomedROI.isEmpty())
        {
            QStringList CustomedROIList = CustomedROI.split(";");
            for(int i = 0; i < CustomedROIList.count(); i++)
            {
                if(!CustomedROIList.at(i).isEmpty())
                    ui->comboBox_RoiSaved->addItem(CustomedROIList.at(i));
            }
        }
        ui->comboBox_RoiSaved->addItem("Customed");
        ui->comboBox_RoiSaved->blockSignals(false);
    }
}

void ManagementMenu::on_bin6x6_toggled(bool checked)
{
    if(checked)
    {
        uint32_t ret = QHYCCD_ERROR;

        ix.BinX = 6;
        ix.BinY = 6;

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

            ix.RoiX = 0;
            ix.RoiY = 0;
            ix.RoiW = ix.ImageW_Min / ix.BinX;
            ix.RoiH = ix.ImageH_Min / ix.BinY;
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

        cv::Mat ImgRoiTemp = cv::Mat::zeros(cv::Size(ui->img_Roi->width(), ui->img_Roi->height()), CV_8UC3);
        QImage *RoiQImg = mainWidget->MatToQImage(ImgRoiTemp);
        ui->img_Roi->setPixmap(QPixmap::fromImage(*RoiQImg));
        ImgRoiTemp.release();

        ui->comboBox_RoiSaved->blockSignals(true);
        ui->comboBox_RoiSaved->clear();
        ui->comboBox_RoiSaved->addItem(QString::number(ix.RoiW) + " x " + QString::number(ix.RoiH));
        ui->comboBox_RoiSaved->addItem(QString::number(ix.RoiW * 8 / 10 / 2 * 2) + " x " + QString::number(ix.RoiH * 8 / 10 / 2 * 2));
        ui->comboBox_RoiSaved->addItem(QString::number(ix.RoiW / 2 / 2 * 2) + " x " + QString::number(ix.RoiH / 2 / 2 * 2));
        ui->comboBox_RoiSaved->addItem(QString::number(ix.RoiW * 3 / 10 / 2 * 2) + " x " + QString::number(ix.RoiH * 3 / 10 / 2 * 2));
        ui->comboBox_RoiSaved->addItem(QString::number(ix.RoiW / 10 / 2 * 2) + " x " + QString::number(ix.RoiH / 10 / 2 * 2));
        QString CustomedROI = "";
        mainWidget->loadParamFromIni(ix.CamModel + "-" + QString::number(ix.camStreamMode), "CustomedROI_Bin6x6", &CustomedROI, "");
        if(!CustomedROI.isEmpty())
        {
            QStringList CustomedROIList = CustomedROI.split(";");
            for(int i = 0; i < CustomedROIList.count(); i++)
            {
                if(!CustomedROIList.at(i).isEmpty())
                    ui->comboBox_RoiSaved->addItem(CustomedROIList.at(i));
            }
        }
        ui->comboBox_RoiSaved->addItem("Customed");
        ui->comboBox_RoiSaved->blockSignals(false);
    }
}

void ManagementMenu::on_bin8x8_toggled(bool checked)
{
    if(checked)
    {
        uint32_t ret = QHYCCD_ERROR;

        ix.BinX = 8;
        ix.BinY = 8;

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

            ix.RoiX = 0;
            ix.RoiY = 0;
            ix.RoiW = ix.ImageW_Min / ix.BinX;
            ix.RoiH = ix.ImageH_Min / ix.BinY;
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

        cv::Mat ImgRoiTemp = cv::Mat::zeros(cv::Size(ui->img_Roi->width(), ui->img_Roi->height()), CV_8UC3);
        QImage *RoiQImg = mainWidget->MatToQImage(ImgRoiTemp);
        ui->img_Roi->setPixmap(QPixmap::fromImage(*RoiQImg));
        ImgRoiTemp.release();

        ui->comboBox_RoiSaved->blockSignals(true);
        ui->comboBox_RoiSaved->clear();
        ui->comboBox_RoiSaved->addItem(QString::number(ix.RoiW) + " x " + QString::number(ix.RoiH));
        ui->comboBox_RoiSaved->addItem(QString::number(ix.RoiW * 8 / 10 / 2 * 2) + " x " + QString::number(ix.RoiH * 8 / 10 / 2 * 2));
        ui->comboBox_RoiSaved->addItem(QString::number(ix.RoiW / 2 / 2 * 2) + " x " + QString::number(ix.RoiH / 2 / 2 * 2));
        ui->comboBox_RoiSaved->addItem(QString::number(ix.RoiW * 3 / 10 / 2 * 2) + " x " + QString::number(ix.RoiH * 3 / 10 / 2 * 2));
        ui->comboBox_RoiSaved->addItem(QString::number(ix.RoiW / 10 / 2 * 2) + " x " + QString::number(ix.RoiH / 10 / 2 * 2));
        QString CustomedROI = "";
        mainWidget->loadParamFromIni(ix.CamModel + "-" + QString::number(ix.camStreamMode), "CustomedROI_Bin8x8", &CustomedROI, "");
        if(!CustomedROI.isEmpty())
        {
            QStringList CustomedROIList = CustomedROI.split(";");
            for(int i = 0; i < CustomedROIList.count(); i++)
            {
                if(!CustomedROIList.at(i).isEmpty())
                    ui->comboBox_RoiSaved->addItem(CustomedROIList.at(i));
            }
        }
        ui->comboBox_RoiSaved->addItem("Customed");
        ui->comboBox_RoiSaved->blockSignals(false);
    }
}

void ManagementMenu::on_checkBox_highSpeed_toggled(bool checked)
{
//    if(checked)
//    {
//        ix.Speed = 1;
//    }
//    else
//    {
//        ix.Speed = 0;
//    }
}

void ManagementMenu::setCaptureExp1s()
{
    ui->comBoxSingleUnit->setCurrentText("1~1200 s");
    ui->hSlider_exposure_capture->setValue(1);
////    ui->label_exposure_capture->setText("1s");
//    ix.ExpTime = 1.0;//1000;
//    ix.ExpUnit = 1000000.0;
}

void ManagementMenu::setCaptureExp5s()
{
    ui->comBoxSingleUnit->setCurrentText("1~1200 s");
    ui->hSlider_exposure_capture->setValue(5);
////    ui->label_exposure_capture->setText("5s");
//    ix.ExpTime = 5.0;//5000;
//    ix.ExpUnit = 1000000.0;
}

void ManagementMenu::setCaptureExp10s()
{
    ui->comBoxSingleUnit->setCurrentText("1~1200 s");
    ui->hSlider_exposure_capture->setValue(10);
////    ui->label_exposure_capture->setText("10s");
//    ix.ExpTime = 10.0;//10000;
//    ix.ExpUnit = 1000000.0;
}

void ManagementMenu::setCaptureExp30s()
{
    ui->comBoxSingleUnit->setCurrentText("1~1200 s");
    ui->hSlider_exposure_capture->setValue(30);
////    ui->label_exposure_capture->setText("30s");
//    ix.ExpTime = 30.0;//30000;
//    ix.ExpUnit = 1000000.0;
}

void ManagementMenu::setCaptureExp60s()
{
    ui->comBoxSingleUnit->setCurrentText("1~1200 s");
    ui->hSlider_exposure_capture->setValue(60);
////    ui->label_exposure_capture->setText("60s");
//    ix.ExpTime = 60.0;//60000;
//    ix.ExpUnit = 1000000.0;
}

void ManagementMenu::setCaptureExp120s()
{
    ui->comBoxSingleUnit->setCurrentText("1~1200 s");
    ui->hSlider_exposure_capture->setValue(120);
////    ui->label_exposure_capture->setText("120s");
//    ix.ExpTime = 120.0;//120000;
//    ix.ExpUnit = 1000000.0;
}

void ManagementMenu::setCaptureExp180s()
{
    ui->comBoxSingleUnit->setCurrentText("1~1200 s");
    ui->hSlider_exposure_capture->setValue(180);
////    ui->label_exposure_capture->setText("180s");
//    ix.ExpTime = 180.0;//180000;
//    ix.ExpUnit = 1000000.0;
}

void ManagementMenu::setCaptureExp240s()
{
    ui->comBoxSingleUnit->setCurrentText("1~1200 s");
    ui->hSlider_exposure_capture->setValue(240);
////    ui->label_exposure_capture->setText("240s");
//    ix.ExpTime = 240.0;//240000;
//    ix.ExpUnit = 1000000.0;
}

void ManagementMenu::setCaptureExp5min()
{
    ui->comBoxSingleUnit->setCurrentText("1~1200 s");
    ui->hSlider_exposure_capture->setValue(300);
////    ui->label_exposure_capture->setText("300s");
//    ix.ExpTime = 300.0;//300000;
//    ix.ExpUnit = 1000000.0;
}

void ManagementMenu::setCaptureExp10min()
{
    ui->comBoxSingleUnit->setCurrentText("1~1200 s");
    ui->hSlider_exposure_capture->setValue(600);
////    ui->label_exposure_capture->setText("600s");
//    ix.ExpTime = 600.0;//600000;
//    ix.ExpUnit = 1000000.0;
}

void ManagementMenu::setCaptureExp15min()
{
    ui->comBoxSingleUnit->setCurrentText("1~1200 s");
    ui->hSlider_exposure_capture->setValue(900);
////    ui->label_exposure_capture->setText("900s");
//    ix.ExpTime = 900.0;//900000;
//    ix.ExpUnit = 1000000.0;
}

void ManagementMenu::setCaptureExp30min()
{
    ui->comBoxSingleUnit->setCurrentText("20~60 min");
    ui->hSlider_exposure_capture->setValue(30);
////    ui->label_exposure_capture->setText("1800s");
//    ix.ExpTime = 30.0;//1800000;
//    ix.ExpUnit = 60 * 1000000.0;
}

void ManagementMenu::setCaptureExp0s()
{
    ui->comBoxSingleUnit->setCurrentText("1~1200 s");
    ui->hSlider_exposure_capture->setValue(0);
////    ui->label_exposure_capture->setText("0s");
//    ix.ExpTime = 0;
//    ix.ExpUnit = 1000000.0;
}

void ManagementMenu::on_hSlider_exposure_capture_customContextMenuRequested(const QPoint &pos)
{
    if(cmenu_captureExp)//保证同时只存在一个menu，及时释放内存
    {
        delete cmenu_captureExp;
        cmenu_captureExp = NULL;
    }
    cmenu_captureExp = new QMenu(ui->hSlider_exposure_capture);

    QAction *exp1s = cmenu_captureExp->addAction("1s");
    QAction *exp5s = cmenu_captureExp->addAction("5s");
    QAction *exp10s = cmenu_captureExp->addAction("10s");
    QAction *exp30s = cmenu_captureExp->addAction("30s");
    QAction *exp60s = cmenu_captureExp->addAction("60s");
    QAction *exp120s = cmenu_captureExp->addAction("120s");
    QAction *exp180s = cmenu_captureExp->addAction("180s");
    QAction *exp240s = cmenu_captureExp->addAction("240s");
    QAction *exp5min = cmenu_captureExp->addAction("5min");
    QAction *exp10min = cmenu_captureExp->addAction("10min");
    QAction *exp15min = cmenu_captureExp->addAction("15min");
    QAction *exp30min = cmenu_captureExp->addAction("30min");
    QAction *exp0s = cmenu_captureExp->addAction("0s");

    connect(exp1s, SIGNAL(triggered(bool)), this, SLOT(setCaptureExp1s()));
    connect(exp5s, SIGNAL(triggered(bool)), this, SLOT(setCaptureExp5s()));
    connect(exp10s, SIGNAL(triggered(bool)), this, SLOT(setCaptureExp10s()));
    connect(exp30s, SIGNAL(triggered(bool)), this, SLOT(setCaptureExp30s()));
    connect(exp60s, SIGNAL(triggered(bool)), this, SLOT(setCaptureExp60s()));
    connect(exp120s, SIGNAL(triggered(bool)), this, SLOT(setCaptureExp120s()));
    connect(exp180s, SIGNAL(triggered(bool)), this, SLOT(setCaptureExp180s()));
    connect(exp240s, SIGNAL(triggered(bool)), this, SLOT(setCaptureExp240s()));
    connect(exp5min, SIGNAL(triggered(bool)), this, SLOT(setCaptureExp5min()));
    connect(exp10min, SIGNAL(triggered(bool)), this, SLOT(setCaptureExp10min()));
    connect(exp15min, SIGNAL(triggered(bool)), this, SLOT(setCaptureExp15min()));
    connect(exp30min, SIGNAL(triggered(bool)), this, SLOT(setCaptureExp30min()));
    connect(exp0s, SIGNAL(triggered(bool)), this, SLOT(setCaptureExp0s()));

    cmenu_captureExp->exec(QCursor::pos());//在当前鼠标位置显示

}

//-----------------------------save---------------------------------------
void ManagementMenu::on_head_save_clicked(bool checked)
{
    ui->head_save->setChecked(checked);
    ui->widget_save->setVisible(checked);
    if(checked)
    {
//#ifdef Q_OS_MAC
//        ui->head_save->setFixedHeight(18);
//#endif
    }
    else
    {
//#ifdef Q_OS_MAC
//        ui->head_save->setFixedHeight(27);
//#endif
    }
}


//---------------------------image format-----------------------------------
void ManagementMenu::on_head_liveimageformat_clicked(bool checked)
{
    ui->head_liveimageformat->setChecked(checked);
    ui->widget_liveimageformat->setVisible(checked);
}

void ManagementMenu::on_comBoxLiveReadMode_currentIndexChanged(int index)
{
    uint32_t ret = QHYCCD_ERROR;

    ///
    /// 结束线程
    ///

    if(mainWidget->threadProcessImage->isRunning())
        mainWidget->threadProcessImage->stop();

    if(mainWidget->liveCap->isRunning())
    {
        mainWidget->liveCap->closeThread();
//        frameQueue.clear();
//        histQueue.clear();

//        mainWidget->liveCap = new LiveCapThread();
//        connect(mainWidget->liveCap, SIGNAL(gotFPSData()), mainWidget, SLOT(showFPS()));
//        connect(mainWidget->liveCap, SIGNAL(gotGPSData()), gpsTool_dialog, SLOT(updateGPSInfo()));
//        connect(mainWidget->liveCap, SIGNAL(gotCalData()), mainWidget, SLOT(saveCal2Image()));
////        connect(mainWidget->liveCap, SIGNAL(gotDarkData()), mainWidget, SLOT(saveDark2Image()));
//        connect(mainWidget->liveCap, SIGNAL(finished()), mainWidget->liveCap, SLOT(deleteLater()));
    }

    ret = libqhyccd->StopQHYCCDLive(camhandle);
    if(ret == QHYCCD_ERROR)
    {
        OutputDebug("EZCAPWARNING | %s | %s | StopQHYCCDLive() Failed!", __FILE__, __FUNCTION__);
        return;
    }

    ix.ReadMode = index;

    if(SwitchReadmodeBinFormat())
    {
        OutputDebug("EZCAPERROR | %s | %s | Switch Readmode Bin Format failed", __FILE__, __FUNCTION__);
        QMessageBox::critical(this,tr("Error"),tr("Camera switch read/bin/format mode failed!"), QMessageBox::Ok);

        ix.ReadMode = ix.ReadMode_Last;
        CloseCamera();
        emit mainWidget->disconnect_camera();
        return;
    }

    if(ResetParameters())
    {
        OutputDebug("EZCAPERROR | %s | %s | Reset Parameters failed", __FILE__, __FUNCTION__);
        QMessageBox::critical(this,tr("Error"),tr("Camera switch reset parameters failed!"), QMessageBox::Ok);

        ix.ReadMode = ix.ReadMode_Last;
        CloseCamera();
        emit mainWidget->disconnect_camera();
        return;
    }

    ix.ReadMode_Last = index;

    ///
    /// 重新开启线程
    ///
    ret = libqhyccd->BeginQHYCCDLive(camhandle);
    if(ret == QHYCCD_ERROR)
    {
        OutputDebug("EZCAPWARNING | %s | %s | BeginQHYCCDLive() Failed!", __FILE__, __FUNCTION__);
    }
    mainWidget->liveCap->start();
    mainWidget->threadProcessImage->start();
}

void ManagementMenu::on_comBoxLiveBin_currentTextChanged(const QString &arg1)
{
    uint32_t ret = QHYCCD_ERROR;

    ///
    /// 结束线程
    ///

    if(mainWidget->threadProcessImage->isRunning())
        mainWidget->threadProcessImage->stop();

    if(mainWidget->liveCap->isRunning())
    {
        mainWidget->liveCap->closeThread();
//        frameQueue.clear();
//        histQueue.clear();

//        mainWidget->liveCap = new LiveCapThread();
//        connect(mainWidget->liveCap, SIGNAL(gotFPSData()), mainWidget, SLOT(showFPS()));
//        connect(mainWidget->liveCap, SIGNAL(gotGPSData()), gpsTool_dialog, SLOT(updateGPSInfo()));
//        connect(mainWidget->liveCap, SIGNAL(gotCalData()), mainWidget, SLOT(saveCal2Image()));
////        connect(mainWidget->liveCap, SIGNAL(gotDarkData()), mainWidget, SLOT(saveDark2Image()));
//        connect(mainWidget->liveCap, SIGNAL(finished()), mainWidget->liveCap, SLOT(deleteLater()));
    }

    ret = libqhyccd->StopQHYCCDLive(camhandle);
    if(ret == QHYCCD_ERROR)
    {
        OutputDebug("EZCAPWARNING | %s | %s | StopQHYCCDLive() Failed!", __FILE__, __FUNCTION__);
        return;
    }

    if(arg1 == "1X1")
    {
        ix.BinX = 1;
        ix.BinY = 1;
    }
    else if(arg1 == "2X2")
    {
        ix.BinX = 2;
        ix.BinY = 2;
    }
    else if(arg1 == "3X3")
    {
        ix.BinX = 3;
        ix.BinY = 3;
    }
    else if(arg1 == "4X4")
    {
        ix.BinX = 4;
        ix.BinY = 4;
    }
    else if(arg1 == "6X6")
    {
        ix.BinX = 6;
        ix.BinY = 6;
    }
    else if(arg1 == "8X8")
    {
        ix.BinX = 8;
        ix.BinY = 8;
    }

    if(ix.Color_Fun)
    {
        if(ix.BinX != 1 && ix.BinY != 1)
        {
            ui->comBoxLiveColor->blockSignals(true);
            ui->comBoxLiveColor->setCurrentText("OFF");
            ui->labelLiveColor->setVisible(false);
            ui->comBoxLiveColor->setVisible(false);
            ui->comBoxLiveColor->blockSignals(false);
            ix.IsCvtColor = false;
        }
        else
        {
            ui->comBoxLiveColor->blockSignals(true);
            ui->comBoxLiveColor->setCurrentText("OFF");
            ui->labelLiveColor->setVisible(true);
            ui->comBoxLiveColor->setVisible(true);
            ui->comBoxLiveColor->blockSignals(false);
            ix.IsCvtColor = false;
        }
    }

    if(SwitchReadmodeBinFormat())
    {
        OutputDebug("EZCAPERROR | %s | %s | Switch Readmode Bin Format failed", __FILE__, __FUNCTION__);
        QMessageBox::critical(this,tr("Error"),tr("Camera switch read/bin/format mode failed!"), QMessageBox::Ok);

        ix.BinX = ix.BinX_Last;
        ix.BinY = ix.BinY_Last;
        CloseCamera();
        emit mainWidget->disconnect_camera();
        return;
    }

    if(ResetParameters())
    {
        OutputDebug("EZCAPERROR | %s | %s | Reset Parameters failed", __FILE__, __FUNCTION__);
        QMessageBox::critical(this,tr("Error"),tr("Camera reset parameters failed!"), QMessageBox::Ok);

        ix.BinX = ix.BinX_Last;
        ix.BinY = ix.BinY_Last;
        CloseCamera();
        emit mainWidget->disconnect_camera();
        return;
    }

    cv::Mat ImgRoiTemp = cv::Mat::zeros(cv::Size(ui->img_Roi->width(), ui->img_Roi->height()), CV_8UC3);
    QImage *RoiQImg = mainWidget->MatToQImage(ImgRoiTemp);
    ui->img_Roi->setPixmap(QPixmap::fromImage(*RoiQImg));
    ImgRoiTemp.release();

    ui->comboBox_RoiSaved->blockSignals(true);
    ui->comboBox_RoiSaved->clear();
    ui->comboBox_RoiSaved->addItem(QString::number(ix.RoiW) + " x " + QString::number(ix.RoiH));
    ui->comboBox_RoiSaved->addItem(QString::number(ix.RoiW * 8 / 10 / 2 * 2) + " x " + QString::number(ix.RoiH * 8 / 10 / 2 * 2));
    ui->comboBox_RoiSaved->addItem(QString::number(ix.RoiW / 2 / 2 * 2) + " x " + QString::number(ix.RoiH / 2 / 2 * 2));
    ui->comboBox_RoiSaved->addItem(QString::number(ix.RoiW * 3 / 10 / 2 * 2) + " x " + QString::number(ix.RoiH * 3 / 10 / 2 * 2));
    ui->comboBox_RoiSaved->addItem(QString::number(ix.RoiW / 10 / 2 * 2) + " x " + QString::number(ix.RoiH / 10 / 2 * 2));
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
                ui->comboBox_RoiSaved->addItem(CustomedROIList.at(i));
        }
    }
    ui->comboBox_RoiSaved->addItem("Customed");
    ui->comboBox_RoiSaved->blockSignals(false);

    ix.BinX_Last = ix.BinX;
    ix.BinY_Last = ix.BinY;

    ///
    /// 重新开启线程
    ///
    ret = libqhyccd->BeginQHYCCDLive(camhandle);
    if(ret == QHYCCD_ERROR)
    {
        OutputDebug("EZCAPWARNING | %s | %s | BeginQHYCCDLive() Failed!", __FILE__, __FUNCTION__);
    }

    mainWidget->liveCap->start();
    mainWidget->threadProcessImage->start();
}

void ManagementMenu::on_comBoxLiveBits_currentTextChanged(const QString &arg1)
{
    /* Bit-depth switch must NOT Close/Release/Open the camera.
       SwitchReadmodeBinFormat() tears down SDK resources and can leave
       camhandle invalid or PCIE live state inconsistent, which crashes on
       8->16 (large pack). Follow LiveFrameSample:
       StopLive -> SetBits -> BeginLive.
       Still fold/restore management panels like other setting switches. */
    uint32_t ret = QHYCCD_ERROR;
    const uint32_t oldBits = ix.Bits;
    const bool oldColor = ix.Color;

    CollapseSettingPanels();
    ui->comBoxLiveBits->blockSignals(true);

    if(mainWidget->threadProcessImage->isRunning())
        mainWidget->threadProcessImage->stop();

    if(mainWidget->liveCap->isRunning())
        mainWidget->liveCap->closeThread();

    ix.locked = false;

    if(camhandle != NULL)
    {
        ret = libqhyccd->StopQHYCCDLive(camhandle);
        if(ret == QHYCCD_ERROR)
        {
            OutputDebug("EZCAPWARNING | %s | %s | StopQHYCCDLive() Failed!", __FILE__, __FUNCTION__);
        }
    }

    if(arg1 == "RAW8" || arg1 == "MONO8")
    {
        if(ix.Color_Fun)
        {
            ui->comBoxLiveColor->setVisible(true);
            ui->labelLiveColor->setVisible(true);
        }
        ix.Bits = 8;
        ix.Color = false;
    }
    else if(arg1 == "RAW16" || arg1 == "MONO16")
    {
        if(ix.Color_Fun)
        {
            ui->comBoxLiveColor->setVisible(true);
            ui->labelLiveColor->setVisible(true);
        }
        ix.Bits = 16;
        ix.Color = false;
    }
    else if(arg1 == "RGB24")
    {
        ui->comBoxLiveColor->setVisible(false);
        ui->labelLiveColor->setVisible(false);
        ix.Bits = 8;
        ix.Color = true;
    }
    else
    {
        OutputDebug("EZCAPWARNING | %s | %s | Unknown live bits text: %s", __FILE__, __FUNCTION__, arg1.toLocal8Bit().data());
        ui->comBoxLiveBits->blockSignals(false);
        RestoreSettingPanels();
        return;
    }

    if(camhandle == NULL)
    {
        OutputDebug("EZCAPERROR | %s | %s | camhandle is NULL", __FILE__, __FUNCTION__);
        ix.Bits = oldBits;
        ix.Color = oldColor;
        ui->comBoxLiveBits->blockSignals(false);
        RestoreSettingPanels();
        return;
    }

    ret = libqhyccd->SetQHYCCDDebayerOnOff(camhandle, ix.Color);
    if(ret != QHYCCD_SUCCESS)
    {
        OutputDebug("EZCAPWARNING | %s | %s | SetQHYCCDDebayerOnOff() Failed! ret=%d", __FILE__, __FUNCTION__, ret);
    }

    ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_TRANSFERBIT, ix.Bits);
    if(ret != QHYCCD_SUCCESS)
    {
        OutputDebug("EZCAPERROR | %s | %s | SetQHYCCDParam() CONTROL_TRANSFERBIT Failed!", __FILE__, __FUNCTION__);
        QMessageBox::critical(this,tr("Error"),tr("Camera switch bit depth failed!"), QMessageBox::Ok);
        ix.Bits = oldBits;
        ix.Color = oldColor;
        ix.Bits_Last = oldBits;
        ix.Color_Last = oldColor;
        libqhyccd->SetQHYCCDDebayerOnOff(camhandle, ix.Color);
        libqhyccd->SetQHYCCDParam(camhandle, CONTROL_TRANSFERBIT, ix.Bits);
        libqhyccd->BeginQHYCCDLive(camhandle);
        mainWidget->liveCap->start();
        mainWidget->threadProcessImage->start();
        ui->comBoxLiveBits->blockSignals(false);
        RestoreSettingPanels();
        return;
    }

    ret = libqhyccd->SetQHYCCDResolution(camhandle, ix.RoiX, ix.RoiY, ix.RoiW, ix.RoiH);
    if(ret != QHYCCD_SUCCESS)
    {
        OutputDebug("EZCAPWARNING | %s | %s | SetQHYCCDResolution() after bit switch Failed! ret=%d", __FILE__, __FUNCTION__, ret);
    }

    if(ResetParameters())
    {
        OutputDebug("EZCAPERROR | %s | %s | Reset Parameters failed", __FILE__, __FUNCTION__);
        QMessageBox::critical(this,tr("Error"),tr("Camera switch reset parameters failed!"), QMessageBox::Ok);
        CloseCamera();
        emit mainWidget->disconnect_camera();
        ui->comBoxLiveBits->blockSignals(false);
        RestoreSettingPanels();
        return;
    }

    ix.Bits_Last = ix.Bits;
    ix.Color_Last = ix.Color;

    ret = libqhyccd->BeginQHYCCDLive(camhandle);
    if(ret == QHYCCD_ERROR)
    {
        OutputDebug("EZCAPWARNING | %s | %s | BeginQHYCCDLive() Failed!", __FILE__, __FUNCTION__);
        ui->comBoxLiveBits->blockSignals(false);
        RestoreSettingPanels();
        return;
    }

    mainWidget->liveCap->start();
    mainWidget->threadProcessImage->start();
    ui->comBoxLiveBits->blockSignals(false);
    RestoreSettingPanels();
}

void ManagementMenu::on_comBoxLiveColor_currentTextChanged(const QString &arg1)
{
    if(arg1 == "ON")
    {
        ix.IsCvtColor = true;
        ix.Bayer = ix.CamBayer;
    }
    else if(arg1 == "OFF")
    {
        ix.IsCvtColor = false;
        ix.Bayer = ix.CamBayer;
    }
    else if(arg1 == "Bayer GBBR")
    {
        ix.IsCvtColor = true;
        ix.Bayer = BAYER_GB;//1;
    }
    else if(arg1 == "Bayer GRBG")
    {
        ix.IsCvtColor = true;
        ix.Bayer = BAYER_GR;//2;
    }
    else if(arg1 == "Bayer BGGR")
    {
        ix.IsCvtColor = true;
        ix.Bayer = BAYER_BG;//3;
    }
    else if(arg1 == "Bayer RGGB")
    {
        ix.IsCvtColor = true;
        ix.Bayer = BAYER_RG;//4;
    }
}

//-----------------------camera setup-----------------------
void ManagementMenu::on_head_livecamerasetup_clicked(bool checked)
{
    ui->head_livecamerasetup->setChecked(checked);
    ui->widget_livecamerasetup->setVisible(checked);
}

void ManagementMenu::on_lineEditLiveExp_textChanged(const QString &arg1)
{
    if(arg1 == "") return;

    uint32_t ret = QHYCCD_ERROR;
//    double unit;

//    if(ui->comBoxLiveUnit->currentIndex() == 0)
//    {
//        unit = 1000.0;
//    }
//    else if(ui->comBoxLiveUnit->currentIndex() == 1)
//    {
//        unit = 1000000.0;
//    }
//    else
//    {
//        unit = 60000000.0;
//    }

    if(ui->lineEditLiveExp->text().toInt() > ui->sliderLiveExposure->maximum())
        ui->lineEditLiveExp->setText(QString::number(ui->sliderLiveExposure->maximum()));
    if(ui->lineEditLiveExp->text().toInt() < ui->sliderLiveExposure->minimum())
        ui->lineEditLiveExp->setText(QString::number(ui->sliderLiveExposure->minimum()));

    ui->sliderLiveExposure->blockSignals(true);
    ui->sliderLiveExposure->setValue(ui->lineEditLiveExp->text().toInt());
    ui->sliderLiveExposure->blockSignals(false);

    ix.ExpTime = ui->lineEditLiveExp->text().toDouble();
    ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_EXPOSURE, ix.ExpTime * ix.ExpUnit);
}

void ManagementMenu::on_sliderLiveExposure_valueChanged(int value)
{
    uint32_t ret = QHYCCD_ERROR;
//    double unit;

//    if(ui->comBoxLiveUnit->currentIndex() == 0)
//    {
//        unit = 1000.0;
//    }
//    else if(ui->comBoxLiveUnit->currentIndex() == 1)
//    {
//        unit = 1000000.0;
//    }
//    else
//    {
//        unit = 60000000.0;
//    }

    ui->lineEditLiveExp->blockSignals(true);
    ui->lineEditLiveExp->setText(QString::number(ui->sliderLiveExposure->value()));
    ui->lineEditLiveExp->blockSignals(false);

    ix.ExpTime = ui->sliderLiveExposure->value();
    ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_EXPOSURE, ix.ExpTime * ix.ExpUnit);
}

void ManagementMenu::on_comBoxLiveUnit_currentIndexChanged(int index)
{
    uint32_t ret = QHYCCD_ERROR;
    double ExpTime_Last = ix.ExpTime * ix.ExpUnit;

    ui->sliderLiveExposure->blockSignals(true);
    ui->lineEditLiveExp->blockSignals(true);
    if(ui->comBoxLiveUnit->currentText() == "1~1000 us")
    {
        ix.ExpUnit = 1.0;
        if(ExpTime_Last >= 1000.0)
        {
            ui->sliderLiveExposure->setValue(1000);
            ui->lineEditLiveExp->setText("1000");
            ix.ExpTime = 1000.0;
        }
    }
    else if(ui->comBoxLiveUnit->currentText() == "1~1000 ms")
    {
        ix.ExpUnit = 1000.0;
        if(ExpTime_Last >= 1000000.0)
        {
            ui->sliderLiveExposure->setValue(1000);
            ui->lineEditLiveExp->setText("1000");
            ix.ExpTime = 1000.0;
        }
        if(ExpTime_Last <= 1000.0)
        {
            ui->sliderLiveExposure->setValue(1);
            ui->lineEditLiveExp->setText("1");
            ix.ExpTime = 1.0;
        }
    }
    else if(ui->comBoxLiveUnit->currentText() == "1~1200 s")
    {
        ix.ExpUnit = 1000000.0;
        if(ExpTime_Last >= 1200000000.0)
        {
            ui->sliderLiveExposure->setValue(1200);
            ui->lineEditLiveExp->setText("1200");
            ix.ExpTime = 1200.0;
        }
        if(ExpTime_Last <= 1000000.0)
        {
            ui->sliderLiveExposure->setValue(1);
            ui->lineEditLiveExp->setText("1");
            ix.ExpTime = 1.0;
        }
    }
    else if(ui->comBoxLiveUnit->currentText() == "20~60 min")
    {
        ix.ExpUnit = 60000000.0;
        if(ExpTime_Last <= 1200000000.0)
        {
            ui->sliderLiveExposure->setValue(20);
            ui->lineEditLiveExp->setText("20");
            ix.ExpTime = 20;
        }
    }
    ui->sliderLiveExposure->blockSignals(false);
    ui->lineEditLiveExp->blockSignals(false);

    ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_EXPOSURE, ix.ExpTime * ix.ExpUnit);
    if(ret == QHYCCD_SUCCESS)
    {
        qDebug() << "SetQHYCCDParam CONTROL_EXPOSURE " << ix.ExpTime;
    }
    else
    {
        qCritical() << "SetQHYCCDParam CONTROL_EXPOSURE failure";
    }
}

void ManagementMenu::on_sliderLiveGain_valueChanged(int value)
{
    uint32_t ret = QHYCCD_ERROR;

    ui->lineEditLiveGain->blockSignals(true);
    ui->lineEditLiveGain->setText(QString::number(ui->sliderLiveGain->value()));
    ui->lineEditLiveGain->blockSignals(false);

    ix.Gain = ui->sliderLiveGain->value();
    ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_GAIN, ix.Gain);
}

void ManagementMenu::on_lineEditLiveGain_textChanged(const QString &arg1)
{
    if(arg1 == "") return;

    uint32_t ret = QHYCCD_ERROR;

    if(ui->lineEditLiveGain->text().toInt() > ui->sliderLiveGain->maximum())
        ui->lineEditLiveGain->setText(QString::number(ui->sliderLiveGain->maximum()));
    if(ui->lineEditLiveGain->text().toInt() < ui->sliderLiveGain->minimum())
        ui->lineEditLiveGain->setText(QString::number(ui->sliderLiveGain->minimum()));

    ui->sliderLiveGain->blockSignals(true);
    ui->sliderLiveGain->setValue(ui->lineEditLiveGain->text().toInt());
    ui->sliderLiveGain->blockSignals(false);

    ix.Gain = ui->lineEditLiveGain->text().toInt();
    ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_GAIN, ix.Gain);
}

void ManagementMenu::on_sliderLiveOffset_valueChanged(int value)
{
    uint32_t ret = QHYCCD_ERROR;

    ui->lineEditLiveOffset->blockSignals(true);
    ui->lineEditLiveOffset->setText(QString::number(ui->sliderLiveOffset->value()));
    ui->lineEditLiveOffset->blockSignals(false);

    ix.Offset = ui->sliderLiveOffset->value();
    ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_OFFSET, ix.Offset);
}

void ManagementMenu::on_lineEditLiveOffset_textChanged(const QString &arg1)
{
    if(arg1 == "") return;

    uint32_t ret = QHYCCD_ERROR;

    if(ui->lineEditLiveOffset->text().toInt() > ui->sliderLiveOffset->maximum())
        ui->lineEditLiveOffset->setText(QString::number(ui->sliderLiveOffset->maximum()));
    if(ui->lineEditLiveOffset->text().toInt() < ui->sliderLiveOffset->minimum())
        ui->lineEditLiveOffset->setText(QString::number(ui->sliderLiveOffset->minimum()));

    ui->sliderLiveOffset->blockSignals(true);
    ui->sliderLiveOffset->setValue(ui->lineEditLiveOffset->text().toInt());
    ui->sliderLiveOffset->blockSignals(false);

    ix.Offset = ui->lineEditLiveOffset->text().toInt();
    ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_OFFSET, ix.Offset);
}

void ManagementMenu::on_sliderLiveTraffic_valueChanged(int value)
{
    uint32_t ret = QHYCCD_ERROR;

    ui->lineEditLiveTraffic->blockSignals(true);
    ui->lineEditLiveTraffic->setText(QString::number(ui->sliderLiveTraffic->value()));
    ui->lineEditLiveTraffic->blockSignals(false);

    ix.Traffic = ui->sliderLiveTraffic->value();
    ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_USBTRAFFIC, ix.Traffic);
}

void ManagementMenu::on_lineEditLiveTraffic_textChanged(const QString &arg1)
{
    if(arg1 == "") return;

    uint32_t ret = QHYCCD_ERROR;

    if(ui->lineEditLiveTraffic->text().toInt() > ui->sliderLiveTraffic->maximum())
        ui->lineEditLiveTraffic->setText(QString::number(ui->sliderLiveTraffic->maximum()));
    if(ui->lineEditLiveTraffic->text().toInt() < ui->sliderLiveTraffic->minimum())
        ui->lineEditLiveTraffic->setText(QString::number(ui->sliderLiveTraffic->minimum()));

    ui->sliderLiveTraffic->blockSignals(true);
    ui->sliderLiveTraffic->setValue(ui->lineEditLiveTraffic->text().toInt());
    ui->sliderLiveTraffic->blockSignals(false);

    ix.Traffic = ui->lineEditLiveTraffic->text().toInt();
    ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_USBTRAFFIC, ix.Traffic);
}

void ManagementMenu::on_sliderLiveSpeed_valueChanged(int value)
{
    OutputDebug("EZCAP | %s | %s | MID1  %d", __FILE__, __FUNCTION__, clock());

    uint32_t ret = QHYCCD_ERROR;

    ui->lineEditLiveSpeed->blockSignals(true);
    ui->lineEditLiveSpeed->setText(QString::number(value));
    ui->lineEditLiveSpeed->blockSignals(false);

    OutputDebug("EZCAP | %s | %s | MID2 = %d", __FILE__, __FUNCTION__, clock());

    ix.Speed = value;
    ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_SPEED, ix.Speed);
    OutputDebug("EZCAP | %s | %s | SetQHYCCDParam() CONTROL_SPEED value = %f %d", __FILE__, __FUNCTION__, ix.Speed, clock());
}

void ManagementMenu::on_lineEditLiveSpeed_textChanged(const QString &arg1)
{
    if(arg1 == "") return;

    uint32_t ret = QHYCCD_ERROR;

    if(arg1.toInt() > ix.Speed_Max) ui->lineEditLiveSpeed->setText(QString::number(ix.Speed_Max));
    if(arg1.toInt() < ix.Speed_Min) ui->lineEditLiveSpeed->setText(QString::number(ix.Speed_Min));

    ui->sliderLiveSpeed->blockSignals(true);
    ui->sliderLiveSpeed->setValue(arg1.toInt());
    ui->sliderLiveSpeed->blockSignals(false);

    ix.Speed = arg1.toInt();
    ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_SPEED, ix.Speed);
    OutputDebug("EZCAP | %s | %s | SetQHYCCDParam() CONTROL_SPEED value = %f", __FILE__, __FUNCTION__, ix.Speed);
}

void ManagementMenu::on_comBoxLiveDDR_currentTextChanged(const QString &arg1)
{
    uint32_t ret = QHYCCD_ERROR;

    if(arg1 == "ON")
    {
        ix.DDR = true;
    }
    else if(arg1 == "OFF")
    {
        ix.DDR = false;
    }

    ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_DDR, ix.DDR);
}

void ManagementMenu::on_comBoxLiveAMPV_currentTextChanged(const QString &arg1)
{
    uint32_t ret = QHYCCD_ERROR;

    if(arg1 == "ON")
    {
        ix.AMPV = true;
    }
    else if(arg1 == "ON")
    {
        ix.AMPV = false;
    }

    ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_AMPV, ix.AMPV);
}

//------------------------image setup---------------------
void ManagementMenu::on_head_liveimagesetup_clicked(bool checked)
{
    ui->head_liveimagesetup->setChecked(checked);
    ui->widget_liveimagesetup->setVisible(checked);
}

void ManagementMenu::on_sliderLiveWBR_valueChanged(int value)
{
    uint32_t ret = QHYCCD_ERROR;

    ui->lineEditLiveWBR->blockSignals(true);
    ui->lineEditLiveWBR->setText(QString::number(ui->sliderLiveWBR->value()));
    ui->lineEditLiveWBR->blockSignals(false);

    ix.WBR = ui->sliderLiveWBR->value();
    ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_WBR, ix.WBR);
}

void ManagementMenu::on_lineEditLiveWBR_textChanged(const QString &arg1)
{
    if(arg1 == "") return;

    uint32_t ret = QHYCCD_ERROR;

    if(ui->lineEditLiveWBR->text().toInt() > ui->sliderLiveWBR->maximum())
        ui->lineEditLiveWBR->setText(QString::number(ui->sliderLiveWBR->maximum()));
    if(ui->lineEditLiveWBR->text().toInt() < ui->sliderLiveWBR->minimum())
        ui->lineEditLiveWBR->setText(QString::number(ui->sliderLiveWBR->minimum()));

    ui->sliderLiveWBR->blockSignals(true);
    ui->sliderLiveWBR->setValue(ui->lineEditLiveWBR->text().toInt());
    ui->sliderLiveWBR->blockSignals(false);

    ix.WBR = ui->lineEditLiveWBR->text().toInt();
    ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_WBR, ix.WBR);
}

void ManagementMenu::on_sliderLiveWBG_valueChanged(int value)
{
    uint32_t ret = QHYCCD_ERROR;

    ui->lineEditLiveWBG->blockSignals(true);
    ui->lineEditLiveWBG->setText(QString::number(ui->sliderLiveWBG->value()));
    ui->lineEditLiveWBG->blockSignals(false);

    ix.WBG = ui->sliderLiveWBG->value();
    ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_WBG, ix.WBG);
}

void ManagementMenu::on_lineEditLiveWBG_textChanged(const QString &arg1)
{
    if(arg1 == "") return;

    uint32_t ret = QHYCCD_ERROR;

    if(ui->lineEditLiveWBG->text().toInt() > ui->sliderLiveWBG->maximum())
        ui->lineEditLiveWBG->setText(QString::number(ui->sliderLiveWBG->maximum()));
    if(ui->lineEditLiveWBG->text().toInt() < ui->sliderLiveWBG->minimum())
        ui->lineEditLiveWBG->setText(QString::number(ui->sliderLiveWBG->minimum()));

    ui->sliderLiveWBG->blockSignals(true);
    ui->sliderLiveWBG->setValue(ui->lineEditLiveWBG->text().toInt());
    ui->sliderLiveWBG->blockSignals(false);

    ix.WBG = ui->lineEditLiveWBG->text().toInt();
    ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_WBG, ix.WBG);
}

void ManagementMenu::on_sliderLiveWBB_valueChanged(int value)
{
    uint32_t ret = QHYCCD_ERROR;

    ui->lineEditLiveWBB->blockSignals(true);
    ui->lineEditLiveWBB->setText(QString::number(ui->sliderLiveWBB->value()));
    ui->lineEditLiveWBB->blockSignals(false);

    ix.WBB = ui->sliderLiveWBB->value();
    ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_WBB, ix.WBB);
}

void ManagementMenu::on_lineEditLiveWBB_textChanged(const QString &arg1)
{
    if(arg1 == "") return;

    uint32_t ret = QHYCCD_ERROR;

    if(ui->lineEditLiveWBB->text().toInt() > ui->sliderLiveWBB->maximum())
        ui->lineEditLiveWBB->setText(QString::number(ui->sliderLiveWBB->maximum()));
    if(ui->lineEditLiveWBB->text().toInt() < ui->sliderLiveWBB->minimum())
        ui->lineEditLiveWBB->setText(QString::number(ui->sliderLiveWBB->minimum()));

    ui->sliderLiveWBB->blockSignals(true);
    ui->sliderLiveWBB->setValue(ui->lineEditLiveWBB->text().toInt());
    ui->sliderLiveWBB->blockSignals(false);

    ix.WBB = ui->lineEditLiveWBB->text().toDouble();
    ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_WBB, ix.WBB);
}

void ManagementMenu::on_sliderLiveBrightness_valueChanged(int value)
{
    uint32_t ret = QHYCCD_ERROR;

    ui->lineEditLiveBrightness->blockSignals(true);
    ui->lineEditLiveBrightness->setText(QString::number((double)ui->sliderLiveBrightness->value() / 10.0, 'f', 1));
    ui->lineEditLiveBrightness->blockSignals(false);

    ix.Brightness = (double)ui->sliderLiveBrightness->value() / 10.0;
    ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_BRIGHTNESS, ix.Brightness);
}

void ManagementMenu::on_lineEditLiveBrightness_textChanged(const QString &arg1)
{
    if(arg1 == "") return;

    uint32_t ret = QHYCCD_ERROR;

    if((ui->lineEditLiveBrightness->text().toDouble() * 10.0) > ui->sliderLiveBrightness->maximum())
        ui->lineEditLiveBrightness->setText(QString::number((double)ui->sliderLiveBrightness->maximum() / 10.0, 'f', 1));
    if((ui->lineEditLiveBrightness->text().toDouble() * 10.0) < ui->sliderLiveBrightness->minimum())
        ui->lineEditLiveBrightness->setText(QString::number((double)ui->sliderLiveBrightness->minimum() / 10.0, 'f', 1));
    if(ui->lineEditLiveBrightness->text() == "-1")
        ui->lineEditLiveBrightness->setText("-1.0");
    if(ui->lineEditLiveBrightness->text() == "0")
        ui->lineEditLiveBrightness->setText("0.0");
    if(ui->lineEditLiveBrightness->text() == "1")
        ui->lineEditLiveBrightness->setText("1.0");

    ui->sliderLiveBrightness->blockSignals(true);
    ui->sliderLiveBrightness->setValue(DoubleToInt(ui->lineEditLiveBrightness->text().toDouble() * 10.0));
    ui->sliderLiveBrightness->blockSignals(false);

    ix.Brightness = ui->lineEditLiveBrightness->text().toDouble();
    ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_BRIGHTNESS, ix.Brightness);
}

void ManagementMenu::on_sliderLiveContrast_valueChanged(int value)
{
    uint32_t ret = QHYCCD_ERROR;

    ui->lineEditLiveContrast->blockSignals(true);
    ui->lineEditLiveContrast->setText(QString::number((double)ui->sliderLiveContrast->value() / 10.0, 'f', 1));
    ui->lineEditLiveContrast->blockSignals(false);

    ix.Contrast = (double)ui->sliderLiveContrast->value() / 10.0;
    ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_CONTRAST, ix.Contrast);
}

void ManagementMenu::on_lineEditLiveContrast_textChanged(const QString &arg1)
{
    if(arg1 == "") return;

    uint32_t ret = QHYCCD_ERROR;

    if((ui->lineEditLiveContrast->text().toDouble() * 10.0) > ui->sliderLiveContrast->maximum())
        ui->lineEditLiveContrast->setText(QString::number((double)ui->sliderLiveContrast->maximum() / 10.0, 'f', 1));
    if((ui->lineEditLiveContrast->text().toDouble() * 10.0) < ui->sliderLiveContrast->minimum())
        ui->lineEditLiveContrast->setText(QString::number((double)ui->sliderLiveContrast->minimum() / 10.0, 'f', 1));
    if(ui->lineEditLiveContrast->text() == "-1")
        ui->lineEditLiveContrast->setText("-1.0");
    if(ui->lineEditLiveContrast->text() == "0")
        ui->lineEditLiveContrast->setText("0.0");
    if(ui->lineEditLiveContrast->text() == "1")
        ui->lineEditLiveContrast->setText("1.0");

    ui->sliderLiveContrast->blockSignals(true);
    ui->sliderLiveContrast->setValue(DoubleToInt(ui->lineEditLiveContrast->text().toDouble() * 10.0));
    ui->sliderLiveContrast->blockSignals(false);

    ix.Contrast = ui->lineEditLiveContrast->text().toDouble();
    ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_CONTRAST, ix.Contrast);
}

void ManagementMenu::on_sliderLiveGamma_valueChanged(int value)
{
    uint32_t ret = QHYCCD_ERROR;

    ui->lineEditLiveGamma->blockSignals(true);
    ui->lineEditLiveGamma->setText(QString::number((double)ui->sliderLiveGamma->value() / 10.0, 'f', 1));
    ui->lineEditLiveGamma->blockSignals(false);

    ix.Gamma = (double)ui->sliderLiveGamma->value() / 10.0;
    OutputDebug("EZCAPDEBUG | %s | %s | SetQHYCCDParam() CONTROL_GAMMA | ix.Gamma = %f", __FILE__, __FUNCTION__, ix.Gamma);
    ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_GAMMA, ix.Gamma);
}

void ManagementMenu::on_lineEditLiveGamma_textChanged(const QString &arg1)
{
    if(arg1 == "") return;

    uint32_t ret = QHYCCD_ERROR;

    if((ui->lineEditLiveGamma->text().toDouble() * 10.0) > ui->sliderLiveGamma->maximum())
        ui->lineEditLiveGamma->setText(QString::number((double)ui->sliderLiveGamma->maximum() / 10.0, 'f', 1));
    if((ui->lineEditLiveGamma->text().toDouble() * 10.0) < ui->sliderLiveGamma->minimum())
        ui->lineEditLiveGamma->setText(QString::number((double)ui->sliderLiveGamma->minimum() / 10.0, 'f', 1));
    if(ui->lineEditLiveGamma->text() == "-1")
        ui->lineEditLiveGamma->setText("-1.0");
    if(ui->lineEditLiveGamma->text() == "0")
        ui->lineEditLiveGamma->setText("0.0");
    if(ui->lineEditLiveGamma->text() == "1")
        ui->lineEditLiveGamma->setText("1.0");

    ui->sliderLiveGamma->blockSignals(true);
    ui->sliderLiveGamma->setValue(DoubleToInt(ui->lineEditLiveGamma->text().toDouble() * 10.0));
    ui->sliderLiveGamma->blockSignals(false);

    ix.Gamma = ui->lineEditLiveGamma->text().toDouble();
    OutputDebug("EZCAPDEBUG | %s | %s | SetQHYCCDParam() CONTROL_GAMMA | ix.Gamma = %f", __FILE__, __FUNCTION__, ix.Gamma);
    ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_GAMMA, ix.Gamma);
    if(ret == QHYCCD_ERROR)
    {
        OutputDebug("EZCAPWARNING | %s | %s | SetQHYCCDParam() CONTROL_GAMMA Failed!", __FILE__, __FUNCTION__);
    }
}


//----------------------------ROI-----------------------------------------------
void ManagementMenu::on_head_Roi_clicked(bool checked)
{
    ui->head_Roi->setChecked(checked);
    ui->widget_Roi->setVisible(checked);
}

void ManagementMenu::on_comboBox_RoiSaved_currentIndexChanged(const QString &arg1)
{
    if(arg1 == "") return;

    if(arg1 == "Customed")
    {
        ui->lineEdit_sizex_Roi->clear();
        ui->lineEdit_sizey_Roi->clear();
        ui->label_sizex_Roi->setVisible(true);
        ui->label_sizey_Roi->setVisible(true);
        ui->lineEdit_sizex_Roi->setVisible(true);
        ui->lineEdit_sizey_Roi->setVisible(true);
        ui->pushButton_SetROI->setVisible(true);
        ui->pushButton_SaveROI->setVisible(true);

        return;
    }

    QStringList roi = arg1.split(" x ");
    if(roi.length() < 2) return;

    ui->lineEdit_sizex_Roi->setText(roi.at(0));
    ui->lineEdit_sizey_Roi->setText(roi.at(1));
    ui->label_sizex_Roi->setVisible(false);
    ui->label_sizey_Roi->setVisible(false);
    ui->lineEdit_sizex_Roi->setVisible(false);
    ui->lineEdit_sizey_Roi->setVisible(false);
    ui->pushButton_SetROI->setVisible(false);
    ui->pushButton_SaveROI->setVisible(false);

    ix.RoiX = (ix.ImageW_Min / ix.BinX - roi.at(0).toInt()) / 2;
    ix.RoiY = (ix.ImageH_Min / ix.BinY - roi.at(1).toInt()) / 2;
    ix.RoiW = roi.at(0).toInt();
    ix.RoiH = roi.at(1).toInt();

    cv::Mat ImgRoiTemp = cv::Mat::zeros(cv::Size(ui->img_Roi->width(), ui->img_Roi->height()), CV_8UC3);
    if(ix.RoiW < ix.ImageW_Min / ix.BinX && ix.RoiH < ix.ImageH_Min / ix.BinY)
        rectangle(ImgRoiTemp,
                  Point((double)ix.RoiX / ix.ImageW_Min * ix.BinX * ui->img_Roi->width(),
                        (double)ix.RoiY / ix.ImageH_Min * ix.BinY * ui->img_Roi->height()),
                  Point((double)(ix.RoiX + ix.RoiW) / ix.ImageW_Min * ix.BinX * ui->img_Roi->width(),
                        (double)(ix.RoiY + ix.RoiH) / ix.ImageH_Min * ix.BinY * ui->img_Roi->height()),
                  Scalar(255, 0, 0), 1, LINE_8, 0);
    QImage *RoiQImg = mainWidget->MatToQImage(ImgRoiTemp);
    managerMenu->ui->img_Roi->setPixmap(QPixmap::fromImage(*RoiQImg));
    ImgRoiTemp.release();

    if(ix.camStreamMode == 0) return;

    if(ix.RoiX != ix.RoiX_Last || ix.RoiY != ix.RoiY_Last ||
       ix.RoiW != ix.RoiW_Last || ix.RoiH != ix.RoiH_Last)
    {
        uint32_t ret = QHYCCD_ERROR;

        ///
        /// 结束线程
        ///

        if(mainWidget->threadProcessImage->isRunning())
            mainWidget->threadProcessImage->stop();

        if(mainWidget->liveCap->isRunning())
        {
            mainWidget->liveCap->closeThread();
        }

        ret = libqhyccd->StopQHYCCDLive(camhandle);
        if(ret == QHYCCD_ERROR)
        {
            DBGOPT_ERROR("StopQHYCCDLive() Failed!");
            return;
        }

        if(SwitchReadmodeBinFormat())
        {
            DBGOPT_ERROR("Switch Readmode Bin Format failed");
            QMessageBox::critical(this,tr("Error"),tr("Camera switch read/bin/format mode failed!"), QMessageBox::Ok);

            ix.BinX = ix.BinX_Last;
            ix.BinY = ix.BinY_Last;
            CloseCamera();
            emit mainWidget->disconnect_camera();
            return;
        }

        if(ResetParameters())
        {
            OutputDebug("EZCAPERROR | %s | %s | Reset Parameters failed", __FILE__, __FUNCTION__);
            QMessageBox::critical(this,tr("Error"),tr("Camera reset parameters failed!"), QMessageBox::Ok);

            CloseCamera();
            emit mainWidget->disconnect_camera();
            return;
        }

        mainWidget->adjustScrollBar(ix.RoiW, ix.RoiH);

        ///
        /// 重新开启线程
        ///
        ret = libqhyccd->BeginQHYCCDLive(camhandle);
        if(ret == QHYCCD_ERROR)
        {
            OutputDebug("EZCAPWARNING | %s | %s | BeginQHYCCDLive() Failed!", __FILE__, __FUNCTION__);
        }

        mainWidget->liveCap->start();
        mainWidget->threadProcessImage->start();
    }
}


void ManagementMenu::on_lineEdit_sizex_Roi_textChanged(const QString &arg1)
{
    if(arg1 == "") return;

    ix.RoiW = ui->lineEdit_sizex_Roi->text().toUInt();

    if(ix.RoiW > ix.ImageW_Min / ix.BinX)
    {
        ui->lineEdit_sizex_Roi->setText(QString::number(ix.ImageW_Min / ix.BinX));
        ix.RoiW = ix.ImageW_Min / ix.BinX;
    }
}

void ManagementMenu::on_lineEdit_sizey_Roi_textChanged(const QString &arg1)
{
    if(arg1 == "") return;

    ix.RoiH = ui->lineEdit_sizey_Roi->text().toUInt();

    if(ix.RoiH > ix.ImageH_Min / ix.BinY)
    {
        ui->lineEdit_sizey_Roi->setText(QString::number(ix.ImageH_Min / ix.BinY));
        ix.RoiH = ix.ImageH_Min / ix.BinY;
    }
}

void ManagementMenu::on_pushButton_SetROI_clicked()
{
    if((ui->lineEdit_sizex_Roi->text().toUInt() > (ix.ImageW_Min / ix.BinX)) ||
       (ui->lineEdit_sizey_Roi->text().toUInt() > (ix.ImageH_Min / ix.BinY)))
    {
        QMessageBox::critical(this,tr("Error"),tr("Invalid image size!"), QMessageBox::Ok);
        return;
    }

    ui->comboBox_RoiSaved->blockSignals(true);
    ui->comboBox_RoiSaved->setCurrentText("Customed");
    ui->comboBox_RoiSaved->blockSignals(false);

    ix.RoiX = (ix.ImageW_Min / ix.BinX - ui->lineEdit_sizex_Roi->text().toInt()) / 2;
    ix.RoiY = (ix.ImageH_Min / ix.BinY - ui->lineEdit_sizey_Roi->text().toInt()) / 2;
    ix.RoiW = ui->lineEdit_sizex_Roi->text().toInt();
    ix.RoiH = ui->lineEdit_sizey_Roi->text().toInt();

    cv::Mat ImgRoiTemp = cv::Mat::zeros(cv::Size(ui->img_Roi->width(), ui->img_Roi->height()), CV_8UC3);
    if(ix.RoiW < ix.ImageW_Min / ix.BinX && ix.RoiH < ix.ImageH_Min / ix.BinY)
        rectangle(ImgRoiTemp,
                  Point((double)ix.RoiX / ix.ImageW_Min * ix.BinX * ui->img_Roi->width(),
                        (double)ix.RoiY / ix.ImageH_Min * ix.BinY * ui->img_Roi->height()),
                  Point((double)(ix.RoiX + ix.RoiW) / ix.ImageW_Min * ix.BinX * ui->img_Roi->width(),
                        (double)(ix.RoiY + ix.RoiH) / ix.ImageH_Min * ix.BinY * ui->img_Roi->height()),
                  Scalar(255, 0, 0), 1, LINE_8, 0);
    QImage *RoiQImg = mainWidget->MatToQImage(ImgRoiTemp);
    managerMenu->ui->img_Roi->setPixmap(QPixmap::fromImage(*RoiQImg));
    ImgRoiTemp.release();

    if(ix.RoiX != ix.RoiX_Last || ix.RoiY != ix.RoiY_Last ||
       ix.RoiW != ix.RoiW_Last || ix.RoiH != ix.RoiH_Last)
    {
        uint32_t ret = QHYCCD_ERROR;

        ///
        /// 结束线程
        ///

        if(mainWidget->threadProcessImage->isRunning())
            mainWidget->threadProcessImage->stop();

        if(mainWidget->liveCap->isRunning())
        {
            mainWidget->liveCap->closeThread();
        }

        ret = libqhyccd->StopQHYCCDLive(camhandle);
        if(ret == QHYCCD_ERROR)
        {
            DBGOPT_ERROR("StopQHYCCDLive() Failed!");
            return;
        }

        if(SwitchReadmodeBinFormat())
        {
            DBGOPT_ERROR("Switch Readmode Bin Format failed");
            QMessageBox::critical(this,tr("Error"),tr("Camera switch read/bin/format mode failed!"), QMessageBox::Ok);

            ix.BinX = ix.BinX_Last;
            ix.BinY = ix.BinY_Last;
            CloseCamera();
            emit mainWidget->disconnect_camera();
            return;
        }

        if(ResetParameters())
        {
            OutputDebug("EZCAPERROR | %s | %s | Reset Parameters failed", __FILE__, __FUNCTION__);
            QMessageBox::critical(this,tr("Error"),tr("Camera reset parameters failed!"), QMessageBox::Ok);

            CloseCamera();
            emit mainWidget->disconnect_camera();
            return;
        }

        mainWidget->adjustScrollBar(ix.RoiW, ix.RoiH);

        ///
        /// 重新开启线程
        ///
        ret = libqhyccd->BeginQHYCCDLive(camhandle);
        if(ret == QHYCCD_ERROR)
        {
            OutputDebug("EZCAPWARNING | %s | %s | BeginQHYCCDLive() Failed!", __FILE__, __FUNCTION__);
        }

        mainWidget->liveCap->start();
        mainWidget->threadProcessImage->start();
    }
}


void ManagementMenu::on_pushButton_SaveROI_clicked()
{
    QString str = QString::number(ix.RoiW) + " x " + QString::number(ix.RoiH);
    ui->comboBox_RoiSaved->setItemText(ui->comboBox_RoiSaved->count() - 1, str);

    QString CustomedROI = "";
    mainWidget->loadParamFromIni(ix.CamModel + "-" + QString::number(ix.camStreamMode),
                                 "CustomedROI_Bin" + QString::number(ix.BinX) + "x" + QString::number(ix.BinY),
                                 &CustomedROI, "");
    CustomedROI = CustomedROI + ";" + str;

    mainWidget->saveParamToIni(ix.CamModel + "-" + QString::number(ix.camStreamMode),
                               "CustomedROI_Bin" + QString::number(ix.BinX) + "x" + QString::number(ix.BinY),
                               CustomedROI);

    ui->comboBox_RoiSaved->addItem("Customed");
}

//---------------------screenView groupBox--------------------------------------------------
void ManagementMenu::on_head_screenView_clicked(bool checked)
{
    ui->head_screenView->setChecked(checked);
    ui->widget_screenView->setVisible(checked);
    if(checked)
    {
//#ifdef Q_OS_MAC
//        ui->head_screenView->setFixedHeight(18);
//#endif
    }
    else
    {
//#ifdef Q_OS_MAC
//        ui->head_screenView->setFixedHeight(27);
//#endif
    }
}

//---------------------histogram groupBox----------------------------------------------------
void ManagementMenu::on_head_hist_clicked(bool checked)
{
    ui->head_hist->setChecked(checked);
    ui->widget_hist->setVisible(checked);
    if(checked)
    {
//#ifdef Q_OS_MAC
//        ui->head_hist->setFixedHeight(18);
//#endif
    }
    else
    {
//#ifdef Q_OS_MAC
//        ui->head_hist->setFixedHeight(27);
//#endif
    }
}

void ManagementMenu::on_hSlider_wPos_valueChanged(int value)
{
    int dstPos = value * ui->img_hist->width() / 65536 + ui->img_hist->pos().x();

    ui->lineW->move(dstPos, ui->lineW->y());
    ui->label_W->move((dstPos - ui->label_W->width() / 2 + 1), ui->label_W->y());

    if(ui->hSlider_wPos->value() < ui->hSlider_bPos->value())
    {
        ui->hSlider_bPos->setValue(ui->hSlider_wPos->value() - 1);
    }
}

void ManagementMenu::on_hSlider_bPos_valueChanged(int value)
{
    int dstPos = value * ui->img_hist->width() / 65536 + ui->img_hist->pos().x();

    ui->lineB->move(dstPos, ui->lineB->y());
    ui->label_B->move((dstPos - ui->label_B->width()/2 + 1), ui->label_B->y());

    if(ui->hSlider_bPos->value() > ui->hSlider_wPos->value())
    {
        ui->hSlider_wPos->setValue(ui->hSlider_bPos->value() + 1);
    }
}

void ManagementMenu::on_pBtn_coarse_clicked()
{
    if(ui->pBtn_coarse->isChecked())
    {
        ix.StretchStep = 16;
        ui->pBtn_coarse->setText(tr("Fine"));
        ui->pBtn_stretchMinusB->setText("<");
        ui->pBtn_stretchPlusB->setText(">");
        ui->pBtn_stretchMinusW->setText("<");
        ui->pBtn_stretchPlusW->setText(">");
    }
    else
    {
        ix.StretchStep = 256;
        ui->pBtn_coarse->setText(tr("Coarse"));
        ui->pBtn_stretchMinusB->setText("<<");
        ui->pBtn_stretchPlusB->setText(">>");
        ui->pBtn_stretchMinusW->setText("<<");
        ui->pBtn_stretchPlusW->setText(">>");
    }
}

void ManagementMenu::on_cBox_autoStretchList_currentIndexChanged(int index)
{
    ix.autoStretchMode = index;
    iniFileParams.autoStretchMode = ix.autoStretchMode;

    ui->pBtn_auto_histogram->click();
}

