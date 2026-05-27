#include "liveCapThread.h"
#include "dllqhyccd.h"
#include <QDateTime>
#include <cstdio>
#include <QDebug>
#include "outputdebug.h"
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <QImage>
#include <QMutex>
#include <QQueue>

extern QMutex fpsMutex;
extern qhyccd_handle *camhandle;

LiveCapThread::LiveCapThread(QObject *parent) :
    QThread(parent)
{
    quit = false;
    quitted = false;
}

void LiveCapThread::run()
{
    uint32_t ret = QHYCCD_ERROR;

    quit = false;
    quitted = false;
    ix.frame = 0;
    ix.frameLast = 0;
    ix.timeLast = QDateTime::currentDateTime().toMSecsSinceEpoch();
    timeStart = QDateTime::currentDateTime().toMSecsSinceEpoch();

    while(quit == false)
    {
        //FPS
        ix.time = QDateTime::currentDateTime().toMSecsSinceEpoch();

        ret = libqhyccd->GetQHYCCDLiveFrame(camhandle, &ix.FrameW, &ix.FrameH, &ix.FrameB, &ix.FrameC, ix.ImgData);
        if(ret == QHYCCD_SUCCESS)
        {
            ret = QHYCCD_ERROR;
            ix.frame++;

            if(ix.time-ix.timeLast > 1000 && fpsMutex.tryLock())
            {
                ix.timeCount  = ix.time - ix.timeLast;
                ix.frameCount = ix.frame - ix.frameLast;
                ix.timeLast   = ix.time;
                ix.frameLast  = ix.frame;
                emit gotFPSData();

                fpsMutex.unlock();
            }

            //GPS
            if(ix.GPS)
            {
                ix.GPS_LocalTime = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss-zzz");
                memset(ix.ImgData_GPS, 0, 1024);
                memcpy(ix.ImgData_GPS, ix.ImgData, 1024);
                emit gotGPSData();
            }

            //Calibrate
            if(ix.darkCal && !ix.frameCal)
            {
                if(ix.FrameB == 8 && ix.FrameC == 1)
                {
                    for(int i = 0; i < ix.FrameW*ix.FrameH; i++)
                    {
                        ix.ImgData[i] = ix.ImgData[i] - ix.ImgData_Dark[i];
                        if(ix.ImgData[i] < 0) ix.ImgData[i] = 0;
                    }
                }
                else if (ix.FrameB == 16 && ix.FrameC == 1)
                {
                    int temp = 0;
                    for(int i = 0; i < ix.FrameW*ix.FrameH; i++)
                    {
                        temp = (ix.ImgData[2*i+1]*256+ix.ImgData[2*i]) - (ix.ImgData_Dark[2*i+1]*256+ix.ImgData_Dark[2*i]);
                        if(temp < 0) temp = 0;
                        ix.ImgData[2*i] = temp % 256;
                        ix.ImgData[2*i+1] = temp / 256;
                    }
                }
                else if (ix.FrameB == 8 && ix.FrameC == 3)
                {
                    for(int i = 0; i < ix.FrameW*ix.FrameH; i++)
                    {
                        ix.ImgData[3*i] = ix.ImgData[3*i] - ix.ImgData_Dark[3*i];
                        if(ix.ImgData[3*i] < 0) ix.ImgData[3*i] = 0;
                        ix.ImgData[3*i+1] = ix.ImgData[3*i+1] - ix.ImgData_Dark[3*i+1];
                        if(ix.ImgData[3*i+1] < 0) ix.ImgData[3*i+1] = 0;
                        ix.ImgData[3*i+2] = ix.ImgData[3*i+2] - ix.ImgData_Dark[3*i+2];
                        if(ix.ImgData[3*i+2] < 0) ix.ImgData[3*i+2] = 0;
                    }
                }
            }

            if(!ix.darkCal && ix.frameCal)
            {
                double temp = 0.0;
                if(ix.FrameB == 8 && ix.FrameC == 1)
                {
                    for(int i = 0; i < ix.FrameW*ix.FrameH; i++)
                    {
                        temp = static_cast<double>(ix.ImgData[i]-ix.ImgData_Dark[i]);
                        if(temp < 0.0) temp = 0.0;
                        temp /= static_cast<double>(ix.ImgData_FB[i]);
                        ix.ImgData[i] = static_cast<uint8_t>(temp*ix.avgFlatBias1);
                    }
                }
                else if(ix.FrameB == 16 && ix.FrameC == 1)
                {
                    for(int i = 0; i < ix.FrameW*ix.FrameH; i++)
                    {
                        temp = static_cast<double>((ix.ImgData[2*i+1]*256+ix.ImgData[2*i])-(ix.ImgData_Dark[2*i+1]*256+ix.ImgData_Dark[i]));
                        if(temp < 0.0) temp = 0.0;
                        temp /= static_cast<double>(ix.ImgData_FB[i]);
                        ix.ImgData[2*i]   = static_cast<uint16_t>(temp*ix.avgFlatBias1) % 256;
                        ix.ImgData[2*i+1] = static_cast<uint16_t>(temp*ix.avgFlatBias1) / 256;
                    }
                }
                else if(ix.FrameB == 8 && ix.FrameC == 3)
                {
                    for(int i = 0; i < ix.FrameW*ix.FrameH; i++)
                    {
                        temp = static_cast<double>(ix.ImgData[3*i]-ix.ImgData_Dark[3*i]);
                        if(temp < 0.0) temp = 0.0;
                        temp /= static_cast<double>(ix.ImgData_FB[3*i]);
                        ix.ImgData[3*i] = static_cast<uint8_t>(temp*ix.avgFlatBias1);
                        temp = static_cast<double>(ix.ImgData[3*i+1]-ix.ImgData_Dark[3*i+1]);
                        if(temp < 0.0) temp = 0.0;
                        temp /= static_cast<double>(ix.ImgData_FB[3*i+1]);
                        ix.ImgData[3*i+1] = ix.ImgData[3*i+1] - ix.ImgData_Dark[3*i+1];
                        temp = static_cast<double>(ix.ImgData[3*i+2]-ix.ImgData_Dark[3*i+2]);
                        if(temp < 0.0) temp = 0.0;
                        temp /= static_cast<double>(ix.ImgData_FB[3*i+2]);
                        ix.ImgData[3*i+2] = ix.ImgData[3*i+2] - ix.ImgData_Dark[3*i+2];
                    }
                }
            }

            if(ix.saveFlag)
            {
                memcpy(ix.ImgData_Save, ix.ImgData, ix.FrameW*ix.FrameH*ix.FrameC*(ix.FrameB/8));
                ix.FrameW_Save = ix.FrameW;
                ix.FrameH_Save = ix.FrameH;
                ix.FrameB_Save = ix.FrameB;
                ix.FrameC_Save = ix.FrameC;
                ix.saveFlag        = false;
                emit gotSaveData();
            }

            if(!ix.locked)
            {
                memcpy(ix.ImgData_Last, ix.ImgData, ix.FrameW*ix.FrameH*ix.FrameC*(ix.FrameB/8));
                ix.FrameW_Last = ix.FrameW;
                ix.FrameH_Last = ix.FrameH;
                ix.FrameB_Last = ix.FrameB;
                ix.FrameC_Last = ix.FrameC;
                ix.locked = true;

                if(ix.time - timeStart > 500)
                {
                    timeStart = ix.time;
                    emit gotHistData();
                }
            }
        }
    }

    quitted = true;
}

void LiveCapThread::closeThread()
{
    quit = true;
    while(quitted != true) QThread::msleep(100);
}

