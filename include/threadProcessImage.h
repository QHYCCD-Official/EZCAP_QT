#ifndef THREADPROCESSIMAGE_H
#define THREADPROCESSIMAGE_H

#include <QThread>

class ThreadProcessImage : public QThread
{
    Q_OBJECT
public:
    explicit ThreadProcessImage(QObject *parent = 0);

signals:
    void gotFPSData();
    void gotGPSData();
    void gotHistData();
    void gotShowData();
    void gotSaveData();
    void gotDarkData();
    void gotCalData();

private:
    bool quitFlag;
    bool quittedFlag;

public:
    void run();
    void stop();
};

#endif
