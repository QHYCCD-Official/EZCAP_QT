#ifndef TECHNICALSUPPORT_H
#define TECHNICALSUPPORT_H

#include <QDialog>
#if defined (WIN32)
#else
#include "libusb-1.0/libusb.h"
#endif

namespace Ui {
class TechnicalSupport;
}

class TechnicalSupport : public QDialog
{
    Q_OBJECT

public:
    explicit TechnicalSupport(QWidget *parent = 0);
    ~TechnicalSupport();

#if defined (WIN32)
#else
    libusb_device_handle *dev_handle;
    libusb_context *ctx = NULL;
#endif

private slots:
    void on_pushButton_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_5_clicked();

    void on_pushButton_7_clicked();


    void on_TsButtonScanLibUsb_2_clicked();

    void on_TsButtonScanLibUsb_clicked();

    void on_TsButtonOpenDeviceLibUsb_clicked();

    void on_pBtnVendRead_clicked();

    void on_pBtnVenWrite_clicked();

    void on_pBtnBulkRead_clicked();

    void on_pBtnCloseDevice_clicked();

    void on_pBtnLowLevelD5_clicked();

private:
    Ui::TechnicalSupport *ui;
#if defined (WIN32)
#else
    void print_devs(libusb_device **devs);
#endif
};


extern class TechnicalSupport *technicalSupport_dialog;
#endif // TECHNICALSUPPORT_H
