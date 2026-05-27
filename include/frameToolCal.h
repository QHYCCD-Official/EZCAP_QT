#ifndef FRAMETOOLCAL_H
#define FRAMETOOLCAL_H

#include <QDialog>

namespace Ui {
class FrameToolCal;
}

class FrameToolCal : public QDialog
{
    Q_OBJECT

public:
    explicit FrameToolCal(QWidget *parent = nullptr);
    ~FrameToolCal();

private slots:

    void on_rBtn_FrameCal_toggled(bool checked);

    void on_rBtn_DarkCal_toggled(bool checked);

    void on_pBtn_SelectDark_clicked();

    void on_pBtn_SelectFlat_clicked();

    void on_pBtn_SelectBias_clicked();

    void on_pBtn_EnableCal_clicked();

private:
    Ui::FrameToolCal *ui;

    QString darkPath;
    QString flatPath;
    QString biasPath;
};

extern class FrameToolCal *frameToolCal_dialog;

#endif // FRAMETOOLCAL_H
