#include "downloadPreThread.h"
#include "include/dllqhyccd.h"
//#include "qhyccdStatus.h"
#include "myStruct.h"
#include "opencv2/opencv.hpp"
#include "outputdebug.h"

#include <QDebug>
#include <QTime>

extern qhyccd_handle *camhandle;

DownloadPreThread::DownloadPreThread(QObject *parent) :
    QThread(parent)
{
}

void DownloadPreThread::run()
{
    unsigned int ret = QHYCCD_ERROR;

    //get one frame data
    ret = libqhyccd->GetQHYCCDSingleFrame(camhandle,&ix.FrameW,&ix.FrameH,&ix.FrameB,&ix.FrameC,ix.ImgData);
    if(ret != QHYCCD_SUCCESS)
    {
        DBGOPT_ERROR("GetQHYCCDSingleFrame() Failed!!!");
        ix.imageReady = GetSingleFrame_Failed;
    }
    else
    {
        DBGOPT_INFO("w = %d h = %d b = %d c = %d", ix.FrameW, ix.FrameH, ix.FrameB, ix.FrameC);
        ix.frame++;

        if(ix.FrameB == 8)
        {
            unsigned char *temparray = (unsigned char *)malloc(ix.FrameW * ix.FrameH * 2);

            int i = 0,j = 1;
            for(;i < (int)(ix.FrameW * ix.FrameH);i++)
            {
                temparray[j] = ix.ImgData[i];
                j += 2;
            }

            memcpy(ix.ImgData,temparray,ix.FrameW * ix.FrameH * 2);
            delete(temparray);
        }

        memset(ix.ImgData_Last, 0, ix.ImageW_Max * ix.ImageH_Max * 2);
        memcpy(ix.ImgData_Last, ix.ImgData, ix.FrameW * ix.FrameH * 2);
        ix.FrameW_Last = ix.FrameW;
        ix.FrameH_Last = ix.FrameH;
        ix.FrameB_Last = ix.FrameB;
        ix.FrameC_Last = ix.FrameC;

        cv::Mat histImg(Size(256, 100), CV_8UC3);
        histImg.setTo(Scalar(0, 0, 0));

        uint32_t Histogram[256] = { 0 };
        long k = 0, s = ix.FrameW_Last * ix.FrameH_Last;
        uint32_t maxHist;
        int index = 0;

        while(s)
        {
            index = ix.ImgData_Last[2 * k + 1];
            Histogram[index]++;
            k = k + 1;
            s--;
        }

        maxHist = Histogram[0];
        for(int i = 1; i < 255; i++)
        {
            if(Histogram[i] > maxHist)
            {
                maxHist = Histogram[i];
            }
        }
        if(maxHist==0) maxHist=1;

        for(int i = 0; i < 256; i++)
        {
            line(histImg, Point(i, 100), Point(i,100-Histogram[i]*256/maxHist), Scalar(255, 0, 0), 1, LINE_8, 0);
        }

        cv::resize(histImg, mainWidget->ImgHist, mainWidget->ImgHist.size(), 0, 0, INTER_CUBIC);
        histImg.release();

        memset(ix.ImgData_Save, 0, ix.ImageW_Max * ix.ImageH_Max * 2);
        memcpy(ix.ImgData_Save, ix.ImgData, ix.FrameW * ix.FrameH * 2);
        ix.FrameW_Save = ix.FrameW;
        ix.FrameH_Save = ix.FrameH;
        ix.FrameB_Save = ix.FrameB;
        ix.FrameC_Save = ix.FrameC;

        ix.imageReady = GetSingleFrame_Success;
    }

}
