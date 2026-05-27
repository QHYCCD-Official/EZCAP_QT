#include "toolBurst.h"
#include "ui_toolBurst.h"
#include "dllqhyccd.h"
#include "outputdebug.h"
#include "ezCap.h"
#include "liveCapThread.h"
#include "managementMenu.h"
#include <QFont>
#include <QColor>
#include <QMessageBox>

ToolBurst *toolBurst_dialog;
extern qhyccd_handle *camhandle;

ToolBurst::ToolBurst(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ToolBurst)
{
    ui->setupUi(this);

    this->setFixedSize(this->width(), this->height());//fixed size of window
    this->setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);// hide the "?" help button ont the title bar
}

ToolBurst::~ToolBurst()
{
    delete ui;
}

void ToolBurst::camera_connected()
{
    ui->comboBox_Burst->blockSignals(true);
    ui->comboBox_Burst->clear();
    ui->comboBox_Burst->addItem("ON");
    ui->comboBox_Burst->addItem("OFF");
    ui->comboBox_Burst->setCurrentText("OFF");
    ui->comboBox_Burst->blockSignals(false);

    ui->tableWidget_Burst->clear();
    ui->tableWidget_Burst->setColumnCount(7);
    ui->tableWidget_Burst->setColumnWidth(0, 22);
    ui->tableWidget_Burst->setColumnWidth(1, 75);
    ui->tableWidget_Burst->setColumnWidth(2, 75);
    ui->tableWidget_Burst->setColumnWidth(3, 75);
    ui->tableWidget_Burst->setColumnWidth(4, 75);
    ui->tableWidget_Burst->setColumnWidth(5, 75);
    ui->tableWidget_Burst->setColumnWidth(6, 75);
    QStringList labelList;
    labelList.append("Use");
    labelList.append("Exp(ms)");
    labelList.append("Traffic");
    labelList.append("Count");
    labelList.append("CFW");
    labelList.append("Repeat");
    labelList.append("Delay(ms)");
    ui->tableWidget_Burst->setHorizontalHeaderLabels(labelList);
    ui->tableWidget_Burst->verticalHeader()->setFixedWidth(22);
    ui->tableWidget_Burst->verticalHeader()->setDefaultAlignment(Qt::AlignCenter);
    ui->tableWidget_Burst->horizontalHeader()->setEnabled(false);

    if(!ix.canFilterWheel || !ix.CFW_Plugged)
    {
        ui->tableWidget_Burst->horizontalHeaderItem(4)->setText("CFW(N/A)");
    }

    int rowCount = ui->tableWidget_Burst->rowCount();
    DBGOPT_INFO("rowCount = %d", rowCount);

    for(; rowCount < 5; rowCount++)
    {
        ui->tableWidget_Burst->insertRow(rowCount);
        ui->tableWidget_Burst->setRowHeight(rowCount, 22);

        QTableWidgetItem *itemUse = new QTableWidgetItem();
        itemUse->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        itemUse->setCheckState(Qt::Unchecked);
        itemUse->setFlags(itemUse->flags() & ~Qt::ItemIsEditable);
        ui->tableWidget_Burst->setItem(rowCount, 0, itemUse);

        QTableWidgetItem *itemExpms = new QTableWidgetItem();
        itemExpms->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        itemExpms->setData(Qt::DisplayRole, 100);
        ui->tableWidget_Burst->setItem(rowCount, 1, itemExpms);

        QTableWidgetItem *itemTraffic = new QTableWidgetItem();
        itemTraffic->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        itemTraffic->setData(Qt::DisplayRole, 0);
        ui->tableWidget_Burst->setItem(rowCount, 2, itemTraffic);

        QTableWidgetItem *itemCount = new QTableWidgetItem();
        itemCount->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        itemCount->setData(Qt::DisplayRole, 1);
        ui->tableWidget_Burst->setItem(rowCount, 3, itemCount);

        QTableWidgetItem *itemCFW = new QTableWidgetItem();
        itemCFW->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        itemCFW->setData(Qt::DisplayRole, 1);
        if(!ix.canFilterWheel || !ix.CFW_Plugged)
        {
            itemCFW->setFlags(itemCFW->flags() & ~Qt::ItemIsEditable);
            itemCFW->setFlags(itemCFW->flags() & ~Qt::ItemIsEnabled);
            itemCFW->setText("");
        }
        ui->tableWidget_Burst->setItem(rowCount, 4, itemCFW);

        QTableWidgetItem *itemRepeat = new QTableWidgetItem();
        itemRepeat->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        itemRepeat->setData(Qt::DisplayRole, 1);
        ui->tableWidget_Burst->setItem(rowCount, 5, itemRepeat);

        QTableWidgetItem *itemDelayms = new QTableWidgetItem();
        itemDelayms->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        itemDelayms->setData(Qt::DisplayRole, 0);
        ui->tableWidget_Burst->setItem(rowCount, 6, itemDelayms);
    }
}

void ToolBurst::camera_disconnected()
{
    ui->comboBox_Burst->blockSignals(true);
    ui->comboBox_Burst->setCurrentText("OFF");
    ui->comboBox_Burst->blockSignals(false);

    int rowCount = ui->tableWidget_Burst->rowCount();
    for(int i = 0; i < rowCount; i++)
    {
        ui->tableWidget_Burst->removeRow(0);
    }

    libqhyccd->EnableQHYCCDBurstMode(camhandle, false);

    this->close();
}

void ToolBurst::updateBurstInfo(int iGroup, int iRepeat, int iCount, int iSeqence)
{
    ui->textBrowser_Info->append("Burst Capture : " +
                                 QString::number(iGroup)  + " - " +
                                 QString::number(iRepeat) + " - " +
                                 QString::number(iCount)  + " - " +
                                 QString::number(iSeqence));

    if(iRepeat == ix.BurstRepeat && iCount == ix.BurstCount)
        ui->textBrowser_Info->append("Busrt Capture : delay " + threadBurstCapture->DelaymsList.at(iGroup - 1) + " ms");

    if(iGroup  == ix.BurstGroup &&
       iRepeat == ix.BurstRepeat &&
       iCount  == ix.BurstCount) ui->textBrowser_Info->append("Burst Capture Finished ...\n");
}

void ToolBurst::burstThreadDelete()
{
    delete threadBurstCapture;
    threadBurstCapture = NULL;
}

void ToolBurst::on_comboBox_Burst_currentTextChanged(const QString &arg1)
{
    uint32_t ret = QHYCCD_ERROR;

    if(arg1 == "ON")
    {
        if(mainWidget->liveCap->isRunning())
        {
            mainWidget->liveCap->closeThread();
        }

        ret = libqhyccd->EnableQHYCCDBurstMode(camhandle, true);
        ret = libqhyccd->SetQHYCCDBurstModeStartEnd(camhandle, 1, 3);
        ret = libqhyccd->SetQHYCCDBurstModePatchNumber(camhandle, ui->spinBox_Patch->value());
        ret = libqhyccd->EnableQHYCCDBurstCountFun(camhandle, true);

        ix.Burst = true;
    }
    else
    {
        DBGOPT_INFO("EnableQHYCCDBurstMode false");
        ret = libqhyccd->EnableQHYCCDBurstMode(camhandle, false);
        ret = libqhyccd->ReleaseQHYCCDBurstIDLE(camhandle);

        if(managerMenu->ResetParameters())
        {
            QMessageBox::critical(this,tr("Error"),tr("Camera re-set parameters failed!"), QMessageBox::Ok);

            ix.ReadMode = ix.ReadMode_Last;
            managerMenu->CloseCamera();
            emit mainWidget->disconnect_camera();
            return;
        }
        mainWidget->liveCap->start();

        ix.Burst = false;
    }
}

void ToolBurst::on_spinBox_Patch_valueChanged(int arg1)
{
    if(!ix.Burst)
    {
        ui->textBrowser_Info->append("Please turn on burst mode firstly ...\n");
        return;
    }

    uint32_t ret = QHYCCD_ERROR;
    ret = libqhyccd->SetQHYCCDBurstModePatchNumber(camhandle, arg1);
}

void ToolBurst::on_pushButton_BurstCapure_clicked()
{
    if(!ix.Burst)
    {
        ui->textBrowser_Info->append("Please turn on burst mode firstly ...\n");
        return;
    }

    if(threadBurstCapture != NULL)
    {
        delete threadBurstCapture;
        threadBurstCapture = NULL;
    }

    threadBurstCapture = new ThreadBurstCapture(this);
    connect(threadBurstCapture, SIGNAL(updateBurstInfo(int, int, int, int)), this, SLOT(updateBurstInfo(int, int, int, int)));
    connect(threadBurstCapture, SIGNAL(finished()), this, SLOT(burstThreadDelete()));

    threadBurstCapture->ExpmsList.clear();
    threadBurstCapture->TrafficList.clear();
    threadBurstCapture->CountList.clear();
    threadBurstCapture->CFWList.clear();
    threadBurstCapture->RepeatList.clear();
    threadBurstCapture->DelaymsList.clear();

    int group = 0;
    for(int i = 0; i < ui->tableWidget_Burst->rowCount(); i++)
    {
        if(ui->tableWidget_Burst->item(i, 0)->checkState() == Qt::Checked)
        {
            group++;
            threadBurstCapture->ExpmsList.append(ui->tableWidget_Burst->item(i, 1)->text());
            threadBurstCapture->TrafficList.append(ui->tableWidget_Burst->item(i, 2)->text());
            threadBurstCapture->CountList.append(ui->tableWidget_Burst->item(i, 3)->text());
            if(ix.canFilterWheel && ix.CFW_Plugged) threadBurstCapture->CFWList.append(ui->tableWidget_Burst->item(i, 4)->text());
            threadBurstCapture->RepeatList.append(ui->tableWidget_Burst->item(i, 5)->text());
            threadBurstCapture->DelaymsList.append(ui->tableWidget_Burst->item(i, 6)->text());
        }
    }
    ix.BurstGroup = group;

    threadBurstCapture->quit = false;
    threadBurstCapture->start();

    ui->textBrowser_Info->append("Burst Capture : Group - Repeat - Count - Sequence");
}

void ToolBurst::on_pushButton_BurstAbort_clicked()
{
    if(!ix.Burst)
    {
        ui->textBrowser_Info->append("Please turn on burst mode firstly ...\n");
        return;
    }

    uint32_t ret = QHYCCD_ERROR;
    ret = libqhyccd->SetQHYCCDBurstIDLE(camhandle);
    if(ret == QHYCCD_ERROR) ui->textBrowser_Info->append("SetQHYCCDBurstIDLE() failed ...");

    threadBurstCapture->quit = true;
    while(threadBurstCapture->quit)
    {
        QThread::usleep(100);
    }

    ui->textBrowser_Info->append("Abort Burst Capture ...\n");
}

void ToolBurst::on_pushButton_AddGroup_clicked()
{
    int rowCount = ui->tableWidget_Burst->rowCount();
    ui->tableWidget_Burst->insertRow(rowCount);
    ui->tableWidget_Burst->setRowHeight(rowCount, 22);

    QTableWidgetItem *itemUse = new QTableWidgetItem();
    itemUse->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    itemUse->setCheckState(Qt::Unchecked);
    itemUse->setFlags(itemUse->flags() & ~Qt::ItemIsEditable);
    ui->tableWidget_Burst->setItem(rowCount, 0, itemUse);

    QTableWidgetItem *itemExpms = new QTableWidgetItem();
    itemExpms->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    itemExpms->setData(Qt::DisplayRole, 100);
    ui->tableWidget_Burst->setItem(rowCount, 1, itemExpms);

    QTableWidgetItem *itemTraffic = new QTableWidgetItem();
    itemTraffic->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    itemTraffic->setData(Qt::DisplayRole, 0);
    ui->tableWidget_Burst->setItem(rowCount, 2, itemTraffic);

    QTableWidgetItem *itemCount = new QTableWidgetItem();
    itemCount->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    itemCount->setData(Qt::DisplayRole, 1);
    ui->tableWidget_Burst->setItem(rowCount, 3, itemCount);

    QTableWidgetItem *itemCFW = new QTableWidgetItem();
    itemCFW->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    itemCFW->setData(Qt::DisplayRole, 1);
    if(!ix.canFilterWheel || !ix.CFW_Plugged)
    {
        itemCFW->setFlags(itemCFW->flags() & ~Qt::ItemIsEditable);
        itemCFW->setFlags(itemCFW->flags() & ~Qt::ItemIsEnabled);
        itemCFW->setText("");
    }
    ui->tableWidget_Burst->setItem(rowCount, 4, itemCFW);

    QTableWidgetItem *itemRepeat = new QTableWidgetItem();
    itemRepeat->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    itemRepeat->setData(Qt::DisplayRole, 1);
    ui->tableWidget_Burst->setItem(rowCount, 5, itemRepeat);

    QTableWidgetItem *itemDelayms = new QTableWidgetItem();
    itemDelayms->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    itemDelayms->setData(Qt::DisplayRole, 0);
    ui->tableWidget_Burst->setItem(rowCount, 6, itemDelayms);
}


void ToolBurst::on_pushButton_ClearInfo_clicked()
{
    ui->textBrowser_Info->clear();
}
