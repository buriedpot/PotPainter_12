#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "resetcanvasdlg.h"
#include "command.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle(tr("小锅画图v1.0"));
    this->setWindowIcon(QIcon(":/icons/icons/appicon"));
    this->drawActions.resize(numClass);
    this->transActions.resize(numTrans);
    this->resize(2500, 1600);

    ui->openpic->setIcon(QIcon(":/icons/images/file-open"));
    connect(ui->openpic, &QAction::triggered, this, &MainWindow::openFile);

    aboutAction = new QAction(QIcon(":/icons/icons/appicon"), tr("&About..."), this);
    aboutAction->setStatusTip(tr("About QT"));
    connect(aboutAction, &QAction::triggered, this, &MainWindow::about);

    addAction(drawActions, PENCIL, QIcon(":/icons/toolsicon/pencil2"), "笔", "铅笔\n用选定的线宽画一个任意形状的线条", "铅笔");
    drawActions[PENCIL]->setCheckable(1);
    drawActions[PENCIL]->setChecked(1);
    connect(drawActions[PENCIL], &QAction::triggered, ui->canvas, &Canvas::choosePencil);
    connect(drawActions[PENCIL], &QAction::triggered, this, &MainWindow::choosePencil);

    addAction(drawActions, LINE, QIcon(":/icons/toolsicon/line"), "直线", "绘制直线(默认Bresenham)",
              "绘制直线(DDA or Bresenham)(默认Bresenham)");
    connect(drawActions[LINE], &QAction::triggered, ui->canvas, &Canvas::chooseLine);
    connect(drawActions[LINE], &QAction::triggered, this, &MainWindow::chooseLine);

    addAction(drawActions, ELLIPSE, QIcon(":/icons/toolsicon/ellipse"), "椭圆", "绘制椭圆", "绘制直线(Bresenham中点椭圆)");
    connect(drawActions[ELLIPSE], &QAction::triggered, ui->canvas, &Canvas::chooseEllipse);
    connect(drawActions[ELLIPSE], &QAction::triggered, this, &MainWindow::chooseEllipse);

    addAction(drawActions, POLYGON, QIcon(":/icons/toolsicon/polygon"), "多边形", "绘制多边形", "绘制多边形");
    connect(drawActions[POLYGON], &QAction::triggered, ui->canvas, &Canvas::choosePolygon);
    connect(drawActions[POLYGON], &QAction::triggered, this, &MainWindow::choosePolygon);

    addAction(drawActions, FILLPOLYGON, QIcon(":/icons/toolsicon/fillpolygon"), "填充多边形", "绘制填充多边形", "绘制填充多边形");
    connect(drawActions[FILLPOLYGON], &QAction::triggered, ui->canvas, &Canvas::chooseFillPolygon);
    connect(drawActions[FILLPOLYGON], &QAction::triggered, this, &MainWindow::chooseFillPolygon);

    setcolor = new QAction(QIcon(":/icons/toolsicon/pencolor"), tr("画笔颜色"), this);
    setcolor->setToolTip(tr("选择画笔颜色"));
    setcolor->setStatusTip(tr("选择画笔颜色"));
    connect(setcolor, &QAction::triggered, ui->canvas, &Canvas::changecolor);
    //connect(drawActions[PENCIL], &QAction::triggered, this, &MainWindow::chooseLine);

    penwidthScroll = new QComboBox(this);
    QStringList strList;
    strList<<"1"<<"2"<<"4"<<"8"<<"12"<<"16"<<"20"<<"25"<<"30"<<"40";
    penwidthScroll->addItems(strList);
    penwidthScroll->setToolTip(tr("画笔粗细"));penwidthScroll->setStatusTip(tr("画笔粗细调节"));
    //penwidthScroll->resize(100, 30);
    //qDebug() << penwidthScroll->width() << penwidthScroll->height();
    connect(penwidthScroll, static_cast<void(QComboBox::*)(const QString&)>(&QComboBox::currentIndexChanged), this, &MainWindow::QStr2Int);
    connect(this, &MainWindow::emitWidth, ui->canvas, &Canvas::setpenWidth);

    addAction(drawActions, FILL, QIcon(":/icons/toolsicon/fill"), "用颜色填充", "用选定颜色填充与选中位置同像素的连通区域", "区域填充");
    connect(drawActions[FILL], &QAction::triggered, ui->canvas, &Canvas::chooseFill);
    connect(drawActions[FILL], &QAction::triggered, this, &MainWindow::chooseFill);

    addAction(drawActions, BEZIER, QIcon(":/icons/toolsicon/bezier"), "Bézier曲线", "Bézier曲线", "绘制曲线");
    connect(drawActions[BEZIER], &QAction::triggered, ui->canvas, &Canvas::chooseBezier);
    connect(drawActions[BEZIER], &QAction::triggered, this, &MainWindow::chooseBezier);

    addAction(drawActions, BSPLINE, QIcon(":/icons/toolsicon/bspline"), "B样条曲线", "B样条曲线", "绘制曲线");
    connect(drawActions[BSPLINE], &QAction::triggered, ui->canvas, &Canvas::chooseBSpline);
    connect(drawActions[BSPLINE], &QAction::triggered, this, &MainWindow::chooseBSpline);

    addAction(transActions, TRANSLATE, QIcon(":/icons/transicon/translate"), "平移", "平移", "平移");
    connect(transActions[TRANSLATE], &QAction::triggered, this, &MainWindow::chooseTranslate);
    connect(transActions[TRANSLATE], &QAction::triggered, ui->canvas, &Canvas::chooseTranslate);

    addAction(transActions, ROTATE, QIcon(":/icons/transicon/rotate"), "旋转", "绕指定点的旋转", "鼠标控制旋转中心，滚轮旋转，后滚为顺时针");
    connect(transActions[ROTATE], &QAction::triggered, this, &MainWindow::chooseRotate);
    connect(transActions[ROTATE], &QAction::triggered, ui->canvas, &Canvas::chooseRotate);

    addAction(transActions, SCALE, QIcon(":/icons/transicon/scale"), "缩放", "以基准点的缩放", "鼠标控制缩放中心，前滚为放大");
    connect(transActions[SCALE], &QAction::triggered, this, &MainWindow::chooseScale);
    connect(transActions[SCALE], &QAction::triggered, ui->canvas, &Canvas::chooseScale);

    addAction(transActions, CLIP, QIcon(":/icons/transicon/clip"), "裁剪", "对线段的裁剪", "鼠标按住拖动产生裁剪窗口，裁剪线段");
    connect(transActions[CLIP], &QAction::triggered, this, &MainWindow::chooseClip);
    connect(transActions[CLIP], &QAction::triggered, ui->canvas, &Canvas::chooseClip);

    addAction(transActions, ADJUST, QIcon(":/icons/transicon/bezier_adjust"), "曲线调整", "调整\n用鼠标对控制点调整以变换多边形或曲线控制多边形", "用鼠标对控制点调整以变换已绘制的曲线");
    connect(transActions[ADJUST], &QAction::triggered, this, &MainWindow::chooseAdjust);
    connect(transActions[ADJUST], &QAction::triggered, ui->canvas, &Canvas::chooseAdjust);
    //connect(ui->canvas, &Canvas::settransEnabled, this->transbar, &QToolBar::setEnabled);

    toolbar = this->addToolBar(tr("图形绘制"));
    toolbar->addAction(drawActions[PENCIL]);
    toolbar->addAction(drawActions[LINE]);
    toolbar->addAction(drawActions[ELLIPSE]);
    toolbar->addAction(drawActions[POLYGON]);
    toolbar->addAction(drawActions[FILLPOLYGON]);
    toolbar->addAction(drawActions[FILL]);
    toolbar->addAction(drawActions[BEZIER]);
    toolbar->addAction(drawActions[BSPLINE]);
    //toolbar->addAction()
    this->addToolBarBreak();
    transbar = this->addToolBar(tr("图元变换"));
    //transbar->setOrientation(Qt::Vertical);
    transbar->addAction(transActions[TRANSLATE]);
    transbar->addAction(transActions[ROTATE]);
    transbar->addAction(transActions[SCALE]);
    transbar->addAction(transActions[CLIP]);
    transbar->addAction(transActions[ADJUST]);
    transActions[TRANSLATE]->setCheckable(1);
    transActions[ROTATE]->setCheckable(1);
    transActions[SCALE]->setCheckable(1);
    transActions[CLIP]->setCheckable(1);
    transActions[ADJUST]->setCheckable(1);

    transActions[TRANSLATE]->setEnabled(0);
    transActions[ROTATE]->setEnabled(0);
    transActions[SCALE]->setEnabled(0);
    transActions[CLIP]->setEnabled(0);
    transActions[ADJUST]->setEnabled(0);
    setbar = this->addToolBar(tr("绘制选项"));
    setbar->addAction(setcolor);
    setbar->addWidget(penwidthScroll);
    setbar->addWidget(new QLabel("曲线控制点数", this));
    nCurveBox = new QSpinBox(this);
    nCurveBox->setRange(2, 20);
    nCurveBox->setValue(4);
    kBSplineBox = new QSpinBox(this);
    kBSplineBox->setRange(2, 6);
    kBSplineBox->setValue(4);
    connect(nCurveBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), ui->canvas, &Canvas::setnCurve);
    connect(kBSplineBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), ui->canvas, &Canvas::setkBSpline);
    setbar->addWidget(nCurveBox);
    setbar->addWidget(new QLabel("B样条阶数", this));
    setbar->addWidget(kBSplineBox);

    //setbar->addWidget(QComboBox)
    //setbar->addWidget(new QLabel(tr("Bézier阶数"), this));


    ui->clipalgorithm->setIcon(QIcon(":/icons/icons/clipalgorithm"));
    ui->impscript->setIcon(QIcon(":/icons/icons/script"));
    ui->menu_About->addAction(aboutAction);
    ui->resetcanvas->setIcon(QIcon(":/icons/icons/canvas"));
    ui->openpic->setShortcuts(QKeySequence::Open);
    ui->savepic->setShortcuts(QKeySequence::Save);
    ui->savepic->setIcon(QIcon(":/icons/images/file-save"));
    ui->linealgorithm->setIcon(QIcon(":/icons/icons/linealgorithm"));
    ui->resizecanvas->setIcon(QIcon(":/icons/icons/resizecanvas"));


    connect(ui->savepic, &QAction::triggered, this, &MainWindow::saveFile);

    connect(ui->actionDDA, &QAction::triggered, this, &MainWindow::chooseDDA);
    connect(ui->actionBresenham, &QAction::triggered, this, &MainWindow::chooseBresenham);
    connect(ui->impscript, &QAction::triggered, this, &MainWindow::importScript);
    painter = new QPainter(ui->canvas);
    ui->canvas->resize(1600, 1200);

    toolbarChecked(PENCIL, drawActions);
    connect(ui->canvas, &Canvas::setTranslateEnabled, this->transActions[TRANSLATE], &QAction::setEnabled);
    connect(ui->canvas, &Canvas::setRotateEnabled, this->transActions[ROTATE], &QAction::setEnabled);
    connect(ui->canvas, &Canvas::setScaleEnabled, this->transActions[SCALE], &QAction::setEnabled);
    connect(ui->canvas, &Canvas::setClipEnabled, this->transActions[CLIP], &QAction::setEnabled);
    connect(ui->canvas, &Canvas::setAdjustEnabled, this->transActions[ADJUST], &QAction::setEnabled);
    connect(ui->action_Barsky, &QAction::triggered, ui->canvas, &Canvas::chooseLiang_Barsky);
    connect(ui->actionCohen_Sutherland, &QAction::triggered, ui->canvas, &Canvas::chooseCohen_Sutherland);
    connect(ui->resizecanvas, &QAction::triggered, this, &MainWindow::resizecanvas);
    //ui->canvas->pixmap.scaled()
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::closeEvent(QCloseEvent *event)
 {
      if (ui->canvas->isSaved == true) {
          event->accept();//接受，关闭
      } else {
          QMessageBox box;
          box.setIcon(QMessageBox::Warning);
          QPushButton *yes = box.addButton(tr("保存"),QMessageBox::YesRole);
          box.addButton(tr("不保存"),QMessageBox::NoRole);
          QPushButton *cancel = box.addButton(tr("取消"), QMessageBox::RejectRole);
          box.setWindowTitle(tr("文件未保存"));
          box.setText(tr("是否需要保存文件") + ui->canvas->filename + tr("?"));
          box.exec();
          if (box.clickedButton() == yes) {
              saveFile();
              if (ui->canvas->isSaved == false) {
                  event->ignore();
              }
          }
          else if (box.clickedButton() == cancel){
              event->ignore();
          }
          else {
              event->accept();
          }
      }
 }
void MainWindow::addAction(QVector<QAction*>& acts, int index, QIcon icon, const char text[],
               const char tooltip[], const char statustip[]) {
    acts[index] = new QAction(icon, tr(text), this);
    acts[index]->setToolTip(tr(tooltip));
    acts[index]->setStatusTip(tr(statustip));
}

void MainWindow::about()
{
    QMessageBox::about(this, "小锅画图v1.0", "Copyright@2019 Ruijie Guo.\n All rights reserved.");
}

void MainWindow::actAction(QAction *action) {
    action->setEnabled(true);
}
void MainWindow::actActions(QVector<QAction *> &acts) {
    size_t length = acts.size();
    for (size_t i = 0; i < length; ++i) {
        acts[i]->setEnabled(true);
    }
}
void MainWindow::inactAction(QAction *action) {
    action->setEnabled(false);
}
void MainWindow::inactActions(QVector<QAction *> &acts) {
    size_t length = acts.size();
    for (size_t i = 0; i < length; ++i) {
        acts[i]->setChecked(false);
    }
}
void MainWindow::actTools(QToolBar *bar) {
    bar->setEnabled(true);
}
void MainWindow::inactTools(QToolBar *bar){
    bar->setEnabled(false);
}



void MainWindow::on_resetcanvas_triggered()
{
    Canvas *tmpc = ui->canvas;
    tmpc->isDrawing = false;
    tmpc->Transforming = -1;
    tmpc->isTransforming = 0;
    tmpc->isBezierDrawDone = true;
    tmpc->isPolygonDrawDone = true;
    tmpc->vb.clear();
    tmpc->graphseq.clear();
    ui->impscript->setEnabled(1);
    setTransEnabled(0);
    ResetCanvasDlg *dlg = new ResetCanvasDlg(this);
    dlg->setWindowTitle(tr("选择画布大小"));
    dlg->resize(600,400);
    int len, wid;
    len = ui->canvas->size().width();
    wid = ui->canvas->size().height();
    /*把当前对话框中的宽度和高度设置为当前画布的宽度和高度*/
    dlg->setlen(len);
    dlg->setwid(wid);
    if (dlg->exec() == 1) {
        /*设置一下用来贴图的Widget组件*/
        ui->canvas->setStyleSheet("background-color: rgb(255, 255, 255)");
        ui->canvas->resize(dlg->getlen(), dlg->getwid());
        /*重置绘图设备pixmap*/
        ui->canvas->pixmap = QPixmap(ui->canvas->size());
        ui->canvas->pixmap.fill(Qt::white);
        ui->canvas->penColor = Qt::black;
    }
}
void MainWindow::resizecanvas() {
    ResetCanvasDlg *dlg = new ResetCanvasDlg(this);
    dlg->setWindowTitle(tr("选择画布大小"));
    dlg->resize(600,400);
    int len, wid;
    len = ui->canvas->size().width();
    wid = ui->canvas->size().height();
    /*把当前对话框中的宽度和高度设置为当前画布的宽度和高度*/
    dlg->setlen(len);
    dlg->setwid(wid);
    if (dlg->exec() == 1) {
        /*设置一下用来贴图的Widget组件*/
        ui->canvas->resize(dlg->getlen(), dlg->getwid());
        /*重置绘图设备pixmap*/
        QPixmap tmpmap = ui->canvas->pixmap;
        ui->canvas->pixmap = QPixmap(ui->canvas->size());
        QPainter tmppainter(&ui->canvas->pixmap);
        ui->canvas->pixmap.fill(Qt::white);
        tmppainter.drawPixmap(0, 0, tmpmap);
    }
}
void MainWindow::toolbarChecked(int c, QVector<QAction*>& acts) {
    for (int i = 0; i < acts.size(); ++i) {
        if (i != c) {
            acts[i]->setCheckable(true);
            acts[i]->setChecked(false);
        }
    }
    acts[c]->setChecked(true);
}
void MainWindow::chooseLine() {
    setTransEnabled(0);
    setTransChecked(0);
    toolbarChecked(LINE, drawActions);
}
void MainWindow::choosePencil() {
    setTransEnabled(0);
    setTransChecked(0);
    toolbarChecked(PENCIL, drawActions);
}
void MainWindow::chooseEllipse() {
    setTransEnabled(0);
    setTransChecked(0);
    toolbarChecked(ELLIPSE, drawActions);
}
void MainWindow::choosePolygon() {
    setTransEnabled(0);
    setTransChecked(0);
    toolbarChecked(POLYGON, drawActions);
}
void MainWindow::chooseFillPolygon() {
    setTransEnabled(0);
    setTransChecked(0);
    toolbarChecked(FILLPOLYGON, drawActions);
}
void MainWindow::chooseFill() {
    setTransEnabled(0);
    setTransChecked(0);
    toolbarChecked(FILL, drawActions);
}
void MainWindow::chooseBezier() {
    setTransEnabled(0);
    setTransChecked(0);
    toolbarChecked(BEZIER, drawActions);
}
void MainWindow::chooseBSpline() {
    setTransEnabled(0);
    setTransChecked(0);
    toolbarChecked(BSPLINE, drawActions);
}
void MainWindow::chooseDDA() {
    ui->canvas->lineClass = DDALINE;
}
void MainWindow::chooseBresenham() {
    ui->canvas->lineClass = BRELINE;
}
void MainWindow::chooseTranslate() {
    toolbarChecked(TRANSLATE, transActions);
    inactActions(drawActions);
}
void MainWindow::chooseRotate() {
    toolbarChecked(ROTATE, transActions);
    inactActions(drawActions);
}
void MainWindow::chooseScale() {
    toolbarChecked(SCALE, transActions);
    inactActions(drawActions);
}
void MainWindow::chooseClip() {
    toolbarChecked(CLIP, transActions);
    inactActions(drawActions);
}
void MainWindow::chooseAdjust() {
    toolbarChecked(ADJUST, transActions);
    inactActions(drawActions);
}

void MainWindow::openFile()
{
    QFileDialog dlg;
    dlg.setWindowTitle(tr("打开图片文件"));
    dlg.setAcceptMode(QFileDialog::AcceptOpen);
    dlg.setNameFilter(tr("PNG Files(*.png);;JPEG Files(*.jpg);;BMP Files(*.bmp)"));
    if(dlg.exec() == QDialog::Accepted) {//注意使用的是QFileDialog::Accepted或者QDialog::Accepted,不是QFileDialog::Accept
        QString path = dlg.selectedFiles()[0];//得到用户选择的文件名
        ui->canvas->filename = path;
        if(!path.isEmpty()) {
            QPixmap tmppix;
            if (tmppix.load(path)) {
                QPainter painter(&ui->canvas->pixmap);
                ui->canvas->pixmap.fill(Qt::white);
                painter.drawPixmap(0, 0, tmppix);
                ui->canvas->isSaved = true;
                Canvas *tmpc = ui->canvas;
                tmpc->isDrawing = false;
                tmpc->Transforming = -1;
                tmpc->isTransforming = 0;
                tmpc->isBezierDrawDone = true;
                tmpc->isPolygonDrawDone = true;
                tmpc->vb.clear();
                tmpc->graphseq.clear();
                ui->impscript->setEnabled(1);
                setTransEnabled(0);
                update();
            }
            //textEdit->setText(in.readAll());
        } else {
            QMessageBox::warning(this, tr("路径未选择"),
                                 tr("您没有选择任何图片文件!"));
        }
    }
}
void MainWindow::saveFile() {
    QFileDialog dlg;
    dlg.setWindowTitle(tr("保存图片文件"));
    dlg.setAcceptMode(QFileDialog::AcceptSave);
    dlg.selectFile("new");
    dlg.setNameFilter(tr("PNG Files(*.png);;JPEG Files(*.jpg);;BMP Files(*.bmp)"));
    if(dlg.exec() == QDialog::Accepted) {//注意使用的是QFileDialog::Accepted或者QDialog::Accepted,不是QFileDialog::Accept
        QString path = dlg.selectedFiles()[0];//得到用户选择的文件名
        if(!path.isEmpty()) {
            if (!ui->canvas->pixmap.save(path)) {
                QMessageBox::warning(this, "警告", "文件保存失败!");
            }
            else {
                ui->canvas->isSaved = true;
            }
            //textEdit->setText(in.readAll());
        } else {
            QMessageBox::warning(this, tr("路径未选择"),
                                 tr("您没有选择保存位置!"));
        }
    }
}
void MainWindow::importScript() {
    QFileDialog dlg;
    dlg.setWindowTitle(tr("导入脚本文件"));
    dlg.setAcceptMode(QFileDialog::AcceptOpen);
    dlg.setNameFilter(tr("TXT Files(*.txt)"));
    if(dlg.exec() == QDialog::Accepted) {//注意使用的是QFileDialog::Accepted或者QDialog::Accepted,不是QFileDialog::Accept
        QString path = dlg.selectedFiles()[0];//得到用户选择的文件名
        ui->canvas->filename = path;
        if(!path.isEmpty()) {
            QFile script(path);
            script.open(QIODevice::ReadOnly | QIODevice::Text);
            while (!script.atEnd()) {
                QString dir_str = "pngs";
                QDir dir;
                if (!dir.exists(dir_str)) {
                    dir.mkpath(dir_str);
                }
                Command c("pngs");
                QByteArray line = script.readLine();
                QString tmp(line);
                if (tmp[tmp.size() - 1] == '\n')
                    tmp = tmp.left(tmp.size() - 1);
                string command = tmp.toStdString();
                //cout << command << endl;
                int operate = c.cmdAnalyze(command, *ui->canvas, script);
                //canvas->update();
                if (operate == -1) cout << "Invalid Command!" << endl;
            }
            update();
        } else {
            QMessageBox::warning(this, tr("路径未选择"),
                                 tr("您没有选择任何图片文件!"));
        }
    }
    ui->impscript->setEnabled(0);
}
void MainWindow::execScript(QString &filename) {

}
void MainWindow::cmdDraw(){}
void MainWindow::QStr2Int(const QString& text) {
    emit emitWidth(text.toInt());
}
void MainWindow::setTransChecked(bool b) {
    for (int i = 0; i < numTrans; ++i) {
        transActions[i]->setChecked(b);
    }
}
void MainWindow::setTransEnabled(bool b) {
    for (int i = 0; i < numTrans; ++i) {
        transActions[i]->setEnabled(b);
    }
}

