#include "resetcanvasdlg.h"
#include "ui_resetcanvasdlg.h"

ResetCanvasDlg::ResetCanvasDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ResetCanvasDlg)
{
    ui->setupUi(this);
}

ResetCanvasDlg::~ResetCanvasDlg()
{
    delete ui;
}
