#include "threadProcessImage.h"
#include "outputdebug.h"
#include "myStruct.h"
#include "ezCap.h"
#include <QMutex>

extern QMutex showMutex;
extern QMutex histMutex;

ThreadProcessImage::ThreadProcessImage(QObject *parent) :
    QThread(parent)
{
    quitFlag = false;
    quittedFlag = false;
}

void ThreadProcessImage::run()
{
    quitFlag = false;
    quittedFlag = false;

    while(quitFlag != true)
    {
        if(ix.locked)
        {
            int s = 0, k = 0;
            cv::Mat Original;

            if(!ix.Color_Fun || (ix.Color_Fun && !ix.Color && !ix.IsCvtColor)) //MONO && RAW
            {
                Original.create(cv::Size(ix.FrameW_Last, ix.FrameH_Last), CV_8UC3);
                if(ix.FrameB_Last == 8)
                {
                    for(uint32_t j = 0; j < ix.FrameH_Last; j++)
                    {
                        for(uint32_t i = 0; i < ix.FrameW_Last; i++)
                        {
                            Original.data[k] = ix.StretchLUT[ix.ImgData_Last[s] * 256];
                            Original.data[k + 1] = ix.StretchLUT[ix.ImgData_Last[s] * 256];
                            Original.data[k + 2] = ix.StretchLUT[ix.ImgData_Last[s] * 256];
                            s += 1;
                            k += 3;
                        }
                        k += Original.cols * (Original.depth()==CV_8U?1:2) * Original.channels() - Original.channels() * Original.cols;
                    }
                }
                else
                {
                    for(uint32_t j = 0; j < ix.FrameH_Last; j++)
                    {
                        for(uint32_t i = 0; i < ix.FrameW_Last; i++)
                        {
                            Original.data[k]     = ix.StretchLUT[ix.ImgData_Last[s + 1] * 256 + ix.ImgData_Last[s]];
                            Original.data[k + 1] = ix.StretchLUT[ix.ImgData_Last[s + 1] * 256 + ix.ImgData_Last[s]];
                            Original.data[k + 2] = ix.StretchLUT[ix.ImgData_Last[s + 1] * 256 + ix.ImgData_Last[s]];
                            s += 2;
                            k += 3;
                        }
                        k += Original.cols * (Original.depth()==CV_8U?1:2) * Original.channels() - Original.channels() * Original.cols;
                    }
                }
            }
            else if(ix.Color) //BGR24
            {
                Original.create(cv::Size(ix.FrameW_Last, ix.FrameH_Last), CV_8UC3);
                for(uint32_t j = 0; j < ix.FrameH_Last; j++)
                {
                    for(uint32_t i = 0; i < ix.FrameW_Last; i++)
                    {
                        Original.data[k]     = ix.StretchLUT[ix.ImgData_Last[s + 2] * 256];
                        Original.data[k + 1] = ix.StretchLUT[ix.ImgData_Last[s + 1] * 256];
                        Original.data[k + 2] = ix.StretchLUT[ix.ImgData_Last[s] * 256];
                        s += 3;
                        k += 3;
                    }
                    k += Original.cols * (Original.depth()==CV_8U?1:2) * Original.channels() - Original.channels() * Original.cols;
                }
            }
            else if(ix.IsCvtColor) //RAW to BGR24
            {
                Original.create(cv::Size(ix.FrameW_Last, ix.FrameH_Last), CV_8UC1);
                if(ix.FrameB_Last == 8) //COLOR RAW 8bits to BGR
                {
                    for(uint32_t j = 0; j < ix.FrameH_Last; j++)
                    {
                        for(uint32_t i = 0; i < ix.FrameW_Last; i++)
                        {
                            Original.data[k] = ix.StretchLUT[ix.ImgData_Last[s] * 256];
                            s += 1;
                            k += 1;
                        }
                        k += Original.cols * (Original.depth()==CV_8U?1:2) * Original.channels() - Original.channels() * Original.cols;
                    }

                    if(ix.Bayer == BAYER_GB)      cv::cvtColor(Original, Original, COLOR_BayerGB2BGR_VNG);
                    else if(ix.Bayer == BAYER_GR) cv::cvtColor(Original, Original, COLOR_BayerGR2BGR_VNG);
                    else if(ix.Bayer == BAYER_BG) cv::cvtColor(Original, Original, COLOR_BayerBG2BGR_VNG);
                    else if(ix.Bayer == BAYER_RG) cv::cvtColor(Original, Original, COLOR_BayerRG2BGR_VNG);
                }
                else //COLOR RAW 16bits to BGR
                {
                    for(uint32_t j = 0; j < ix.FrameH_Last; j++)
                    {
                        for(uint32_t i = 0; i < ix.FrameW_Last; i++)
                        {
                            Original.data[k] = ix.StretchLUT[ix.ImgData_Last[s + 1] * 256 + ix.ImgData_Last[s]];
                            s += 2;
                            k += 1;
                        }
                        k += Original.cols * (Original.depth()==CV_8U?1:2) * Original.channels() - Original.channels() * Original.cols;
                    }

                    if(ix.Bayer == BAYER_GB)      cv::cvtColor(Original, Original, COLOR_BayerGB2BGR_VNG);
                    else if(ix.Bayer == BAYER_GR) cv::cvtColor(Original, Original, COLOR_BayerGR2BGR_VNG);
                    else if(ix.Bayer == BAYER_BG) cv::cvtColor(Original, Original, COLOR_BayerBG2BGR_VNG);
                    else if(ix.Bayer == BAYER_RG) cv::cvtColor(Original, Original, COLOR_BayerRG2BGR_VNG);
                }
            }

            if(ix.Circle_Correct)
            {
                cv::Scalar scalar;
                if(ix.Circle_Color == CIRCLE_WHITE)
                    scalar = cv::Scalar(255, 255, 255);
                else if(ix.Circle_Color == CIRCLE_BLACK)
                    scalar = cv::Scalar(0, 0, 0);
                else if(ix.Circle_Color == CIRCLE_RED)
                    scalar = cv::Scalar(255, 0, 0);
                else if(ix.Circle_Color == CIRCLE_GREEN)
                    scalar = cv::Scalar(0, 255, 0);
                else if(ix.Circle_Color == CIRCLE_BLUE)
                    scalar = cv::Scalar(0, 0, 255);
                else if(ix.Circle_Color == CIRCLE_YELLOW)
                    scalar = cv::Scalar(255, 255, 0);

                cv::circle(Original, Point(Original.cols / 2, Original.rows / 2), ix.Circle_Radius, scalar, ix.Circle_Thickness, LINE_8, 0);
                cv::circle(Original, Point(Original.cols / 2, Original.rows / 2), ix.Circle_Radius - ix.Circle_Spacing, scalar, ix.Circle_Thickness, LINE_8, 0);
            }

            if(ix.zoomMode == Zoom_FitWindow)
            {
                double rate_label = (double)ix.showLabelW / (double)ix.showLabelH;
                double rate_image = (double)Original.cols / (double)Original.rows;
                if(rate_label > rate_image)
                    cv::copyMakeBorder(Original, Original, 0, 0, 0, Original.rows * rate_label - Original.cols, BORDER_CONSTANT, cv::Scalar(100, 100, 115));
                else
                    cv::copyMakeBorder(Original, Original, 0, Original.cols / rate_label - Original.rows, 0, 0, BORDER_CONSTANT, cv::Scalar(100, 100, 115));
                cv::resize(Original, Original, cv::Size(ix.showLabelW, ix.showLabelH), 0, 0, CV_INTER_LINEAR);
            }
            else if(ix.zoomMode == Zoom_FillWindow)
            {
                cv::resize(Original, Original, cv::Size(ix.showLabelW, ix.showLabelH), 0, 0, INTER_LINEAR);
            }
            else
            {
                uint32_t OffsetX = (double)ix.showLabelX / ix.scaleFactor;
                uint32_t OffsetY = (double)ix.showLabelY / ix.scaleFactor;
                uint32_t OffsetW = (double)ix.showLabelW / ix.scaleFactor;
                uint32_t OffsetH = (double)ix.showLabelH / ix.scaleFactor;
                if(OffsetW >= ix.FrameW_Last)
                {
                    OffsetX = 0;
                    OffsetW = ix.FrameW_Last;
                }
                if(OffsetH >= ix.FrameH_Last)
                {
                    OffsetY = 0;
                    OffsetH = ix.FrameH_Last;
                }
                OffsetX = OffsetX / 2 * 2;
                OffsetY = OffsetY / 2 * 2;

                cv::Mat Roi = Original(cv::Rect(OffsetX, OffsetY, OffsetW, OffsetH));
                Original.release();

                if((double)ix.showLabelW / ix.scaleFactor > ix.FrameW_Last ||
                   (double)ix.showLabelH / ix.scaleFactor > ix.FrameH_Last)
                {
                    cv::Mat Temp = cv::Mat::zeros(cv::Size((double)ix.showLabelW / ix.scaleFactor, (double)ix.showLabelH / ix.scaleFactor), Roi.type());
                    Roi.copyTo(Temp(cv::Rect(0, 0, Roi.cols, Roi.rows)));
                    Original.create(Temp.size(), Temp.type());
                    Temp.copyTo(Original);
                    Temp.release();
                }
                else
                {
                    Original.create(Roi.size(), Roi.type());
                    Roi.copyTo(Original);
                }

                Roi.release();

                if(ix.scaleFactor != 1.0)
                {
                    cv::resize(Original, Original, cv::Size(ix.showLabelW, ix.showLabelH), 0, 0, cv::INTER_LINEAR);
                }
            }

            if(showMutex.tryLock())
            {
                if(!mainWidget->ImgShow.empty()) mainWidget->ImgShow.release();
                mainWidget->ImgShow.create(Original.size(), Original.type());
                Original.copyTo(mainWidget->ImgShow);

                showMutex.unlock();
            }

            Original.release();

            ix.locked = false;
        }

        QThread::msleep(3);
    }

    quittedFlag = true;
}

void ThreadProcessImage::stop()
{
    quitFlag = true;
    while(quittedFlag != true) QThread::msleep(100);
}
