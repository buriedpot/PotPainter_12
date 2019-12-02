#include "resetcanvasdlg.h"
#include "ui_resetcanvasdlg.h"

ResetCanvasDlg::ResetCanvasDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ResetCanvasDlg)
{
    ui->setupUi(this);
    this->resize(300,260);
    //QHBoxLayout *layout = new QHBoxLayout;
    //layout->addLayout(ui->gridLayout);
    //layout->addWidget(ui->buttonBox);
    //this->setLayout(layout);

}

ResetCanvasDlg::~ResetCanvasDlg()
{
    delete ui;
}
int ResetCanvasDlg::getlen() {
    return ui->lenspin->value();
}
int ResetCanvasDlg::getwid() {
    return ui->widspin->value();
}

void ResetCanvasDlg::setlen(int len) {
    ui->lenspin->setValue(len);
}
void ResetCanvasDlg::setwid(int wid) {
    ui->widspin->setValue(wid);
}
