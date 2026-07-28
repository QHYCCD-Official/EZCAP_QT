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
    const bool CanDPC = (libqhyccd->IsQHYCCDControlAvailable(camhandle, CONTROL_DPC) == QHYCCD_SUCCESS);

    // Keep tabs clickable; disable controls inside unsupported tabs only.
    ui->tabWidget_SetupTap->setTabEnabled(0, true);
    ui->tabWidget_SetupTap->setTabEnabled(1, true);

    ui->comboBox_HeatingBoardLevel->setEnabled(hasHeatingBoard);
    ui->doubleSpinBox_CAARotatorAngle->setEnabled(hasCaaRotator);
    ui->pushButton_CAARotatorSet->setEnabled(hasCaaRotator);

    if(CanDPC)
    {
        ui->comboBox_DPC->setEnabled(true);
        ui->horizontalSlider_DPCValue->setEnabled(true);
        ui->spinBox_DPCValue->setEnabled(true);

        ui->comboBox_DPC->blockSignals(true);
        ui->horizontalSlider_DPCValue->blockSignals(true);
        ui->spinBox_DPCValue->blockSignals(true);

        ui->comboBox_DPC->clear();
        ui->comboBox_DPC->addItem("ON");
        ui->comboBox_DPC->addItem("OFF");
        ui->comboBox_DPC->setCurrentText("OFF");

        double min = 0.0, max = 0.0, step = 0.0;
        libqhyccd->GetQHYCCDParamMinMaxStep(camhandle, CONTROL_DPC_value, &min, &max, &step);
        ui->horizontalSlider_DPCValue->setRange((int)min, (int)max);
        ui->horizontalSlider_DPCValue->setValue((int)libqhyccd->GetQHYCCDParam(camhandle, CONTROL_DPC_value));
        ui->spinBox_DPCValue->setRange((int)min, (int)max);
        ui->spinBox_DPCValue->setValue((int)libqhyccd->GetQHYCCDParam(camhandle, CONTROL_DPC_value));

        ui->comboBox_DPC->blockSignals(false);
        ui->horizontalSlider_DPCValue->blockSignals(false);
        ui->spinBox_DPCValue->blockSignals(false);
    }
    else
    {
        ui->comboBox_DPC->setEnabled(false);
        ui->horizontalSlider_DPCValue->setEnabled(false);
        ui->spinBox_DPCValue->setEnabled(false);
    }
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

void OtherCameraSetup::on_comboBox_DPC_currentTextChanged(const QString &arg1)
{
    libqhyccd->SetQHYCCDParam(camhandle, CONTROL_DPC, arg1 == "ON" ? 1.0 : 0.0);
}


void OtherCameraSetup::on_horizontalSlider_DPCValue_valueChanged(int value)
{
    libqhyccd->SetQHYCCDParam(camhandle, CONTROL_DPC_value, (double)value);

    ui->spinBox_DPCValue->blockSignals(true);
    ui->spinBox_DPCValue->setValue(value);
    ui->spinBox_DPCValue->blockSignals(false);
}


void OtherCameraSetup::on_spinBox_DPCValue_valueChanged(int arg1)
{
    libqhyccd->SetQHYCCDParam(camhandle, CONTROL_DPC_value, (double)arg1);

    ui->horizontalSlider_DPCValue->blockSignals(true);
    ui->horizontalSlider_DPCValue->setValue(arg1);
    ui->horizontalSlider_DPCValue->blockSignals(false);
}

