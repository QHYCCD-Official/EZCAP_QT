#include "frameToolCapCal.h"
#include "ui_frameToolCapCal.h"
#include "ezCap.h"
#include "outputdebug.h"
#include "myStruct.h"
#include <QMessageBox>
#include <QDir>
#include <QDesktopServices>

extern qhyccd_handle *camhandle;

FrameToolCapCal *frameToolCapCal_dialog;

FrameToolCapCal::FrameToolCapCal(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FrameToolCapCal)
{
    ui->setupUi(this);
    mainWidget = (EZCAP*)this->parentWidget();
}

FrameToolCapCal::~FrameToolCapCal()
{
    delete ui;
}

void FrameToolCapCal::setEnableUI()
{
    if(capCalFlag == 2) libqhyccd->SetQHYCCDParam(camhandle, CONTROL_EXPOSURE, ClampExposureUs(ix.ExpTime));

    ui->radioButton_CapCalDark->setEnabled(true);
    ui->radioButton_CapCalFlat->setEnabled(true);
    ui->radioButton_CapCalBias->setEnabled(true);
    ui->spinBoxDarkNum->setEnabled(true);
    ui->btnStartDarkCap->setEnabled(true);
}

void FrameToolCapCal::setDisableUI()
{
    ui->radioButton_CapCalDark->setEnabled(false);
    ui->radioButton_CapCalFlat->setEnabled(false);
    ui->radioButton_CapCalBias->setEnabled(false);
    ui->spinBoxDarkNum->setEnabled(false);
    ui->btnStartDarkCap->setEnabled(false);
}

void FrameToolCapCal::showSaveFolder(QString folder)
{
//    ui->labelDarkFrameFolder->setWordWrap(true);
//    ui->labelDarkFrameFolder->setText("Dark Frame has been saved in " + folder);
}

void FrameToolCapCal::on_btnStartDarkCap_clicked()
{
    this->setDisableUI();

    memset(ix.ImgData_CalSave, 0, (ix.ImageW_Max+3)/4*4 * ix.ImageH_Max * 3 * sizeof(uint32_t));
    mainWidget->saveCal();

    if(capCalFlag == 2) libqhyccd->SetQHYCCDParam(camhandle, CONTROL_EXPOSURE, ClampExposureUs(1.0));
    int nowFrame = ix.frame;
    while(ix.frame == nowFrame)
    {
        QThread::msleep(10);
        QCoreApplication::processEvents();
    }

    ix.calNum = static_cast<uint32_t>(ui->spinBoxDarkNum->value());
    ix.saveFlag = true;
}

void FrameToolCapCal::on_radioButton_CapCalDark_clicked()
{
    capCalFlag = 0;
}

void FrameToolCapCal::on_radioButton_CapCalFlat_clicked()
{
    capCalFlag = 1;
}

void FrameToolCapCal::on_radioButton_CapCalBias_clicked()
{
    capCalFlag = 2;
}

void FrameToolCapCal::on_pushButton_OpenCalSaveDir_clicked()
{
    if(savePath.isEmpty())
    {
        QMessageBox::information(this,tr("Warning"), tr("Has no images be saved!"), QMessageBox::Ok);
    }
    else
    {
        savePath = QDir::toNativeSeparators(savePath);
        QUrl url = QUrl::fromLocalFile(savePath);
        QDesktopServices::openUrl(url); //open folder
    }
}
