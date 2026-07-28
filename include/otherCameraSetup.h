#ifndef OTHERCAMERASETUP_H
#define OTHERCAMERASETUP_H

#include <QDialog>

namespace Ui {
class OtherCameraSetup;
}

class OtherCameraSetup : public QDialog
{
    Q_OBJECT

public:
    explicit OtherCameraSetup(QWidget *parent = nullptr);
    ~OtherCameraSetup();

private slots:
    void camera_connected();
    void camera_disconnected();

    void on_comboBox_HeatingBoardLevel_activated(const QString &arg1);
    void on_pushButton_CAARotatorSet_clicked();

    void on_comboBox_DPC_currentTextChanged(const QString &arg1);

    void on_horizontalSlider_DPCValue_valueChanged(int value);

    void on_spinBox_DPCValue_valueChanged(int arg1);

private:
    Ui::OtherCameraSetup *ui;
};

extern class OtherCameraSetup *otherCameraSetup_dialog;//define global class object

#endif // OTHERCAMERASETUP_H
