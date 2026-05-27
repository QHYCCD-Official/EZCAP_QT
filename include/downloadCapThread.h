#ifndef DOWNLOADCAPTHREAD_H
#define DOWNLOADCAPTHREAD_H

#include "ezCap.h"
#include <QThread>

class DownloadCapThread : public QThread
{
    Q_OBJECT
public:
    explicit DownloadCapThread(QObject *parent = 0);
    bool discardFrame;

signals:
    void updateGPSInfo();

public slots:

protected:
    void run();
};

#endif // DOWNLOADCAPTHREAD_H
