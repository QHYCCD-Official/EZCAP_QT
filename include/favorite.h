#ifndef FAVORITE_H
#define FAVORITE_H

#include "threadTempControl.h"
#include <QDialog>

namespace Ui {
class Favorite;
}

class Favorite : public QDialog
{
    Q_OBJECT

public:
    explicit Favorite(QWidget *parent = 0);
    ~Favorite();

    /*设置motor heating 可控*/
    void setMotorHeatingEnable(bool enable);
    /*设置fine tone 可控*/
    void setFineToneEnable(bool enable);

    void set9979Finetone();

private slots:
    void setFineToneValue(int value1, int value2, int value3, int value4, int value5, int value6, int value7, int value8);

    void on_hSlider_1_favorite_valueChanged(int value);

    void on_hSlider_2_favorite_valueChanged(int value);

    void on_hSlider_3_favorite_valueChanged(int value);

    void on_hSlider_4_favorite_valueChanged(int value);

    void on_hSlider_5_favorite_valueChanged(int value);

    void on_hSlider_6_favorite_valueChanged(int value);

    void on_hSlider_7_favorite_valueChanged(int value);

    void on_hSlider_8_favorite_valueChanged(int value);

    void resetUI();//recieve the changeLanguage signal

    void on_saveFineTone_favorite_clicked();

    void on_hSlider_1_favorite_9979_valueChanged(int value);

    void on_hSlider_2_favorite_9979_valueChanged(int value);

    void on_hSlider_3_favorite_9979_valueChanged(int value);

    void on_cBox_SHPorSHD_9979_toggled(bool checked);

    void on_cBox_trigIn_clicked(bool checked);

    void on_spinBox_Traffic_valueChanged(int arg1);

    void camera_connected();

    void on_spinBox_calConstant_valueChanged(int arg1);

    void on_cBox_GPS_clicked(bool checked);

    void on_comboBox_OSD_currentIndexChanged(int index);

    //void on_pBtn_controlSensorChamberCyclePUMP_clicked();

    void on_pushButton_sensor_ulvo_clicked();

    void on_pushButton_retrain_clicked();

    void on_pushButton_sensor_ulvo_getFlash_clicked();

    void on_pushButton_sensor_ulvo_resetFlash_clicked();

    void on_pushButton_sensor_ulvo_testError_clicked();

//    void on_pushButton_debug_timer_off_clicked();

    void on_pushButton_sensor_ulvo_erase_flash_clicked();

    void on_pushButton_sensor_ulvo_debug_d3_clicked();

    void on_checkBox_debug_stateChanged(int arg1);

    void on_checkBox_Is_Test_stateChanged(int arg1);

    void on_checkBox_temperature_timer_stateChanged(int arg1);

    void on_comBox_Interface_currentIndexChanged(int index);

    void on_cBox_trigOut_clicked(bool checked);

    void on_ckBox_EnGlobalReset_stateChanged(int arg1);

    void on_checkBox_Circle_stateChanged(int arg1);

    void on_hSlider_Circle_valueChanged(int value);

    void on_spinBox_Circle_valueChanged(int arg1);

    void on_pushButton_uart_cmd_send_rev_clicked();

    void on_pushButton_uart_cmd_foc46_clicked();

    void on_pushButton_uart_cmd_len0a_clicked();

    void on_pushButton_uart_cmd_foc45_clicked();

    void on_pushButton_uart_cmd_aperture1_clicked();

    void on_pushButton_uart_cmd_aperture2_clicked();

    void on_pushButton_uart_cmd_cfw601_clicked();

    void on_pushButton_uart_cmd_cfw1_test_clicked();

    void on_pushButton_uart_cmd_cfw2_test_clicked();

    void on_pushButton_uart_cmd_cfw600_clicked();

    void on_pushButton_ArrayCamSync_clicked();

    void on_pushButton_uart_cmd_screen_clicked();

    void on_pushButton_uart_cmd_vrs_clicked();

    void on_pushButton_uart_cmd_debug_clicked(bool checked);

    void on_pushButton_uart_cmd_retransfer_clicked();

public:
    Ui::Favorite *ui;
};

extern class Favorite *favorite_dialog;//定义全局类对象，供主界面类中使用

#endif // FAVORITE_H
