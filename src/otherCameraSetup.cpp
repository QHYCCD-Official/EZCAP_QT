#include "otherCameraSetup.h"
#include "ui_otherCameraSetup.h"
#include "dllqhyccd.h"
#include "myStruct.h"
#include "outputdebug.h"

extern struct IX ix;
extern qhyccd_handle *camhandle;

OtherCameraSetup *otherCameraSetup_dialog;

OtherCameraSetup::OtherCameraSetup(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OtherCameraSetup)
{
    ui->setupUi(this);
}

OtherCameraSetup::~OtherCameraSetup()
{
    delete ui;
}

void OtherCameraSetup::camera_connected()
{
    const bool hasHeatingBoard = (libqhyccd->IsQHYCCDControlAvailable(camhandle, CONTROL_HEATINGBOARD) == QHYCCD_SUCCESS);
    const bool hasCaaRotator = (libqhyccd->IsQHYCCDControlAvailable(camhandle, CONTROL_CAA_ROTATOR) == QHYCCD_SUCCESS);

    // Keep tabs clickable; disable controls inside unsupported tabs only.
    ui->tabWidget_SetupTap->setTabEnabled(0, true);
    ui->tabWidget_SetupTap->setTabEnabled(1, true);

    ui->comboBox_HeatingBoardLevel->setEnabled(hasHeatingBoard);
    ui->doubleSpinBox_CAARotatorAngle->setEnabled(hasCaaRotator);
    ui->pushButton_CAARotatorSet->setEnabled(hasCaaRotator);
}

void OtherCameraSetup::camera_disconnected()
{
    ui->tabWidget_SetupTap->setTabEnabled(0, true);
    ui->tabWidget_SetupTap->setTabEnabled(1, true);
    ui->comboBox_HeatingBoardLevel->setEnabled(false);
    ui->doubleSpinBox_CAARotatorAngle->setEnabled(false);
    ui->pushButton_CAARotatorSet->setEnabled(false);
}

void OtherCameraSetup::on_comboBox_HeatingBoardLevel_activated(const QString &arg1)
{
    if(arg1 == "OFF")
        libqhyccd->SetQHYCCDParam(camhandle, CONTROL_HEATINGBOARD, 0.0);
    else if(arg1 == "MID RANGE")
        libqhyccd->SetQHYCCDParam(camhandle, CONTROL_HEATINGBOARD, 2.0);
    else if(arg1 == "HIGH RANGE")
        libqhyccd->SetQHYCCDParam(camhandle, CONTROL_HEATINGBOARD, 3.0);
}

void OtherCameraSetup::on_pushButton_CAARotatorSet_clicked()
{
    const double angle = ui->doubleSpinBox_CAARotatorAngle->value();

    const uint32_t ret = libqhyccd->SetQHYCCDParam(camhandle, CONTROL_CAA_ROTATOR, angle);
    if(ret == QHYCCD_ERROR)
    {
        OutputDebug("EZCAPWARNING | %s | %s | SetQHYCCDParam CONTROL_CAA_ROTATOR failed", __FILE__, __FUNCTION__);
    }
}
