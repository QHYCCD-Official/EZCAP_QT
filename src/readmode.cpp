#include "include/readmode.h"
#include "ui_readmode.h"
#include "ezCap.h"
//#include "qhyccdStatus.h"
#include "include/dllqhyccd.h"
#include "outputdebug.h"
#include <QDebug>
#include <QListView>
#include <QTimer>

extern struct IX ix;
extern qhyccd_handle *camhandle;
ReadMode *readMode;

ReadMode::ReadMode(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ReadMode)
{
    ui->setupUi(this);

    this->setFixedSize(this->width(), this->height());//设置窗口不可改变大小
    this->setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    ui->comboBox_readmode->setView(new QListView());
    mainWidget = (EZCAP*)this->parentWidget();//获取父窗口指针
}

ReadMode::~ReadMode()
{
    delete ui;
}

void ReadMode::showEvent(QShowEvent *){
    if(auto_select){
        qDebug() << "auto select readMode by ini file: autoConnectLive";
        QTimer::singleShot(1000, this, SLOT(on_okpBtn_readmode_clicked()));
    }
}

void ReadMode::on_comboBox_readmode_currentIndexChanged(int index)
{
    if(index == -1)
    {
        ui->okpBtn_readmode->setEnabled(false);
    }
    else
    {
        ix.ReadMode=index;
        ui->okpBtn_readmode->setEnabled(true);
    }
}
void ReadMode::resetUI()
{
    ui->retranslateUi(this);
}

void ReadMode::on_okpBtn_readmode_clicked()
{
    unsigned int ret;
    ui->comboBox_readmode->setDisabled(true);
    ret = libqhyccd->SetQHYCCDReadMode(camhandle, ix.ReadMode);
    if(ret != QHYCCD_SUCCESS)
        qCritical("SetQHYCCDReadMode: failed");
    else
        qDebug() << "SetQHYCCDReadMode success"<<ix.ReadMode;

    ui->okpBtn_readmode->setEnabled(false);

    mainWidget->canConnect = true;

    this->close();//20200226 lyl close dialog after setting
}
