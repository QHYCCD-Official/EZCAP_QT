#ifndef MAINMENU_H
#define MAINMENU_H

#include "outputdebug.h"
#include <QMenuBar>

class MainMenu : public QMenuBar
{
    Q_OBJECT
public:
    explicit MainMenu(QWidget *parent = 0);
    ~MainMenu();

    /*创建菜单action*/
    void createActions();
    /*创建菜单menu*/
    void createMenus();

    /*主菜单menu或action*/
    QMenu *menuFile;

    QAction *actConnect;
    QAction *actCntLive;
    QMenu *menuCamera;

    QMenu *menuPlanner;
    QMenu *menuImageProcess;
    QMenu *menuImageRotate;
    QMenu *menuImageMirror;
    QMenu *menuCameraSetup;
    QMenu *menuTools;
    QMenu *menuZoom;
    QMenu *menuLanguage;
    QMenu *menuHelp;

    /*子菜单menu以及action*/
    QAction *actOpenVideo;
    QAction *actOpenFolder;
    QAction *actSaveFIT;
    QAction *actSaveBMP;
    QAction *actSaveJPG;
    QAction *actSavePNG;
    QAction *actSaveTIF;
    QAction *actFitHeaderEditor;
    QAction *actIgnoreOverScanArea;
    QAction *actCalibrateOverScan;
    QAction *actSaveTHPFile;
    QAction *actExit;
    QAction *actShowPlanTable;
    QAction *actImgAnalyze;
    QAction *actBatchProcess;
    QAction *actNoiseAnalyse;
    QAction *actImgRotate180;
    QAction *actImgRotate90L;
    QAction *actImgRotate90R;
    QAction *actImgMirrorH;
    QAction *actImgMirrorV;
    QAction *actFavorite;
    QAction *actGPSControl;
    QAction *actToolBurst;
    QAction *actToolTrigger;
    QAction *actTempControl;
    QAction *actCFWControl;
    QAction *actPHDLink;
    QAction *actOtherCameraSetup;
    QAction *actFitWindow;
    QAction *actFillWindow;
    QAction *act0_25X;
    QAction *act0_5X;
    QAction *act0_75X;
    QAction *act1X;
    QAction *act1_5X;
    QAction *act2X;
    QAction *actEnglish;
    QAction *actChinese;
    QAction *actJapanese;
    QAction *actFrance;
    QAction *actSpain;
    QAction *actRussia;
    QAction *actGermany;
    QAction *actAbout;
    QAction *actManual;
    QAction *actTestMode;
    QAction *actDebug;
    QAction *actTestGuid;
    QAction *actTestPumpV2;
    QAction *actTestPumpV2_second;
    QAction *actTestPumpV2_cycle;
    QAction *actTestPumpV2_cycle_second;
    QAction *actTestErrorLed;
    QAction *actTestIMG1;
    QAction *actTestIMG3;
    QAction *actFrameToolCapCal;
    QAction *actFrameToolCal;
    QAction *actTechnicalSupport;
    QAction *actCorrectCenter;

signals:

private slots:
    void resetUI();//recieve the changeLanguage signal

    void camera_connected();
    void camera_disconnected();

protected:


};

extern class MainMenu *mainMenuBar;

#endif // MAINMENU_H
