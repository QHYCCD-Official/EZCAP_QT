#include "toolCorrectCenter.h"
#include "ui_toolCorrectCenter.h"
#include "myStruct.h"
#include "ezCap.h"
#include "outputdebug.h"

ToolCorrectCenter *toolCorrectCenter_dialog;

ToolCorrectCenter::ToolCorrectCenter(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ToolCorrectCenter)
{
    ui->setupUi(this);
}

ToolCorrectCenter::~ToolCorrectCenter()
{
    delete ui;
}

void ToolCorrectCenter::camera_connected()
{
    ui->comboBox_Correct->setCurrentText("OFF");

    bool ok = false;
    int value = 0;
    ok = mainWidget->loadParamFromIni(ix.CamModel + "-" + QString::number(ix.camStreamMode), "CircleRadius", &value, 0);
    if(ok && value != 0)
    {
        ix.Circle_Radius = value;
        ui->spinBox_Radius->setValue(ix.Circle_Radius);
    }
    else
    {
        ui->spinBox_Radius->setValue((ix.ImageW_Min < ix.ImageH_Min ? ix.ImageW_Min : ix.ImageH_Min) / 2);
    }
    ok = mainWidget->loadParamFromIni(ix.CamModel + "-" + QString::number(ix.camStreamMode), "CircleSpacing", &value, 0);
    if(ok && value != 0)
    {
        ix.Circle_Spacing = value;
        ui->spinBox_Spacing->setValue(ix.Circle_Spacing);
    }
    else
    {
        ui->spinBox_Spacing->setValue(20);
    }
    ok = mainWidget->loadParamFromIni(ix.CamModel + "-" + QString::number(ix.camStreamMode), "CircleThickness", &value, 0);
    if(ok && value != 0)
    {
        ix.Circle_Thickness = value;
        ui->spinBox_Thickness->setValue(ix.Circle_Thickness);
    }
    else
    {
        ui->spinBox_Thickness->setValue(1);
    }
    ok = mainWidget->loadParamFromIni(ix.CamModel + "-" + QString::number(ix.camStreamMode), "CircleColor", &value, 0);
    if(ok)
    {
        ix.Circle_Color = value;
        if(ix.Circle_Color == CIRCLE_RED)
            ui->radioButton_Red->setChecked(true);
        else if(ix.Circle_Color == CIRCLE_GREEN)
            ui->radioButton_Green->setChecked(true);
        else if(ix.Circle_Color == CIRCLE_BLUE)
            ui->radioButton_Blue->setChecked(true);
        else if(ix.Circle_Color == CIRCLE_YELLOW)
            ui->radioButton_Yellow->setChecked(true);
        else if(ix.Circle_Color == CIRCLE_WHITE)
            ui->radioButton_White->setChecked(true);
        else if(ix.Circle_Color == CIRCLE_BLACK)
            ui->radioButton_Black->setChecked(true);
    }
    else
    {
        ui->radioButton_Red->setChecked(true);
    }
}

void ToolCorrectCenter::camera_disconnected()
{
    ui->comboBox_Correct->setCurrentText("OFF");

    this->close();
}

void ToolCorrectCenter::on_comboBox_Correct_currentTextChanged(const QString &arg1)
{
    if(arg1 == "ON")
    {
        ix.Circle_Correct = true;
    }
    else
    {
        ix.Circle_Correct = false;
    }
}

void ToolCorrectCenter::on_spinBox_Radius_valueChanged(int arg1)
{
    if(arg1 - ix.Circle_Spacing >= 1)
    {
        ix.Circle_Radius = arg1;
    }
    else
    {
        ix.Circle_Radius = ix.Circle_Spacing + 1;
        ui->spinBox_Radius->setValue(ix.Circle_Radius);
    }
}

void ToolCorrectCenter::on_pushButton_RadiusAdd10_clicked()
{
    ui->spinBox_Radius->setValue(ui->spinBox_Radius->value() + 10);
}


void ToolCorrectCenter::on_pushButton_RadiusSub10_clicked()
{
    if(ui->spinBox_Radius->value() - 10 - ix.Circle_Spacing >= 1)
    {
        ui->spinBox_Radius->setValue(ui->spinBox_Radius->value() - 10);
    }
    else
    {
        ui->spinBox_Radius->setValue(ix.Circle_Spacing + 1);
    }
}

void ToolCorrectCenter::on_pushButton_RadiusAdd100_clicked()
{
    ui->spinBox_Radius->setValue(ui->spinBox_Radius->value() + 100);
}

void ToolCorrectCenter::on_pushButton_RadiusSub100_clicked()
{
    if(ui->spinBox_Radius->value() - 100 - ix.Circle_Spacing >= 1)
    {
        ui->spinBox_Radius->setValue(ui->spinBox_Radius->value() - 100);
    }
    else
    {
        ui->spinBox_Radius->setValue(ix.Circle_Spacing + 1);
    }
}

void ToolCorrectCenter::on_spinBox_Spacing_valueChanged(int arg1)
{
    if(arg1 >= 1)
        ix.Circle_Spacing = arg1;
    else
    {
        ix.Circle_Spacing = 1;
        ui->spinBox_Spacing->setValue(1);
    }
}

void ToolCorrectCenter::on_pushButton_SpacingAdd10_clicked()
{
    ui->spinBox_Spacing->setValue(ui->spinBox_Spacing->value() + 10);
}

void ToolCorrectCenter::on_pushButton_SpacingSub10_clicked()
{
    ui->spinBox_Spacing->setValue(ui->spinBox_Spacing->value() - 10);
}

void ToolCorrectCenter::on_spinBox_Thickness_valueChanged(int arg1)
{
    ix.Circle_Thickness = arg1;
}

void ToolCorrectCenter::on_radioButton_Red_toggled(bool checked)
{
    if(checked)
        ix.Circle_Color = CIRCLE_RED;
}

void ToolCorrectCenter::on_radioButton_Green_toggled(bool checked)
{
    if(checked)
        ix.Circle_Color = CIRCLE_GREEN;
}

void ToolCorrectCenter::on_radioButton_Blue_toggled(bool checked)
{
    if(checked)
        ix.Circle_Color = CIRCLE_BLUE;
}

void ToolCorrectCenter::on_radioButton_Yellow_toggled(bool checked)
{
    if(checked)
        ix.Circle_Color = CIRCLE_YELLOW;
}

void ToolCorrectCenter::on_radioButton_White_toggled(bool checked)
{
    if(checked)
        ix.Circle_Color = CIRCLE_WHITE;
}

void ToolCorrectCenter::on_radioButton_Black_toggled(bool checked)
{
    if(checked)
        ix.Circle_Color = CIRCLE_BLACK;
}

void ToolCorrectCenter::on_pushButton_Save_clicked()
{
    mainWidget->saveParamToIni(ix.CamModel + "-" + QString::number(ix.camStreamMode), "CircleRadius", ix.Circle_Radius);
    mainWidget->saveParamToIni(ix.CamModel + "-" + QString::number(ix.camStreamMode), "CircleSpacing", ix.Circle_Spacing);
    mainWidget->saveParamToIni(ix.CamModel + "-" + QString::number(ix.camStreamMode), "CircleThickness", ix.Circle_Thickness);
    mainWidget->saveParamToIni(ix.CamModel + "-" + QString::number(ix.camStreamMode), "CircleColor", ix.Circle_Color);
}

