#include "favorite.h"
#include "ui_favorite.h"
#include "include/dllqhyccd.h"
#include "ezCap.h"
#include "fpgaAccess.h"

//#include "qhyccdStatus.h"
#include "myStruct.h"

#include <QDateTime>
#include <QDebug>
#include <QException>
#include <QThread>

Favorite *favorite_dialog;
extern qhyccd_handle *camhandle;

Favorite::Favorite(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Favorite)
{
    ui->setupUi(this);

    this->setFixedSize(this->width(), this->height());//fixed the size of the window
    this->setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);// hide the "?" help button ont the title bar

#if CCM_CIRCLE
    ui->groupBox_Circle->setVisible(true);
    ui->checkBox_Circle->setVisible(true);
    ui->hSlider_Circle->setVisible(true);
    ui->spinBox_Circle->setVisible(true);
#else
    ui->groupBox_Circle->setVisible(false);
    ui->checkBox_Circle->setVisible(false);
    ui->hSlider_Circle->setVisible(false);
    ui->spinBox_Circle->setVisible(false);
#endif
}

Favorite::~Favorite()
{
    delete ui;
}

void Favorite::on_comBox_Interface_currentIndexChanged(int index)
{
    ix.trigerInterface = index;
}

void Favorite::on_cBox_trigIn_clicked(bool checked)
{
    unsigned int ret;

    if(checked)
    {
        ui->comBox_Interface->setEnabled(false);

        ret = libqhyccd->SetQHYCCDTrigerInterface(camhandle, ix.trigerInterface);
        if(ret == QHYCCD_SUCCESS){
            qDebug() << "SetQHYCCDTrigerInterface" << ix.trigerIn;
        }else{
            qCritical() << "SetQHYCCDTrigerInterface failure";
        }
    }
    else
    {
        ui->comBox_Interface->setEnabled(true);
        ui->cBox_trigOut->setEnabled(true);
        ui->cBox_trigOut->setChecked(false);
    }

    ix.trigerIn = checked;
    ret = libqhyccd->SetQHYCCDTrigerFunction(camhandle, ix.trigerIn);
    if(ret == QHYCCD_SUCCESS){
        qDebug() << "SetQHYCCDTrigerFunction" << ix.trigerIn;
    }else{
        qCritical() << "SetQHYCCDTrigerFunction failure";
    }
}

void Favorite::on_cBox_trigOut_clicked(bool checked)
{
    uint32_t ret = QHYCCD_ERROR;

    if(checked)
    {
        ix.trigerOut = checked;
        ret = libqhyccd->SetQHYCCDTrigerInterface(camhandle, ix.trigerInterface);
        if(ret == QHYCCD_SUCCESS){
            qDebug() << "SetQHYCCDTrigerInterface" << ix.trigerIn;
        }else{
            qCritical() << "SetQHYCCDTrigerInterface failure";
        }
        ret = libqhyccd->EnableQHYCCDTrigerOut(camhandle);
        if(ret == QHYCCD_SUCCESS){
            qDebug() << "EnableQHYCCDTrigerOut" << ix.trigerIn;
        }else{
            qCritical() << "EnableQHYCCDTrigerOut failure";
        }

        ui->cBox_trigOut->setEnabled(false);
        ui->comBox_Interface->setEnabled(false);
    }
}

void Favorite::on_spinBox_Traffic_valueChanged(int arg1)
{
    ix.Traffic = arg1;
}

//------------------------finetone 3210-------------------------------
/**
 * 改变Fine Tone的值
 * @brief Favorite::setFineToneValue
 * @param value1
 * @param value2
 * @param value3
 * @param value4
 * @param value5
 * @param value6
 * @param value7
 * @param value8
 */
void Favorite::setFineToneValue(int value1, int value2, int value3, int value4,int value5, int value6, int value7, int value8)
{
    if(value1 < 0)
        value1 = ui->hSlider_1_favorite->value();
    if(value2 < 0)
        value2 = ui->hSlider_2_favorite->value();
    if(value3 < 0)
        value3 = ui->hSlider_3_favorite->value();
    if(value4 < 0)
        value4 = ui->hSlider_4_favorite->value();
    if(value5 < 0)
        value5 = ui->hSlider_5_favorite->value();
    if(value6 < 0)
        value6 = ui->hSlider_6_favorite->value();
    if(value7 < 0)
        value7 = ui->hSlider_7_favorite->value();
    if(value8 < 0)
        value8 = ui->hSlider_8_favorite->value();

    int value = (value1 * 64 + value2 * 16 + value3 * 4 + value4) * 256
            + value5 * 64 + value6 * 16 + value7 * 4 + value8;

    ui->label_fineTone_favorite->setText(QString::number(value));
}

/**
 * Fine Tone中value1值改变
 * @brief Favorite::on_hSlider_1_favorite_valueChanged
 * @param value
 */
void Favorite::on_hSlider_1_favorite_valueChanged(int value)
{
    setFineToneValue(value,-1,-1,-1,-1,-1,-1,-1);
}

/**
 * Fine Tone中value2值改变
 * @brief Favorite::on_hSlider_2_favorite_valueChanged
 * @param value
 */
void Favorite::on_hSlider_2_favorite_valueChanged(int value)
{
    setFineToneValue(-1,value,-1,-1,-1,-1,-1,-1);
}

/**
 * Fine Tone中value3值改变
 * @brief Favorite::on_hSlider_3_favorite_valueChanged
 * @param value
 */
void Favorite::on_hSlider_3_favorite_valueChanged(int value)
{
    setFineToneValue(-1,-1,value,-1,-1,-1,-1,-1);
}

/**
 * Fine Tone中value4值改变
 * @brief Favorite::on_hSlider_4_favorite_valueChanged
 * @param value
 */
void Favorite::on_hSlider_4_favorite_valueChanged(int value)
{
    setFineToneValue(-1,-1,-1,value,-1,-1,-1,-1);
}

/**
 * Fine Tone中value5值改变
 * @brief Favorite::on_hSlider_5_favorite_valueChanged
 * @param value
 */
void Favorite::on_hSlider_5_favorite_valueChanged(int value)
{
    setFineToneValue(-1,-1,-1,-1,value,-1,-1,-1);
}

/**
 * Fine Tone中value6值改变
 * @brief Favorite::on_hSlider_6_favorite_valueChanged
 * @param value
 */
void Favorite::on_hSlider_6_favorite_valueChanged(int value)
{
    setFineToneValue(-1,-1,-1,-1,-1,value,-1,-1);
}

/**
 * Fine Tone中value7值改变
 * @brief Favorite::on_hSlider_7_favorite_valueChanged
 * @param value
 */
void Favorite::on_hSlider_7_favorite_valueChanged(int value)
{
    setFineToneValue(-1,-1,-1,-1,-1,-1,value,-1);
}

/**
 * Fine Tone中value8值改变
 * @brief Favorite::on_hSlider_8_favorite_valueChanged
 * @param value
 */
void Favorite::on_hSlider_8_favorite_valueChanged(int value)
{
    setFineToneValue(-1,-1,-1,-1,-1,-1,-1,value);
}

void Favorite::on_saveFineTone_favorite_clicked()
{
    //qDebug() << "";
}

//----------------------finetone 9979--------------------------------
void Favorite::on_hSlider_1_favorite_9979_valueChanged(int value)
{
    ui->finetone1Value_9979->setText(QString::number(value));

    set9979Finetone();
}

void Favorite::on_hSlider_2_favorite_9979_valueChanged(int value)
{
    ui->finetone2Value_9979->setText(QString::number(value));

    set9979Finetone();
}

void Favorite::on_hSlider_3_favorite_9979_valueChanged(int value)
{
    ui->finetone3Value_9979->setText(QString::number(value));

    set9979Finetone();
}

void Favorite::on_cBox_SHPorSHD_9979_toggled(bool checked)
{
    qDebug() << "SHP_SHD_9979" << checked;
    set9979Finetone();
}

void Favorite::set9979Finetone()
{
    unsigned int finetone1, finetone2, finetone3, SHPorSHD;

    finetone1 = ui->hSlider_1_favorite_9979->value();
    finetone2 = ui->hSlider_2_favorite_9979->value();
    finetone3 = ui->hSlider_3_favorite_9979->value();
    if(ui->cBox_SHPorSHD_9979->isChecked())
        SHPorSHD = 1; // view SHP
    else
        SHPorSHD = 0;  // view SHD
    libqhyccd->SetQHYCCDFineTone(camhandle, SHPorSHD, finetone1, finetone2, finetone3);
    //SetQHYCCDFineTone(camhandle, SHPorSHD, finetone1, finetone2, finetone3);
    qDebug() << "set finetone" << SHPorSHD << finetone1 << finetone2 << finetone3;
}

//--------------------------------------------------------------------

void Favorite::setFineToneEnable(bool enable)
{
    //ui->gBox_fineTone->setEnabled(enable);
    ui->tabWidget_finetone->setVisible(enable);
}

void Favorite::setMotorHeatingEnable(bool enable)
{
    //ui->gBox_shutterMotor->setEnabled(enable);
    ui->gBox_shutterMotor->setVisible(enable);
}

void Favorite::camera_connected()
{
    uint32_t ret = QHYCCD_ERROR;

    //Traffic init
    ui->label_Traffic->setVisible(ix.Traffic_Fun);
    ui->spinBox_Traffic->setVisible(ix.Traffic_Fun);
    if(ix.Traffic_Fun)
    {
        ui->spinBox_Traffic->setMaximum(ix.Traffic_Max);
        ui->spinBox_Traffic->setMinimum(ix.Traffic_Min);
        ui->spinBox_Traffic->setSingleStep(ix.Traffic_Step);
        ui->spinBox_Traffic->setValue(ix.Traffic);
    }

    //trig-in
//    ui->cBox_trigIn->setVisible(ix.canTriger);
    if(ix.canTriger)
    {
        uint32_t num = 0;
        char name[40] = { 0 };

        ix.trigerInterfaceList.clear();
        ret = libqhyccd->GetQHYCCDTrigerInterfaceNumber(camhandle, &num);
        for(int i = 0; i < num; i ++)
        {
            ret = libqhyccd->GetQHYCCDTrigerInterfaceName(camhandle, i, name);
            ix.trigerInterfaceList.append(name);
        }
        ui->comBox_Interface->clear();
        ui->comBox_Interface->addItems(ix.trigerInterfaceList);

        ret = libqhyccd->IsQHYCCDControlAvailable(camhandle, CAM_TRIGER_OUT);
        if(ret == QHYCCD_ERROR)
        {
            ui->cBox_trigOut->setVisible(false);
        }
    }
    else
    {
        ui->grpBox_Triger->setVisible(true);
    }

    //20200512lyl GPSon
    ui->cBox_GPS->setVisible(ix.GPS_Fun);
    ui->comboBox_OSD->setVisible(ix.GPS_Fun);

    //fineton
    this->setFineToneEnable(ix.canFineTone);

    //motorHeating
    this->setMotorHeatingEnable(ix.canMotorHeating);

    //tec protect
    ui->cBox_TEC->setVisible(ix.canTecOverProtect);

    //calmp
    ui->cBox_signalClamp->setVisible(ix.canSignalClamp);

    //calibrate FPN
    ui->grpBox_calibrateFPN->setVisible(ix.canCalibrateFPN);

    //chip temp
    ui->grpBox_chipTemp->setVisible(ix.canChipTemp);

    //slowest download
    ui->cBox_slowDownload->setVisible(ix.canSlowestDownload);

    //Overscan Calibration
    ui->spinBox_calConstant->setValue(ix.calConstant);

    //20201127 lyl SensorChamberCyclePUMP
    ui->grpBox_SensorChamberCycle_PUMP->setVisible(ix.canContolSensorChamberCyclePUMP);

    ui->hSlider_Circle->blockSignals(true);
    ui->spinBox_Circle->blockSignals(true);
    ui->hSlider_Circle->setValue(ix.circle1);
    ui->spinBox_Circle->setValue(ix.circle1);
    ui->hSlider_Circle->blockSignals(false);
    ui->spinBox_Circle->blockSignals(false);

    if(ix.CamID.contains("CCM")) ui->checkBox_Circle->setChecked(true);
}

void Favorite::resetUI()
{
    ui->retranslateUi(this);
}

void Favorite::on_spinBox_calConstant_valueChanged(int arg1)
{
    ix.calConstant = arg1;
    iniFileParams.calConstant = ix.calConstant;
}
//20200512lyl GPSon
void Favorite::on_cBox_GPS_clicked(bool checked)
{
    ix.GPS=checked;
}
//20200512lyl OSD
void Favorite::on_comboBox_OSD_currentIndexChanged(int index)
{
    unsigned int ret;
    if(libqhyccd->EnableQHYCCDImageOSD){
        ret = libqhyccd->EnableQHYCCDImageOSD(camhandle,index);//ret = EnableQHYCCDImageOSD(camhandle,index);
        if(ret != QHYCCD_SUCCESS)
            qCritical("EnableQHYCCDImageOSD: failed");
        else
            qDebug() << "EnableQHYCCDImageOSD success"<<index;
    }
   else{
    qCritical("EnableQHYCCDImageOSD: have no this function !");
    }
//    try{
//        //ret = EnableQHYCCDImageOSD(camhandle,index);
//        ret = libqhyccd->EnableQHYCCDImageOSD(camhandle,index);
//        if(ret != QHYCCD_SUCCESS)
//            qCritical("EnableQHYCCDImageOSD: failed");
//        else
//            qDebug() << "EnableQHYCCDImageOSD success"<<index;
//    }
//    catch (QException e) {
//    qCritical("EnableQHYCCDImageOSD: have no this function !");
//}
}

//void Favorite::on_pBtn_controlSensorChamberCyclePUMP_clicked()
//{
//    if(ui->pBtn_controlSensorChamberCyclePUMP->text() == "SensorChamberCyclePUMP ON")
//    {
//        ui->pBtn_controlSensorChamberCyclePUMP->setText("SensorChamberCyclePUMP OFF");
//        libqhyccd->SetQHYCCDParam(camhandle, CONTROL_SensorChamberCycle_PUMP, 1);
//    }
//    else
//    {
//        ui->pBtn_controlSensorChamberCyclePUMP->setText("SensorChamberCyclePUMP ON");
//        libqhyccd->SetQHYCCDParam(camhandle, CONTROL_SensorChamberCycle_PUMP, 0);
//    }
//}

void Favorite::on_pushButton_sensor_ulvo_clicked()
{
    if(libqhyccd->IsQHYCCDControlAvailable(camhandle,CAM_Sensor_ULVO_Status)!=QHYCCD_SUCCESS){
        ui->textEdit_sensor_ulvo->append("IsQHYCCDControlAvailable  CAM_Sensor_ULVO_Status Not valid");
        return;
    }
    double result = libqhyccd->GetQHYCCDParam(camhandle,CAM_Sensor_ULVO_Status);
    ui->textEdit_sensor_ulvo->append(QString("ulvo status = %1").arg(QString::number(result)));
}

void Favorite::on_pushButton_retrain_clicked()
{
    if(libqhyccd->IsQHYCCDControlAvailable(camhandle,CAM_SensorPhaseReTrain)!=QHYCCD_SUCCESS){
        ui->textEdit_sensor_retrain->append("IsQHYCCDControlAvailable  CAM_SensorPhaseReTrain Not valid");
        return;
    }
    libqhyccd->QHYCCDSensorPhaseReTrain(camhandle);
    ui->textEdit_sensor_retrain->append("Call QHYCCDSensorPhaseReTrain");
}

void Favorite::on_pushButton_sensor_ulvo_getFlash_clicked()
{
    if(libqhyccd->IsQHYCCDControlAvailable(camhandle,CAM_Sensor_ULVO_Status)!=QHYCCD_SUCCESS){
        ui->textEdit_sensor_ulvo->append("IsQHYCCDControlAvailable  CAM_Sensor_ULVO_Status Not valid");
        return;
    }
    int configLength = 64;
    char configString_raw64[configLength];
    libqhyccd->QHYCCDReadInitConfigFlash(camhandle,configString_raw64);
    QByteArray qData(configString_raw64,configLength);
    QString buffString_hex = qData.toHex(' ');
//    for (int i =0;i< 8;i++) {
//        ui->textEdit_sensor_ulvo->append(QStringRef(&buffString_hex,i*24,24).toString());

//    }

    ui->textEdit_sensor_ulvo->insertPlainText("\n");
    for (int i =0;i<8;i++) {
        ui->textEdit_sensor_ulvo->insertPlainText(buffString_hex.mid(i * 24, 24));
        ui->textEdit_sensor_ulvo->insertPlainText(" ");
        if(i==7){ui->textEdit_sensor_ulvo->insertPlainText(" ");} //the toHex not generate last blank
        ui->textEdit_sensor_ulvo->moveCursor(QTextCursor::End);
        ui->textEdit_sensor_ulvo->insertPlainText(QChar(configString_raw64[i*8+0]));
        ui->textEdit_sensor_ulvo->insertPlainText(QChar(configString_raw64[i*8+1]));
        ui->textEdit_sensor_ulvo->insertPlainText(QChar(configString_raw64[i*8+2]));
        ui->textEdit_sensor_ulvo->insertPlainText(QChar(configString_raw64[i*8+3]));

        ui->textEdit_sensor_ulvo->insertPlainText(QString::number(configString_raw64[i*8+4]));
        ui->textEdit_sensor_ulvo->insertPlainText(QString::number(configString_raw64[i*8+5]));
        ui->textEdit_sensor_ulvo->insertPlainText(QString::number(configString_raw64[i*8+6]));
        ui->textEdit_sensor_ulvo->insertPlainText(QString::number(configString_raw64[i*8+7]));
        ui->textEdit_sensor_ulvo->insertPlainText("\n");

    }

    ui->textEdit_sensor_ulvo->append(QString("%1  %2%3%4%5 ").arg(QDateTime::currentDateTimeUtc().toString("hh:mm:ss")).arg(QChar(configString_raw64[0])).arg(configString_raw64[1]).arg(configString_raw64[2]).arg(QString::number(configString_raw64[7])));
    ui->textEdit_sensor_ulvo->append(QString("%1  %2%3%4%5 ").arg(QDateTime::currentDateTimeUtc().toString("hh:mm:ss")).arg(QChar(configString_raw64[8])).arg(configString_raw64[9]).arg(configString_raw64[10]).arg(QString::number(configString_raw64[15])));
}

void Favorite::on_pushButton_sensor_ulvo_resetFlash_clicked()
{
    if(libqhyccd->IsQHYCCDControlAvailable(camhandle,CAM_Sensor_ULVO_Status)!=QHYCCD_SUCCESS){
        ui->textEdit_sensor_ulvo->append("IsQHYCCDControlAvailable  CAM_Sensor_ULVO_Status Not valid");
        return;
    }
    libqhyccd->QHYCCDResetFlashULVOError(camhandle);
}

void Favorite::on_pushButton_sensor_ulvo_testError_clicked()
{
    if(libqhyccd->IsQHYCCDControlAvailable(camhandle,CAM_Sensor_ULVO_Status)!=QHYCCD_SUCCESS){
        ui->textEdit_sensor_ulvo->append("IsQHYCCDControlAvailable  CAM_Sensor_ULVO_Status Not valid");
        return;
    }
    libqhyccd->QHYCCDTestFlashULVOError(camhandle);
}

//void Favorite::on_pushButton_debug_timer_off_clicked()
//{
////    ((EZCAP*)(parent()))->stopTimerTemp();
//}

void Favorite::on_pushButton_sensor_ulvo_erase_flash_clicked()
{
    if(libqhyccd->IsQHYCCDControlAvailable(camhandle,CAM_Sensor_ULVO_Status)!=QHYCCD_SUCCESS){
        ui->textEdit_sensor_ulvo->append("IsQHYCCDControlAvailable  CAM_Sensor_ULVO_Status Not valid");
        return;
    }
    libqhyccd->QHYCCDEraseInitConfigFlash(camhandle);
}

void Favorite::on_pushButton_sensor_ulvo_debug_d3_clicked()
{
    if(libqhyccd->IsQHYCCDControlAvailable(camhandle,CAM_Sensor_ULVO_Status)!=QHYCCD_SUCCESS){
        ui->textEdit_sensor_ulvo->append("IsQHYCCDControlAvailable  CAM_Sensor_ULVO_Status Not valid");
        return;
    }
    int dataLength = 64;
    char debugData[dataLength];
    libqhyccd->QHYCCDGetDebugDataD3(camhandle,debugData);

    QByteArray qData(debugData,dataLength);
    QString buffString = qData.toHex(' ');
    for (int i =0;i< 8;i++) {
        ui->textEdit_sensor_ulvo->append(buffString.mid(i * 24, 24));
    }
    ui->textEdit_sensor_ulvo->append(QString("%1   P: %2 , PowerChipMonitor:%3").arg(QDateTime::currentDateTimeUtc().toString("hh:mm:ss")).arg(QString::number(debugData[62])).arg(QString::number(debugData[63])));
    ui->textEdit_sensor_ulvo->append(QString("%1   P: %2 , PowerChipMonitor:%3").arg(QDateTime::currentDateTimeUtc().toString("hh:mm:ss")).arg(QString::number(1-(double(debugData[62])/double(127)))).arg(QString::number(debugData[63])));
//    ui->textEdit_sensor_ulvo->append("==================");
}

void Favorite::on_checkBox_debug_stateChanged(int arg1)
{
    libqhyccd->EnableQHYCCDMessage(arg1 == Qt::Checked);
}

void Favorite::on_checkBox_Is_Test_stateChanged(int arg1)
{
    libqhyccd->EnableQHYCCDMessageTest(arg1 == Qt::Checked);
}

void Favorite::on_checkBox_temperature_timer_stateChanged(int arg1)
{
    switch (arg1) {
    case Qt::Unchecked:
        ((EZCAP*)(parent()))->threadTempControl->stop();
        break;
    case Qt::Checked:
        ((EZCAP*)(parent()))->threadTempControl->start();
        break;
    default:
        break;
    }
}

void Favorite::on_ckBox_EnGlobalReset_stateChanged(int arg1)
{
    libqhyccd->SetQHYCCDParam(camhandle, CONTROL_GlobalReset, (double)(arg1==Qt::Checked));
}

void Favorite::on_checkBox_Circle_stateChanged(int arg1)
{
//    ix.enableCircle = arg1;
}


void Favorite::on_hSlider_Circle_valueChanged(int value)
{
    ui->spinBox_Circle->setValue(value);
    ix.circle1 = value;
    ix.circle2 = ix.circle1 + 20;
}


void Favorite::on_spinBox_Circle_valueChanged(int arg1)
{
    ui->hSlider_Circle->setValue(arg1);
}


void Favorite::on_pushButton_uart_cmd_send_rev_clicked()
{
    QString uart_cmd = ui->textEdit_uart_cmd_send->toPlainText();
    //char* uart_rev;
    char uart_rev[64];
    libqhyccd->RedirectCommand(camhandle,uart_cmd.toStdString().c_str(), uart_rev);
    ui->textEdit_uart_cmd_rev->append(QString().fromStdString(uart_rev));

//    ui->textEdit_uart_cmd_rev->append(QString().fromStdString(uart_rev));
}

void Favorite::on_pushButton_uart_cmd_foc46_clicked()
{
    ui->textEdit_uart_cmd_send->setText(R"({"cmd_id":715,"hex":"0046001A"})");
    QString uart_cmd = ui->textEdit_uart_cmd_send->toPlainText();
    char uart_rev[64];
    libqhyccd->RedirectCommand(camhandle,uart_cmd.toStdString().c_str(), uart_rev);
    ui->textEdit_uart_cmd_rev->append(QString().fromStdString(uart_rev));
}

void Favorite::on_pushButton_uart_cmd_len0a_clicked()
{
    ui->textEdit_uart_cmd_send->setText(R"({"cmd_id":715,"hex":"0A"})");
    QString uart_cmd = ui->textEdit_uart_cmd_send->toPlainText();
    char uart_rev[64];
    libqhyccd->RedirectCommand(camhandle,uart_cmd.toStdString().c_str(), uart_rev);
    ui->textEdit_uart_cmd_rev->append(QString().fromStdString(uart_rev));
}

void Favorite::on_pushButton_uart_cmd_foc45_clicked()
{
    ui->textEdit_uart_cmd_send->setText(R"({"cmd_id":715,"hex":"0045001A"})");
    QString uart_cmd = ui->textEdit_uart_cmd_send->toPlainText();
    char uart_rev[64];
    libqhyccd->RedirectCommand(camhandle,uart_cmd.toStdString().c_str(), uart_rev);
    ui->textEdit_uart_cmd_rev->append(QString().fromStdString(uart_rev));
}

void Favorite::on_pushButton_uart_cmd_aperture1_clicked()
{
    ui->textEdit_uart_cmd_send->setText(R"({"cmd_id":715,"hex":"120D1A"})");
    QString uart_cmd = ui->textEdit_uart_cmd_send->toPlainText();
    char uart_rev[64];
    libqhyccd->RedirectCommand(camhandle,uart_cmd.toStdString().c_str(), uart_rev);
    ui->textEdit_uart_cmd_rev->append(QString().fromStdString(uart_rev));
}

void Favorite::on_pushButton_uart_cmd_aperture2_clicked()
{
    ui->textEdit_uart_cmd_send->setText(R"({"cmd_id":715,"hex":"12F01A"})");
    QString uart_cmd = ui->textEdit_uart_cmd_send->toPlainText();
    char uart_rev[64];
    libqhyccd->RedirectCommand(camhandle,uart_cmd.toStdString().c_str(), uart_rev);
    ui->textEdit_uart_cmd_rev->append(QString().fromStdString(uart_rev));
}

void Favorite::on_pushButton_uart_cmd_cfw601_clicked()
{
    ui->textEdit_uart_cmd_send->setText(R"({"cmd_id":601})");
    QString uart_cmd = ui->textEdit_uart_cmd_send->toPlainText();
    char uart_rev[64];
    libqhyccd->RedirectCommand(camhandle,uart_cmd.toStdString().c_str(), uart_rev);
    ui->textEdit_uart_cmd_rev->append(QString().fromStdString(uart_rev));
}

int uart_cmd_cfw_1_position = 0;
int uart_cmd_cfw_2_position = 0;
void Favorite::on_pushButton_uart_cmd_cfw1_test_clicked()
{
    uart_cmd_cfw_1_position++;
    uart_cmd_cfw_1_position = uart_cmd_cfw_1_position%5;
    ui->textEdit_uart_cmd_send->setText(R"({"cmd_id":602,"sem_id":1,"sem_goto":)" + QString::number(uart_cmd_cfw_1_position) + R"(})");
    QString uart_cmd = ui->textEdit_uart_cmd_send->toPlainText();
    char uart_rev[64];
    libqhyccd->RedirectCommand(camhandle,uart_cmd.toStdString().c_str(), uart_rev);
    ui->textEdit_uart_cmd_rev->append(QString().fromStdString(uart_rev));
}

void Favorite::on_pushButton_uart_cmd_cfw2_test_clicked()
{
    uart_cmd_cfw_2_position++;
    uart_cmd_cfw_2_position = uart_cmd_cfw_2_position%5;
    ui->textEdit_uart_cmd_send->setText(R"({"cmd_id":602,"sem_id":2,"sem_goto":)" + QString::number(uart_cmd_cfw_2_position) + R"(})");
    QString uart_cmd = ui->textEdit_uart_cmd_send->toPlainText();
    char uart_rev[64];
    libqhyccd->RedirectCommand(camhandle,uart_cmd.toStdString().c_str(), uart_rev);
    ui->textEdit_uart_cmd_rev->append(QString().fromStdString(uart_rev));
}

void Favorite::on_pushButton_uart_cmd_cfw600_clicked()
{
    ui->textEdit_uart_cmd_send->setText(R"({"cmd_id":600})");
    QString uart_cmd = ui->textEdit_uart_cmd_send->toPlainText();
    char uart_rev[64];
    libqhyccd->RedirectCommand(camhandle,uart_cmd.toStdString().c_str(), uart_rev);
    ui->textEdit_uart_cmd_rev->append(QString().fromStdString(uart_rev));
}

void Favorite::on_pushButton_ArrayCamSync_clicked()
{
    if(ui->pushButton_ArrayCamSync->text() == "ArrayCam Synchronize")
    {
//        if(ix.CamID.contains("QHY992_1"))
//        {
//            OUTPUT_INFO("QHY992_1 Master");
//            uint8_t data[4];

//            libqhyccd->SetQHYCCDWriteFPGA(camhandle, 0, 150, 1);
//            libqhyccd->SetQHYCCDWriteFPGA(camhandle, 0, 151, 1);
//            libqhyccd->SetQHYCCDWriteFPGA(camhandle, 0,  58, 5);
//            libqhyccd->QHYCCDVendRequestWrite(camhandle, 0xbd, 1, 158, 1, data);
//            libqhyccd->QHYCCDVendRequestWrite(camhandle, 0xbd, 1,  18, 1, data);
//        }
//        else if(ix.CamID.contains("QHY992_2") || ix.CamID.contains("QHY992_3") || ix.CamID.contains("QHY992_4") ||
//                 ix.CamID.contains("QHY992_5") || ix.CamID.contains("QHY992_6") || ix.CamID.contains("QHY992_7") ||
//                 ix.CamID.contains("QHY992_8") || ix.CamID.contains("QHY992_9"))
//        {
//            OUTPUT_INFO("QHY992_2~9 Slave");
//            libqhyccd->SetQHYCCDWriteFPGA(camhandle, 0, 150, 1);
//            libqhyccd->SetQHYCCDWriteFPGA(camhandle, 0, 151, 1);
//            libqhyccd->SetQHYCCDWriteFPGA(camhandle, 0,  58, 5);
//        }
//        else if(ix.CamID.contains("QHY411M_1") || ix.CamID.contains("QHY411MERIS_1"))
//        {
//            uint8_t data[4];
//            OUTPUT_INFO("QHY411M_1 QHY411MERIS_1 Master");
//            libqhyccd->SetQHYCCDWriteFPGA(camhandle, 0, 36, 1);
//            libqhyccd->QHYCCDVendRequestWrite(camhandle, 0xbd, 1, 158, 1, data);
//            libqhyccd->QHYCCDVendRequestWrite(camhandle, 0xbd, 1,  18, 1, data);
//        }
//        else if(ix.CamID.contains("QHY411M_2") || ix.CamID.contains("QHY411M_3") || ix.CamID.contains("QHY411M_4") ||
//                 ix.CamID.contains("QHY411MERIS_2") || ix.CamID.contains("QHY411MERIS_3") || ix.CamID.contains("QHY411MERIS_4"))
//        {
//            OUTPUT_INFO("QHY411M_2~4 QHY411MERIS_2~4 Slave");
//            libqhyccd->SetQHYCCDWriteFPGA(camhandle, 0, 36, 0);
//        }
        libqhyccd->SetQHYCCDArrayCamSync(camhandle, true);

        ui->pushButton_ArrayCamSync->setText("Exit Synchronize");
    }
    else
    {
        ui->pushButton_ArrayCamSync->setText("ArrayCam Synchronize");
        libqhyccd->SetQHYCCDArrayCamSync(camhandle, false);
//        if(ix.CamID.contains("QHY992_1"))
//        {
//            OUTPUT_INFO("QHY992_1 Master");
//            libqhyccd->SetQHYCCDWriteFPGA(camhandle, 0,  58, 0);
//        }
//        else if(ix.CamID.contains("QHY992_2") || ix.CamID.contains("QHY992_3") || ix.CamID.contains("QHY992_4") ||
//                 ix.CamID.contains("QHY992_5") || ix.CamID.contains("QHY992_6") || ix.CamID.contains("QHY992_7") ||
//                 ix.CamID.contains("QHY992_8") || ix.CamID.contains("QHY992_9"))
//        {
//            OUTPUT_INFO("QHY992_2~9 Exit Slave");
//            libqhyccd->SetQHYCCDWriteFPGA(camhandle, 0, 150, 0);
//            libqhyccd->SetQHYCCDWriteFPGA(camhandle, 0, 151, 0);
//            libqhyccd->SetQHYCCDWriteFPGA(camhandle, 0,  58, 0);
//        }
//        else if(ix.CamID.contains("QHY411M_1") || ix.CamID.contains("QHY411MERIS_1"))
//        {
//            OUTPUT_INFO("QHY411M_1 QHY411MERIS_1 Master");
//            libqhyccd->SetQHYCCDWriteFPGA(camhandle, 0, 36, 1);
//        }
//        else if(ix.CamID.contains("QHY411M_2") || ix.CamID.contains("QHY411M_3") || ix.CamID.contains("QHY411M_4") ||
//                 ix.CamID.contains("QHY411MERIS_2") || ix.CamID.contains("QHY411MERIS_3") || ix.CamID.contains("QHY411MERIS_4"))
//        {
//            OUTPUT_INFO("QHY411M_2~4 QHY411MERIS_2~4 Slave");
//            libqhyccd->SetQHYCCDWriteFPGA(camhandle, 0, 36, 1);
//        }
    }
}


 char uart_cmd_screen_light = 0x01;
void Favorite::on_pushButton_uart_cmd_screen_clicked()
{
    uart_cmd_screen_light=uart_cmd_screen_light+1;
    char hexChar = "0123456789ABCDEF"[uart_cmd_screen_light & 0x0F];
    QString screen_light_str = QChar(hexChar);
    ui->textEdit_uart_cmd_send->setText(R"({"cmd_id":721,"hex":"5AA504820082)" + screen_light_str + R"(000"})");
    QString uart_cmd = ui->textEdit_uart_cmd_send->toPlainText();
    char uart_rev[64];
    libqhyccd->RedirectCommand(camhandle,uart_cmd.toStdString().c_str(), uart_rev);
    ui->textEdit_uart_cmd_rev->append(QString().fromStdString(uart_rev));
}

void Favorite::on_pushButton_uart_cmd_vrs_clicked()
{
    ui->textEdit_uart_cmd_send->setText(R"({"cmd_id":101})");
    QString uart_cmd = ui->textEdit_uart_cmd_send->toPlainText();
    char uart_rev[64];
    libqhyccd->RedirectCommand(camhandle,uart_cmd.toStdString().c_str(), uart_rev);
    ui->textEdit_uart_cmd_rev->append(QString().fromStdString(uart_rev));
}

void Favorite::on_pushButton_uart_cmd_debug_clicked(bool checked)
{
    ui->textEdit_uart_cmd_send->setText(R"({"cmd_id":102,"s_debug":)" + QString::number(checked?1:0) + R"(})");
    QString uart_cmd = ui->textEdit_uart_cmd_send->toPlainText();
    char uart_rev[64];
    libqhyccd->RedirectCommand(camhandle,uart_cmd.toStdString().c_str(), uart_rev);
    ui->textEdit_uart_cmd_rev->append(QString().fromStdString(uart_rev));
}

void Favorite::on_pushButton_uart_cmd_retransfer_clicked()
{
    // 重传按钮本身只负责响应 UI 点击，真正的重传和取图流程交给主窗口处理。
    // 这样可以复用 EZCAP 中已有的图像接收、显示、直方图和计数逻辑，避免在 Favorite
    // 这个调试窗口里再复制一套取图流程。
    EZCAP *ezcap = qobject_cast<EZCAP *>(parent());
    if(ezcap && ezcap->triggerRetransferAndReceiveFrame()){
        // 返回 true 表示重传触发指令已成功发送；后续取图成功。
        ui->textEdit_uart_cmd_rev->append("Retransfer");
    }else{
        // 返回 false 表示 FPGA 触发指令发送失败，或者单帧模式下后续没有成功取回图像。
        ui->textEdit_uart_cmd_rev->append("Retransfer failed");
    }
}

void Favorite::on_pushButton_uart_cmd_emmc_enable_clicked(bool checked)
{
    uint32_t ret = qhyWriteFPGAExtend(camhandle, 700, checked ? 1 : 0);
    ui->textEdit_uart_cmd_rev->append(QString("eMMC %1 ret=%2")
                                      .arg(checked ? "enable" : "disable")
                                      .arg(ret));

    if(ret != QHYCCD_SUCCESS)
    {
        ui->pushButton_uart_cmd_emmc_enable->setChecked(!checked);
    }
}
