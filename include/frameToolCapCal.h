#ifndef FRAMETOOLCAPCAL_H
#define FRAMETOOLCAPCAL_H

#include <QDialog>

namespace Ui {
class FrameToolCapCal;
}

class FrameToolCapCal : public QDialog
{
    Q_OBJECT

public:
    explicit FrameToolCapCal(QWidget *parent = 0);
    ~FrameToolCapCal();

    int capCalFlag; //0:dark 1:flat 2:bias
    QString savePath;

    void setEnableUI();
    void setDisableUI();
    void showSaveFolder(QString folder);

private slots:
    void on_btnStartDarkCap_clicked();

    void on_radioButton_CapCalDark_clicked();

    void on_radioButton_CapCalFlat_clicked();

    void on_radioButton_CapCalBias_clicked();

    void on_pushButton_OpenCalSaveDir_clicked();

private:
    Ui::FrameToolCapCal *ui;
};

extern class FrameToolCapCal *frameToolCapCal_dialog;

#endif // FRAMETOOLCAPCAL_H
