#ifndef THREADTEMPCONTROL_H
#define THREADTEMPCONTROL_H

#include <QThread>
#include <QPoint>
#include <QVector>

class ThreadTempControl : public QThread
{
    Q_OBJECT
public:
    explicit ThreadTempControl(QObject *parent = 0);
    ~ThreadTempControl();

    bool TESTED_PID;

signals:

public slots:

private:
    bool quitFlag = false;
    bool quittedFlag = false;
    bool suspendFlag = false;
    bool suspendedFlag = false;
    bool Flag_Timer = false;
    int CurveX;
    int posX_tempImg;//温度曲线图的起始点坐标
    int posY_tempImg;
    int posX_tempImg_RH;//湿度曲线图起始点坐标
    int posY_tempImg_RH;
    int posX_tempImg_Press;//压力曲线图起始点坐标
    int posY_tempImg_Press;
    QPoint *pointTemp;
    QPoint *pointHumidity;
    QPoint *pointPressure;
    int pointIndex = 0;
    int pointX = 0;

    double DegreeToR(double degree);
    double RToDegree(double R);
    double DegreeTomV(double degree);
    double mVToDegree(double V);

public:
    void run();
    void stop();
    void suspend();
    void resume();
};

#endif
