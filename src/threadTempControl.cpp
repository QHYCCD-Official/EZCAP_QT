#include "threadTempControl.h"
#include "outputdebug.h"
#include "myStruct.h"
#include "favorite.h"
#include "ui_favorite.h"
#include "tempControl.h"
#include "ui_tempControl.h"
#include "ezCap.h"
#include "ui_ezCap.h"
#include "fpgaAccess.h"
#include <math.h>
#include <QPainter>

extern qhyccd_handle *camhandle;

ThreadTempControl::ThreadTempControl(QObject *parent) :
    QThread(parent)
{
    quitFlag = false;
    quittedFlag = false;
    suspendFlag = false;
    suspendedFlag = false;

    pointTemp = new QPoint[tempControl_dialog->ui->label_image_tempControl->width()];
    pointHumidity = new QPoint[tempControl_dialog->ui->label_image_tempControl->width()];
    pointPressure = new QPoint[tempControl_dialog->ui->label_image_tempControl->width()];
}

ThreadTempControl::~ThreadTempControl()
{
    delete[] pointTemp;
    delete[] pointHumidity;
    delete[] pointPressure;
}

double ThreadTempControl::DegreeToR(double degree)
{

#define SQR3(x) ((x)*(x)*(x))
#define SQRT3(x) (exp(log(x)/3))

        if (degree<-50) degree=-50;
        if (degree>50)  degree=50;

        double x,y;
        double R;
        double T;

        double A=0.002679;
        double B=0.000291;
        double C=4.28e-7;

        T=degree+273.15;


        y=(A-1/T)/C;
        x=sqrt( SQR3(B/(3*C))+(y*y)/4);
        R=exp(  SQRT3(x-y/2)-SQRT3(x+y/2));

        return R;
}


double ThreadTempControl::RToDegree(double R)
{
        double 	T;
        double LNR;

        if (R>400) R=400;
        if (R<1) R=1;

        LNR=log(R);
        T=1 / (0.002679+0.000291*LNR + LNR*LNR*LNR*4.28e-7);
        T=T-273.15;
        return T;
}

double ThreadTempControl::DegreeTomV(double degree)
{
        double V;
        double R;

        R=DegreeToR(degree);
        V=33000/(R+10)-1625;

        return V;
}

double ThreadTempControl::mVToDegree(double V)
{
        double R;
        double T;

        R=33/(V/1000+1.625)-10;

        T=RToDegree(R);

        return T;
}

void ThreadTempControl::run()
{
    if(!ix.isConnected)
    {
        return;
    }

    quitFlag = false;
    quittedFlag = false;
    suspendFlag = false;
    suspendedFlag = false;

    QVector<QPoint> vectorTemp;
    QVector<QPoint> vectorHumidity;
    QVector<QPoint> vectorPressure;
    QImage img_tempcontrol;
    pointIndex = 0;
    pointX = 1;
    while(quitFlag != true)
    {
        if(!suspendFlag)
        {
            Flag_Timer = !Flag_Timer;//计时器标识

            if(favorite_dialog->ui->cBox_TEC->isChecked())
                tempControl_dialog->ui->vSlider_power_tempControl->setMaximum(180);
            else
                tempControl_dialog->ui->vSlider_power_tempControl->setMaximum(255);

            if(Flag_Timer == 1)
            {
                //        QApplication::processEvents();//防止长时间导致界面假死
                //取数
                ix.Temp_Now = libqhyccd->GetQHYCCDParam(camhandle,CONTROL_CURTEMP);
                qDebug()<<"ix.Temp_Now:="<<ix.Temp_Now;
                ix.Voltage_Now = DegreeTomV(ix.Temp_Now);//计算电压值
                if(ix.Humidity_Fun)
                {
                    int ret = libqhyccd->GetQHYCCDHumidity(camhandle, &ix.Humidity);
                    if(ret == QHYCCD_SUCCESS)
                    {
                        mainWidget->statusLabel_RH->setText(QString("RH:") + QString::number(ix.Humidity, 'f', 1) + "%");
                        tempControl_dialog->ui->HumidityStatus_tempControl->setText(QString("RH:") +QString::number(ix.Humidity, 'f', 1)+"%");
                        tempControl_dialog->ui->vSlider_Humidity_tempCotrol->setValue(ix.Humidity);
                    }
                }
                //20200318
                if(ix.Pressure_Fun)
                {
                    int ret = libqhyccd->GetQHYCCDPressure(camhandle, &ix.Pressure);
                    if(ret == QHYCCD_SUCCESS)
                    {
                        mainWidget->statusLabel_PRESS->setText( QString::number(ix.Pressure, 'f', 1) + "mbar");//20200329去掉显示QString("PRESS:") +
                        tempControl_dialog->ui->PressStatus_tempControl->setText(QString::number(ix.Pressure) + "mbar");
                        tempControl_dialog->ui->vSlider_Press_tempCotrol->setValue(ix.Pressure/10.0);
                    }
                }
                if(1)//ix.workMode == 3 || ix.workMode == 2)
                {
                    char temp[16];
                    char info[32];
                    char time[32];
                    char mode[32];
                    sprintf(temp,"%.2f",ix.Temp_Now);
                    sprintf(info,"FWHM%d,PEAK%d",FocusInfo.FWHM_Result,FocusInfo.DeltaPixel);

                    sprintf(time,"          ");
#ifdef WIN32
                    sprintf(mode,"USB Camera");
#else
                    sprintf(mode,"StandAlone");
#endif

#ifdef Q_OS_MAC
                    sprintf(mode,"USB Camera");
#endif
                    int ret = libqhyccd->SendFourLine2QHYCCDInterCamOled(camhandle,temp,info,time,mode);
                    if(ret != QHYCCD_SUCCESS)
                    {
                        //qDebug() << "SendFourLine2QHYCCDInterCamOled failed";
                    }
                }

                mainWidget->statusLabel_Temp->setText(QString("SENSORTEMP:") + QString::number(ix.Temp_Now, 'f', 1) + QString::fromUtf8("℃"));//显示温度值
                if(ix.FPGATemp_Fun)
                {
                    qhyWriteFPGA(camhandle, 150, 0);
                    qhyWriteFPGA(camhandle, 150, 1);
                    QThread::msleep(100);
                    qhyWriteFPGA(camhandle, 150, 0);

                    const uint16_t fpgaRaw =
                        (static_cast<uint16_t>(qhyReadFPGA(camhandle, 209)) << 8) |
                        qhyReadFPGA(camhandle, 208);
                    ix.FPGATemp_Now = 693.0 * static_cast<double>(fpgaRaw) / 1024.0 - 265.0;
                    mainWidget->statusLabel_FPGATemp->setText(
                        QString("FPGATEMP:") + QString::number(ix.FPGATemp_Now, 'f', 1) + QString::fromUtf8("℃"));
                }
                tempControl_dialog->ui->tempStatus_tempControl->setText(QString::number(ix.Temp_Now, 'f', 1) + QString::fromUtf8("℃"));
                //tempControl_dialog->ui->PRESSStatus_tempControl->setText(QString::number(ix.Pressure) + "mbar");//显示电压值20200329更换为压力值
                tempControl_dialog->ui->PWMStatus_tempControl->setText(QString("Power:") +QString::number(ix.PWM_Now*100/255, 'f', 1) + "%");//显示功率
                if(ix.Cooler_Mode == Cooler_Manual)
                {   //手动控温模式，设置当前显示温度
                    tempControl_dialog->ui->vSlider_temp_tempCotrol->setValue((int)(ix.Temp_Now + 100));
                }
                else if(ix.Cooler_Mode == Cooler_Auto)
                {   //自动控温模式，设置当前功率
                    tempControl_dialog->ui->vSlider_power_tempControl->setValue(ix.PWM_Now);
                }

                pointTemp[pointIndex].setX(pointX);
                pointTemp[pointIndex].setY(127-(ix.Temp_Now+50)*127.0/90.0);
                if(ix.Humidity_Fun)
                {
                    pointHumidity[pointIndex].setX(pointX);
                    pointHumidity[pointIndex].setY(127-ix.Humidity*127.0/100.0);
                }
                if(ix.Pressure_Fun)
                {
                    pointPressure[pointIndex].setX(pointX);
                    pointPressure[pointIndex].setY(127-(ix.Pressure/10.0)*127.0/200.0);
                }

                if(pointX >= img_tempcontrol.width() - 1)
                {
                    for(int i = 0; i < pointIndex; i++)
                    {
                        pointTemp[i].setY(pointTemp[i + 1].y());
                    }
                    if(ix.Humidity_Fun)
                    {
                        for(int i = 0; i < pointIndex; i++)
                        {
                            pointHumidity[i].setY(pointHumidity[i + 1].y());
                        }
                    }
                    if(ix.Pressure_Fun)
                    {
                        for(int i = 0; i < pointIndex; i++)
                        {
                            pointPressure[i].setY(pointPressure[i + 1].y());
                        }
                    }
                }

                vectorTemp.clear();
                for(int i = 0; i <= pointIndex; i++)
                {
                    vectorTemp.append(pointTemp[i]);
                }
                if(ix.Humidity_Fun)
                {
                    vectorHumidity.clear();
                    for(int i = 0; i <= pointIndex; i++)
                    {
                        vectorHumidity.append(pointHumidity[i]);
                    }
                }
                if(ix.Pressure_Fun)
                {
                    vectorPressure.clear();
                    for(int i = 0; i <= pointIndex; i++)
                    {
                        vectorPressure.append(pointPressure[i]);
                    }
                }
                if(pointX < img_tempcontrol.width() - 1)
                {
                    pointIndex += 1;
                    pointX += 1;
                }
//                OUTPUT_INFO("index = %d x = %d", pointIndex, pointX);

                img_tempcontrol = QPixmap(":/image/black.bmp").toImage();
                mainWidget->DrawGridBox(&img_tempcontrol);

                QPainter paint_temp(&img_tempcontrol); //为这个QImage构造一个QPainter
                paint_temp.setCompositionMode(QPainter::CompositionMode_SourceIn);//设置画刷的组合模式CompositionMode_SourceOut这个模式为目标图像在上。
                QPen pen_temp = paint_temp.pen();

                //draw the temp curve
                pen_temp.setColor(QColor(255,0,0));//设置画笔颜色红色
                paint_temp.setPen(pen_temp);//set pen of paint
                paint_temp.drawLines(vectorTemp);
                if(ix.Humidity_Fun)
                {
                    pen_temp.setColor(QColor(0,255,255));//设置画笔颜色红色
                    paint_temp.setPen(pen_temp);//set pen of paint
                    paint_temp.drawLines(vectorHumidity);
                }
                if(ix.Pressure_Fun)
                {
                    pen_temp.setColor(QColor(0,255,0));//设置画笔颜色红色
                    paint_temp.setPen(pen_temp);//set pen of paint
                    paint_temp.drawLines(vectorPressure);
                }

#if THP_File_Saved
                if(ix.saveTHPFile)
                {
                    //20201125lyl温度湿度压力数据保存
                    QDateTime time = QDateTime::currentDateTime();
                    QString str = time.toString("yyyy-MM-dd hh:mm:ss");
                    QFile file(fileTHP);
                    file.open(QIODevice::Text | QIODevice::WriteOnly | QIODevice::Append);
                    QTextStream putin(&file);
                    putin<<qSetFieldWidth(1)<<"\n";
                    putin<<qSetFieldWidth(5)<<str<<"\t"<<ix.Temp_Now<<"\t"<<ix.Humidity<<"\t"<<ix.Pressure<<"\t";
                    file.flush();
                    file.close();
                }
#endif

                //       file.open(QIODevice::Text | QIODevice::ReadWrite | QIODevice::Append);
                //       QTextStream out(&file);
                //       out.seek(0);
                //       QString text=out.readAll();       //qDebug() << "All line:"<<text;
                //       QStringList lines=text.split("\t\n");
                //       QString line1=lines[1];
                //       QStringList parts = line1.split("\t");
                //       QString timeStart = parts[0].trimmed();
                //       QDateTime Start = QDateTime::fromString(timeStart,"yyyy-MM-dd hh:mm:ss");
                //       uint timestart = Start.toTime_t();
                //       double pressStart = parts[3].toDouble();
                //       int n=lines.size();       //qDebug() << "timeStart:"<<timeStart<<"pressStart:"<<pressStart<<"n:"<<n;
                //       int sec=0;
                //       if(n>=3)
                //       {
                //           QString lastLine = lines[n-1];
                //           QStringList lastparts = lastLine.split("\t");
                //           QString timeEnd = lastparts[0].trimmed();
                //           double pressEnd = lastparts[3].toDouble();           //qDebug() << "timeEnd:"<<timeEnd<<"pressEnd:"<<pressEnd;
                //           double rate = pressStart - pressEnd;
                //           QDateTime End = QDateTime::fromString(timeEnd,"yyyy-MM-dd hh:mm:ss");
                //           uint timeend = End.toTime_t();
                //           sec =(timeend - timestart);           //qDebug() << "sec:"<<sec<<"timeend"<<timeend<<"timestart"<<timestart;
                //           if( sec%60==0)
                //           {
                //               out<<rate<<" \t";
                //           }
                //          if( rate/(double)(sec/60)>=0.5)
                //          {
                //              QMessageBox::critical(this,tr("Warning"),tr("Pressure rate > 17/0.5h"),QMessageBox::Ok);
                //           }
                //       }
                //       file.flush();
                //       file.close();
                //       if(sec==30*60)
                //       {
                //           QMessageBox::critical(this,tr("Prompt"),tr("Pressure test is 30min!"),QMessageBox::Ok);
                //       }
                //       if(sec==60*60)
                //       {
                //           QMessageBox::critical(this,tr("Prompt"),tr("Pressure test is 60min!"),QMessageBox::Ok);
                //       }

                tempControl_dialog->ui->label_image_tempControl->setPixmap(QPixmap::fromImage(img_tempcontrol));//show image in the label
                posX_tempImg = CurveX;
                posY_tempImg = (127-(ix.Temp_Now+50)*127.0/90.0);//(-ix.Voltage_Now + 200) / 15;
                if(ix.Humidity_Fun){
                    posX_tempImg_RH= CurveX;
                    posY_tempImg_RH= (127-ix.Humidity*127/100.0);//CurveX;ix.Humidity
                }
                if(ix.Pressure_Fun){
                    posX_tempImg_Press= CurveX;
                    posY_tempImg_Press=(127-(ix.Pressure/10.0)*127.0/200.0);
                }
            }
            else
            {   //控制
                uint32_t ret = QHYCCD_ERROR;
                if(ix.Cooler_Mode == Cooler_Off)
                {
                    //停止制冷
                    ix.PWM_Now = 0;
                    ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_MANULPWM, 0);
                    if(ret != QHYCCD_SUCCESS)
                    {
                        qCritical() << "SetQHYCCDParam(camhandle,CONTROL_MANULPWM,0) failure";
                    }
                }
                else if(ix.Cooler_Mode == Cooler_Manual)
                {
                    //开启手动制冷
                    ix.PWM_Now = tempControl_dialog->ui->vSlider_power_tempControl->value();
                    ret = libqhyccd->SetQHYCCDParam(camhandle,CONTROL_MANULPWM,ix.PWM_Now);
                    if(ret != QHYCCD_SUCCESS)
                    {
                        qCritical() << "SetQHYCCDParam CONTROL_MANULPWM" << ix.PWM_Now << " failure";
                    }
                }
                else if(ix.Cooler_Mode == Cooler_Auto)
                {
                    ix.PWM_Now =libqhyccd-> GetQHYCCDParam(camhandle,CONTROL_CURPWM);
                    //开始自动温控制冷

                    if(this->TESTED_PID)
                    {
                        //PID参数调试模式下，设置当前PID参数
                        double pVal = tempControl_dialog->ui->doubleSpinBox_P->value();
                        double iVal = tempControl_dialog->ui->doubleSpinBox_I->value();
                        double dVal = tempControl_dialog->ui->doubleSpinBox_D->value();
                        libqhyccd->TestQHYCCDPIDParas(camhandle, pVal, iVal, dVal);
                    }
                    ix.Temp_Target = tempControl_dialog->ui->vSlider_temp_tempCotrol->value() - 100;
                    ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_COOLER, ix.Temp_Target);
                    if(ret != QHYCCD_SUCCESS)
                    {
                        qCritical() << "ControlQHYCCDTemp failure";
                    }
                    else
                    {
                        qDebug() << "SetQHYCCDParam Cooler" << ix.Temp_Target;
                    }
                }
            }

            QThread::msleep(1000);
        }
        else
        {
            suspendedFlag = true;
        }
    }

    quittedFlag = false;
}

void ThreadTempControl::stop()
{
    quitFlag = true;
    while(quittedFlag == true) QThread::msleep(100);
}

void ThreadTempControl::suspend()
{
    suspendFlag = true;
    while(suspendedFlag == false) QThread::msleep(100);
}

void ThreadTempControl::resume()
{
    suspendFlag = false;
    suspendedFlag = false;
}
