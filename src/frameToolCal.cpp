#include "frameToolCal.h"
#include "ui_frameToolCal.h"
#include <QDir>
#include <QStandardPaths>
#include <QFileDialog>
#include <QMessageBox>
#include "fitHeader.h"
#include "myStruct.h"

FrameToolCal *frameToolCal_dialog;

FrameToolCal::FrameToolCal(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FrameToolCal)
{
    ui->setupUi(this);

    darkPath = "";
    flatPath = "";
    biasPath = "";
}

FrameToolCal::~FrameToolCal()
{
    delete ui;
}

void FrameToolCal::on_rBtn_FrameCal_toggled(bool checked)
{
    if(checked)
    {
        ui->label_Dark->setEnabled(true);
        ui->label_Flat->setEnabled(true);
        ui->label_Bias->setEnabled(true);

        ui->lineEdit_Dark->setEnabled(true);
        ui->lineEdit_Flat->setEnabled(true);
        ui->lineEdit_Bias->setEnabled(true);

        ui->pBtn_SelectDark->setEnabled(true);
        ui->pBtn_SelectFlat->setEnabled(true);
        ui->pBtn_SelectBias->setEnabled(true);
    }
}

void FrameToolCal::on_rBtn_DarkCal_toggled(bool checked)
{
    if(checked)
    {
        ui->label_Dark->setEnabled(true);
        ui->label_Flat->setEnabled(false);
        ui->label_Bias->setEnabled(false);

        ui->lineEdit_Dark->setEnabled(true);
        ui->lineEdit_Flat->setEnabled(false);
        ui->lineEdit_Bias->setEnabled(false);

        ui->pBtn_SelectDark->setEnabled(true);
        ui->pBtn_SelectFlat->setEnabled(false);
        ui->pBtn_SelectBias->setEnabled(false);
    }
}

void FrameToolCal::on_pBtn_SelectDark_clicked()
{
    QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QDir dir(desktopPath+"/EZCAP Captures/Dark/");
    if(!dir.exists())
    {
        dir.setPath(desktopPath);
    }

    QString filePath;
    filePath = QFileDialog::getOpenFileName(this, tr("Open"), dir.path(), tr("FITS Files(*.fit *.fits)"));
    if(!filePath.isEmpty())
    {
        darkPath = filePath;

        QFileInfo fi(filePath);
        ui->lineEdit_Dark->setText(fi.fileName());
    }
}

void FrameToolCal::on_pBtn_SelectFlat_clicked()
{
    QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);// + "/EZCAP Captures/";
    QDir dir(desktopPath+"/EZCAP Captures/Flat/");
    if(!dir.exists())
    {
        dir.setPath(desktopPath);
    }

    QString filePath;
    filePath = QFileDialog::getOpenFileName(this, tr("Open"), dir.path(), tr("FITS Files(*.fit *.fits)"));
    if(!filePath.isEmpty())
    {
        flatPath = filePath;

        QFileInfo fi(filePath);
        ui->lineEdit_Flat->setText(fi.fileName());
    }
}

void FrameToolCal::on_pBtn_SelectBias_clicked()
{
    QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);// + "/EZCAP Captures/";
    QDir dir(desktopPath+"/EZCAP Captures/Bias/");
    if(!dir.exists())
    {
        dir.setPath(desktopPath);
    }

    QString filePath;
    filePath = QFileDialog::getOpenFileName(this, tr("Open"), dir.path(), tr("FITS Files(*.fit *.fits)"));
    if(!filePath.isEmpty())
    {
        biasPath = filePath;

        QFileInfo fi(filePath);
        ui->lineEdit_Bias->setText(fi.fileName());
    }
}

void FrameToolCal::on_pBtn_EnableCal_clicked()
{
    if(ui->pBtn_EnableCal->text() == "Calibration ON")
    {
        uint32_t imgw, imgh, imgb, imgc;
        if(ui->rBtn_DarkCal->isChecked())
        {
            if(darkPath == "") return;

            if(fitHeader_dialog->FITRead_Common(darkPath, &imgw, &imgh, &imgb, &imgc, ix.ImgData_Dark))
            {
                ix.darkCal = false;
                ix.frameCal  = false;
                return;
            }
            ix.darkCal = true;
            ix.frameCal  = false;
        }
        else
        {
            if(darkPath == "" || flatPath == "" || biasPath == "") return;

            if(fitHeader_dialog->FITRead_Common(darkPath, &imgw, &imgh, &imgb, &imgc, ix.ImgData_Dark))
            {
                QMessageBox::critical(this, tr("Warning"), tr("Can not read dark FIT file"), QMessageBox::Ok);
                return;
            }

            uint8_t *flatData = (uint8_t *)malloc(ix.FrameW*ix.FrameH*ix.FrameB*ix.FrameC/8);
            uint8_t *biasData = (uint8_t *)malloc(ix.FrameW*ix.FrameH*ix.FrameB*ix.FrameC/8);
            if(fitHeader_dialog->FITRead_Common(flatPath, &imgw, &imgh, &imgb, &imgc, flatData))
            {
                free(flatData);
                free(biasData);
                QMessageBox::critical(this, tr("Warning"), tr("Can not read flat FIT file"), QMessageBox::Ok);
                return;
            }
            if(fitHeader_dialog->FITRead_Common(biasPath, &imgw, &imgh, &imgb, &imgc, biasData))
            {
                free(flatData);
                free(biasData);
                QMessageBox::critical(this, tr("Warning"), tr("Can not read bias FIT file"), QMessageBox::Ok);
                return;
            }

//            double total1 = 0.0, total2 = 0.0, total3 = 0.0;
            ix.avgFlatBias1 = ix.avgFlatBias2 = ix.avgFlatBias3 = 0.0;
            if(ix.FrameB == 8 && ix.FrameC == 1)
            {
                for(int i = 0; i < ix.FrameW*ix.FrameH; i ++)
                {
                    ix.ImgData_FB[i] = flatData[i] - biasData[i];
                    if(ix.ImgData_FB[i] < 1) ix.ImgData_FB[i] = 1;
                    ix.avgFlatBias1 += ix.ImgData_FB[i];
                }
                ix.avgFlatBias1 /= static_cast<double>(ix.FrameW*ix.FrameH);
            }
            else if(ix.FrameB == 16 && ix.FrameC == 1)
            {
                for(int i = 0; i < ix.FrameW*ix.FrameH; i ++)
                {
                    ix.ImgData_FB[i] = (flatData[2*i+1]+flatData[2*i]) - (biasData[2*i+1]*256+biasData[2*i]);
                    if(ix.ImgData_FB[i] < 1) ix.ImgData_FB[i] = 1;
                    ix.avgFlatBias1 += ix.ImgData_FB[i];
                }
                ix.avgFlatBias1 /= static_cast<double>(ix.FrameW*ix.FrameH);
            }
            else if(ix.FrameB == 8 && ix.FrameC == 3)
            {
                for(int i = 0; i < ix.FrameW*ix.FrameH; i++)
                {
                    ix.ImgData_FB[3*i]   = flatData[3*i]   - biasData[3*i];
                    if(ix.ImgData_FB[3*i] < 1) ix.ImgData_FB[3*i] = 1;
                    ix.ImgData_FB[3*i+1] = flatData[3*i+1] - biasData[3*i+1];
                    if(ix.ImgData_FB[3*i+1] < 1) ix.ImgData_FB[3*i+1] = 1;
                    ix.ImgData_FB[3*i+2] = flatData[3*i+2] - biasData[3*i+2];
                    if(ix.ImgData_FB[3*i+2] < 1) ix.ImgData_FB[3*i+2] = 1;
                    ix.avgFlatBias1 += ix.ImgData_FB[3*i];
                    ix.avgFlatBias2 += ix.ImgData_FB[3*i+1];
                    ix.avgFlatBias3 += ix.ImgData_FB[3*i+2];
                }
                ix.avgFlatBias1 /= static_cast<double>(ix.FrameW*ix.FrameH);
                ix.avgFlatBias2 /= static_cast<double>(ix.FrameW*ix.FrameH);
                ix.avgFlatBias3 /= static_cast<double>(ix.FrameW*ix.FrameH);
            }

            ix.darkCal = false;
            ix.frameCal  = true;
            free(flatData);
            free(biasData);
        }
        ui->pBtn_EnableCal->setText("Calibration OFF");
    }
    else
    {
        ix.darkCal = false;
        ix.frameCal  = false;
        ui->pBtn_EnableCal->setText("Calibration ON");
    }
}
