#ifndef GPSTOOL_H
#define GPSTOOL_H

#include <QDialog>
#include <QDateTime>
#include <QElapsedTimer>

namespace Ui {
class gpsTool;
}

class gpsTool : public QDialog
{
    Q_OBJECT

public:
    explicit gpsTool(QWidget *parent = nullptr);
    ~gpsTool();

private slots:
    void on_comBox_GPSOnOff_currentTextChanged(const QString &arg1);

    void on_comBox_GPSFreq_currentTextChanged(const QString &arg1);

    void on_comBox_GPSSlaveOnOff_currentTextChanged(const QString &arg1);

    void on_pBtn_GPSSetUTC_clicked();

    void updateGPSInfo();

    void on_pBtn_GPSSyncSystemTime_clicked();

    void syncSystemTimeToGPS();

    void syncSystemTimeOnTimer();

    void on_comBox_LEDCalMode_currentTextChanged(const QString &arg1);

    void on_spBox_GPSStart1_valueChanged(int arg1);
    void on_spBox_GPSStart2_valueChanged(int arg1);
    void on_spBox_GPSStart3_valueChanged(int arg1);
    void on_spBox_GPSStart4_valueChanged(int arg1);
    void on_spBox_GPSStart5_valueChanged(int arg1);
    void on_spBox_GPSStart6_valueChanged(int arg1);
    void on_spBox_GPSStart7_valueChanged(int arg1);
    void on_spBox_GPSStart8_valueChanged(int arg1);
    void on_spBox_GPSStart9_valueChanged(int arg1);

    void on_spBox_GPSEnd1_valueChanged(int arg1);
    void on_spBox_GPSEnd2_valueChanged(int arg1);
    void on_spBox_GPSEnd3_valueChanged(int arg1);
    void on_spBox_GPSEnd4_valueChanged(int arg1);
    void on_spBox_GPSEnd5_valueChanged(int arg1);
    void on_spBox_GPSEnd6_valueChanged(int arg1);
    void on_spBox_GPSEnd7_valueChanged(int arg1);
    void on_spBox_GPSEnd8_valueChanged(int arg1);
    void on_spBox_GPSEnd9_valueChanged(int arg1);

    void on_pBtnCloseTool_clicked();

//    void on_pBtn_GPSMode_clicked();

    void on_spinBox_GPSAntennaMode_valueChanged(int arg1);

private:
    Ui::gpsTool *ui;

    QDateTime gpsUtcTime;
    QElapsedTimer gpsFrameAge;
    bool gpsTimeValid;
    bool gpsSyncEnabled;

    double Date2Js1995(int y, int M, int d, int h, int m, int s);
};

extern class gpsTool *gpsTool_dialog;

#endif // GPSTOOL_H
