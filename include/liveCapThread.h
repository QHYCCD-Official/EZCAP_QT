#ifndef LIVECAPTHREAD_H
#define LIVECAPTHREAD_H

#include <QThread>
#include <QDateTime>

#include "myStruct.h"

class LiveCapThread : public QThread
{
    Q_OBJECT
public:
    explicit LiveCapThread(QObject *parent = 0);

signals:
    void gotFPSData();
    void gotGPSData();
    void gotHistData();
    void gotShowData();
    void gotSaveData();
    void gotDarkData();
    void gotCalData();

public:
    void run();
    void closeThread();

private:
    bool quit;
    bool quitted;
    qint64 timeStart;
    qint64 timeEnd;
    qint64 timeShowStart;
    qint64 timeShowEnd;
};

#endif // LIVECAPTHREAD_H
