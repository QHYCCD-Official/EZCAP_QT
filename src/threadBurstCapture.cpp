#include "threadBurstCapture.h"
#include "dllqhyccd.h"
#include "outputdebug.h"
#include "cfwControl.h"
#include <QDateTime>
#include <cstdio>
#include <QDebug>
#include <QMutex>

extern qhyccd_handle *camhandle;

//extern QMutex ShowImgMutex;

ThreadBurstCapture::ThreadBurstCapture(QObject *parent) :
    QThread(parent)
{
    connect(this, SIGNAL(updateCFWPos()), cfwControl_dialog, SLOT(updateCFWPos()));
}

void ThreadBurstCapture::run()
{
    uint32_t ret = QHYCCD_ERROR;

    iGroup = 0;
    while(iGroup < ix.BurstGroup && !quit)
    {
        ret = libqhyccd->SetQHYCCDBurstModeStartEnd(camhandle, 0, 0);
        double exp = ExpmsList.at(iGroup).toDouble();
        ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_EXPOSURE, exp * 1000.0);
        double traffic = TrafficList.at(iGroup).toDouble();
        ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_USBTRAFFIC, traffic);
        uint32_t start = 1;
        uint32_t end = CountList.at(iGroup).toInt() + 2;
        ix.BurstCount = CountList.at(iGroup).toInt();
        ret = libqhyccd->SetQHYCCDBurstModeStartEnd(camhandle, start, end);
        ix.BurstRepeat = RepeatList.at(iGroup).toInt();
        uint32_t delayms = DelaymsList.at(iGroup).toInt();

        if(ix.canFilterWheel && ix.CFW_Plugged)
        {
            double dstPos = CFWList.at(iGroup).toDouble() + 47.0;
            ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_CFWPORT, dstPos);
            double curPos = libqhyccd->GetQHYCCDParam(camhandle, CONTROL_CFWPORT);
            DBGOPT_INFO("curPos = %f dstPos = %f", curPos, dstPos);
            while (curPos != dstPos && !quit)
            {
                curPos = libqhyccd->GetQHYCCDParam(camhandle, CONTROL_CFWPORT);
                QThread::msleep(100);
            }
            emit updateCFWPos();
        }

        iGroup++;

        iRepeat = 0;
        while(iRepeat < ix.BurstRepeat && !quit)
        {
            iRepeat++;

            ret = libqhyccd->SetQHYCCDBurstIDLE(camhandle);
            msleep(300);
            ret = libqhyccd->ReleaseQHYCCDBurstIDLE(camhandle);

            iCount = 0;
            while(iCount < ix.BurstCount && !quit)
            {
                ret = libqhyccd->GetQHYCCDLiveFrame(camhandle, &ix.FrameW, &ix.FrameH, &ix.FrameB, &ix.FrameC, ix.ImgData);
                if(ret == QHYCCD_SUCCESS)
                {
                    iCount++;
                    iSeqence = ix.ImgData[0] * 256 * 256 * 256 + ix.ImgData[1] * 256 * 256 + ix.ImgData[2] * 256 + ix.ImgData[3];
                    emit updateBurstInfo(iGroup, iRepeat, iCount, iSeqence);

                    if(!ix.locked)
                    {
                        memcpy(ix.ImgData_Last, ix.ImgData, ix.FrameW * ix.FrameH * ix.FrameC * (ix.FrameB / 8));
                        ix.FrameW_Last = ix.FrameW;
                        ix.FrameH_Last = ix.FrameH;
                        ix.FrameB_Last = ix.FrameB;
                        ix.FrameC_Last = ix.FrameC;
                        ix.locked = true;
                    }
                }
            }
        }

        if(delayms > 0 && !quit) msleep(delayms);
    }

    quit = false;
}
