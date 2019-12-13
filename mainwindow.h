#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <canvas.h>

#include <QMainWindow>
#include <QAction>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QDialog>
#include <QImage>
#include <QPixmap>
#include <QPainter>
#include <QPaintEvent>
#include <QVector>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    friend class Canvas;
    void closeEvent(QCloseEvent *event);

private:
    Ui::MainWindow *ui;


    void about();

    QAction *openAction;
    QAction *aboutAction;

    int currentGraph;
    QAction *drawPencil;//用铅笔随意绘画
    QAction *drawLine;//绘制直线
    QAction *drawEllipse;
    QAction *drawPolygon;
    QVector<QAction *> drawActions;
    QVector<QAction *> transActions;
    QToolBar* toolbar, *transbar, *setbar;

    QAction *setcolor;
    QAction *fillAction;

    QComboBox *penwidthScroll;


    QPixmap *canvas;
    QPainter *painter;



private:
    void addAction(QVector<QAction*>& acts, int index, QIcon icon, const char text[] = "",
                   const char tooltip[] = "", const char statustip[] = "");
    void inactActions(vector<QAction*>&);//Inactivate Actions.使图标可点击
    void inactTools(QToolBar *);
    void inactAction(QAction*);
    void actActions(vector<QAction*>&);//Activate Actions.使图标不可点击
    void actTools(QToolBar *);
    void actAction(QAction*);
    void setTransChecked(bool);
    void setTransEnabled(bool);

Q_SIGNALS:
    void emitWidth(int);
private slots:
    void on_resetcanvas_triggered();
    void toolbarChecked(int, QVector<QAction*>& acts);
    void choosePencil();
    void chooseLine();
    void chooseEllipse();
    void choosePolygon();
    void chooseFillPolygon();
    void chooseFill();
    void chooseBezier();
    void chooseBSpline();
    void chooseTranslate();
    void chooseRotate();
    void chooseScale();
    void chooseClip();
    void chooseAdjust();

    void saveFile();
    void openFile();
    void importScript();

    void execScript(QString &filename);
    void cmdDraw();//开启命令行画图窗口
    void chooseDDA();
    void chooseBresenham();
    void QStr2Int(const QString& text);
        //void chooseSetcolor();
};

#endif // MAINWINDOW_H
