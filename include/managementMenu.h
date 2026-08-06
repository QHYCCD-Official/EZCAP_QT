#ifndef MANAGEMENTMENU_H
#define MANAGEMENTMENU_H

#include "threadTempControl.h"
#include <QWidget>
#include <QMenu>

class QSlider;
class QLineEdit;

namespace Ui {
class ManagementMenu;
}

class ManagementMenu : public QWidget
{
    Q_OBJECT

public:
    explicit ManagementMenu(QWidget *parent = 0);
    ~ManagementMenu();

    Ui::ManagementMenu *ui;

    QMenu *cmenu_captureExp;//capture曝光滑动条 右键菜单

    int DoubleToInt(double d);
    int SwitchReadmodeBinFormat();
    int ResetParameters();
    int CloseCamera();
    void CollapseSettingPanels(); /* fold live panels while applying readmode/bin/bits/roi */
    void RestoreSettingPanels();  /* restore panels after setting finishes */

    // 将曝光(us)约束到 ExpTime_Min/Max，同步 ix.ExpTime 及对应滑条/输入框，返回约束后的 us
    double applyClampedExposure(double expUs, double unit, QSlider *slider = nullptr, QLineEdit *lineEdit = nullptr);

private slots:

    void resetUI();//recieve the changeLanguage signal

    void camera_connected();
    void camera_disconnected();

    //-------------preview--------------
    void on_head_preview_clicked(bool checked);

    void on_hSlider_Gain_preview_valueChanged(int value);

    void on_lineEdit_Gain_preview_textChanged(const QString &arg1);

    void on_hSlider_Offset_preview_valueChanged(int value);

    void on_lineEdit_Offset_preview_textChanged(const QString &arg1);

    void on_comBoxPreviewUnit_currentTextChanged();

    void on_hSlider_exposure_preview_valueChanged(int value);

    void on_lineEdit_exposure_preview_textChanged(const QString &arg1);

    //------------------focus---------------------
    void on_head_focus_clicked(bool checked);

    void on_hSlider_Gain_focus_valueChanged(int value);

    void on_lineEdit_Gain_focus_textChanged(const QString &arg1);

    void on_hSlider_Offset_focus_valueChanged(int value);

    void on_lineEdit_Offset_focus_textChanged(const QString &arg1);

    void on_comBoxFocusUnit_currentTextChanged();

    void on_hSlider_exposure_focus_valueChanged(int value);

    void on_lineEdit_exposure_focus_textChanged(const QString &arg1);

    //------------------capture--------------------
    void on_head_capture_clicked(bool checked);

    void on_comboBox_readmode_capture_currentIndexChanged(int index);

    void on_comboBox_color_capture_currentIndexChanged(const QString &arg1);

    void on_hSlider_Gain_capture_valueChanged(int value);

    void on_lineEdit_Gain_capture_textChanged(const QString &arg1);

    void on_hSlider_Offset_capture_valueChanged(int value);

    void on_lineEdit_Offset_capture_textChanged(const QString &arg1);

    void on_hSlider_exposure_capture_valueChanged(int value);

    void on_lineEdit_exposure_capture_textChanged(const QString &arg1);

    void on_hSlider_exposure_capture_customContextMenuRequested(const QPoint &pos);

    void setCaptureExp1s();
    void setCaptureExp5s();
    void setCaptureExp10s();
    void setCaptureExp30s();
    void setCaptureExp60s();
    void setCaptureExp120s();
    void setCaptureExp180s();
    void setCaptureExp240s();
    void setCaptureExp5min();
    void setCaptureExp10min();
    void setCaptureExp15min();
    void setCaptureExp30min();
    void setCaptureExp0s();

    void on_comBoxSingleUnit_currentTextChanged();

    void on_checkBox_highSpeed_toggled(bool checked);

    void on_bin1x1_toggled(bool checked);

    void on_bin2x2_toggled(bool checked);

    void on_bin3x3_toggled(bool checked);

    void on_bin4x4_toggled(bool checked);

    void on_bin6x6_toggled(bool checked);

    void on_bin8x8_toggled(bool checked);

    //-------------------------save------------------------
    void on_head_save_clicked(bool checked);

    //---------------------image format-------------------
    void on_head_liveimageformat_clicked(bool checked);

    void on_comBoxLiveReadMode_currentIndexChanged(int index);

    void on_comBoxLiveBin_currentTextChanged(const QString &arg1);

    void on_comBoxLiveBits_currentTextChanged(const QString &arg1);

    void on_comBoxLiveColor_currentTextChanged(const QString &arg1);

    //--------------------camera setup--------------------
    void on_head_livecamerasetup_clicked(bool checked);

    void on_sliderLiveExposure_valueChanged(int value);

    void on_comBoxLiveUnit_currentIndexChanged(int index);

    void on_lineEditLiveExp_textChanged(const QString &arg1);

    void on_sliderLiveGain_valueChanged(int value);

    void on_lineEditLiveGain_textChanged(const QString &arg1);

    void on_sliderLiveOffset_valueChanged(int value);

    void on_lineEditLiveOffset_textChanged(const QString &arg1);

    void on_sliderLiveTraffic_valueChanged(int value);

    void on_lineEditLiveTraffic_textChanged(const QString &arg1);

    void on_sliderLiveSpeed_valueChanged(int value);

    void on_lineEditLiveSpeed_textChanged(const QString &arg1);

    void on_comBoxLiveDDR_currentTextChanged(const QString &arg1);

    void on_comBoxLiveAMPV_currentTextChanged(const QString &arg1);


    //--------------------image setup---------------------
    void on_head_liveimagesetup_clicked(bool checked);

    void on_sliderLiveWBR_valueChanged(int value);

    void on_lineEditLiveWBR_textChanged(const QString &arg1);

    void on_sliderLiveWBG_valueChanged(int value);

    void on_lineEditLiveWBG_textChanged(const QString &arg1);

    void on_sliderLiveWBB_valueChanged(int value);

    void on_lineEditLiveWBB_textChanged(const QString &arg1);

    void on_sliderLiveBrightness_valueChanged(int value);

    void on_lineEditLiveBrightness_textChanged(const QString &arg1);

    void on_sliderLiveContrast_valueChanged(int value);

    void on_lineEditLiveContrast_textChanged(const QString &arg1);

    void on_sliderLiveGamma_valueChanged(int value);

    void on_lineEditLiveGamma_textChanged(const QString &arg1);

    //-------------------------ROI-------------------------
    void on_head_Roi_clicked(bool checked);

    void on_comboBox_RoiSaved_currentIndexChanged(const QString &arg1);

    void on_lineEdit_sizex_Roi_textChanged(const QString &arg1);

    void on_lineEdit_sizey_Roi_textChanged(const QString &arg1);

    void on_pushButton_SetROI_clicked();

    void on_pushButton_SaveROI_clicked();

    //---------------------screen view---------------------
    void on_head_screenView_clicked(bool checked);

    //------------------------hist------------------------
    void on_head_hist_clicked(bool checked);

    void on_hSlider_wPos_valueChanged(int value);

    void on_hSlider_bPos_valueChanged(int value);

    void on_cBox_autoStretchList_currentIndexChanged(int index);

    void on_pBtn_coarse_clicked();

    // void on_pBtn_retransfer_clicked();



signals:
    void switchWorkMode(int workmode);

private:
    QString lastBayer;

    bool saveStatus;
    bool cameraSetupStatus;
    bool imageSetupStatus;
    bool screenViewStatus;
    bool histStatus;
    bool RoiStatus;
};

extern ManagementMenu *managerMenu;

#endif // MANAGEMENTMENU_H
