#ifndef CANVAS_H
#define CANVAS_H
/*
 * 定义图形的宏
 */
#define numClass 7
#define numTrans 4
typedef enum GRAPHCLASS{
    CHOOSE=-1, PENCIL, LINE, ELLIPSE, POLYGON, FILLPOLYGON, FILL, BEZIER, DDALINE, BRELINE, DDAPOLYGON, BREPOLYGON,
    BEZIERCURVE, BSPLINECURVE
} GRAPHCLASS;
typedef enum TRANSCLASS {
    TRANSLATE = 0, ROTATE, SCALE, CLIP
} TRANSCLASS;

typedef struct ClassandId {
    enum GRAPHCLASS c;
    int id;
} ClassandId;

#include <QObject>
#include <QWidget>
#include <QPainter>
#include <algorithm>
#include <QVector>
#include <QDebug>
#include <QPixmap>
#include <QImage>
#include <QToolBar>
#include <QColorDialog>
#include <QPoint>
#include <QMouseEvent>
#include <QPen>
#include <vector>
#include <stack>
#include <iostream>
#include <QTransform>
#include <math.h>
#include "graphstruct.h"

using namespace std;

//#include

class Canvas : public QWidget
{
    Q_OBJECT
    friend class MainWindow;
    friend class Command;
public:
    QPixmap pixmap;
    QPixmap transmap;//进行变换之前的map，是即将被变换的图元的底板，是上一个pixmap
    int graphClass;//区分当前画的是直线还是圆还是别的
    QColor penColor;//记录当前笔画颜色
    int penWidth;
    QFont penFont;
    Qt::PenCapStyle pencapStyle;
    int background;//背景颜色
    int lineClass;//决定用DDA算法绘制还是Bresenham算法绘制

    int xl, xr, yd, yu;//绘制的图元的外接多边形


public:
    explicit Canvas(QWidget *parent = nullptr);
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);
    void redraw();

private:
    bool pixmapdrawDone;
    bool isSaved;
    bool isDrawing;//判断用户是否正在使用鼠标拖动绘图
    bool isMouseMove;//用于追踪多边形绘制
    QPoint firstPoint;//多边形第一个顶点坐标
    int lineDrawed;//多边形已经绘制了几条边或者贝塞尔曲线已经绘制了几加一个点
    bool isPolygonDrawDone;
    bool isBezierDrawDone;
    bool needtoUpdate;//比如图元进行了平移、裁剪，需要擦除整个画布进行重绘
    bool needtoTrans;//还没变换

    /*用于用户鼠标拖动绘图时的跟踪*/
    bool updateSuccess;//由于PaintEvent处理时间不够，看看是否成功调用了Update
    QPoint lastendPoint;//由于PaintEvent处理时间不够，需要用直线去插值中间点的技术手段
    QPoint startPoint;
    QPoint endPoint;

    QPoint prePlgpnt;//上一个多边形顶点
    vector<QPoint> v;//存多边形或Bezier曲线所有顶点
    vector<Point> vb;//贝塞尔曲线控制多边形
    QString filename;
    vector<Graphseq> graphseq;

public:
    void painterInit(QPainter *p);
    unsigned char encode(const QPoint& p, int minwx, int minwy, int maxwx, int maxwy);
    void DDADrawFoldLines(QPixmap *map, const vector<QPoint> &v);//DDA画折线（也就是不封边的多边形）
    void BREDrawFoldLines(QPixmap *map, const vector<QPoint> &v);//BRE画折线（也就是不封边的多边形）

    void DDADrawLine(QPixmap *map, int x0, int y0, int x1, int y1);//DDA算法画直线
    void BREDrawLine(QPixmap *map, int x0, int y0, int x1, int y1);//Bresenham算法画直线
    void BREDrawEllipse(QPixmap *map, long long int xc, long long int yc, long long int rx, long long int ry);//Bresenham算法画椭圆,参数依次为中心坐标和半轴长
    void DDADrawPolygon(QPixmap *map, const vector<QPoint> &v);
    void BREDrawPolygon(QPixmap *map, const vector<QPoint> &v);
    void ETFillPolygon(QPixmap *map, const vector<QPoint> &v);
    void AreaFill(QPixmap *map, const QPoint& seed);
    int Cohen_Sutherland(QPoint &p1, QPoint &p2,
                          const QPoint& w1, const QPoint& w2);
    int Liang_Barsky(QPoint &p1, QPoint& p2,
                             const QPoint& w1, const QPoint& w2);
    void BezierDrawCurve(QPixmap *map, vector<QPoint>& points);
    void BezierDrawCurve(QPixmap *map, vector<Point> & points);
private:
    void translate(QPoint &p, int dx, int dy);
    void translate(int &x, int &y, int dx, int dy);
    void translate(long long int &x, long long int &y, int dx, int dy);
    void translate(QPoint &p, qreal dx, qreal dy);
    void rotate(QPoint &p, const QPoint& center, qreal angle);
    void rotate(int &x, int &y, const QPoint& center, qreal angle);
signals:

public slots:
    void setpenWidth(int w);
    void changecolor();//弹出画笔颜色选择对话框
    void chooseLine();//用户选择要画线
    void chooseEllipse();//用户选择要画椭圆
    void choosePencil();//用户选择自由涂鸦
    void choosePolygon();//用户选择画多边形
    void chooseFillPolygon();//填充多边形
    void chooseFill();
    void chooseBezier();
    void setSurround(const QPoint&);
};

#endif // CANVAS_H
