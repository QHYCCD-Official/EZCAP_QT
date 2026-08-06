#ifndef EZCAP_H
#define EZCAP_H

#include <QMainWindow>
#include <QThread>
#include <QEvent>
#include <QPoint>
#include "opencv2/opencv.hpp"
#include <QVBoxLayout>
#include <QTranslator>
#include <myStruct.h>
#include <qhyccdstruct.h>
#include "disktools.h"
#include <QUdpSocket>
#include "msgclient.h"


//#define CALAB_YAU_PLANETARIUM


#ifdef CALAB_YAU_PLANETARIUM
 #include <QtWebSockets/QWebSocketServer>
 #include <QtWebSockets/QWebSocket>
 #include <QNetworkInterface>
#endif

#ifdef WIN32
#include <windows.h>

//#endif
//#ifdef LINUX
//#include <sys/stat.h>
#else
#include <unistd.h>
#endif

using namespace cv;

class BorderLayout;

class QImage;
class QScrollArea;
class QScrollBar;
class QLabel;
class QMenu;

class MenuForm;
class DownloadPreThread;
class DownloadCapThread;
class DownloadFocThread;
class LiveCapThread;
class VideoShowThread;
class ThreadTempControl;
class ThreadProcessImage;
class PixelMagnifierWidget;

//-----------执行计划任务线程类-------
class ExecutePlanTable : public QThread
{
    Q_OBJECT

protected:
    void run();

signals:
    void changeRowColor(int row, QColor color);
    void startCaptureImage();
    void showErrorInfo(QString,QString);
    void finish_baisImages();

    void changeCurCFWPos(int dstIndex);

};

//-----------发送滤镜轮控制命令-------
class ExecuteCFWOrder : public QThread
{
    Q_OBJECT

protected:
    void run();

public:
};

//------------查询色轮状态线程-----------

//---------------------------

namespace Ui {
class EZCAP;
}
/**
 * @brief The EZCAP class
 */
class EZCAP : public QMainWindow
{
    Q_OBJECT

public:
    explicit EZCAP(QWidget *parent = 0);
    ~EZCAP();

    bool triggerRetransferAndReceiveFrame();
    bool triggerDdrRetransferAndReceiveFrame();
    void resetFrameCount();

    QString EZCAP_VER;
    QString EZCAP_VER_SHORT;
    QString RELEASE_TIME;
    static const bool TESTED_PID;
    //static const int CFW_WAITING_TIME;

    bool getParamsFromCamera();//连接相机后，设置界面控件初始状态
    void scaleImage(double factor);//图像显示规模改变（zoom中设置）

    /*Mat转QImage*/
    QImage *MatToQImage(const cv::Mat image);
    /*QImage转Mat*/
    cv::Mat QImageToMat(const QImage *qImage);

    //显示单帧模式图像
    void displaySingleFrame(uint32_t imgw, uint32_t imgh, unsigned char *imgdata);
    //显示缩略图图像
    void displayScreenViewImage(int boxWidth,int boxHeight,int boxX,int boxY);
    //重绘缩略图框选位置和尺寸
    void screenViewBoxResize();
    //显示直方图
    void displayHistogramImage(int x,int y,unsigned char *buf);

    //显示focus图像
    void displayFocusImage(int x,int y);
    void displayFocusImage_Ex(int x, int y, unsigned char *dataBuf);
    //显示FocusAssistant区域的图像
    void displayFocusAssistantImage(cv::Mat image);
    //获取Focus信息值
    void FWHMFocus(cv::Mat Img,FOCUSINFO &FocusInfo);
    //在QImage上绘制网格线
    void DrawGridBox(QImage *img);
    //伪彩色转换
    void FalseColorConvert(cv::Mat inputImg, cv::Mat outputImg);
    //加载伪彩色图片
    void LoadFalseColor(QString LUTMAP);

    //保存图像前检查路径是否安全
    bool CheckWritePath(QString filename);

    bool SearchCamFromIni(QString camName);
    void saveParasAsIni();
    bool loadParasFromIni();
    void saveParamToIni(QString group, QString key, QString  value);
    void saveParamToIni(QString group, QString key,     int  value);
    void saveParamToIni(QString group, QString key,  double  value);
    void saveParamToIni(QString group, QString key,    bool  value);
    void loadParamFromIni(QString group, QString key, QString *value, QString defaultValue);
    bool loadParamFromIni(QString group, QString key,     int *value,     int defaultValue);
    bool loadParamFromIni(QString group, QString key,  double *value,  double defaultValue);
    bool loadParamFromIni(QString group, QString key,    bool *value,    bool defaultValue);


    void languageChanged();

    void setStretchLUT(unsigned short W, unsigned short B);

    void getOverScanBlack(unsigned char *Buf, int x, int y);
    void getImageInfo(unsigned char *Buf,int ImageWidth,int ImageHeight, int startX,int startY,int sizeX,int sizeY, float &std,float &rms,double &max,double &min);
    void ImageAnalyze(cv::Mat Img,int x,int y);

    QString AutoFileName();

    //-------色轮状态控制定时器-------
    void startCFWTimer();
    void stopCFWTimer();
    //-------dither状态查询定时器 启动控制----
    void startDitherTimer();
    void stopDitherTimer();
    //-------循环泵定时器------
    void startPumpTimer();
    void stopPumpTimer();

    void updateWindowsTitle();
    void pnpEventFunc();
    void update_device_status(char *id,int dev_status);

    void saveCal();

    void adjustScrollBar(uint32_t w, uint32_t h);

private slots:
    /*File 菜单中的槽*/
    void saveAsFIT();
    void saveAsBMP();
    void saveAsJPG();
    void saveAsTIF();
    void saveAsPNG();
    void saveVideo();
    void saveImage2Video();
    void saveSnapShot();
    void saveSnapShot2Image();
    void saveCal2Image();
    void openVideo();
    void displayLiveImage();
    void openFolder();
    void showFITHeaderEditor();
    void ignoreOverScanAreaClicked(bool checked);
    void calibrateOverScanClicked(bool checked);
    void enableSaveTHPFile(bool checked);
    void exitMainWindow();
    /*connect action 槽*/
    void showCameraChooser();
    /*planner 菜单中的槽*/
    void showPlanTable();
    void imageRotateMirror();
    /*camera setup 菜单中的槽*/
    void showFavoriteSetting();
    void showGPSTool();
    void showToolBurst();
    void showToolTrigger();
    void showPHDLink();
    void showCaptureDarkFrameTool();
    void darkFrameCalibration();
    void showToolCorrectCenter();
    void showImgAnalyze();
    void showTempControl();
    void cfwPositionChanged();
    void showCFWControl();
    void showOtherCameraSetup();
    /*help 菜单中的槽*/
    void showAbout();
    void showManual();
    void technicalSupport();
    /*zoom 菜单中的槽*/
    void zoomFitWindow();
    void zoomFillWindow();
    void zoom0_25X();
    void zoom0_5X();
    void zoom0_75X();
    void zoom1X();
    void zoom1_5X();
    void zoom2X();

    void showFPS();
    void showFrameCount();
    void retransferDownloadFinished();

    void displayVideoImage();

    void displayHistogramImageLive();

    //preview/focus/capture当前活动页改变响应槽函数
    void currentWorkingModeChanged(int workmode);

    //preview tab页的槽函数
    void mgrMenu_pBtn_preview_clicked();
    void mgrMenu_pBtn_live_preview_clicked();
    void mgrMenu_pBtn_cross_clicked();
    void mgrMenu_pBtn_grid_clicked();
    void mgrMenu_pBtn_circle_clicked();

    //focus tab页的槽函数
    void mgrMenu_pBtn_focus_clicked();
    void mgrMenu_pBtn_live_focus_clicked();

    //focus assistant页槽函数
    void on_pBtn_linear_clicked();
    void on_pBtn_thermal_clicked();
    void on_pBtn_false_clicked();
    void on_pBtn_invert_clicked();
    void focusAssistantImageDblClick();

    //capture tab页的槽函数
    void mgrMenu_pBtn_capture_clicked();
    void mgrMenu_pBtn_stop_clicked();
    void mgrMenu_hSlider_bPos_sliderReleased();
    void mgrMenu_hSlider_wPos_sliderReleased();
    void mgrMenu_pBtn_stretchMinusB_clicked();
    void mgrMenu_pBtn_stretchPlusB_clicked();
    void mgrMenu_pBtn_stretchMinusW_clicked();
    void mgrMenu_pBtn_stretchPlusW_clicked();
    void mgrMenu_pBtn_auto_histogram_clicked();

    void favorite_pBtn_calibrateFrame_clicked();
    void favorite_pBtn_getRealTemp_clicked();
    void favorite_pBtn_controlSensorChamberCyclePUMP_clicked();

    /*screenview 鼠标左击响应槽*/
    void screenViewAreaMouseDown(int posX, int posY);

    /*点击图像区域*/
    void displayedImageMouseDown(int posX, int posY);
    /*图像显示区域label_imgShow 右键菜单*/
    void on_label_ImgShow_customContextMenuRequested(const QPoint &pos);

    void goFocusCenter();
    //void doTempControl();
    //定时器响应槽
//    void tempTimer_timeout();
    void cfwTimer_timeout();
    void ditherTimer_timeout();
    void PumpTimer_timeout();
    void PumpV2CycleTimer_timeout();
    void PumpV2CycleSecondTimer_timeout();
    void initLibqhyccd();
    //切换显示语言
    void changeToEnglish();
    void changeToChinese();
    void changeToJapanese();
    void activeTestMode();
    void switchDebug();
    void switchTestGuid();
    void switchTestPumpV2(bool checked);
    void switchTestPumpV2_second(bool checked);
    void switchTestPumpV2_cycle(bool checked);
    void switchTestPumpV2_cycle_second(bool checked);
    void switchTestErrorLed(bool checked);
    void switchTestIMG1(bool checked);
    void switchTestIMG3(bool checked);

    void hScrollBarValueChanged(int value);
    void vScrollBarValueChanged(int value);

    void camera_connected();
    void camera_disconnected();

signals:
    //改变显示语言信号
    void changeLanguage();
    void connect_camera();
    void disconnect_camera();

    void change_fitHeaderInfo();

//    void updateGPSInfo();

protected:
    void showEvent(QShowEvent *);

    void closeEvent(QCloseEvent * event );

    void resizeEvent(QResizeEvent *);

    bool eventFilter(QObject *target, QEvent *event);
    bool mapHoverPointToImage(const QPoint &labelPos, const cv::Mat &image, QPoint &imagePos, QPoint &matPos);
    bool updateHoverLabelPosFromCursor();
    bool readRawGrayPixel(const QPoint &imagePos, int &gray);
    void updateHoverPixelStatus(const QPoint &labelPos, const cv::Mat &image);
    void clearHoverPixelStatus();
    void updatePixelMagnifier(const QPoint &labelPos);//更新像素放大镜（悬停区域放大图+像素值+区域统计）
    void hidePixelMagnifier();
    QImage sampleMagnifierRegion(const QPoint &imagePos, int regionSize);//从显示内容采样 NxN 区域图
    bool computeRegionStats(const QPoint &imagePos, const QImage &region, QString &pixelText, QStringList &statLines);//计算像素值与区域统计

public:
    Ui::EZCAP *ui;

    QTranslator *translator;

    BorderLayout *mainLayout;
    QVBoxLayout *managerLayout;

    //----------定义线程对象-------------
    ExecutePlanTable  *exePlanTable;
    DownloadPreThread *downloadPre;
    DownloadCapThread *downloadCap;
    DownloadFocThread *downloadFoc;
    LiveCapThread     *liveCap;
    VideoShowThread   *videoShowThread;
    ThreadTempControl *threadTempControl;
    ThreadProcessImage *threadProcessImage;

   //-----------滤镜轮调用API改成阻塞型，因此需要放入线程执行-------------
    ExecuteCFWOrder *runCFWOrder;

    //主管理面板的滚动条区域对象
    QScrollArea *scrollArea_ImgShow;//显示图像的滚动条区域对象
    QScrollArea *scrollArea_ImgShow_Total;//显示图像的滚动条区域对象
    QGridLayout *gridLayout_ImgShow_Total;

    QLabel *statusLabel_imgSize;//显示当前图像分辨率的label对象
    QLabel *statusLabel_mousePos;//显示当前鼠标位置坐标的label对象
    QLabel *statusLabel_rgb;//显示当前位置图像rgb值的对象
    QLabel *statusLabel_Temp;//显示当前相机温度
    QLabel *statusLabel_FPGATemp;//显示当前 FPGA 温度
    QLabel *statusLabel_RH;//显示当前密封腔内的湿度
    QLabel *statusLabel_PRESS;//显示当前压力
    QLabel *statusLabel_msg;//显示提示信息
    QLabel *statusLabel_SDKmsg;//显示提示信息
    QLabel *statusLabel_dev_status;//显示设备状态
    QLabel *statusLabel_frame_status;// frame debug  message
    QLabel *statusLabel_ImgMean;
    QMenu *cmenu_captureExp;//capture曝光滑动条 右键菜单
    QMenu *cmenu_imgArea;//图像显示区域右键菜单

    MenuForm *menuForm;
    QTimer *cfwTimer;//色轮状态查询定时器
    QTimer *ditherTimer; //PHD2 dither状态查询定时器
    QTimer *PumpTimer;//循环泵开关定时器
    QTimer *PumpV2CycleTimer;//Test Pump V2 Cycle定时器
    QTimer *PumpV2CycleSecondTimer;//Test Pump V2 Cycle Second定时器
    QTimer *updateImgTimer;
    QTimer *updateFrameTimer;
    int pumpcount;

    cv::Mat ImgShow;
    cv::Mat ImgView;
    cv::Mat ImgHist;
    QPoint hoverLabelPos;
    bool hoverLabelPosValid;
    PixelMagnifierWidget *pixelMagnifier;//像素放大镜浮动控件

    QImage *qImg_show;
    QImage *qImg_focus;
    QImage *qImg_video;

    //---Focus
    unsigned short FocusCenterX_Pre;  //the focus center point in Preview image
    unsigned short FocusCenterY_Pre;

    int focusAreaStartX;   //focus area
    int focusAreaStartY;
    int focusAreaSizeX;
    int focusAreaSizeY;
    int SubX0;
    int focusAreaW;
    int focusAreaH;

    //---FocusAssistant
    int ZoomFocus_X;  //focus center point in FocusAssistantImage
    int ZoomFocus_Y;
    bool FocusZoomMode;
    int FocusCurveX;
    int fwhm_x;//focus assistant中fwhm图的起始点坐标
    int fwhm_y;
    int peak_x;//focus assistant中peak图的起始点坐标
    int peak_y;

    //-----stretch
    unsigned short OverScanRMS; //overscan black value, used for Auto hist.

    unsigned short Preview_WPOS;
    unsigned short Preview_BPOS;
    unsigned short Focus_WPOS;
    unsigned short Focus_BPOS;
    unsigned short Capture_WPOS;
    unsigned short Capture_BPOS;
    unsigned short Live_WPOS;
    unsigned short Live_BPOS;

    //-------rectange area size and center point in the screen view------
    int viewBoxCX;
    int viewBoxCY;
    int viewBoxW;
    int viewBoxH;

    bool isSettleDone; //标记dither功能中，Settle是否完成

    //save
    QString lastSavedPath;

    bool noImgInWorkMode;  //标志当前工作模式下是否已拍摄了图片，用于避免从preview拍摄图像后，切换到capture点击拍摄，未拍摄完成时进行灰度拉伸会刷新显示preview中拍摄的图片。

    bool canConnect = false;

    QStringList devList; //list to hold on the id of the cameras
#if THP_File_Saved
    QString fileTHP;//20201125lyl温度湿度压力数据保存
#endif

    int doubleClick = 0;

    void UDPServerScan();
    void clientReadMessage();
    void on_avi_action(bool startOrStop);
    void on_windows_resize_action(bool maxOrNormal);
    void on_network_error(QString errorMessage);



#ifdef CALAB_YAU_PLANETARIUM
//websocket p2p connection
    void decodeTextMessage(QString message);
    void sendQMountInfo(void);


private:

    QTimer *wstimer;
    QWebSocketServer::SslMode _sslMode;
    QWebSocketServer *_wss;
    QHash<QString,QWebSocket*> _hashIpPort2PWebSocket;
    CvCapture *mjpeg_capture;
    QTcpSocket *tcpSocket ;





public slots:
   void wss_start(QHostAddress hostAddress = QHostAddress(QHostAddress::Any),qint32 port = 8800);
   void wss_stop();
   void wss_listAllConnection();
   void wss_sendText(QString ip,qint32 port , QString message);
   void wss_sendByte(QString ip,qint32 port , QByteArray message);
   QString wss_getState(QString ip,qint32 port);
   bool wss_checkAlive(QString ip,qint32 port);

protected slots:
   void wss_newconnection();
   void wss_serverError(QWebSocketProtocol::CloseCode closeCode);
   void wss_closed();

   void wss_disconnected();
   void wss_error(QAbstractSocket::SocketError error);
   void wss_textFrameReceived(const QString &frame, bool isLastFrame);
   void wss_textMessageReceived(const QString &message);
   void wss_binaryMessageReceived(const QByteArray &message);

   void wss_pong(quint64 elapsedTime, const QByteArray &payload);
   void wss_stateChanged(QAbstractSocket::SocketState state);


private slots:
    void onDisconnected();
    void onConnected();
    void onTextReceived(QString msg);
    void reconnect();
#endif

    void on_horizontalScrollBar_ImgShow_valueChanged(int value);
    void on_verticalScrollBar_ImgShow_valueChanged(int value);
};

extern class EZCAP *mainWidget;//define global class object

#endif // EZCAP_H
