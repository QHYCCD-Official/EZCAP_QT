#ifndef THREADBURSTCAPTURE_H
#define THREADBURSTCAPTURE_H

#include <QThread>
#include "myStruct.h"

class ThreadBurstCapture : public QThread
{
    Q_OBJECT
public:
    explicit ThreadBurstCapture(QObject *parent = 0);

signals:
    void updateBurstInfo(int iGroup, int iRepeat, int iCount, int seq);
    void updateCFWPos();

public:
    void run();

    bool quit;

    unsigned long interval;
//    int Group;
    QStringList ExpmsList;
    QStringList TrafficList;
    QStringList CountList;
    QStringList CFWList;
    QStringList RepeatList;
    QStringList DelaymsList;
    int iGroup;
    int iRepeat;
    int iCount;
    int iSeqence;
};

#endif // THREADBURSTCAPTURE_H
