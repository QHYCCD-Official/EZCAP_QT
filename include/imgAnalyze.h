#ifndef IMGANALYZE_H
#define IMGANALYZE_H

#include <QDialog>

namespace Ui {
class ImgAnalyze;
}

class ImgAnalyze : public QDialog
{
    Q_OBJECT

public:
    explicit ImgAnalyze(QWidget *parent = 0);
    ~ImgAnalyze();

private:
    Ui::ImgAnalyze *ui;
};

extern ImgAnalyze *imgAnalyze_dialog;

#endif // IMGANALYZE_H
