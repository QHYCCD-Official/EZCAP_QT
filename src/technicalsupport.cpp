#include "technicalsupport.h"
#include "ui_technicalsupport.h"


#include "include/dllqhyccd.h"
#include "include/fpgaAccess.h"
#include "ezCap.h"
#include <QDebug>
#include <QByteArray>


TechnicalSupport *technicalSupport_dialog;
extern qhyccd_handle *camhandle;


TechnicalSupport::TechnicalSupport(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TechnicalSupport)
{
    ui->setupUi(this);
}

TechnicalSupport::~TechnicalSupport()
{
    delete ui;
}

void TechnicalSupport::on_pushButton_clicked()
{
    int deviceNumber;
    deviceNumber=ui->spinBox->value();

    unsigned int ret=NULL;
    char cameraID[64];

    ret = libqhyccd->GetQHYCCDId(deviceNumber,cameraID);

    ui->textEdit->append(QString(QLatin1String(cameraID)));

    camhandle = libqhyccd->OpenQHYCCD(cameraID);


}

void TechnicalSupport::on_pushButton_4_clicked()
{
    unsigned int ret = NULL;
    ret = libqhyccd->InitQHYCCDResource();

    if(ret!=QHYCCD_SUCCESS){
        ui->textEdit->append("InitQHYCCDResource: failed");
    }
    else{
       ui->textEdit->append("InitQHYCCDResource: success");
    }

    int num = 0;

    num = libqhyccd->ScanQHYCCD();
    ui->textEdit->append("find qhyccd device:"+QString::number(num));

    if(num>0){
        ui->spinBox->setMaximum(num-1);
    }

}

void TechnicalSupport::on_pushButton_5_clicked()
{
    uint32_t ret = NULL;
    uint32_t NumberOfReadMode=0;
    ret = libqhyccd->GetQHYCCDNumberOfReadModes(camhandle,&NumberOfReadMode);
    if(NumberOfReadMode>0){
        ui->spinBox_2->setMaximum(NumberOfReadMode-1);
    }

    char readModeName[50];

    for(int i=0;i<NumberOfReadMode;i++){
        ret=libqhyccd->GetQHYCCDReadModeName(camhandle,i,readModeName);
        ui->textEdit->append(QString(QLatin1String(readModeName)));
    }
}



#if defined (WIN32)
#else
void TechnicalSupport::print_devs(libusb_device **devs)
{
    libusb_device *dev;
    int i = 0, j = 0;
    uint8_t path[8];

    while ((dev = devs[i++]) != NULL) {
        struct libusb_device_descriptor desc;
        int r = libusb_get_device_descriptor(dev, &desc);
        if (r < 0) {
             ui->textEdit->append("failed to get device descriptor");
            return;
        }

        QString s;
        s.asprintf("%04x:%04x (bus %d, device %d)",desc.idVendor, desc.idProduct,libusb_get_bus_number(dev), libusb_get_device_address(dev));
        ui->textEdit->append(s);
        r = libusb_get_port_numbers(dev, path, sizeof(path));
        if (r > 0) {
            s.asprintf(" path: %d", path[0]);
            ui->textEdit->append(s);
            for (j = 1; j < r; j++)
            {
                s.asprintf(".%d", path[j]);
                ui->textEdit->append(s);
            }
        }

    }
}

unsigned char device_status(libusb_device_handle *hd)
{

    int interface = 0;
    unsigned char value;
    libusb_control_transfer(hd, LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE,
            LIBUSB_REQUEST_CLEAR_FEATURE,
            0,
            interface,
            &value, 1, 5000);

    //printf("status:0x%x\n", value);
/**
 * byte
 * normal:0x18
 * other :0x10
 */
    return value;
}
#endif


void TechnicalSupport::on_pushButton_7_clicked()
{

}

void TechnicalSupport::on_TsButtonScanLibUsb_2_clicked()
{


}

void TechnicalSupport::on_TsButtonScanLibUsb_clicked()
{
#if defined (WIN32)
#else
    libusb_device **devs;
    int r;
    ssize_t cnt;

     ui->textEdit->append("libusb test");




    r = libusb_init(NULL);
    if (r < 0)

    {
        ui->textEdit->append("libusb_init error");
        return;
    }

    cnt = libusb_get_device_list(NULL, &devs);
       if (cnt < 0){
           ui->textEdit->append("libusb_get_device_list<0");
           libusb_exit(NULL);
           return;
       }
       else{
           ui->textEdit->append("found libusb device"+QString::number(cnt));
       }

       print_devs(devs);

       libusb_free_device_list(devs, 1);

       libusb_exit(NULL);
       return ;
#endif

}

void TechnicalSupport::on_TsButtonOpenDeviceLibUsb_clicked()
{
#if defined (WIN32)
#else
    libusb_device **devs;
    int r;
    ssize_t cnt;

//    libusb_device_handle *dev_handle; //a device handle


    uint16_t VID,PID;


    VID= ui->TsLineEditVID->text().toUShort(NULL,16);
    PID= ui->TsLineEditPID->text().toUShort(NULL,16);


    ui->textEdit->append("libusb open device");
    r = libusb_init(&ctx);
    if (r < 0)

    {
        ui->textEdit->append("libusb_init error");
        return;
    }

    libusb_set_debug(ctx,LIBUSB_LOG_LEVEL_INFO);//set verbosity level to 3, as suggested in the documentation

    cnt = libusb_get_device_list(ctx, &devs);
       if (cnt < 0){
           ui->textEdit->append("libusb_get_device_list<0");
           libusb_exit(ctx);
           return;
       }
       else{
           ui->textEdit->append("found libusb device"+QString::number(cnt));
       }

    libusb_free_device_list(devs, 1); //free the list, unref the devices in it







       dev_handle=libusb_open_device_with_vid_pid(ctx, VID, PID);

       if(dev_handle == NULL)  {
           ui->textEdit->append("Cannot open device");
           libusb_exit(ctx);
           return;
       }

       else {
           ui->textEdit->append("Device Opened handle");
       }








//       if(libusb_kernel_driver_active(dev_handle, 0) == 1) { //find out if kernel driver is attached
//              ui->textEdit->append("Kernel Driver Active");
//              if(libusb_detach_kernel_driver(dev_handle, 0) == 0)  ui->textEdit->append("Kernel Driver Detached!");
//       }



//       r = libusb_claim_interface(dev_handle, 0); //claim interface 0 (the first) of device (mine had jsut 1)
//       if(r < 0)
//       {
//               ui->textEdit->append("Cannot Claim Interface");
//       }
//       else{
//              ui->textEdit->append("Claimed Interface");
//              unsigned char status=0;
//              status=device_status(dev_handle);
//              ui->textEdit->append(QString::number(status));
//       }





//      libusb_release_interface(dev_handle,0);
//      libusb_close(dev_handle);
//      libusb_exit(ctx);
      return ;
#endif

}

void TechnicalSupport::on_pBtnVendRead_clicked()
{
#if defined (WIN32)
#else

    ui->textEdit->append("VendRequest Read start!");

    int ret = 0;
    uint8_t req = ui->lineEditVendReadReq->text().toUShort(NULL, 16);
    uint16_t value = ui->lineEditVendReadValue->text().toUShort(NULL, 16);
    uint16_t index = ui->lineEditVendReadIndex->text().toUShort(NULL, 16);
    uint16_t length = ui->lineEditVendReadLength->text().toUShort(NULL, 16);
    if(length > 0)
    {
        unsigned char *data = (unsigned char *)malloc(length);
        ret = libusb_control_transfer(dev_handle, 0xC0, req, value, index, data, length, 1000);
        if(ret < 0)
        {
            ui->textEdit->append("VendRequest Read failed!");
        }
        else
        {
            QString info = "";
            for(int i = 0; i < length; i ++)
            {
                info += QString::number(data[i], 16) + " ";
                if((length+1) % 8 == 0)
                    info += "\n";
            }
            ui->textEdit->append(info);
            ui->textEdit->append("VendRequest Read success!");
        }
    }
#endif
}

void TechnicalSupport::on_pBtnVenWrite_clicked()
{
#if defined (WIN32)
#else
    int ret = 0;
    unsigned char data[64] = { 0 };
    uint8_t req = ui->lineEditVendWriteReq->text().toUShort(NULL, 16);
    uint16_t value = ui->lineEditVendWriteValue->text().toUShort(NULL, 16);
    uint16_t index = ui->lineEditVendWriteIndex->text().toUShort(NULL, 16);
    uint16_t length = ui->lineEditVendWriteLength->text().toUShort(NULL, 16);
    //data[0] = ui->lineEditVendWriteData->text().toUShort(NULL, 16);

    QString hexString = ui->lineEditVendWriteData->text();
    qDebug()<<hexString;

    QStringList hexList = hexString.split(" ");

    qDebug()<<hexList.length();

    int i=0;
    bool ok;
    int decimalValue;
    foreach (QString hex, hexList){
        decimalValue=hex.toInt(&ok,16);
        if(ok){
          data[i]=decimalValue;
          i++;
        }

    }



    ret = libusb_control_transfer(dev_handle, 0x40, req, value, index, data, length, 1000);
    if(ret < 0)
    {
        ui->textEdit->append("VendRequest Write failed!");
    }
    else
    {
        ui->textEdit->append("VendRequest Write success!");
    }
#endif
}

void TechnicalSupport::on_pBtnBulkRead_clicked()
{
#if defined (WIN32)
#else
    int ret = 0, actual_len = 0;
    unsigned char endpoint = ui->lineEditBuldReadEp->text().toUShort(NULL, 16);
    int length = ui->lineEditBulkReadLen->text().toUShort(NULL, 16);
    if(length > 0)
    {
        unsigned char *data = (unsigned char *)malloc(length);
        ret = libusb_bulk_transfer(dev_handle, endpoint, data, length, &actual_len, 1000);
        if(ret < 0)
        {
            ui->textEdit->append("libusb_bulk_transfer() read failed!");
        }
        else
        {
            if(actual_len > 0)
            {
                QString info = "";
                for(int i = 0; i < actual_len; i ++)
                {
                    info += QString::number(data[i], 16) + " ";
                    if((i+1) % 8 == 0)
                        info += "\n";
                }
                ui->textEdit->append(info);
            }
        }
    }
#endif
}

void TechnicalSupport::on_pBtnCloseDevice_clicked()
{
#if defined (WIN32)
#else
    libusb_close(dev_handle);
    libusb_exit(ctx);
    ui->textEdit->append("Libusb Closed.");
#endif
}

void TechnicalSupport::on_pBtnLowLevelD5_clicked()
{
    uint32_t ret = QHYCCD_ERROR;
    uint8_t buffer[64] = { 0 };
    uint16_t index = ui->lineEditLowLevelD5Index->text().toUShort(NULL, 16);
    ret = libqhyccd->QHYCCDVendRequestRead(camhandle, 0xd5, 0, index, 64, buffer);
    QString info = "  1  2  3  4  5  6  7  8\n";
    for(int i = 0; i < 64; i ++)
    {
        if(buffer[i] >= 0x10)
            info += " ";
        else
            info += "  ";
        info += QString::number(buffer[i], 16);

        if((i+1) % 8 == 0)
            info += "\n";
    }
    ui->textEdit->append(info);
}

bool TechnicalSupport::checkCameraHandle()
{
    if(camhandle == NULL)
    {
        ui->textEdit->append("Camera not connected.");
        return false;
    }
    return true;
}

void TechnicalSupport::writeFPGA(uint16_t index, uint16_t value)
{
    qhyWriteFPGA(camhandle, index, value);
}

uint8_t TechnicalSupport::readFPGA(uint16_t index)
{
    return qhyReadFPGA(camhandle, index);
}

void TechnicalSupport::writeFPGA2(uint16_t index, uint16_t value)
{
    if(libqhyccd->SetQHYCCDWriteFPGA != NULL)
    {
        libqhyccd->SetQHYCCDWriteFPGA(camhandle, 1, index, (uint8_t)value);
        return;
    }

    uint8_t data[10] = { 0 };
    libqhyccd->QHYCCDVendRequestWrite(camhandle, 0xbd, value, index, 1, data);
}

uint8_t TechnicalSupport::readFPGA2(uint16_t index)
{
    uint8_t data[10] = { 0 };
    libqhyccd->QHYCCDVendRequestRead(camhandle, 0xbe, 0, index, 1, data);
    return data[0];
}

void TechnicalSupport::writeFPGAExtend(uint16_t index, uint32_t value)
{
    qhyWriteFPGAExtend(camhandle, index, value);
}

uint32_t TechnicalSupport::readFPGAExtend(uint16_t index)
{
    return qhyReadFPGAExtend(camhandle, index);
}

void TechnicalSupport::on_TsButtonWriteFPGA_clicked()
{
    if(!checkCameraHandle())
        return;

    bool ok = false;
    uint16_t index = ui->TsLineEditWriteFPGA_Address->text().toUShort(&ok, 10);
    if(!ok)
    {
        ui->textEdit->append("Invalid FPGA address.");
        return;
    }

    uint16_t value = ui->TsLineEditWriteFPGA_Data->text().toUShort(&ok, 16);
    if(!ok)
    {
        ui->textEdit->append("Invalid FPGA data.");
        return;
    }

    writeFPGA(index, value);
    ui->textEdit->append(QString("WriteFPGA %1 = 0x%2").arg(index).arg(value, 2, 16, QChar('0')));
}

void TechnicalSupport::on_TsButtonReadFPGA_clicked()
{
    if(!checkCameraHandle())
        return;

    bool ok = false;
    uint16_t index = ui->TsLineEditReadFPGA_Address->text().toUShort(&ok, 10);
    if(!ok)
    {
        ui->textEdit->append("Invalid FPGA address.");
        return;
    }

    uint8_t value = readFPGA(index);
    ui->TsLineReadFPGA_Data->setText(QString::number(value, 16).rightJustified(2, '0'));
    ui->textEdit->append(QString("ReadFPGA %1 = 0x%2").arg(index).arg(value, 2, 16, QChar('0')));
}

void TechnicalSupport::on_TsButtonWriteFPGA2_clicked()
{
    if(!checkCameraHandle())
        return;

    bool ok = false;
    uint16_t index = ui->TsLineEditWriteFPGA2_Address->text().toUShort(&ok, 10);
    if(!ok)
    {
        ui->textEdit->append("Invalid FPGA2 address.");
        return;
    }

    uint16_t value = ui->TsLineEditWriteFPGA2_Data->text().toUShort(&ok, 16);
    if(!ok)
    {
        ui->textEdit->append("Invalid FPGA2 data.");
        return;
    }

    writeFPGA2(index, value);
    ui->textEdit->append(QString("WriteFPGA2 %1 = 0x%2").arg(index).arg(value, 2, 16, QChar('0')));
}

void TechnicalSupport::on_TsButtonReadFPGA2_clicked()
{
    if(!checkCameraHandle())
        return;

    bool ok = false;
    uint16_t index = ui->TsLineEditReadFPGA2_Address->text().toUShort(&ok, 10);
    if(!ok)
    {
        ui->textEdit->append("Invalid FPGA2 address.");
        return;
    }

    uint8_t value = readFPGA2(index);
    ui->TsLineEditReadFPGA2_Data->setText(QString::number(value, 16).rightJustified(2, '0'));
    ui->textEdit->append(QString("ReadFPGA2 %1 = 0x%2").arg(index).arg(value, 2, 16, QChar('0')));
}

void TechnicalSupport::on_TsButtonWriteExpandFPGA_clicked()
{
    if(!checkCameraHandle())
        return;

    bool ok = false;
    uint16_t index = ui->TsLineEditWriteExpandFPGA_Address->text().toUShort(&ok, 10);
    if(!ok)
    {
        ui->textEdit->append("Invalid expand register address.");
        return;
    }

    uint32_t value = ui->TsLineEditWriteExpandFPGA_Data->text().toUInt(&ok, 16);
    if(!ok)
    {
        ui->textEdit->append("Invalid expand register data.");
        return;
    }

    writeFPGAExtend(index, value);
    ui->textEdit->append(QString("WriteExpand %1 = 0x%2")
                             .arg(index)
                             .arg(value, 8, 16, QChar('0')));
}

void TechnicalSupport::on_TsButtonReadExpandFPGA_clicked()
{
    if(!checkCameraHandle())
        return;

    bool ok = false;
    uint16_t index = ui->TsLineEditWriteExpandFPGA_Address->text().toUShort(&ok, 10);
    if(!ok)
    {
        ui->textEdit->append("Invalid expand register address.");
        return;
    }

    uint32_t value = readFPGAExtend(index);
    ui->TsLineEditReadExpandFPGA_Data->setText(QString::number(value, 16).rightJustified(8, '0').toUpper());
    ui->textEdit->append(QString("ReadExpand %1 = 0x%2")
                             .arg(index)
                             .arg(value, 8, 16, QChar('0')));
}
