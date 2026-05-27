#ifndef VIDEOSHOWTHREAD_H
#define VIDEOSHOWTHREAD_H

#include <QThread>
#include <QString>

#include "opencv2/highgui.hpp"

using namespace cv;

class VideoShowThread : public QThread
{
    Q_OBJECT
public:
    explicit VideoShowThread(QObject *parent = 0);

signals:
    void gotVideoFrame();

public:
    void run();
    void closeThread();

    QString fileName;

private:
    bool quit;
    bool hasquit;
    bool hassent;
};

#endif // VIDEOSHOWTHREAD_H
