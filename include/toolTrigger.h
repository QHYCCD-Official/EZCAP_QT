#ifndef TOOLTRIGGER_H
#define TOOLTRIGGER_H

#include <QDialog>

namespace Ui {
class ToolTrigger;
}

class ToolTrigger : public QDialog
{
    Q_OBJECT

public:
    explicit ToolTrigger(QWidget *parent = nullptr);
    ~ToolTrigger();

private slots:
    void camera_connected();

    void camera_disconnected();

    void on_comboBox_TrigInterface_currentIndexChanged(int index);

    void on_comboBox_TrigMode_currentIndexChanged(int index);

    void on_comboBox_Trigger_currentTextChanged(const QString &arg1);

    void on_comboBox_TrigInOnly_currentTextChanged(const QString &arg1);

    void on_comboBox_TrigOutOnly_currentTextChanged(const QString &arg1);

private:
    Ui::ToolTrigger *ui;
};

extern ToolTrigger *toolTrigger_dialog;

#endif // TOOLTRIGGER_H
