#include "downloadCapThread.h"
#include "include/dllqhyccd.h"
//#include "qhyccdStatus.h"
#include "myStruct.h"
#include "outputdebug.h"
#include "opencv2/opencv.hpp"

#include <QDebug>
#include <QTime>

extern qhyccd_handle *camhandle;

DownloadCapThread::DownloadCapThread(QObject *parent) :
    QThread(parent)
{
    discardFrame = false;
}

QImage* MatToQImage(const Mat imageMat)
{
    QImage *image = NULL;
    uchar *imgData = imageMat.data;

    if(imageMat.channels() == 1)
        image = new QImage(imgData, imageMat.cols, imageMat.rows, imageMat.cols, QImage::Format_Indexed8);
    else if(imageMat.channels() == 3)
        image = new QImage(imgData, imageMat.cols, imageMat.rows, imageMat.cols * 3, QImage::Format_RGB888);
    else
        qDebug() << " ";

    return image;
}

int calibrateOverscan(unsigned char* inbuf, unsigned char*outbuf, int ImgW, int ImgH, int OSStartX, int OSStartY, int OSSizeX, int OSSizeY)
{
    unsigned short rms;
    int ret = -1;

    cv::Scalar RMS;
    cv::Mat srcImg;
    cv::Mat roi;

    if(inbuf)
    {
        if(((OSStartX + OSSizeX) > ImgW) || ((OSStartY + OSSizeY) > ImgH))
        {
            ret = -1;
        }
        else if(OSSizeX == 0 || OSSizeY == 0)
        {
            ret = 0;
        }
        else
        {
            srcImg.create(cv::Size(ImgW, ImgH), CV_16UC1);
            roi.create(cv::Size(OSSizeX, OSSizeY), CV_16UC1);

            int copyLen = srcImg.cols * srcImg.rows * (srcImg.depth()==CV_8U?1:2) * srcImg.channels();
            memcpy(srcImg.data, inbuf, copyLen);

            roi = srcImg(cv::Rect(OSStartX, OSStartY, OSSizeX, OSSizeY));

            RMS = mean(roi);

            rms = RMS.val[0];
            if(rms < 1000)
            {
                DBGOPT_WARNING("Warning Offset is too low,Please increase the Offset!");
            }

            if(rms < ix.calConstant)
            {
                RMS.val[0] = ix.calConstant - rms;
                add(srcImg, RMS, srcImg);
            }
            else
            {
                RMS.val[0] = rms - ix.calConstant;
                subtract(srcImg, RMS, srcImg);
            }
            memcpy(outbuf, srcImg.data, copyLen);

            ret = 1;

            srcImg.release();
            roi.release();
        }
    }

    return ret;
}

void DownloadCapThread::run()
{
    unsigned int ret = QHYCCD_ERROR;

    //获取图像数据
    ret = libqhyccd->GetQHYCCDSingleFrame(camhandle,&ix.FrameW,&ix.FrameH,&ix.FrameB,&ix.FrameC,ix.ImgData);
    if(ret != QHYCCD_SUCCESS)
    {
        qCritical("GetQHYCCDSingleFrame: failed");
        ix.imageReady = GetSingleFrame_Failed;
    }
    else
    {
        DBGOPT_INFO("GetQHYCCDSingleFrame() w = %d h = %d b = %d c = %d", ix.FrameW, ix.FrameH, ix.FrameB, ix.FrameC);
        if(discardFrame || ix.ForceStop)
        {
            ix.imageReady = GetSingleFrame_Failed;
            return;
        }
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
            free(temparray);
        }

        memset(ix.ImgData_Last, 0, ix.ImageW_Max * ix.ImageH_Max * 2);
        memcpy(ix.ImgData_Last, ix.ImgData, ix.FrameW * ix.FrameH * 2);
        ix.FrameW_Last = ix.FrameW;
        ix.FrameH_Last = ix.FrameH;
        ix.FrameB_Last = ix.FrameB;
        ix.FrameC_Last = ix.FrameC;

        if(ix.CalibrateOverscan)
        {
            uint32_t x = 0, y = 0, w = 0, h = 0;
            ret =libqhyccd->GetQHYCCDOverScanArea(camhandle,&x, &y, &w, &h);
            if(ret == QHYCCD_SUCCESS)
            {
                ix.OverscanX = x;
                ix.OverscanY = y;
                ix.OverscanW = w;
                ix.OverscanH = h;
            }

            ret = libqhyccd->GetQHYCCDEffectiveArea(camhandle,&x, &y, &w, &h);
            if(ret == QHYCCD_SUCCESS)
            {
                ix.EffectiveX = x;
                ix.EffectiveY = y;
                ix.EffectiveW = w;
                ix.EffectiveH = h;
            }

            calibrateOverscan(ix.ImgData_Last, ix.ImgData_Last, ix.FrameW_Last, ix.FrameH_Last, ix.OverscanX, ix.OverscanY, ix.OverscanW, ix.OverscanH);
        }

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
        memcpy(ix.ImgData_Save, ix.ImgData_Last, ix.FrameW * ix.FrameH * 2);
        ix.FrameW_Save = ix.FrameW;
        ix.FrameH_Save = ix.FrameH;
        ix.FrameB_Save = ix.FrameB;
        ix.FrameC_Save = ix.FrameC;

        if(ix.GPS_Fun && ix.GPS)
        {
            memset(ix.ImgData_GPS, 0, 1024);
            memcpy(ix.ImgData_GPS, ix.ImgData, 1024);
            emit updateGPSInfo();
        }

        ix.imageReady = GetSingleFrame_Success;
    }
}
