#ifndef RESETCANVASDLG_H
#define RESETCANVASDLG_H

#include <QDialog>

namespace Ui {
class ResetCanvasDlg;
}

class ResetCanvasDlg : public QDialog
{
    Q_OBJECT

public:
    explicit ResetCanvasDlg(QWidget *parent = nullptr);
    ~ResetCanvasDlg();

private:
    Ui::ResetCanvasDlg *ui;

public:
    int getlen();
    int getwid();
    void setlen(int);
    void setwid(int);
};

#endif // RESETCANVASDLG_H
