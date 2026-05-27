#ifndef TOOLCORRECTCENTER_H
#define TOOLCORRECTCENTER_H

#include <QDialog>

namespace Ui {
class ToolCorrectCenter;
}

class ToolCorrectCenter : public QDialog
{
    Q_OBJECT

public:
    explicit ToolCorrectCenter(QWidget *parent = nullptr);
    ~ToolCorrectCenter();

private slots:
    void camera_connected();

    void camera_disconnected();

    void on_comboBox_Correct_currentTextChanged(const QString &arg1);

    void on_spinBox_Radius_valueChanged(int arg1);

    void on_pushButton_RadiusAdd10_clicked();

    void on_pushButton_RadiusSub10_clicked();

    void on_pushButton_RadiusAdd100_clicked();

    void on_pushButton_RadiusSub100_clicked();

    void on_spinBox_Spacing_valueChanged(int arg1);

    void on_pushButton_SpacingAdd10_clicked();

    void on_pushButton_SpacingSub10_clicked();

    void on_spinBox_Thickness_valueChanged(int arg1);

    void on_radioButton_Red_toggled(bool checked);

    void on_radioButton_Green_toggled(bool checked);

    void on_radioButton_Blue_toggled(bool checked);

    void on_radioButton_Yellow_toggled(bool checked);

    void on_radioButton_White_toggled(bool checked);

    void on_radioButton_Black_toggled(bool checked);

    void on_pushButton_Save_clicked();

private:
    Ui::ToolCorrectCenter *ui;
};

extern class ToolCorrectCenter *toolCorrectCenter_dialog;

#endif // TOOLCORRECTCENTER_H
