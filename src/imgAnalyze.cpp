#include "imgAnalyze.h"
#include "ui_imgAnalyze.h"

extern struct IX ix;
ImgAnalyze *imgAnalyze_dialog;

ImgAnalyze::ImgAnalyze(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ImgAnalyze)
{
    ui->setupUi(this);
}

ImgAnalyze::~ImgAnalyze()
{
    delete ui;
}
