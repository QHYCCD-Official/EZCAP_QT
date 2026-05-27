#include "videoShowThread.h"
#include "myStruct.h"
#include "outputdebug.h"

VideoShowThread::VideoShowThread(QObject *parent) :
    QThread (parent)
{
    quit    = false;
    hasquit = false;
    hassent = false;
}

void VideoShowThread::run()
{
    while(1)
    {
//        QByteArray encodedString = fileName.toLocal8Bit();
//        CvCapture *capture = cvCreateFileCapture(encodedString.data());
        VideoCapture capture(fileName.toStdString().c_str());
        unsigned long interval = 1000 / (unsigned long)capture.get(CAP_PROP_FPS);

        while(quit != true)
        {
            msleep(interval);
            if(ix.canReadVideo)
            {
//                ix.videoImage = cvQueryFrame(capture);
                if(capture.read(ix.videoImage) && quit != true)
                {
                    ix.canReadVideo = false;
                    emit gotVideoFrame();
//                    hassent = true;
                }
                else
                {
//                    while(ix.canClose != true && hassent == false)
//                    {
//                        usleep(100);
//                    }
                    break;
                }
            }
        }

        if(quit)
        {
//            while(ix.canClose != true)
//            {
//                usleep(100);
//            }
            break;
        }

//        cvReleaseCapture(&capture);
        capture.release();
    }

    hasquit = true;
}

void VideoShowThread::closeThread()
{
    quit = true;
    while(hasquit != true)
    {
        usleep(100);
    }

//    hassent = false;
    this->wait();
}
