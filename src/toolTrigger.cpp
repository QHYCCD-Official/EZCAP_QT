#include "toolTrigger.h"
#include "ui_toolTrigger.h"
#include "dllqhyccd.h"
#include "outputdebug.h"

ToolTrigger *toolTrigger_dialog;

extern qhyccd_handle *camhandle;

ToolTrigger::ToolTrigger(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ToolTrigger)
{
    ui->setupUi(this);
}

ToolTrigger::~ToolTrigger()
{
    delete ui;
}

void ToolTrigger::camera_connected()
{
    if (!libqhyccd->GetQHYCCDTrigerInterfaceNumber ||
        !libqhyccd->GetQHYCCDTrigerInterfaceName   ||
        !libqhyccd->SetQHYCCDTrigerInterface       ||
        !libqhyccd->GetQHYCCDTrigerModeNumber      ||
        !libqhyccd->GetQHYCCDTrigerModeName        ||
        !libqhyccd->SetQHYCCDTrigerMode            ||
        !libqhyccd->SetQHYCCDTrigerFunction        ||
        !libqhyccd->SetQHYCCDTrigerInOnly          ||
        !libqhyccd->SetQHYCCDTrigerOutOnly)
        return;

    ui->comboBox_TrigInterface->blockSignals(true);
    ui->comboBox_TrigMode->blockSignals(true);
    ui->comboBox_Trigger->blockSignals(true);
    ui->comboBox_TrigInOnly->blockSignals(true);
    ui->comboBox_TrigOutOnly->blockSignals(true);

    uint32_t ret = QHYCCD_ERROR;
    uint32_t num = 0;
    char name[40] = { 0 };
    ui->comboBox_TrigInterface->clear();
    libqhyccd->GetQHYCCDTrigerInterfaceNumber(camhandle, &num);
    for(uint32_t i = 0; i < num; i++)
    {
        ret = libqhyccd->GetQHYCCDTrigerInterfaceName(camhandle, i, name);
        if(ret == QHYCCD_SUCCESS) ui->comboBox_TrigInterface->addItem(name);
    }
    ui->comboBox_TrigInterface->setCurrentIndex(0);
    ret = libqhyccd->SetQHYCCDTrigerInterface(camhandle, 0);

    ui->comboBox_TrigMode->clear();
    ret = libqhyccd->GetQHYCCDTrigerModeNumber(camhandle, &num);
    for(uint32_t i = 0; i < num; i++)
    {
        ret = libqhyccd->GetQHYCCDTrigerModeName(camhandle, i, name);
        if(ret == QHYCCD_SUCCESS) ui->comboBox_TrigMode->addItem(name);
    }
    ui->comboBox_TrigMode->setCurrentIndex(0);
//    ret = libqhyccd->SetQHYCCDTrigerMode(camhandle, 0);

    ui->comboBox_Trigger->clear();
    ui->comboBox_Trigger->addItem("ON");
    ui->comboBox_Trigger->addItem("OFF");
    ui->comboBox_Trigger->setCurrentText("OFF");
//    ret = libqhyccd->SetQHYCCDTrigerFunction(camhandle, false);

    ui->comboBox_TrigInOnly->clear();
    ui->comboBox_TrigInOnly->addItem("ON");
    ui->comboBox_TrigInOnly->addItem("OFF");
    ui->comboBox_TrigInOnly->setCurrentText("OFF");
//    ret = libqhyccd->SetQHYCCDTrigerInOnly(camhandle, false);

    ui->comboBox_TrigOutOnly->clear();
    ui->comboBox_TrigOutOnly->addItem("ON");
    ui->comboBox_TrigOutOnly->addItem("OFF");
    ui->comboBox_TrigOutOnly->setCurrentText("OFF");
//    ret = libqhyccd->SetQHYCCDTrigerOutOnly(camhandle, false);

    ui->comboBox_TrigInterface->blockSignals(false);
    ui->comboBox_TrigMode->blockSignals(false);
    ui->comboBox_Trigger->blockSignals(false);
    ui->comboBox_TrigInOnly->blockSignals(false);
    ui->comboBox_TrigOutOnly->blockSignals(false);
}

void ToolTrigger::camera_disconnected()
{
    this->close();

//    ui->comboBox_TrigInterface->blockSignals(true);
//    ui->comboBox_TrigMode->blockSignals(true);
//    ui->comboBox_Trigger->blockSignals(true);
//    ui->comboBox_TrigInOnly->blockSignals(true);
//    ui->comboBox_TrigOutOnly->blockSignals(true);

//    uint32_t ret = QHYCCD_ERROR;
//    ui->comboBox_TrigInterface->setCurrentIndex(0);
//    ret = libqhyccd->SetQHYCCDTrigerInterface(camhandle, 0);

//    ui->comboBox_TrigMode->setCurrentIndex(0);
//    ret = libqhyccd->SetQHYCCDTrigerMode(camhandle, 0);

//    ui->comboBox_Trigger->setCurrentText("OFF");
//    ret = libqhyccd->SetQHYCCDTrigerFunction(camhandle, false);

//    ui->comboBox_TrigInOnly->setCurrentText("OFF");
//    ret = libqhyccd->SetQHYCCDTrigerInOnly(camhandle, false);

//    ui->comboBox_TrigOutOnly->setCurrentText("OFF");
//    ret = libqhyccd->SetQHYCCDTrigerOutOnly(camhandle, false);

//    ui->comboBox_TrigInterface->blockSignals(false);
//    ui->comboBox_TrigMode->blockSignals(false);
//    ui->comboBox_Trigger->blockSignals(false);
//    ui->comboBox_TrigInOnly->blockSignals(false);
//    ui->comboBox_TrigOutOnly->blockSignals(false);
}

void ToolTrigger::on_comboBox_TrigInterface_currentIndexChanged(int index)
{
    if(!libqhyccd->SetQHYCCDTrigerInterface) return;

    libqhyccd->SetQHYCCDTrigerInterface(camhandle, index);
}

void ToolTrigger::on_comboBox_TrigMode_currentIndexChanged(int index)
{
    if (!libqhyccd->SetQHYCCDTrigerMode) return;

    libqhyccd->SetQHYCCDTrigerMode(camhandle, index);
}

void ToolTrigger::on_comboBox_Trigger_currentTextChanged(const QString &arg1)
{
    if (!libqhyccd->SetQHYCCDTrigerFunction) return;

    if(arg1 == "ON")
    {
        ui->groupBox_TrigInterfaceMode->setEnabled(false);
        ui->groupBox_TrigInOrOut->setEnabled(false);
        libqhyccd->SetQHYCCDTrigerFunction(camhandle, true);
    }
    else
    {
        ui->groupBox_TrigInterfaceMode->setEnabled(true);
        ui->groupBox_TrigInOrOut->setEnabled(true);
        libqhyccd->SetQHYCCDTrigerFunction(camhandle, false);
    }
}

void ToolTrigger::on_comboBox_TrigInOnly_currentTextChanged(const QString &arg1)
{
    if (!libqhyccd->SetQHYCCDTrigerInOnly) return;

    if(arg1 == "ON")
    {
        ui->groupBox_TrigInterfaceMode->setEnabled(false);
        ui->groupBox_TrigInAndOut->setEnabled(false);
        libqhyccd->SetQHYCCDTrigerInOnly(camhandle, true);
    }
    else
    {
        if (ui->comboBox_TrigOutOnly->currentText() == "OFF")
        {
            ui->groupBox_TrigInterfaceMode->setEnabled(true);
            ui->groupBox_TrigInAndOut->setEnabled(true);
        }
        libqhyccd->SetQHYCCDTrigerInOnly(camhandle, false);
    }
}


void ToolTrigger::on_comboBox_TrigOutOnly_currentTextChanged(const QString &arg1)
{
    if (!libqhyccd->SetQHYCCDTrigerOutOnly) return;

    if(arg1 == "ON")
    {
        ui->groupBox_TrigInterfaceMode->setEnabled(false);
        ui->groupBox_TrigInAndOut->setEnabled(false);
        libqhyccd->SetQHYCCDTrigerOutOnly(camhandle, true);
    }
    else
    {
        if (ui->comboBox_TrigInOnly->currentText() == "OFF")
        {
            ui->groupBox_TrigInterfaceMode->setEnabled(true);
            ui->groupBox_TrigInAndOut->setEnabled(true);
        }
        libqhyccd->SetQHYCCDTrigerOutOnly(camhandle, false);
    }
}
