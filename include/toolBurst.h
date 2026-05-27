#ifndef TOOLBURST_H
#define TOOLBURST_H

#include "threadBurstCapture.h"
#include <QDialog>

namespace Ui {
class ToolBurst;
}

class ToolBurst : public QDialog
{
    Q_OBJECT

public:
    explicit ToolBurst(QWidget *parent = nullptr);
    ~ToolBurst();

private:
    Ui::ToolBurst *ui;

    ThreadBurstCapture *threadBurstCapture;

private slots:
    void camera_connected();
    void camera_disconnected();

    void updateBurstInfo(int iGroup, int iRepeat, int iCount, int iSeqence);
    void burstThreadDelete();

    void on_comboBox_Burst_currentTextChanged(const QString &arg1);
    void on_spinBox_Patch_valueChanged(int arg1);
    void on_pushButton_BurstCapure_clicked();
    void on_pushButton_BurstAbort_clicked();
    void on_pushButton_AddGroup_clicked();
    void on_pushButton_ClearInfo_clicked();
};

extern class ToolBurst *toolBurst_dialog;

#endif // TOOLBURST_H
