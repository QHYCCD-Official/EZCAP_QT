#include "opencv2/highgui.hpp"

#include <QVector>
#include <QImage>

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
#  define QT_ENDL Qt::endl
#else
#  define QT_ENDL endl
#endif

#ifndef MYSTRUCT_H
#define MYSTRUCT_H

#define CALAB_YAU_PLANETARIUM 0

#define THP_File_Saved 0

//define camera status
#define Camera_Idle      0
#define Camera_Waiting   1
#define Camera_Exposing  2
#define Camera_Reading   3
#define Camera_Download  4
#define Camera_Error     5

//get frame status
#define GetSingleFrame_Failed   2
#define GetSingleFrame_Success  1
#define GetSingleFrame_Waiting  0

//define camera work mode
#define WorkMode_Preview   1
#define WorkMode_Focus     2
#define WorkMode_Capture   3

//define auto stretch mode
#define StretchMode_NoiseFloor          0
#define StretchMode_BackGroundLevel     1
#define StretchMode_3timesBackGround    2
#define StretchMode_10timesBackGround   3
#define StretchMode_MaxRange            4
#define StretchMode_OverScanX256        5
#define StretchMode_OverScanX128        6
#define StretchMode_OverScanX64         7
#define StretchMode_OverScanX32         8
#define StretchMode_OverScanX16         9
#define StretchMode_OverScanX8          10

//define CFW status
#define CFW_Moving   1
#define CFW_Idle     0

//define cross/grid/circle status
#define Cross_Enabled    1
#define Cross_Disabled   0
#define Grid_Enabled     1
#define Grid_Disabled    0
#define Circle_Enabled   1
#define Circle_Disabled  0

#define CIRCLE_RED    0
#define CIRCLE_GREEN  1
#define CIRCLE_BLUE   2
#define CIRCLE_YELLOW 3
#define CIRCLE_WHITE  4
#define CIRCLE_BLACK  5

//define FitHeader status
#define FitHeader_Add   0
#define FitHeader_Set   1

//define Planner status
#define PlannerStatus_Start  1
#define PlannerStatus_Stop   0
#define PlannerStatus_Done   2

//define Zoom mode
#define Zoom_FitWindow       0
#define Zoom_FillWindow      1
#define Zoom_SpecifyScaling  2

//define cooler mode
#define Cooler_Off           0
#define Cooler_Manual        1
#define Cooler_Auto          2

//define CFW waiting time  (ms)
#define CFW_WAITING_TIME  28000

//using namespace cv;

//前置声明
class QString;

struct IX
{
    QString lang;   //current language

    bool FoundCam;

    int autoStretchMode;  //auto stretch mode:
    unsigned long Histogram[256];//用于直方图像素值对照表
    unsigned char LUT_1[256][3];//伪彩色转换对照表
    unsigned char StretchLUT[65536];
    int StretchStep;//灰度拉伸调节幅度

    int  calConstant;

    bool saveTHPFile;

    ///
    /// Camera Read Mode
    ///
    //20200220 lyl Add ReadMode Dialog
//    unsigned int currentReadMode;   //current readmode
    uint32_t    ReadMode;
    uint32_t    ReadMode_Last;
    uint32_t    ReadMode_Num; //number of readmodes
    char        ReadMode_Name[50];
    QStringList ReadMode_List;       //保存readmode名称列表
    uint32_t    ReadMode_ImageW;
    uint32_t    ReadMode_ImageH;

    ///
    /// Camera Stream Mode
    ///
    uint32_t camStreamMode;
    uint32_t lastCamStreamMode;

    ///
    /// Camera info
    ///

    //camera ID & model
    QString CamID;
    QString CamModel;

    //Fireware version
    QString driverVer;

    //20200303lyl FPGA version
    QString FPGAVer;
    QString FPGAVer1;

    //screen max size
    uint32_t maxScreenW;
    uint32_t maxScreenH;

    //CCD chip info
    double   CCD_ChipW;
    double   CCD_ChipH;
    double   CCD_PixelW;
    double   CCD_PixelH;
    uint32_t CCD_ImageW;
    uint32_t CCD_ImageH;
    uint32_t CCD_ImageB;

    ///
    /// Image Format
    ///
    //binning
    bool Bin11_Fun;
    bool Bin22_Fun;
    bool Bin33_Fun;
    bool Bin44_Fun;
    bool Bin66_Fun;
    bool Bin88_Fun;
    uint32_t BinX;
    uint32_t BinY;
    uint32_t BinX_Last;
    uint32_t BinY_Last;
    uint32_t BinX_Max;
    uint32_t BinY_Max;
    //camera maximum iamge size under current read mode
    uint32_t ImageW_Max;
    uint32_t ImageH_Max;
    uint32_t Resolution_Max;
    //camera minimum iamge size compared between GetQHYCCDReadModeResolution and GetQHYCCDChipInfo
    uint32_t ImageW_Min;
    uint32_t ImageH_Min;
    uint32_t Resolution_Min;

    //effective area (just used for Capture mode)
    uint32_t EffectiveX;
    uint32_t EffectiveY;
    uint32_t EffectiveW;
    uint32_t EffectiveH;
    //overscan area (just used for Capture mode)
    uint32_t OverscanX;
    uint32_t OverscanY;
    uint32_t OverscanW;
    uint32_t OverscanH;
    //bool isOverScanCalibrated;
    bool IgnoreOverscan;
    bool CalibrateOverscan;

    //use to setup ROI
    uint32_t RoiX;
    uint32_t RoiY;
    uint32_t RoiW;
    uint32_t RoiH;
    uint32_t RoiX_Last;
    uint32_t RoiY_Last;
    uint32_t RoiW_Last;
    uint32_t RoiH_Last;

    //current image size bpp, channels
    uint32_t FrameW;
    uint32_t FrameH;
    uint32_t FrameB;
    uint32_t FrameC;
    uint32_t FrameW_Last;
    uint32_t FrameH_Last;
    uint32_t FrameB_Last;
    uint32_t FrameC_Last;
    uint32_t FrameW_Save;
    uint32_t FrameH_Save;
    uint32_t FrameB_Save;
    uint32_t FrameC_Save;

    uint32_t showLabelX = 0;
    uint32_t showLabelY = 0;
    uint32_t showLabelW;
    uint32_t showLabelH;

    //bits mode
    bool Bits8_Fun;
    bool Bits16_Fun;
    uint32_t Bits;
    uint32_t Bits_Last;

    //color mode
    bool Color_Fun;
    bool Color; //ON/OFF for SetQHYCCDDebayerOnOff
    bool Color_Last;
    bool IsCvtColor;
    int  Bayer;
    int  CamBayer;//彩色相机bayer矩阵格式

    //exposure setting   [ms]
    bool   ExpTime_Fun;
    double ExpUnit;
    double ExpUnit_Last;
    double ExpTime;
    double ExpTime_Last;
    double ExpTime_Max;
    double ExpTime_Min;
    double ExpTime_Step;

    //Gain setting
    bool   Gain_Fun;
    double Gain;
    double Gain_Last;
    double Gain_Max;
    double Gain_Min;
    double Gain_Step;

    //Offset setting
    bool   Offset_Fun;
    double Offset;
    double Offset_Last;
    double Offset_Max;
    double Offset_Min;
    double Offset_Step;

    //speed setting
    bool   Speed_Fun;
    double Speed;
    double Speed_Last;
    double Speed_Max;
    double Speed_Min;
    double Speed_Step;

    //Traffic
    bool   Traffic_Fun;
    double Traffic;
    double Traffic_Last;
    double Traffic_Max;
    double Traffic_Min;
    double Traffic_Step;

    //Brightness parameter
    bool   Brightness_Fun;
    double Brightness;
    double Brightness_Last;
    double Brightness_Max;
    double Brightness_Min;
    double Brightness_Step;

    //Contrast parameter
    bool   Contrast_Fun;
    double Contrast;
    double Contrast_Last;
    double Contrast_Max;
    double Contrast_Min;
    double Contrast_Step;

    //Gamma parameter
    bool   Gamma_Fun;
    double Gamma;
    double Gamma_Last;
    double Gamma_Max;
    double Gamma_Min;
    double Gamma_Step;

    //WBR
    bool   WBR_Fun;
    double WBR;
    double WBR_Last;
    double WBR_Max;
    double WBR_Min;
    double WBR_Step;

    //WBG
    bool   WBG_Fun;
    double WBG;
    double WBG_Last;
    double WBG_Max;
    double WBG_Min;
    double WBG_Step;

    //WBB
    bool   WBB_Fun;
    double WBB;
    double WBB_Last;
    double WBB_Max;
    double WBB_Min;
    double WBB_Step;

    //DDR
    bool DDR_Fun;
    bool DDR;
    bool DDR_Last;

    //AMPV
    bool AMPV_Fun;
    bool AMPV;
    bool AMPV_Last;

    //20200512lyl GPSon
    bool GPS_Fun;
    bool GPS;
    bool GPS_Last;
    QString GPS_LocalTime;

    bool Burst_Fun;
    bool Burst;
    int BurstCount;
    int BurstRepeat;
    int BurstGroup;

    //cooler setting
    bool   Cooler_Fun;
    int    Cooler_Mode; //the cooler control mode, 0 disalble; 1:manual; 2:auto control
    double Temp_Now; //当前监控到的温度
    bool   FPGATemp_Fun;
    double FPGATemp_Now;
    double PWM_Now;//当前功率
    int    Voltage_Now;//当前电压
    double Temp_Target;
    int    Voltage_Target;

    //20200318 humidity
    bool Humidity_Fun;
    double Humidity;//当前湿度

    //pressure
    bool Pressure_Fun;
    double Pressure ;//当前压力









    //shutter
    bool canMechanicalShutter;
    int MechanicalShutterMode;
    int LastMechanicalShutterMode;

    //FineTone
    bool canFineTone;//是否支持FineTone
    bool fineToneOnOff;

    //MotorHeating
    bool canMotorHeating;//是否支持MotorHeating
    bool motorHeatingOnOff;

    //TECProtect
    bool canTecOverProtect;
    bool tecPretect;

    //CLAMP
    bool canSignalClamp;
    bool clamp;

    //Calibrate FPN
    bool canCalibrateFPN;
    bool calibrateFPNOnOff;
    bool isCalibrateFrame;//是否正在进行帧校准

    //Slowest Download
    bool canSlowestDownload;
    bool slowestDowload;

    //chip temperature
    bool canChipTemp;
    bool chipTempOnOff;

    //Triger
    bool canTriger; //外触发支持
    uint32_t trigerMode;
    bool trigerIn;
    bool trigerOut;
    unsigned int  trigerInterface;
    QStringList trigerInterfaceList;

    //CFW
    bool canFilterWheel;//是否支持ColorWheel
    bool CFW_Plugged;
    char dstCfwPos;//色轮目标位置
    char curCfwPos[64];// 色轮当前位置
    int CFWStatus;// 0:闲置/运行完成， 1：正在运行
    int CFWSlotsNum;
    int CFWCalculateValue;
    QVector<QString> filterNames_2;

    bool locked;

    //for single and live
    unsigned char *ImgData;          //存放从相机获取到的图像数据，只存储不用于图像处理
    unsigned char *ImgData_Last;      //用来保存需要显示的图像数据，拷贝自ix.ImgData
    unsigned char *ImgData_GPS;
    unsigned char *ImgData_Save;      //存放用来保存的图像数据
    unsigned char *ImgData_Dark;      //存放暗场数据
    uint16_t      *ImgData_FB;      //存放Flat Bias数据
    uint32_t      *ImgData_CalSave;

    bool saveFlag;

    bool saveVideo;
    bool canSaveVideo;

    bool darkCapOnOff;
    bool darkSave;
    bool canDarkSave;
    uint32_t darkNum;
    uint32_t addedNum;
    bool darkCal;

    bool calCapOnOff;
    bool calSave;
    bool canCalSave;
    uint32_t calNum;
    bool frameCal;
    double avgFlatBias1;
    double avgFlatBias2;
    double avgFlatBias3;

    bool canReadVideo;
//    bool quit;
//    bool hasquit;
    bool canClose;
    QString videoPath;
    cv::Mat videoImage;

    QString filename;
    cv::VideoWriter video;

    QImage *showImage;

    ///
    /// Software function
    ///
    // camera connect status
    bool isConnected;  //camea is connected or not

    // camera work status
    int cameraState;  //camera status ,one of the following status : 0 CameraIdle; 1 CameraWaiting; 2 CameraExposing; 3 CameraReading; 4 CameraDownload ; 5 CameraError

    int workMode;    //camer workmode     1：preview，2：focus，3：capture
    int lastWorkMode;

    int imageReady;  // 1 - get frame success; 2 - get frame failed; 0 - wait for downlaod frame

    int crossBtnState; //cross status, 0：hide cross, 1: show cross
    int gridBtnState; //grid status, 0：hide grid, 1: show grid
    int circleBtnState; //circle status, 0：hide circle, 1: show circle

    int fitHeadEditState;//fit头编辑状态 0: add; 1:set

    int plannerState; //planner status, one of the following status： 2 Done/Idle;  1 Start; 0 Stop

    //FPS
    double fps;
//    int fps1;             //整数部分
//    int fps2;             //小数部分
//    int timeStart1;
//    int timeEnd1;
    long long time;        //计时开始时间
    long long timeLast;          //计时结束时间
    long long timeCount;
    int frame;
    int frameLast;
    int frameCount;

    bool ForceStop;//标记强制停止曝光
    bool onLiveMode; //a flag indicating whether EZCAP is working on PreviewLive/FocusLive mode

//    int imgShowWidth;
//    int imgShowHeight;
    double scaleFactor;   //缩放比例

    int zoomMode;  //缩放模式， 0-自动缩放； 1-指定缩放比例
    int lastZoomMode;

    QString dateOBS;

    //20200512 lyl OSD
    QStringList OSDList;       //保存OSD名称列表
    //20201127 lyl SensorChamberCyclePUMP
    bool canContolSensorChamberCyclePUMP;
    bool cyclePUMBStatus;

    bool Circle_Correct;
    int Circle_Radius;
    int Circle_Spacing;
    int Circle_Thickness;
    int Circle_Color;
    int circle1;
    int circle2;

    double avgTotal;
    double avgLocal;
};

struct FOCUSINFO
{
    unsigned char MaxPixel;
    unsigned char MinPixel;
    unsigned char DeltaPixel;
    unsigned char CenterX;
    unsigned char CenterY;

    unsigned char FWHM_X1;
    unsigned char FWHM_X2;
    unsigned char FWHM_Y1;
    unsigned char FWHM_Y2;
    unsigned char FWHM_ResultX;
    unsigned char FWHM_ResultY;
    unsigned char FWHM_Result;
    unsigned char width;
    unsigned char height;
    unsigned char Row[1000];   // x1 x2
    unsigned char Column[1000]; // y1 y2
};

struct INIFILEPARAM
{
    bool iniFileExist;

    QString lang;
    int Gain;
    int Offset;
    bool tecPretect;
    bool slowestDowload;
    bool clamp;
    unsigned short bPos_Preview;
    unsigned short wPos_Preview;
    unsigned short bPos_Focus;
    unsigned short wPos_Focus;
    unsigned short bPos_Capture;
    unsigned short wPos_Capture;
    int autoStretchMode;
    bool ignoreOverScan;
    bool calibrateOverScan;
    unsigned short calConstant;
    int CFWSlotsNum;
    QVector<QString> filterNames_2;

    bool testGuider;
    bool autoConnect;
    bool autoConnectLive;
    bool fullScreen;
    bool enableMsgClient;
    QString msgClientName;
    bool oldSDK;

};

extern struct IX ix;
extern struct FOCUSINFO FocusInfo;
extern struct INIFILEPARAM iniFileParams;

#endif // MYSTRUCT_H
