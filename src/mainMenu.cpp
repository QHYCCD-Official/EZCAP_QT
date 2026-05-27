#include "mainMenu.h"
#include "ezCap.h"

#include <QEvent>
#include <QMouseEvent>
#include <QDebug>
#include <QActionGroup>

MainMenu *mainMenuBar;
extern struct IX ix;

MainMenu::MainMenu(QWidget *parent) :
    QMenuBar(parent)
{
    createActions();
    createMenus();

    //依次加入menu或action
    this->addMenu(menuFile);
    this->addMenu(menuCamera);
    this->addMenu(menuPlanner);
    this->addMenu(menuImageProcess);
    this->addMenu(menuCameraSetup);
    this->addMenu(menuTools);
    this->addMenu(menuZoom);
    this->addMenu(menuLanguage);
    this->addMenu(menuHelp);

    menuPlanner->setEnabled(false);
    menuImageProcess->setEnabled(false);
    menuCameraSetup->setEnabled(false);
    menuTools->setEnabled(false);
    menuZoom->setEnabled(false);
    actOpenFolder->setVisible(false);
    actSaveJPG->setVisible(false);
    actSavePNG->setVisible(false);
    actSaveTIF->setVisible(false);
    actSaveBMP->setVisible(false);
    actSaveFIT->setVisible(false);
    actIgnoreOverScanArea->setVisible(false);
    actCalibrateOverScan->setVisible(false);
    actSaveTHPFile->setVisible(true);
    actPHDLink->setVisible(false);
    actOtherCameraSetup->setVisible(false);
}

MainMenu::~MainMenu()
{
    if(actOpenVideo)
    {
        delete actOpenVideo;
        actOpenVideo = NULL;
    }
    if(actOpenFolder)
    {
        delete actOpenFolder;
        actOpenFolder = NULL;
    }
    if(actSaveFIT)
    {
        delete actSaveFIT;
        actSaveFIT = NULL;
    }
    if(actSaveBMP)
    {
        delete actSaveBMP;
        actSaveBMP = NULL;
    }
    if(actSaveJPG)
    {
        delete actSaveJPG;
        actSaveJPG = NULL;
    }
    if(actSavePNG)
    {
        delete actSavePNG;
        actSavePNG = NULL;
    }
    if(actSaveTIF)
    {
        delete actSaveTIF;
        actSaveTIF = NULL;
    }
    if(actFitHeaderEditor)
    {
        delete actFitHeaderEditor;
        actFitHeaderEditor = NULL;
    }
    if(actIgnoreOverScanArea)
    {
        delete actIgnoreOverScanArea;
        actIgnoreOverScanArea = NULL;
    }
    if(actCalibrateOverScan)
    {
        delete actCalibrateOverScan;
        actCalibrateOverScan = NULL;
    }
    if(actSaveTHPFile)
    {
        delete actSaveTHPFile;
        actSaveTHPFile = NULL;
    }
    if(actExit)
    {
        delete actExit;
        actExit = NULL;
    }

    if(actConnect)
    {
        delete actConnect;
        actConnect = NULL;
    }

    if(actCntLive)
    {
        delete actCntLive;
        actCntLive = NULL;
    }

    if(actShowPlanTable)
    {
        delete actShowPlanTable;
        actShowPlanTable = NULL;
    }

    if(actNoiseAnalyse)
    {
        delete actNoiseAnalyse;
        actNoiseAnalyse = NULL;
    }
    if(actImgAnalyze)
    {
        delete actImgAnalyze;
        actImgAnalyze = NULL;
    }
    if(actBatchProcess)
    {
        delete actBatchProcess;
        actBatchProcess = NULL;
    }

    if(actFavorite)
    {
        delete actFavorite;
        actFavorite = NULL;
    }
    if(actGPSControl)
    {
        delete actGPSControl;
        actGPSControl = NULL;
    }
    if(actToolBurst)
    {
        delete actToolBurst;
        actToolBurst = NULL;
    }
    if(actToolTrigger)
    {
        delete actToolTrigger;
        actToolTrigger = NULL;
    }

    if(actPHDLink)
    {
        delete actPHDLink;
        actPHDLink = NULL;
    }
    if(actOtherCameraSetup)
    {
        delete actOtherCameraSetup;
        actOtherCameraSetup = NULL;
    }
    if(actTempControl)
    {
        delete actTempControl;
        actTempControl = NULL;
    }

    if(actFrameToolCapCal)
    {
        delete actFrameToolCapCal;
        actFrameToolCapCal = NULL;
    }

    if(actFrameToolCal)
    {
        delete actFrameToolCal;
        actFrameToolCal = NULL;
    }

    if(actCorrectCenter)
    {
        delete actCorrectCenter;
        actCorrectCenter = NULL;
    }

    if(actCFWControl)
    {
        delete actCFWControl;
        actCFWControl = NULL;
    }

    if(actImgRotate180)
    {
        delete actImgRotate180;
        actImgRotate180 = NULL;
    }

    if(actImgRotate90L)
    {
        delete actImgRotate90L;
        actImgRotate90L = NULL;
    }

    if(actImgRotate90R)
    {
        delete actImgRotate90R;
        actImgRotate90R = NULL;
    }

    if(actImgMirrorH)
    {
        delete actImgMirrorH;
        actImgMirrorH = NULL;
    }
    if(actImgMirrorV)
    {
        delete actImgMirrorV;
        actImgMirrorV = NULL;
    }

    if(actFitWindow)
    {
        delete actFitWindow;
        actFitWindow = NULL;
    }
    if(actFillWindow)
    {
        delete actFillWindow;
        actFillWindow = NULL;
    }
    if(act0_25X)
    {
        delete act0_25X;
        act0_25X = NULL;
    }
    if(act0_5X)
    {
        delete act0_5X;
        act0_5X = NULL;
    }
    if(act0_75X)
    {
        delete act0_75X;
        act0_75X = NULL;
    }
    if(act1X)
    {
        delete act1X;
        act1X = NULL;
    }
    if(act1_5X)
    {
        delete act1_5X;
        act1_5X = NULL;
    }
    if(act2X)
    {
        delete act2X;
        act2X = NULL;
    }

    if(actEnglish)
    {
        delete actEnglish;
        actEnglish = NULL;
    }
    if(actChinese)
    {
        delete actChinese;
        actChinese = NULL;
    }
    if(actJapanese)
    {
        delete actJapanese;
        actJapanese = NULL;
    }
    if(actFrance)
    {
        delete actFrance;
        actFrance = NULL;
    }
    if(actSpain)
    {
        delete actSpain;
        actSpain = NULL;
    }
    if(actRussia)
    {
        delete actRussia;
        actRussia = NULL;
    }
    if(actGermany)
    {
        delete actGermany;
        actGermany = NULL;
    }

    if(actAbout)
    {
        delete actAbout;
        actAbout = NULL;
    }
    if(actManual)
    {
        delete actManual;
        actManual = NULL;
    }
    if(actTestMode)
    {
        delete actTestMode;
        actTestMode = NULL;
    }

    if(menuFile)
    {
        delete menuFile;
        menuFile = NULL;
    }

    if(menuCamera)
    {
        delete menuCamera;
        menuCamera = NULL;
    }

    if(menuPlanner)
    {
        delete menuPlanner;
        menuPlanner = NULL;
    }
    if(menuImageProcess)
    {
        delete menuImageProcess;
        menuImageProcess = NULL;
    }
    if(menuImageRotate)
    {
        delete menuImageRotate;
        menuImageRotate = NULL;
    }
    if(menuImageMirror)
    {
        delete menuImageMirror;
        menuImageMirror = NULL;
    }
    if(menuCameraSetup)
    {
        delete menuCameraSetup;
        menuCameraSetup = NULL;
    }

    if(menuTools)
    {
        delete menuTools;
        menuTools = NULL;
    }

    if(menuZoom)
    {
        delete menuZoom;
        menuZoom = NULL;
    }
    if(menuLanguage)
    {
        delete menuLanguage;
        menuLanguage = NULL;
    }
    if(menuHelp)
    {
        delete menuHelp;
        menuHelp = NULL;
    }
}

void MainMenu::createActions()
{
    actOpenVideo         = new QAction(tr("Open Video"), this);
    actConnect = new QAction(tr("Connect to Single"),this);
    actConnect->setIconText(tr("Connect to Single"));
    actConnect->setMenuRole(QAction::NoRole);
    actCntLive = new QAction(tr("Connect to Live"), this);
    actCntLive->setIconText(tr("Connect to Live"));
    actCntLive->setMenuRole(QAction::NoRole);

    actOpenFolder = new QAction(tr("Open Saved Folder"),this);
    actSaveFIT = new QAction(tr("Save FIT"), this);
    actSaveBMP = new QAction(tr("Save BMP"), this);
    actSaveJPG = new QAction(tr("Save JPG"), this);
    actSavePNG = new QAction(tr("Save PNG"), this);
    actSaveTIF = new QAction(tr("Save TIF"), this);
    actFitHeaderEditor = new QAction(tr("FIT Header Editor"), this);
    actIgnoreOverScanArea = new QAction(tr("Ignore Overscan Area"), this);
    actIgnoreOverScanArea->setCheckable(true);    
    actCalibrateOverScan = new QAction(tr("Overscan Calibration"), this);
    actCalibrateOverScan->setCheckable(true);
    actSaveTHPFile = new QAction(tr("Save THP File"), this);
    actSaveTHPFile->setCheckable(true);
    actExit = new QAction(tr("Exit"), this);
    actExit->setMenuRole(QAction::NoRole);

    actShowPlanTable = new QAction(tr("Show Plan Table"), this);

    actBatchProcess = new QAction(tr("Batch Process"), this);
    actBatchProcess->setEnabled(false);
    actBatchProcess->setCheckable(true);
    actImgAnalyze = new QAction(tr("Image Analyze"), this);
    actImgAnalyze->setCheckable(false);
    actNoiseAnalyse = new QAction(tr("Noise Analyse"), this);
    actNoiseAnalyse->setCheckable(true);

    actFavorite = new QAction(tr("Favorite"), this);
    actGPSControl = new QAction(tr("GPS Control"), this);
    actToolBurst = new QAction(tr("Burst Control"), this);
    actToolTrigger = new QAction(tr("Trigger Control"), this);
    actPHDLink = new QAction(tr("PHD Link"), this);
    actOtherCameraSetup = new QAction(tr("Other Camera Setup"), this);

    actFrameToolCapCal = new QAction(tr("CapCalibration"),this);
    actFrameToolCal = new QAction(tr("FrameCalibration"),this);
    actFrameToolCal->setCheckable(true);

    actCorrectCenter = new QAction(tr("Correct Center"), this);

    actTempControl = new QAction(tr("Temp Control"), this);
    actCFWControl = new QAction(tr("Filter Wheel Control"), this);

    actImgRotate180 = new QAction(tr("Rotate180"), this);
    actImgRotate180->setCheckable(true);
    actImgRotate90L = new QAction(tr("Rotate90L"), this);
    actImgRotate90L->setCheckable(true);
    actImgRotate90R = new QAction(tr("Rotate90R"), this);
    actImgRotate90R->setCheckable(true);
    actImgMirrorH   = new QAction(tr("MirrorH"),   this);
    actImgMirrorH->setCheckable(true);
    actImgMirrorV   = new QAction(tr("MirrorV"),   this);
    actImgMirrorV->setCheckable(true);

    act1X = new QAction("1X", this);
    act1X->setCheckable(true);
    actFitWindow = new QAction(tr("Fit Window"), this);
    actFitWindow->setCheckable(true);
    actFillWindow = new QAction(tr("Fill Window"), this);
    actFillWindow->setCheckable(true);
    act0_75X = new QAction("0.75X", this);
    act0_75X->setCheckable(true);
    act0_5X = new QAction("0.5X", this);
    act0_5X->setCheckable(true);
    act0_25X = new QAction("0.25X", this);
    act0_25X->setCheckable(true);
    act1_5X = new QAction("1.5X", this);
    act1_5X->setCheckable(true);
    act2X = new QAction("2X", this);
    act2X->setCheckable(true);

    actEnglish = new QAction(tr("English"), this);
    actEnglish->setCheckable(true);
    actChinese = new QAction(QString::fromUtf8("简体中文"), this);
    actChinese->setCheckable(true);
    actJapanese = new QAction(QString::fromUtf8("日本語"), this);
    actJapanese->setCheckable(true);
    actFrance = new QAction(tr("France"), this);
    actFrance->setCheckable(true);
    actSpain = new QAction(tr("Spain"), this);
    actSpain->setCheckable(true);
    actRussia = new QAction(tr("Russia"), this);
    actRussia->setCheckable(true);
    actGermany = new QAction(tr("Germany"), this);
    actGermany->setCheckable(true);

    actAbout = new QAction(tr("About"), this);
    actAbout->setMenuRole(QAction::NoRole);
    actManual = new QAction(tr("Manual"), this);
    actManual->setMenuRole(QAction::NoRole);
    actTestMode = new QAction(tr("Test Mode"), this);
    actTestMode->setCheckable(true);
    actTestGuid = new QAction(tr("Test Guid"), this);
    actTestGuid->setCheckable(true);
    actTestPumpV2 = new QAction(tr("Test Pump V2"), this);
    actTestPumpV2->setCheckable(true);
    actTestPumpV2_second = new QAction(tr("Test Pump V2 Second"), this);
    actTestPumpV2_second->setCheckable(true);
    actTestPumpV2_cycle = new QAction(tr("Test Pump V2 Cycle"), this);
    actTestPumpV2_cycle->setCheckable(true);
    actTestPumpV2_cycle_second = new QAction(tr("Test Pump V2 Cycle Second"), this);
    actTestPumpV2_cycle_second->setCheckable(true);
    actTestErrorLed = new QAction(tr("Test Error Led"), this);
    actTestErrorLed->setCheckable(true);

    actTestIMG1 = new QAction(tr("Test Image 1"), this);
    actTestIMG1->setCheckable(true);
    actTestIMG3 = new QAction(tr("Test Image 3"), this);
    actTestIMG3->setCheckable(true);
    actDebug = new QAction(tr("Debug"), this);
    actDebug->setCheckable(true);

    actTechnicalSupport = new QAction(tr("TechnicalSupport"),this);


}

void MainMenu::createMenus()
{
    /*File菜单*/
    menuFile = new QMenu(tr("File"));
    menuFile->addAction(actOpenVideo);
    menuFile->addAction(actOpenFolder);
    menuFile->addAction(actSaveFIT);
    menuFile->addAction(actSaveBMP);
    menuFile->addAction(actSaveJPG);
    menuFile->addAction(actSavePNG);
    menuFile->addAction(actSaveTIF);
    menuFile->addSeparator();
    menuFile->addAction(actFitHeaderEditor);
    menuFile->addAction(actIgnoreOverScanArea);
    menuFile->addAction(actCalibrateOverScan);
    menuFile->addSeparator();
    menuFile->addAction(actSaveTHPFile);
    menuFile->addSeparator();
    menuFile->addAction(actExit);
    //connect menu
    menuCamera = new QMenu(tr("Camera"),this);
    menuCamera->addAction(actConnect);
    actConnect->setShortcut(QKeySequence("Alt+S"));
    menuCamera->addAction(actCntLive);
    actCntLive->setShortcut(QKeySequence("Alt+L"));
    /*planner菜单*/
    menuPlanner = new QMenu(tr("Planner"));
    menuPlanner->addAction(actShowPlanTable);
    /*Image process菜单*/
    menuImageProcess = new QMenu(tr("Image Process"));
    menuImageRotate = new QMenu(tr("Image Rotate"));
    menuImageMirror = new QMenu(tr("Image Mirror"));
    menuImageRotate->addAction(actImgRotate180);
    menuImageRotate->addAction(actImgRotate90L);
    menuImageRotate->addAction(actImgRotate90R);
    menuImageMirror->addAction(actImgMirrorH);
    menuImageMirror->addAction(actImgMirrorV);
    menuImageProcess->addAction(actBatchProcess);
//    menuImageProcess->addAction(actImgAnalyze);
    menuImageProcess->addAction(actNoiseAnalyse);
    menuImageProcess->addMenu(menuImageRotate);
    menuImageProcess->addMenu(menuImageMirror);

    /*camera setup菜单*/
    menuCameraSetup = new QMenu(tr("Camera Setup"));
    menuCameraSetup->addAction(actFavorite);
    menuCameraSetup->addAction(actGPSControl);
    menuCameraSetup->addAction(actToolBurst);
    menuCameraSetup->addAction(actToolTrigger);
    menuCameraSetup->addAction(actPHDLink);
    menuCameraSetup->addAction(actTempControl);
    menuCameraSetup->addAction(actCFWControl);
    menuCameraSetup->addAction(actOtherCameraSetup);

    /*Tools*/
    menuTools = new QMenu(tr("Tools"));
    menuTools->addAction(actFrameToolCapCal);
    menuTools->addAction(actFrameToolCal);
    menuTools->addAction(actCorrectCenter);

    /*zoom菜单*/
    menuZoom = new QMenu(tr("Zoom"));
    menuZoom->addAction(act1X);
    menuZoom->addAction(actFitWindow);
    menuZoom->addAction(actFillWindow);
    menuZoom->addAction(act0_75X);
    menuZoom->addAction(act0_5X);
    menuZoom->addAction(act0_25X);
    menuZoom->addAction(act1_5X);
    menuZoom->addAction(act2X);
    /*language菜单*/
    menuLanguage = new QMenu(tr("Language"));
    menuLanguage->addAction(actEnglish);
    menuLanguage->addAction(actChinese);
    menuLanguage->addAction(actJapanese);
    menuLanguage->addAction(actFrance);
    menuLanguage->addAction(actSpain);
    menuLanguage->addAction(actRussia);
    menuLanguage->addAction(actGermany);
    /*help菜单*/
    menuHelp = new QMenu(tr("Help"));
    menuHelp->addAction(actAbout);
    menuHelp->addAction(actManual);
    menuHelp->addAction(actTestMode);
    menuHelp->addAction(actTestGuid);
    menuHelp->addAction(actTestPumpV2);
    menuHelp->addAction(actTestPumpV2_second);
    menuHelp->addAction(actTestPumpV2_cycle);
    menuHelp->addAction(actTestPumpV2_cycle_second);
    menuHelp->addAction(actTestErrorLed);
    menuHelp->addAction(actTestIMG1);
    menuHelp->addAction(actTestIMG3);
    menuHelp->addAction(actDebug);
    menuHelp->addAction(actTechnicalSupport);
    actTestMode->setShortcut(QKeySequence("Alt+T"));
    actTestGuid->setShortcut(QKeySequence("Alt+G"));
    actTestPumpV2->setShortcut(QKeySequence("Alt+U"));
    actTestPumpV2_second->setShortcut(QKeySequence("Alt+I"));

    //设置zoom子菜单项互斥
    QActionGroup *zoomGrp = new QActionGroup(menuZoom);
    zoomGrp->addAction(act0_5X);
    zoomGrp->addAction(act0_25X);
    zoomGrp->addAction(act0_75X);
    zoomGrp->addAction(act1X);
    zoomGrp->addAction(act1_5X);
    zoomGrp->addAction(act2X);
    zoomGrp->addAction(actFitWindow);
    zoomGrp->addAction(actFillWindow);
    zoomGrp->setExclusive(true);
    actFitWindow->setChecked(true);
//    act1X->setChecked(true);
    //设置language子菜单项互斥
    QActionGroup *languageGrp = new QActionGroup(menuLanguage);
    languageGrp->addAction(actEnglish);    
    languageGrp->addAction(actChinese);
    languageGrp->addAction(actJapanese);
    languageGrp->addAction(actFrance);
    languageGrp->addAction(actGermany);
    languageGrp->addAction(actRussia);
    languageGrp->addAction(actSpain);
    languageGrp->setExclusive(true);
    actEnglish->setChecked(true);
}

void MainMenu::resetUI()
{
    /*主菜单menu或action*/
    menuFile->setTitle(tr("File"));

    if(ix.isConnected)
    {
        if(ix.camStreamMode == 0)
        {
            actConnect->setText(tr("Disconnect"));
            actConnect->setShortcut(QKeySequence("Alt+D"));
            actCntLive->setVisible(false);
            actCntLive->setShortcut(QKeySequence(""));
        }
        else if(ix.camStreamMode == 1)
        {
            actCntLive->setText(tr("Disconnect"));
            actCntLive->setShortcut(QKeySequence("Alt+D"));
            actConnect->setVisible(false);
            actConnect->setShortcut(QKeySequence(""));
        }
        else
        {

        }
    }
    else
    {
        actConnect->setText(tr("Connect to Single"));
        actConnect->setShortcut(QKeySequence("Alt+S"));
        actConnect->setVisible(true);
        actCntLive->setText(tr("Connect to Live"));
        actCntLive->setShortcut(QKeySequence("Alt+L"));
        actCntLive->setVisible(true);
    }
    menuCamera->setTitle(tr("Camera"));

    menuPlanner->setTitle(tr("Planner"));
    menuImageProcess->setTitle(tr("Image Process"));
    menuImageRotate->setTitle(tr("Image Rotate"));
    menuImageMirror->setTitle(tr("Image Mirror"));
    menuCameraSetup->setTitle(tr("Camera Setup"));
    menuTools->setTitle(tr("Tools"));
    menuZoom->setTitle(tr("Zoom"));
    menuLanguage->setTitle(tr("Language"));
    menuHelp->setTitle(tr("Help"));

    /*子菜单menu以及action*/
    actOpenVideo->setText(tr("Open Video"));
    actOpenFolder->setText(tr("Open Saved Folder"));
    actSaveFIT->setText(tr("Save FIT"));
    actSaveBMP->setText(tr("Save BMP"));
    actSaveJPG->setText(tr("Save JPG"));
    actSavePNG->setText(tr("Save PNG"));
    actSaveTIF->setText(tr("Save TIF"));
    actFitHeaderEditor->setText(tr("FIT Header Editor"));
    actIgnoreOverScanArea->setText(tr("Ignore Overscan Area"));
    actCalibrateOverScan->setText(tr("Overscan Calibration"));
    actSaveTHPFile->setText(tr("Save THP File"));
    actExit->setText(tr("Exit"));

    actShowPlanTable->setText(tr("Show Plan Table"));

    actBatchProcess->setText(tr("Batch Process"));
    actImgAnalyze->setText(tr("Image Analyze"));
    actNoiseAnalyse->setText(tr("Noise Analyse"));

    actFavorite->setText(tr("Favorite"));
    actGPSControl->setText(tr("GPS Control"));
    actToolBurst->setText(tr("Burst Control"));
    actToolTrigger->setText(tr("Trigger Control"));
    actPHDLink->setText(tr("PHD Link"));
    actTempControl->setText(tr("Temp Control"));
    //actColorWheelSetting->setText(tr("Color Wheel Setting"));
    actCFWControl->setText(tr("Filter Wheel Control"));
    actOtherCameraSetup->setText(tr("Other Camera Setup"));

    actFrameToolCapCal->setText(tr("CapCalibration"));
    actFrameToolCal->setText(tr("FrameCalibration"));
    actCorrectCenter->setText(tr("Correct Center"));

    actFitWindow->setText(tr("Fit Window"));
    actFillWindow->setText(tr("Fill Window"));
    actAbout->setText(tr("About"));
    actManual->setText(tr("Manual"));
}

void MainMenu::camera_connected()
{
    this->actTempControl->setEnabled(true);
    this->actCFWControl->setEnabled(true);
    this->actFrameToolCapCal->setEnabled(true);
    this->actFrameToolCal->setEnabled(true);
    this->actCorrectCenter->setEnabled(true);
    this->actFavorite->setEnabled(true);
    this->actGPSControl->setEnabled(true);
    this->actToolBurst->setEnabled(true);
    this->actToolTrigger->setEnabled(true);

    this->actCFWControl->setVisible(ix.canFilterWheel);
    this->actTempControl->setVisible(ix.Cooler_Fun);
    this->actGPSControl->setVisible(ix.GPS_Fun);
    this->actToolBurst->setVisible(ix.Burst_Fun & ix.camStreamMode);
    this->actToolTrigger->setVisible(ix.canTriger);
    this->actFrameToolCapCal->setVisible(true);
    this->actFrameToolCal->setVisible(true);
    this->actCorrectCenter->setVisible(ix.camStreamMode);
    this->actFavorite->setVisible(true);
    actOtherCameraSetup->setVisible(true);

    //修改actionConnect控件为Disconnect
    if(ix.camStreamMode == 0)
    {
        this->menuPlanner->setEnabled(true);
        this->menuImageProcess->setEnabled(false);
        this->menuCameraSetup->setEnabled(true);
        this->menuTools->setEnabled(true);
        this->menuZoom->setEnabled(true);

        this->actConnect->setText(tr("Disconnect"));
        actConnect->setShortcut(QKeySequence("Alt+D"));
        actCntLive->setVisible(false);
        actCntLive->setShortcut(QKeySequence(""));

        actOpenFolder->setVisible(true);
        actSaveJPG->setVisible(true);
        actSavePNG->setVisible(true);
        actSaveTIF->setVisible(true);
        actSaveBMP->setVisible(true);
        actSaveFIT->setVisible(true);
        actIgnoreOverScanArea->setVisible(true);
        actCalibrateOverScan->setVisible(true);
        actSaveTHPFile->setVisible(true);
        actPHDLink->setVisible(true);
    }
    else if(ix.camStreamMode == 1)
    {
        this->menuPlanner->setEnabled(false);
        this->menuImageProcess->setEnabled(true);
        this->actImgAnalyze->setVisible(false);
        this->actNoiseAnalyse->setVisible(false);
        this->actBatchProcess->setVisible(false);
        this->menuCameraSetup->setEnabled(true);
        this->menuTools->setEnabled(true);
        this->menuZoom->setEnabled(true);

        this->actCntLive->setText(tr("Disconnect"));
        actCntLive->setShortcut(QKeySequence("Alt+D"));
        actConnect->setVisible(false);
        actConnect->setShortcut(QKeySequence(""));

        actOpenFolder->setVisible(false);
        actSaveJPG->setVisible(false);
        actSavePNG->setVisible(false);
        actSaveTIF->setVisible(false);
        actSaveBMP->setVisible(false);
        actSaveFIT->setVisible(false);
        actIgnoreOverScanArea->setVisible(false);
        actCalibrateOverScan->setVisible(false);
        actSaveTHPFile->setVisible(true);
        actPHDLink->setVisible(false);
    }
    else
    {

    }

    this->actOpenVideo->setEnabled(false);

    //保存图像功能是否可用
    this->actSaveFIT->setEnabled(false);
    this->actSaveBMP->setEnabled(false);
    this->actSaveJPG->setEnabled(false);
    this->actSavePNG->setEnabled(false);
    this->actSaveTIF->setEnabled(false);
}

void MainMenu::camera_disconnected()
{
    ix.isConnected =false;

    this->menuPlanner->setEnabled(false);
    this->menuImageProcess->setEnabled(false);
    this->menuCameraSetup->setEnabled(false);
    this->menuTools->setEnabled(false);
    this->menuZoom->setEnabled(false);

    this->actConnect->setText(tr("Connect to Single"));
    actConnect->setShortcut(QKeySequence("Alt+S"));
    actConnect->setVisible(true);
    this->actCntLive->setText(tr("Connect to Live"));
    actCntLive->setShortcut(QKeySequence("Alt+L"));
    actCntLive->setVisible(true);
    this->actCFWControl->setEnabled(false);
    this->actFavorite->setEnabled(false);
    this->actGPSControl->setEnabled(false);
    this->actToolBurst->setEnabled(false);
    this->actToolTrigger->setEnabled(false);
    this->actOpenVideo->setEnabled(true);
    this->actOpenFolder->setEnabled(false);
    this->actCalibrateOverScan->setEnabled(false);
    this->actSaveTHPFile->setEnabled(true);
    this->actIgnoreOverScanArea->setEnabled(false);
    this->actFrameToolCapCal->setEnabled(false);
    this->actFrameToolCal->setEnabled(false);
    this->actCorrectCenter->setEnabled(false);

    actOpenFolder->setVisible(false);
    actSaveJPG->setVisible(false);
    actSavePNG->setVisible(false);
    actSaveTIF->setVisible(false);
    actSaveBMP->setVisible(false);
    actSaveFIT->setVisible(false);
    actIgnoreOverScanArea->setVisible(false);
    actCalibrateOverScan->setVisible(false);
    actSaveTHPFile->setVisible(true);
    actPHDLink->setVisible(false);
    actOtherCameraSetup->setVisible(false);
}
