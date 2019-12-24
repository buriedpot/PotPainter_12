#include "canvas.h"
#include "structure.h"
#include "graphstruct.h"
#include "functions.h"
#include "matrix.h"


Canvas::Canvas(QWidget *parent) : QWidget(parent)
{
    this->setAttribute(Qt::WA_StyledBackground,true);
    this->setStyleSheet("background-color: rgb(255,255, 255)");
    this->resize(1600,1200);
    setMouseTracking(1);
    /*Initialization of Painter parametres*/
    this->penColor = QColor(0,0,0);//初始化笔颜色为纯黑
    this->penWidth = 1;
    this->pencapStyle = Qt::RoundCap;
    this->background = Qt::white;
    this->kBSpline = 4;
    nCurve = 3;
    /*Drawing status init*/
    this->isDrawing = false;//初始时用户没有在画图
    this->isSaved = true;
    this->pixmapdrawDone = true;//原始画布绘制完成
    this->updateSuccess = true;
    this->isMouseMove = false;
    this->lineDrawed = 0;
    this->isPolygonDrawDone = true;
    this->isBezierDrawDone = true;
    bezierReadyDone = true;
    this->needtoUpdate = false;
    Transforming = -1;
    isTransforming = false;
    isClipped = true;
    AdjustNode = -1;
    clipagrthm = LIANG_BARSKY;
    /*Initialization of pixmap*/
    this->pixmap = QPixmap(size());
    this->pixmap.fill(Qt::white);
    this->transmap = pixmap;
    this->graphClass = PENCIL;
    this->filename = "";
    this->lineClass = BRELINE;
}
void Canvas::paintEvent(QPaintEvent *event) {
    int w,h;
    int sx = startPoint.x();
    int sy = startPoint.y();
    int ex = endPoint.x();
    int ey = endPoint.y();
    w = abs(ex - sx);
    h = abs(ey - sy);
    QPainter painter(this);
    painter.setPen(QPen(this->penColor));
    if (isDrawing && Transforming == -1) {
        QPixmap tempmap = pixmap;
        vector<Point> tmpv;
        if (this->graphClass == LINE) {
            if (this->lineClass == DDALINE)
                this->DDADrawLine(&tempmap, sx, sy, ex, ey);
            else
                this->BREDrawLine(&tempmap, sx, sy, ex, ey);
        }
        else if (this->graphClass == ELLIPSE) {
            //TODO 需要分类讨论
            this->BREDrawEllipse(&tempmap, (sx + ex) / 2, (sy + ey) / 2, w / 2, h / 2);
        }
        else if (this->graphClass == PENCIL) {
            this->BREDrawLine(&pixmap, lastendPoint.x(), lastendPoint.y(), endPoint.x(), endPoint.y());
            updateSuccess = true;

            //lastendPoint = endPoint;
        }
        else if (this->graphClass == POLYGON || this->graphClass == FILLPOLYGON) {
            if (lineDrawed == 0) {
                this->BREDrawLine(&tempmap, sx, sy, ex, ey);
            }
            else {
                this->BREDrawFoldLines(&tempmap, vb);
                this->BREDrawLine(&tempmap, prePlgpnt.x(), prePlgpnt.y(), ex, ey);
            }
        }
        else if (this->graphClass == BEZIERCURVE) {
            if (lineDrawed == 0) {
                this->BREDrawLine(&tempmap, sx, sy, ex, ey);
            }
            else {
                tmpv = vb;
                vector<Point>::iterator it = tmpv.begin() + lineDrawed;
                tmpv.insert(it, endPoint);
                this->BezierDrawCurve(&tempmap, tmpv);
            }
        }
        else if (this->graphClass == BSPLINECURVE) {
            tmpv = vb;
            tmpv.push_back(endPoint);
            this->BSplineDrawCurve(&tempmap, tmpv, kBSpline);
        }
        painter.drawPixmap(0,0,tempmap);
        if (graphClass == BEZIERCURVE || graphClass == BSPLINECURVE) {
            setPainterDotted(&painter, QColor(114,128,144), 1);
            if (tmpv.size() > 0)
                for (int i = 0; i < tmpv.size() - 1; ++i) {
                    painter.drawLine(tmpv[i].xp, tmpv[i].yp, tmpv[i+1].xp, tmpv[i+1].yp);
                }
        }
    }
    else {
        if (!pixmapdrawDone) { //Pixmap还未绘制结束
            if (this->graphClass == LINE) {
                emit setTranslateEnabled(1); emit setRotateEnabled(1); emit setScaleEnabled(1); emit setClipEnabled(1);
                x00 = sx; y00 = sy; ddx = ex - sx; ddy = ey - sy;
                setSurround2(sx,sy,ex,ey);
                transmap = pixmap;
                if (this->lineClass == DDALINE)
                    this->DDADrawLine(&pixmap, sx, sy, ex, ey);
                else
                    this->BREDrawLine(&pixmap, sx, sy, ex, ey);
            }
            else if (this->graphClass == ELLIPSE) {
                emit setTranslateEnabled(1); emit setScaleEnabled(1);
                x00 = sx; y00 = sy; ddx = ex - sx; ddy = ey - sy;
                setSurround2(sx,sy,ex,ey);
                transmap = pixmap;
                this->BREDrawEllipse(&pixmap, (sx + ex) / 2, (sy + ey) / 2, w / 2, h / 2);
            }
            else if (this->graphClass == PENCIL) {
                transmap = pixmap;
                this->BREDrawLine(&pixmap, lastendPoint.x(), lastendPoint.y(), endPoint.x(), endPoint.y());
                updateSuccess = true;
            }            
            else if (this->graphClass == POLYGON || this->graphClass == FILLPOLYGON) {
                QPixmap polygonmap = pixmap;
                if (!isPolygonDrawDone) {
                    if (endPoint == firstPoint){
                        setSurround(vb);
                        if (graphClass == POLYGON) {
                            if (this->lineClass == DDALINE) {
                                this->DDADrawPolygon(&polygonmap, vb);
                            }
                            else {
                                this->BREDrawPolygon(&polygonmap, vb);
                            }
                            emit setTranslateEnabled(1); emit setRotateEnabled(1); emit setScaleEnabled(1); emit setAdjustEnabled(1); emit setClipEnabled(1);
                            transmap = pixmap;
                            transpoints = vb;
                            pixmap = polygonmap;
                            prePlgpnt = endPoint;//多边形已作出的上一个顶点赋值为目前的顶点
                            vb.clear();
                            isPolygonDrawDone = true;
                            lineDrawed = 0;
                        }
                        else {
                            //qDebug() << "end = first" << endl;
                            ETFillPolygon(&polygonmap, vb);
                            emit setTranslateEnabled(1); emit setRotateEnabled(1); emit setScaleEnabled(1);emit setAdjustEnabled(1); emit setClipEnabled(1);
                            transpoints = vb;
                            transmap = pixmap;
                            pixmap = polygonmap;
                            vb.clear();
                            prePlgpnt = endPoint;//多边形已作出的上一个顶点赋值为目前的顶点
                            isPolygonDrawDone = true;
                            lineDrawed = 0;
                        }
                    }
                    else {
                        if (this->lineClass == DDALINE) {
                            this->DDADrawFoldLines(&polygonmap, vb);
                            //this->DDADrawLine(&pixmap, prePlgpnt.x(), prePlgpnt.y(), ex, ey);
                        }
                        else {
                            this->BREDrawFoldLines(&polygonmap, vb);
                            //this->DDADrawLine(&pixmap, prePlgpnt.x(), prePlgpnt.y(), ex, ey);
                        }
                        //v.push_back(endPoint);
                        prePlgpnt = endPoint;//多边形已作出的上一个顶点赋值为目前的顶点
                        ++lineDrawed;
                    }
                }
                pixmapdrawDone = true;
                isSaved = false;
                painter.drawPixmap(0, 0, polygonmap);
                return;
            }
            else if (graphClass == BEZIERCURVE) {
                QPixmap beziermap = pixmap;
                if (!isBezierDrawDone) {
                    if (bezierReadyDone){
                        BezierDrawCurve(&beziermap, vb);
                        emit setTranslateEnabled(1); emit setRotateEnabled(1); emit setScaleEnabled(1);emit setAdjustEnabled(1);
                        transpoints = vb;
                        setSurround(vb);
                        transmap = pixmap;
                        pixmap = beziermap;
                        vb.clear();
                        isBezierDrawDone = true;
                        lineDrawed = 0;
                    }
                    else {
                        BezierDrawCurve(&beziermap, vb);
                        ++lineDrawed;
                    }
                }
                pixmapdrawDone = true;
                isSaved = false;
                painter.drawPixmap(0, 0, beziermap);
                return;
            }
            else if (graphClass == BSPLINECURVE) {
                QPixmap beziermap = pixmap;
                if (!isBezierDrawDone) {
                    BSplineDrawCurve(&beziermap, vb, kBSpline);
                    ++lineDrawed;
                    if (bezierReadyDone){
                        emit setTranslateEnabled(1); emit setRotateEnabled(1); emit setScaleEnabled(1);emit setAdjustEnabled(1);
                        transpoints = vb;
                        setSurround(vb);
                        transmap = pixmap;
                        pixmap = beziermap;
                        vb.clear();
                        isBezierDrawDone = true;
                        lineDrawed = 0;
                    }
                }
                pixmapdrawDone = true;
                isSaved = false;
                painter.drawPixmap(0, 0, beziermap);
                setPainterDotted(&painter, QColor(114,128,144), 1);
                if (vb.size() > 0)
                    for (int i = 0; i < vb.size() - 1; ++i) {
                        painter.drawLine(vb[i].xp, vb[i].yp, vb[i+1].xp, vb[i+1].yp);
                    }
                return;
            }
            pixmapdrawDone = true;
            isSaved = false;
            painter.drawPixmap(0,0,pixmap);
        }
        if (Transforming != -1) {
            if (Transforming == TRANSLATE) {
                pixmap = transmap;
                if (isTransforming == true){
                    if (graphClass == LINE || graphClass == ELLIPSE) {
                        x00 = endPoint.x() - startPoint.x() + x00; y00 = endPoint.y() - startPoint.y() + y00;
                        setSurround2(x00,y00,x00+ddx,y00+ddy);
                    }
                    else if (graphClass == BEZIERCURVE || graphClass == BSPLINECURVE || graphClass == POLYGON ||
                             graphClass == FILLPOLYGON) {
                        for (int i = 0; i < transpoints.size(); ++i) {
                            transpoints[i].xp += endPoint.x() - startPoint.x();
                            transpoints[i].yp += endPoint.y() - startPoint.y();
                        }
                        setSurround(transpoints);
                    }
                }
                if (graphClass == LINE) {
                    if (lineClass == DDALINE)
                        this->DDADrawLine(&pixmap, x00, y00, x00 + ddx, y00 + ddy);
                    else
                        this->BREDrawLine(&pixmap, x00, y00, x00 + ddx, y00 + ddy);
                }
                else if (graphClass == ELLIPSE) {
                    this->BREDrawEllipse(&pixmap, (endPoint.x() - startPoint.x() + x00) + ddx / 2, endPoint.y() - startPoint.y() + y00 +ddy/2,
                                         abs(ddx / 2), abs(ddy / 2));
                }
                else if (graphClass == BEZIERCURVE) {
                    BezierDrawCurve(&pixmap, transpoints);
                }
                else if (graphClass == BSPLINECURVE) {
                    BSplineDrawCurve(&pixmap, transpoints, kBSpline);
                }
                else if (graphClass == POLYGON || graphClass == FILLPOLYGON) {
                    if (graphClass == POLYGON) {
                        if (lineClass == DDALINE) DDADrawPolygon(&pixmap, transpoints);
                        else BREDrawPolygon(&pixmap, transpoints);
                    }
                    else {
                        ETFillPolygon(&pixmap, transpoints);
                    }
                }
                startPoint = endPoint;
            }
            else if (Transforming == ROTATE) {
                QPixmap originmap = pixmap;
                pixmap = transmap;
                if (graphClass == LINE) {
                    setSurround2(x00,y00,x00+ddx,y00+ddy);
                    if (lineClass == DDALINE)
                        this->DDADrawLine(&pixmap, x00, y00,
                                      x00 + ddx, y00 + ddy);
                    else
                        this->BREDrawLine(&pixmap, x00, y00,
                                          x00 + ddx, y00 + ddy);
                }
                else if (graphClass == POLYGON) {
                    setSurround(transpoints);
                    if (lineClass == DDALINE)
                        this->DDADrawPolygon(&pixmap, transpoints);
                    else
                        this->BREDrawPolygon(&pixmap, transpoints);
                }
                else if (graphClass == FILLPOLYGON){
                    setSurround(transpoints);
                    ETFillPolygon(&pixmap, transpoints);
                }
                else if (graphClass == BEZIERCURVE){
                    setSurround(transpoints);
                    BezierDrawCurve(&pixmap, transpoints);
                }
                else if (graphClass == BSPLINECURVE){
                    setSurround(transpoints);
                    BSplineDrawCurve(&pixmap, transpoints, kBSpline);
                }
                else pixmap = originmap;
            }
            else if (Transforming == SCALE) {
                QPixmap originmap = pixmap;
                pixmap = transmap;
                if (graphClass == LINE) {
                    setSurround2(x00,y00,x00+ddx,y00+ddy);
                    if (lineClass == DDALINE)
                        this->DDADrawLine(&pixmap, x00, y00,
                                      x00 + ddx, y00 + ddy);
                    else
                        this->BREDrawLine(&pixmap, x00, y00,
                                          x00 + ddx, y00 + ddy);
                }
                else if (graphClass == ELLIPSE) {
                    setSurround2(x00,y00,x00+ddx,y00+ddy);
                    this->BREDrawEllipse(&pixmap, x00 + ddx / 2, y00 +ddy/2,
                                         abs(ddx / 2), abs(ddy / 2));
                }
                else if (graphClass == POLYGON) {
                    setSurround(transpoints);
                    if (lineClass == DDALINE)
                        this->DDADrawPolygon(&pixmap, transpoints);
                    else
                        this->BREDrawPolygon(&pixmap, transpoints);
                }
                else if (graphClass == FILLPOLYGON){
                    setSurround(transpoints);
                    ETFillPolygon(&pixmap, transpoints);
                }
                else if (graphClass == BEZIERCURVE){
                    setSurround(transpoints);
                    BezierDrawCurve(&pixmap, transpoints);
                }
                else if (graphClass == BSPLINECURVE){
                    setSurround(transpoints);
                    BSplineDrawCurve(&pixmap, transpoints, kBSpline);
                }
                else pixmap = originmap;
            }
            else if (Transforming == CLIP) {
                QPixmap originmap = pixmap;
                pixmap = transmap;
                if (graphClass == LINE && isClipped == false) {
                    QPoint p1(x00, y00), p2(x00 + ddx, y00 + ddy);
                    int result = 0;
                    if (clipagrthm == LIANG_BARSKY)
                        result = Liang_Barsky(p1, p2, startPoint, endPoint);
                    else
                        result = Cohen_Sutherland(p1, p2, startPoint, endPoint);
                    if (result == 1) {
                        x00 = p1.x(); y00 = p1.y(); ddx = p2.x() - x00; ddy = p2.y() - y00;
                    }
                    else {
                        x00 = -1; y00 = -1; ddx = 0; ddy = 0;
                    }
                    setSurround2(x00, y00, x00 + ddx, y00 + ddy);
                    BREDrawLine(&pixmap, x00, y00, x00 + ddx, y00 + ddy);
                    isClipped = true;
                }
                else if ((graphClass == POLYGON || graphClass == FILLPOLYGON) && isClipped == false) {
                    Sutherland_Hodgeman(transpoints, startPoint, endPoint);
                    setSurround(transpoints);
                    if (graphClass == POLYGON) {
                        if (lineClass == DDALINE) {
                            DDADrawPolygon(&pixmap, transpoints);
                        }
                        else BREDrawPolygon(&pixmap, transpoints);
                    }
                    else {
                        ETFillPolygon(&pixmap, transpoints);
                    }
                    isClipped = true;
                }
                else pixmap = originmap;
            }
            else if (Transforming == ADJUST) {
                pixmap = transmap;
                if (isTransforming == true){
                    if (graphClass == BEZIERCURVE || graphClass == BSPLINECURVE || graphClass == POLYGON ||
                            graphClass == FILLPOLYGON) {
                        transpoints[AdjustNode].xp = endPoint.x();
                        transpoints[AdjustNode].yp = endPoint.y();
                        setSurround(transpoints);
                    }
                }
                if (graphClass == BEZIERCURVE) {
                    BezierDrawCurve(&pixmap, transpoints);
                }
                else if (graphClass == BSPLINECURVE) {
                    BSplineDrawCurve(&pixmap, transpoints, kBSpline);
                }
                else if (graphClass == POLYGON || graphClass == FILLPOLYGON) {
                    if (graphClass == POLYGON) {
                        if (lineClass == DDALINE) DDADrawPolygon(&pixmap, transpoints);
                        else BREDrawPolygon(&pixmap, transpoints);
                    }
                    else {
                        ETFillPolygon(&pixmap, transpoints);
                    }
                }
            }
            painter.drawPixmap(0,0,pixmap);
        }else painter.drawPixmap(0,0,pixmap);
        if (Transforming == ROTATE || Transforming == SCALE) {
            QPen pen;
            pen.setColor(Qt::blue);
            pen.setWidth(5);
            pen.setCapStyle(Qt::RoundCap);
            painter.setPen(pen);
            painter.drawEllipse(endPoint, 10, 10);
        }
        if (Transforming == TRANSLATE || Transforming == SCALE) {
            setPainterDotted(&painter, Qt::blue, 2);
            painter.drawRect(xl, yd, xr - xl, yu - yd);
        }
        if (Transforming == CLIP) {
            setPainterDotted(&painter, Qt::red, 2);
            painter.drawRect(min(sx,ex), min(sy,ey), abs(ex - sx),abs(ey - sy));
        }
        if (Transforming == ADJUST) {
            setPainterDotted(&painter, QColor(25, 25, 112), 2);
            if (transpoints.size() > 0){
                int i, length = transpoints.size();
                if (graphClass == POLYGON || graphClass == FILLPOLYGON)
                    for (i = 0; i < transpoints.size(); ++i) {
                        painter.drawEllipse(transpoints[i].xp - 10, transpoints[i].yp - 10, 20, 20);
                    }
                else if (graphClass == BEZIERCURVE || graphClass == BSPLINECURVE) {
                    for (i = 0; i < transpoints.size() - 1; ++i) {
                        painter.drawLine(transpoints[i].xp, transpoints[i].yp, transpoints[(i+1)%length].xp, transpoints[(i+1)%length].yp);
                        painter.drawEllipse(transpoints[i].xp - 10, transpoints[i].yp - 10, 20, 20);
                        painter.drawStaticText(transpoints[i].xp, transpoints[i].yp, QStaticText(QString("P") + QString("%1").arg(i)));
                    }
                    painter.drawEllipse(transpoints[i].xp - 10, transpoints[i].yp - 10, 20, 20);
                    painter.drawStaticText(transpoints[i].xp, transpoints[i].yp, QStaticText(QString("P") + QString("%1").arg(i)));
                }
            }
        }
    }
}
void Canvas::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton){
        startPoint=event->pos();
        endPoint = startPoint;
        if (this->graphClass == FILL) {
            this->AreaFill(&pixmap, this->startPoint);
        }
        qDebug() << "before enabled";
        if (Transforming != -1) {
            qDebug() << "no enabled";
            AdjustNode = (getCurveNode(event->pos()));
            if((event->pos().x() >= xl && event->pos().x() <= xr && event->pos().y() >= yd && event->pos().y() <= yu) ||
                    AdjustNode != -1) {
                isTransforming = true;
            }
            update();
            return;
        }
        else {qDebug() << "should enabled";emit setTranslateEnabled(0); emit setRotateEnabled(0); emit setScaleEnabled(0); emit setClipEnabled(0); emit setAdjustEnabled(0);}
        qDebug() << "after enabled" << endl;
        lastendPoint = startPoint;

        isDrawing = true;
        pixmapdrawDone = false;
        if (this->graphClass == POLYGON || this->graphClass == FILLPOLYGON) {
            if (lineDrawed == 0) {
                firstPoint = startPoint;
                prePlgpnt = startPoint;
                vb.push_back(startPoint);
                isPolygonDrawDone = false;
            }
            isMouseMove = false;
        }
        else if (this->graphClass == BEZIERCURVE || this->graphClass == BSPLINECURVE) {
            if (lineDrawed == 0) {
                //firstPoint = startPoint;
                bezierReadyDone = false;
                vb.push_back(startPoint);
                isBezierDrawDone = false;
            }
            isMouseMove = false;
        }
        update();
    }
}

void Canvas::mouseMoveEvent(QMouseEvent *event) {
    if (this->graphClass == FILL) return;
    if (Transforming == TRANSLATE) {
        if (event->pos().x() >= xl && event->pos().x() <= xr && event->pos().y() >= yd && event->pos().y() <= yu) {
            setCursor(Qt::SizeAllCursor);
        }
        else setCursor(Qt::ArrowCursor);
    }
    if (Transforming == ADJUST) {
        if (getCurveNode(event->pos()) != -1) {
            setCursor(Qt::SizeAllCursor);
        }
        else setCursor(Qt::ArrowCursor);
    }
    if (event->buttons() & Qt::LeftButton) {
        if (updateSuccess == true) {
            lastendPoint = endPoint;
            updateSuccess = false;
        }
        endPoint = event->pos();
        update();
    }
}

void Canvas::mouseReleaseEvent(QMouseEvent *event) {
    //qDebug() << lineDrawed;
    if (this->graphClass == FILL) return;
    if (event->button() == Qt::LeftButton) {
        endPoint = event->pos();
        if (Transforming != -1) {
            if (Transforming == CLIP) isClipped = false;
            isTransforming = false;
            AdjustNode = -1;
            update();
            return;
        }
        isDrawing = false;
        if (this->graphClass == POLYGON || this->graphClass == FILLPOLYGON) {
        /*如果下一点十分接近多边形起始点，则认为多边形封边*/
            if (square(endPoint.x() - firstPoint.x()) + square(endPoint.y() - firstPoint.y()) <= 400) {
                endPoint = firstPoint;
            }
            else {
                vb.push_back(endPoint);
            }
            //prePlgpnt = endPoint;
        }
        else if (graphClass == BEZIERCURVE) {
            if (lineDrawed >= nCurve - 1) {
                vector<Point>::iterator it = vb.begin() + lineDrawed;
                vb.insert(it, endPoint);
                bezierReadyDone = true;
            }
            else {
                if (lineDrawed == 0)
                    vb.push_back(endPoint);
                else {
                    vector<Point>::iterator it = vb.begin() + lineDrawed;
                    vb.insert(it, endPoint);
                }
            }
        }
        else if (graphClass == BSPLINECURVE) {
            if (lineDrawed >= nCurve - 1) {
                vb.push_back(endPoint);
                bezierReadyDone = true;
            }
            else {
                vb.push_back(endPoint);
            }
        }
        update();
    }
}

void Canvas::wheelEvent(QWheelEvent *event) {
    if (Transforming == ROTATE) {
        if (graphClass == LINE) {
            double x11 = x00 + ddx, y11 = y00 + ddy;
            rotate(x00, y00, endPoint, -event->delta() / 6.0);
            rotate(x11, y11, endPoint, -event->delta() / 6.0);
            ddx = x11 - x00; ddy = y11 - y00;
        }
        else if (graphClass == POLYGON || graphClass == FILLPOLYGON || graphClass == BEZIERCURVE || graphClass == BSPLINECURVE) {
            for (int i = 0; i < transpoints.size(); ++i) {
                rotate(transpoints[i].xp, transpoints[i].yp, endPoint, -event->delta() / 6.0);
            }
        }
        else return;
        update();
    }
    else if (Transforming == SCALE) {
        double angle = abs(event->delta());
        if (graphClass == LINE || graphClass == ELLIPSE) {
            QPoint center2(0, 0);
            if (event->delta() > 0) {
                scale(x00, y00, endPoint, 1 + angle / 630);
                scale(ddx, ddy, center2, 1 + angle / 630);
            }
            else {
                scale(x00, y00, endPoint, 1 - angle / 630);
                scale(ddx, ddy, center2, 1 - angle / 630);
            }
        }
        else if (graphClass == POLYGON || graphClass == FILLPOLYGON || graphClass == BEZIERCURVE || graphClass == BSPLINECURVE) {
            if (event->delta() > 0) {
                for (int i = 0; i < transpoints.size(); ++i) {
                    scale(transpoints[i].xp, transpoints[i].yp, endPoint, 1 + angle / 630);
                }
            }
            else {
                for (int i = 0; i < transpoints.size(); ++i) {
                    scale(transpoints[i].xp, transpoints[i].yp, endPoint, 1 - angle / 630);
                }
            }
        }
        else return;
        update();
    }
}

void Canvas::enterEvent(QEvent *event) {
    /*if (Transforming == TRANSLATE) {
        setCursor(Qt::SizeAllCursor);
    }*/
    if (Transforming == ROTATE) setCursor(QCursor(QPixmap(":/icons/transicon/rotate").scaled(40,40), -1, -1));
    else if (Transforming == SCALE) setCursor(QCursor(QPixmap(":/icons/transicon/scale").scaled(40,40), -1, -1));
    else if (Transforming == CLIP) setCursor(QCursor(QPixmap(":/icons/transicon/cut").scaled(40,40), -1, -1));
    else if (graphClass == FILL) setCursor(QCursor(QPixmap(":/icons/toolsicon/fillcursor").scaled(40,40), -1, -1));
    else setCursor(Qt::CrossCursor);
}

void Canvas::leaveEvent(QEvent *event) {
    setCursor(Qt::ArrowCursor);
}

void Canvas::painterInit(QPainter *p) {
    QPen pen;
    pen.setColor(penColor);
    pen.setWidth(penWidth);
    pen.setCapStyle(pencapStyle);
    p->setPen(pen);
}

unsigned char Canvas::encode(const QPoint &p, int minwx, int minwy, int maxwx, int maxwy) {
    int px = p.x(), py = p.y();
    unsigned char result = 0;
    if (px < minwx) result |= 1;
    if (px > maxwx) result |= 2;
    if (py < minwy) result |= 4;
    if (py > maxwy) result |= 8;
    return result;
}

void Canvas::setpenWidth(int w) {
    this->penWidth = w;
}
void Canvas::setnCurve(int n) {
    this->nCurve = n - 1;//canvas的nCurve指的是边数
}
void Canvas::setkBSpline(int k) {
    this->kBSpline = k;
}
void Canvas::DDADrawFoldLines(QPixmap *map, const vector<QPoint> &v) {
    int pointnum = v.size();
    for (int i = 0; i < pointnum - 1; ++i) {
        DDADrawLine(map, v[i].x(), v[i].y(), v[i + 1].x(),
                v[i + 1].y());
    }
}
void Canvas::DDADrawFoldLines(QPixmap *map, const vector<Point> &v) {
    int pointnum = v.size();
    for (int i = 0; i < pointnum - 1; ++i) {
        DDADrawLine(map, v[i].xp, v[i].yp, v[i + 1].xp,
                v[i + 1].yp);
    }
}

void Canvas::BREDrawFoldLines(QPixmap *map, const vector<QPoint> &v) {
    int pointnum = v.size();
    for (int i = 0; i < pointnum - 1; ++i) {
        BREDrawLine(map, v[i].x(), v[i].y(), v[i + 1].x(),
                v[i + 1].y());
    }
}
void Canvas::BREDrawFoldLines(QPixmap *map, const vector<Point> &v) {
    int pointnum = v.size();
    for (int i = 0; i < pointnum - 1; ++i) {
        BREDrawLine(map, v[i].xp, v[i].yp, v[i + 1].xp,
                v[i + 1].yp);
    }
}
/*DDA画直线段*/
void Canvas::DDADrawLine(QPixmap *map, int x0, int y0, int x1, int y1) {
    QPainter painter(map);
    painterInit(&painter);
    double x = x0, y = y0, dx, dy, m, m_;//m_是斜率倒数,，dxdy是未来要使用的坐标间隔
    int bigger = std::max(abs(x1 - x0), abs(y1 - y0));


    if (x0 == x1 && y0 == y1) {
        painter.drawPoint(x0, y0);
        return;
    }

    /*斜率绝对值在0到1之间*/
    else if (abs(y1 - y0) <= abs(x1 - x0)) {
        m = (double)(y1 - y0) / (double)(x1 - x0);//斜率
        if (x0 <= x1) {
            dx = 1;
            dy = m;
        }
        else {
            dx = -1;
            dy = -m;
        }
    }
    else {
        m_ = (double)(x1 - x0) / (double)(y1 - y0);//斜率倒数
        if (y0 <= y1) {
            dx = m_;
            dy = 1;
        }
        else {
            dx = -m_;
            dy = -1;
        }
    }

    for (int i = 1; i <= bigger; i++)
    {
        painter.drawPoint((int)(x + 0.5),(int)(y+0.5));
        x = x + dx;
        y = y + dy;
    }
}
/*Bresenham画直线段*/
void Canvas::BREDrawLine(QPixmap *map, int x0, int y0, int x1, int y1) {
    QPainter painter(map);
    painterInit(&painter);
    int dx, dy;//dxdy是未来要使用的坐标间隔
    bool fk = false;//fk判断斜率绝对值是否大于1

    int deltax = abs(x1 - x0), deltay = abs(y1 - y0);//两点的坐标差

    if (x0 == x1 && y0 == y1) {
        painter.drawPoint(x0, y0);
        return;
    }

    if (deltax < deltay) {
        std::swap(x0, y0);
        std::swap(x1, y1);
        std::swap(deltax, deltay);
        fk = 1;
    }

    int ph = 2 * (deltay - deltax);
    int pl = 2 * deltay;

    int pk = 2 * deltay - deltax;
    if ((x1 - x0) > 0) dx = 1;
    else dx = -1;
    if ((y1 - y0) > 0) dy = 1;
    else dy = -1;

    int x = x0, y = y0;
    int *xx = &x, *yy = &y;
    if (fk == 1) {xx = &y; yy = &x;}
    painter.drawPoint(*xx, *yy);

    while (x != x1) {
        if (pk < 0) {
            pk += pl;
        }
        else {
            pk += ph;
            y += dy;
        }
        x += dx;
        painter.drawPoint(*xx, *yy);
    }
}
/*Bresenham中点思想画椭圆*/
void Canvas::BREDrawEllipse(QPixmap *map, long long int xc, long long int yc, long long int rx, long long int ry) {
    QPainter painter(map);
    painterInit(&painter);
    painter.drawPoint(xc, ry + yc);
    painter.drawPoint(xc, -ry + yc);

    long long int ry2 = ry * ry, rx2= rx * rx;
    double p1k = ry * ry - rx * rx * ry + rx * rx / 4.0;

    long long int xk = 0, yk = ry;

    long long int p1l, p1h, p2l, p2h;

    long long int dx = 1, dy = -1;

    p1l = 2 * ry2 * xk + 3 * ry2;
    p1h = 2 * ry2 * xk - 2 * rx2 * yk + 2 * rx2 + 3 * ry2;


    while (ry2 * xk < rx2 * yk) {
        if (p1k < 0) {
            p1k += p1l;
        }
        else {
            p1k += p1h;
            yk += dy;
        }
        xk += dx;
        painter.drawPoint(xk + xc, yk + yc);
        painter.drawPoint(-xk + xc, yk + yc);
        painter.drawPoint(xk + xc, -yk + yc);
        painter.drawPoint(-xk + xc, -yk + yc);
        p1l = 2 * ry2 * xk + 3 * ry2;
        p1h = 2 * ry2 * xk - 2 * rx2 * yk + 2 * rx2 + 3 * ry2;
    }

    double p2k = ry2 * (xk + 0.5) * (xk + 0.5) + rx2 *(yk - 1) * (yk - 1) - rx2 * ry2;

    p2l = -2 * rx2 * yk + 3 * rx * rx;
    p2h = 2 * ry2 * xk - 2 * rx2 * yk + 2 * ry2 + 3 * rx2;

    while (yk != 0) {
        if (p2k <= 0) {
            p2k += (double)p2h;
            xk += dx;
        }
        else {
            p2k += (double)p2l;

        }
        yk += dy;
        painter.drawPoint(xk + xc, yk + yc);
        painter.drawPoint(-xk + xc, yk + yc);
        painter.drawPoint(xk + xc, -yk + yc);
        painter.drawPoint(-xk + xc, -yk + yc);
        p2l = - 2 * rx2 * yk + 3 * rx2;
        p2h = 2 * ry2 * xk - 2 * rx2 * yk + 2 * ry2 + 3 * rx2;
    }
}
/*多边形绘制DDA*/
void Canvas::DDADrawPolygon(QPixmap *map, const vector<QPoint> &v) {
    int pointnum = v.size();
    for (int i = 0; i < pointnum; ++i) {
        DDADrawLine(map, v[i].x(), v[i].y(), v[(i + 1) % pointnum].x(),
                v[(i + 1) % pointnum].y());
    }
}
void Canvas::DDADrawPolygon(QPixmap *map, const vector<Point> &v) {
    int pointnum = v.size();
    for (int i = 0; i < pointnum; ++i) {
        DDADrawLine(map, (int)v[i].xp, (int)v[i].yp, (int)v[(i + 1) % pointnum].xp, \
                (int)v[(i + 1) % pointnum].yp);
    }
}
/*多边形绘制Bresenham*/
void Canvas::BREDrawPolygon(QPixmap *map, const vector<QPoint> &v) {
    int pointnum = v.size();
    for (int i = 0; i < pointnum; ++i) {
        BREDrawLine(map, v[i].x(), v[i].y(), v[(i + 1) % pointnum].x(), \
                v[(i + 1) % pointnum].y());
    }
}
void Canvas::BREDrawPolygon(QPixmap *map, const vector<Point> &v) {
    int pointnum = v.size();
    for (int i = 0; i < pointnum; ++i) {
        BREDrawLine(map, (int)v[i].xp, (int)v[i].yp, (int)v[(i + 1) % pointnum].xp, \
                (int)v[(i + 1) % pointnum].yp);

    }
}

/*多边形扫描转换填充*/
void Canvas::ETFillPolygon(QPixmap *map, const vector<QPoint> &v) {
    if (v.size() <= 0) return;
    QPainter painter(map);
    QPen pen;
    pen.setWidth(1);
    pen.setColor(this->penColor);
    pen.setCapStyle(this->pencapStyle);
    painter.setPen(pen);
    int pointnum = v.size();//Numbers of points of Polygon
    int ymax = -1, ymin = 2147483647;
    /*循环遍历顶点，找到最低扫描线和最高扫描线*/
    for (unsigned int i = 0; i < v.size(); ++i) {
        if (v[i].y() > ymax) ymax = v[i].y();
        if (v[i].y() < ymin) ymin = v[i].y();
    }
    vector<vector<ETunit>> NET;//建立有序边表
    NET.resize(ymax - ymin + 1);//ymax-ymin+1条扫描线
    /*create NET*/
    for (int i = 0; i < v.size(); ++i) {
        QPoint front = v[(i - 1 + pointnum) % pointnum], current = v[i], next = v[(i + 1) % pointnum]\
                , nenext = v[(i + 2) % pointnum];//前一个，当前的点，下一个点，下下个点。
        /*
         * 要处理的是当前点和下一个点连接而成的线段
         * 这里缩边缩的都是下面的线段
         */

        ETunit tmp;
        //当前点比较高，是一个从左下到右上的线段
        if (next.y() > current.y()) {
            tmp.x = current.x();//设置最低点x值
            tmp.ymax = next.y();//最高点y值
            tmp.k_1 = (double)(next.x() - current.x()) //求k的倒数
                    / (next.y() - current.y());
            if (nenext.y() > next.y()) {
                --tmp.ymax;//缩边
            }
            NET[current.y() - ymin].push_back(tmp);
        }
        //当前点比较低，是一个从左上到右下的线段
        else if (next.y() < current.y()) {
            tmp.x = next.x();
            tmp.ymax = current.y();
            tmp.k_1 = (double)(next.x() - current.x())
                    / (next.y() - current.y());
            if (front.y() > current.y()) {//因为缩边缩的都是较低的线段，所以与前面的去比，不与后面的比。反之就要与后边的比
                --tmp.ymax;
            }
            NET[next.y() - ymin].push_back(tmp);
        }
    }
    //Debug,看看NET对不对
    /*对新边表排个序*/
    /*create AET*/
    vector<ETunit> AET;
    for (int i = 0; i < ymax - ymin + 1; ++i) {
        if (!NET[i].empty()) {
            for (int j = 0; j < NET[i].size(); ++j) {
                AET.push_back(NET[i][j]);
            }
        }
        sort(AET.begin(), AET.end(), ETcmpl);
        /*扫描线填充*/
        if (AET.size() % 2 == 1) {
            qDebug() << "Why" << i << endl;
            return;
        }

        for (int j = 0; j < AET.size(); j += 2) {
            double x1 = AET[j].x, x2 = AET[j + 1].x;
            for (int k = x1; k <= x2; ++k) {
                painter.drawPoint(k, i + ymin);
            }
        }
        /*delete inactive edges*/
        for (int j = 0; j < AET.size(); ++j) {
            if (AET[j].ymax <= i + ymin) {
                vector<ETunit>::iterator it = AET.begin() + j;
                AET.erase(it);
                --j;//关键的一步，bug找了半天原来在这
            }
        }
        /*update x*/
        for (int j = 0; j < AET.size(); ++j) {
            AET[j].x += AET[j].k_1;
        }
    }
}
void Canvas::ETFillPolygon(QPixmap *map, const vector<Point> &v) {
    vector<QPoint> vtmp(v.size());
    for (int i = 0; i < vtmp.size(); ++i) {
        vtmp[i].setX((int)v[i].xp);
        vtmp[i].setY((int)v[i].yp);
    }
    ETFillPolygon(map, vtmp);
}

/*区域点击填充*/
void Canvas::AreaFill(QPixmap *map, const QPoint& seed) {
    QImage img = pixmap.toImage();
    QRgb sdcolor = img.pixel(seed);
    if (QColor(sdcolor) == this->penColor) return;
    QPainter painter(&img);
    QPen pen;
    pen.setWidth(1);
    pen.setColor(this->penColor);
    pen.setCapStyle(this->pencapStyle);
    painter.setPen(pen);
    //qDebug() << "SA" << seed.x()<<',' << seed.y();

    stack<QPoint> st;
    st.push(seed);
    while (!st.empty()) {
        QPoint p = st.top();
        st.pop();
        int x = p.x(), y = p.y();
        int tmpx;
        for (tmpx = x - 1; tmpx >= 0 && img.pixel(tmpx, y) == sdcolor; --tmpx) {
            painter.drawPoint(tmpx, y);
            //qDebug() << p.x() << ",,,"<< p.y();
        }
        int xl = tmpx + 1;
        for (tmpx = x; tmpx <= img.width() - 1 && img.pixel(tmpx, y) == sdcolor; ++tmpx) {
            painter.drawPoint(tmpx, y);
        }
        int xr = tmpx - 1;
        tmpx = xl;
        y = y + 1;
        bool spanNeedFill;
        if (y <= img.height() - 1) {
            while(tmpx <= xr)
            {
                spanNeedFill=false;
                while(tmpx < img.width() && img.pixel(tmpx, y) == sdcolor)
                {

                    spanNeedFill=true;
                    tmpx++;
                }
                if (tmpx == img.width() - 1) break;
                if(spanNeedFill)
                {
                    st.push(QPoint(tmpx - 1, y));
                    spanNeedFill=false;
                }
                while(tmpx<=xr && img.pixel(tmpx, y) != sdcolor)
                    tmpx++;
            }
        }
        /*if (y <= img.height() - 1) {
            while (img.pixel(tmpx, y) != sdcolor && tmpx <= xr) {
                ++tmpx;
            }
            while (tmpx <= xr && img.pixel(tmpx, y) == sdcolor) {
                if (img.pixel(tmpx, y) == sdcolor) {
                    st.push(QPoint(tmpx, y));
                    break;
                }
                ++tmpx;
            }
        }*/
        tmpx = xl;
        y = y - 2;
        if (y >= 0) {
            while(tmpx<=xr)
            {
                spanNeedFill=false;
                while(tmpx < img.width() && img.pixel(tmpx, y) == sdcolor)
                {
                    spanNeedFill=true;
                    tmpx++;
                }
                if (tmpx == img.width() - 1) break;
                if(spanNeedFill)
                {
                    st.push(QPoint(tmpx - 1, y));
                    spanNeedFill=false;
                }
                while(tmpx<=xr && img.pixel(tmpx, y) != sdcolor)
                    tmpx++;
            }
        }
        /*if (y >= 0) {
            while (img.pixel(tmpx, y) != sdcolor && tmpx <= xr) {
                ++tmpx;
            }
            while (tmpx <= xr && img.pixel(tmpx, y) == sdcolor) {
                if (img.pixel(tmpx, y) == sdcolor) {
                    st.push(QPoint(tmpx, y));
                    break;
                }
                ++tmpx;
            }
        }*/

    }
    QRectF target(0, 0, pixmap.width(), pixmap.height()); //建立目标矩形
    QRectF source(0.0, 0.0, img.width(), img.height()); //建立源矩形，用来框定源图像文件中要显示的部分

    QPainter p(map);
    p.drawImage(target, img, source);
}
/*B样条基函数递归定义，其中k为阶数阶数阶数！不是多项式次数！！！*/
double Canvas::N(int i, int k, double u) {
    if (k == 1) {
        double t = u - i;
        if (0 <= t && t < 1) {
            return 1;
        }
        else return 0;
    }
    else {
        return (u - i) / (k - 1) * N(i, k - 1, u) + (i + k - u) / (k - 1) * N(i + 1, k - 1, u);
    }
}
void Canvas::BSplineDrawCurve(QPixmap *map, vector<Point> &v, int k){
    vector<Point> curve;
    //qDebug() << "fuck" << v.size();
    double du = 1 / 512.0;
    for(double u = (double)k - 1; u < v.size(); u += du) {
        //qDebug() << "fuck";
        Point tmp(0, 0);
        for(int i = 0; i < v.size(); ++i) {
            Point t = v[i];
            t *= N(i, k, u);
            tmp += t;
        }
        curve.push_back(tmp);
    }
    if (curve.size() > 0)
        for (int i = 0; i < curve.size() - 1; ++i) {
            BREDrawLine(map, curve[i].xp, curve[i].yp, curve[i + 1].xp, curve[i + 1].yp);
        }
}
/*平移变换*/
void Canvas::translate(QPoint &p, int dx, int dy) {
    p.setX(p.x() + dx);
    p.setY(p.y() + dy);
}
void Canvas::translate(Point &p, int dx, int dy) {
    p.xp += dx;
    p.yp += dy;
}
void Canvas::translate(int &x, int &y, int dx, int dy) {
    x += dx;
    y += dy;
}
void Canvas::translate(long long int &x, long long int &y, int dx, int dy) {
    x += dx;
    y += dy;
}
/*平移变换矩阵方法*/
void Canvas::translate(QPoint &p, qreal dx, qreal dy) {
    Matrix location(1, 3);
    location.setValue(0, 0, p.x());
    location.setValue(0, 1, p.y());
    location.setValue(0, 2, 1);
    Matrix trans(3, 3);
    trans.setValue(0, 0, 1);trans.setValue(1, 1, 1);trans.setValue(2, 2, 1);
    trans.setValue(2, 0, dx);trans.setValue(2, 1, dy);
    Matrix result = location * trans;
    QPoint point(static_cast<int>(result[0][0]), static_cast<int>(result[0][1]));
    p = point;
}
/*旋转变换*/
void Canvas::rotate(QPoint &p, const QPoint& center, qreal angle) {
    int newx, newy;
    qreal wiseangle = angle * M_PI / 180.0;//逆时针方向上的旋转角度
    qreal sinsita = sin(wiseangle);
    qreal cossita = cos(wiseangle);
    int px = p.x(), py = p.y();
    int cx = center.x(), cy = center.y();
    newx = static_cast<int>(cx + (px - cx) * cossita - (py - cy) * sinsita);
    newy = static_cast<int>(cy + (px - cx) * sinsita + (py - cy) * cossita);
    QPoint point(newx, newy);
    p = point;
}
void Canvas::rotate(int &x, int &y, const QPoint &center, qreal angle) {
    int newx, newy;
    qreal wiseangle = angle * M_PI / 180.0;//逆时针方向上的旋转角度
    qreal sinsita = sin(wiseangle);
    qreal cossita = cos(wiseangle);
    int cx = center.x(), cy = center.y();
    newx = static_cast<int>(cx + (x - cx) * cossita - (y - cy) * sinsita);
    newy = static_cast<int>(cy + (x - cx) * sinsita + (y - cy) * cossita);
    x = newx;
    y = newy;
}
void Canvas::rotate(double &x, double &y, const QPoint &center, qreal angle) {
    int newx, newy;
    qreal wiseangle = angle * M_PI / 180.0;//逆时针方向上的旋转角度
    qreal sinsita = sin(wiseangle);
    qreal cossita = cos(wiseangle);
    int cx = center.x(), cy = center.y();
    newx = static_cast<int>(cx + (x - cx) * cossita - (y - cy) * sinsita);
    newy = static_cast<int>(cy + (x - cx) * sinsita + (y - cy) * cossita);
    x = newx;
    y = newy;
}
/*缩放变换*/
void Canvas::scale(int &x, int &y, const QPoint &center, double s) {
    int cx = center.x(), cy = center.y();
    x = (int)(x * s + cx * (1 - s) + 0.5);
    y = (int)(y * s + cy * (1 - s) + 0.5);
}
void Canvas::scale(long long &x, long long &y, const QPoint &center, double s) {
    long long int cx = center.x(), cy = center.y();
    x = (long long int)(x * s + cx * (1 - s) + 0.5);
    y = (long long int)(y * s + cy * (1 - s) + 0.5);
}
void Canvas::scale(QPoint &p, const QPoint &center, double s) {
    int cx = center.x(), cy = center.y();
    p.setX((int)(p.x() * s + cx * (1 - s) + 0.5));
    p.setY((int)(p.y() * s + cy * (1 - s) + 0.5));
}
void Canvas::scale(double &x, double &y, const QPoint &center, double s) {
    double cx = center.x(), cy = center.y();
    x = x * s + cx * (1 - s) + 0.5;
    y = y * s + cy * (1 - s) + 0.5;
}
/*线段裁剪Cohen-Sutherland算法*/
int Canvas::Cohen_Sutherland(QPoint &p1, QPoint &p2,
                              const QPoint& w1, const QPoint& w2) {
    int x1 = p1.x(), y1 = p1.y(), x2 = p2.x(), y2 = p2.y();
    int w1x, w1y, w2x, w2y;
    w1x = w1.x(), w1y = w1.y();
    w2x = w2.x(), w2y = w2.y();
    int minwx = min(w1.x(), w2.x());
    int minwy = min(w1.y(), w2.y());
    int maxwx = max(w1.x(), w2.x());
    int maxwy = max(w1.y(), w2.y());
    unsigned char p1code = encode(p1, minwx, minwy, maxwx, maxwy),
                  p2code = encode(p2, minwx, minwy, maxwx, maxwy);
    /*区域内线段*/
    if ((p1code | p2code) == 0) {
        return 1;
    }
    /*区域外线段*/
    else if((p1code & p2code) != 0) {
        return 2;
    }
    /*不确定线段*/
    else {
        if (x2 == x1) {
            if (min(y1, y2) > maxwy || max(y1, y2) < minwy) return 2;
            if (y2 >= y1) {
                if (y2 > maxwy) {
                    p2.setY(maxwy);
                }
                if (y1 < minwy) {
                    p1.setY(minwy);
                }
            }
            else if (y1 > y2) {
                if (y1 > maxwy) {
                    p1.setY(maxwy);
                }
                if (y2 < minwy) {
                    p2.setY(minwy);
                }
            }
        }
        else if (y1 == y2) {
            if (min(x1, x2) > maxwx || max(x1, x2) < minwx) return 2;
            if (x2 >= x1) {
                if (x2 > maxwx) {
                    p2.setX(maxwx);
                }
                if (x1 < minwx) {
                    p1.setX(minwx);
                }
            }
            else if (x1 > x2) {
                if (x1 > maxwx) {
                    p1.setX(maxwx);
                }
                if (x2 < minwx) {
                    p2.setX(minwx);
                }
            }
        }
        else {
            while (1) {
                p1code = encode(p1, minwx, minwy, maxwx, maxwy);
                p2code = encode(p2, minwx, minwy, maxwx, maxwy);
                if ((p1code | p2code) == 0) {
                    return 1;
                }
                else if((p1code & p2code) != 0) {
                    return 2;
                }
                else {
                    double k = double(y2 - y1) / (x2 - x1);
                    /*p1不在窗口内*/
                    if (p1code == 0) {
                        swap(p1, p2);
                        swap(x1, x2);
                        swap(y1, y2);
                        swap(p1code, p2code);
                    }
                    if((p1code & 1) == 1) {
                        y1 = k * (minwx - x1) + y1;
                        x1 = minwx;
                    }
                    if((p1code & 2) == 2) {
                        y1 = k * (maxwx - x1) + y1;
                        x1 = maxwx;
                    }
                    if(p1code == 4) {
                        x1 = (minwy - y1) / k + x1;
                        y1 = minwy;
                    }
                    if(p1code == 8) {
                        x1 = (maxwy - y1) / k + x1;
                        y1 = maxwy;
                    }
                    p1.setX(x1);
                    p1.setY(y1);
                }
            }
        }
        return 1;
    }
}
/*线段裁剪梁友栋-Barsky算法*/
int Canvas::Liang_Barsky(QPoint &p1, QPoint& p2,
                         const QPoint& w1, const QPoint& w2) {
    int x1 = p1.x(), y1 = p1.y(), x2 = p2.x(), y2 = p2.y();
    int minwx = min(w1.x(), w2.x()), maxwx = max(w1.x(), w2.x()),
        minwy = min(w1.y(), w2.y()), maxwy = max(w1.y(), w2.y());
    int dx = x2 - x1, dy = y2 - y1;
    double u;
    /*算法关键参数p和q*/
    int p[4];
    p[0] = -dx, p[1] = dx, p[2] = -dy, p[3] = dy;
    int q[4];
    q[0] = x1 - minwx, q[1] = maxwx - x1, q[2] = y1 - minwy, q[3] = maxwy - y1;
    double u1 = 0, u2 = 1; //最终得到的参数
    /*开始处理*/
    if (p[0] == 0) {
        /*倘若在区域以左或以右*/
        if (q[0] < 0 || q[1] < 0) {
            return 2;
        }
        for (int i = 2; i < 4; ++i) {
            u = (double)q[i] / p[i];
            if (p[i] < 0) {
                u1 = max(u, u1);
            }
            else {
                u2 = min(u, u2);
            }
        }
    }
    else if (p[2] == 0){
        if (q[2] < 0 || q[3] < 0) {
            return 2;
        }
        for (int i = 0; i < 2; ++i) {
            u = (double)q[i] / p[i];
            if (p[i] < 0) {
                u1 = max(u, u1);
            }
            else {
                u2 = min(u, u2);
            }
        }
    }
    else {
        double uenter[2], uexit[2];
        unsigned int ienter = 0, iexit = 0;
        for (int i = 0; i < 4; ++i) {
            if (p[i] < 0) {
                uenter[ienter++] = (double)q[i] / p[i];
            }
            else if (p[i] > 0){
                uexit[iexit++] = (double)q[i] / p[i];
            }
        }
        u1 = max(0.0, max(uenter[0], uenter[1]));
        u2 = min(1.0, min(uexit[0], uexit[1]));
    }
    if (u1 > u2) return 2;//在窗口外
    else {
        p1.setX(static_cast<int>(x1 + u1 * (x2 - x1)));
        p1.setY(static_cast<int>(y1 + u1 * (y2 - y1)));
        p2.setX(static_cast<int>(x1 + u2 * (x2 - x1)));
        p2.setY(static_cast<int>(y1 + u2 * (y2 - y1)));
        return 1;
    }
}
/*多边形裁剪Sutherland-Hodgeman算法*/
void Canvas::Sutherland_Hodgeman(vector<QPoint> &points, const QPoint& w1, const QPoint& w2) {
    int wxl = min(w1.x(), w2.x()), wxr = max(w1.x(), w2.x()),
            wyb = min(w1.y(), w2.y()), wyt = max(w1.y(), w2.y());
    /*vector<int> clipLines;
    clipLines.push_back(wxl);clipLines.push_back(wyb);clipLines.push_back(wxr);clipLines.push_back(wyt);*/
    vector<QPoint> tmp = points;
    vector<QPoint> result;
    int len = tmp.size();
    for (int j = 0; j < tmp.size(); ++j) {
        if (tmp[j].x() < wxl && tmp[(j + 1) % len].x() >= wxl) {
            int y = (double)(tmp[(j+1)%len].y() - tmp[j].y())/(tmp[(j+1)%len].x() - tmp[j].x())*(wxl-tmp[j].x())+tmp[j].y();
            result.push_back(QPoint(wxl, y));
            result.push_back(tmp[(j + 1)%len]);
        }
        else if (tmp[j].x() >= wxl && tmp[(j+1)%len].x() >= wxl) {
            result.push_back(tmp[(j+1)%len]);
        }
        else if (tmp[j].x() >= wxl && tmp[(j + 1)%len].x() < wxl) {
            int y = (double)(tmp[(j+1)%len].y() - tmp[j].y())/(tmp[(j+1)%len].x() - tmp[j].x())*(wxl-tmp[j].x())+tmp[j].y();
            result.push_back(QPoint(wxl, y));
        }
    }
    tmp=result; len = tmp.size(); result.clear();
    for (int j = 0; j < tmp.size(); ++j) {
        if (tmp[j].y() < wyb && tmp[(j + 1) % len].y() >= wyb) {
            int x = (double)(tmp[(j+1)%len].x() - tmp[j].x())/(tmp[(j+1)%len].y() - tmp[j].y())*(wyb-tmp[j].y())+tmp[j].x();
            result.push_back(QPoint(x, wyb));
            result.push_back(tmp[(j + 1)%len]);
        }
        else if (tmp[j].y() >= wyb && tmp[(j+1)%len].y() >= wyb) {
            result.push_back(tmp[(j+1)%len]);
        }
        else if (tmp[j].y() >= wyb && tmp[(j + 1)%len].y() < wyb) {
            int x = (double)(tmp[(j+1)%len].x() - tmp[j].x())/(tmp[(j+1)%len].y() - tmp[j].y())*(wyb-tmp[j].y())+tmp[j].x();
            result.push_back(QPoint(x, wyb));
        }
    }
    tmp=result; len = tmp.size(); result.clear();
    for (int j = 0; j < tmp.size(); ++j) {
        if (tmp[j].x() > wxr && tmp[(j + 1) % len].x() <= wxr) {
            int y = (double)(tmp[(j+1)%len].y() - tmp[j].y())/(tmp[(j+1)%len].x() - tmp[j].x())*(wxr-tmp[j].x())+tmp[j].y();
            result.push_back(QPoint(wxr, y));
            result.push_back(tmp[(j + 1)%len]);
        }
        else if (tmp[j].x() <= wxr && tmp[(j+1)%len].x() <= wxr) {
            result.push_back(tmp[(j+1)%len]);
        }
        else if (tmp[j].x() <= wxr && tmp[(j + 1)%len].x() > wxr) {
            int y = (double)(tmp[(j+1)%len].y() - tmp[j].y())/(tmp[(j+1)%len].x() - tmp[j].x())*(wxr-tmp[j].x())+tmp[j].y();
            result.push_back(QPoint(wxr, y));
        }
    }
    tmp=result; len = tmp.size(); result.clear();
    for (int j = 0; j < tmp.size(); ++j) {
        if (tmp[j].y() > wyt && tmp[(j + 1) % len].y() <= wyt) {
            int x = (double)(tmp[(j+1)%len].x() - tmp[j].x())/(tmp[(j+1)%len].y() - tmp[j].y())*(wyt-tmp[j].y())+tmp[j].x();
            result.push_back(QPoint(x, wyt));
            result.push_back(tmp[(j + 1)%len]);
        }
        else if (tmp[j].y() <= wyt && tmp[(j+1)%len].y() <= wyt) {
            result.push_back(tmp[(j+1)%len]);
        }
        else if (tmp[j].y() <= wyt && tmp[(j + 1)%len].y() > wyt) {
            int x = (double)(tmp[(j+1)%len].x() - tmp[j].x())/(tmp[(j+1)%len].y() - tmp[j].y())*(wyt-tmp[j].y())+tmp[j].x();
            result.push_back(QPoint(x, wyt));
        }
    }
    points = result;
    cout << "haha" << points.size();
}
void Canvas::Sutherland_Hodgeman(vector<Point> &points, const QPoint &w1, const QPoint &w2) {
    vector<QPoint> ps;
    for (int i = 0; i < points.size(); ++i) {
        ps.push_back(QPoint(points[i].xp, points[i].yp));
    }
    Sutherland_Hodgeman(ps, w1, w2);
    points.clear();
    for (int i = 0; i < ps.size(); ++i) {
        points.push_back(ps[i]);
    }
    qDebug() << "XIIX";
}
/*贝塞尔曲线*/
void Canvas::BezierDrawCurve(QPixmap *map, vector<QPoint>& points) {
    double a, b, c, sqavg_ab;//ab的平方平均数，根号下a2+b2
    int length = points.size();
    a = points[length - 1].y() - points[0].y();
    b = points[0].x() - points[length - 1].x();
    c = points[length - 1].x() * points[0].y()
      - points[0].x() * points[length - 1].y();
    sqavg_ab = sqrt(a * a + b * b);
    bool flag = true;//flag表示是否已经足够接近
    for (int i = 1; i < length - 1; ++i) {
        //cout << abs(a * points[i].x() + b * points[i].y() + c) / sqavg_ab << endl;
        if ((sqavg_ab == 0 && pow(points[i].x() - points[0].x(), 2) + pow(points[i].y() - points[0].y(), 2) > 0.1)
                || abs((a * points[i].x() + b * points[i].y() + c) / sqavg_ab) > 1) {
            flag = false; //不够接近
            break;
        }
    }
    if (flag == true) { //足够接近，可以直接绘制多边形
        for (int i = 0; i < length - 1; ++i) {
            this->BREDrawLine(map, (int)(points[i].x() + 0.5), (int)(points[i].y() + 0.5),
                              (int)(points[i + 1].x() + 0.5), (int)(points[i + 1].y() + 0.5));
        }
        return;
    }
    else {
        vector<vector<QPoint>> dp(length);
        dp[0] = points;
        for (int i = 1; i < length; ++i) {
            for (int j = 0; j < dp[i - 1].size() - 1; ++j) {
                dp[i].push_back((dp[i - 1][j] + dp[i - 1][j + 1]) / 2.0);
            }
        }
        vector<QPoint> points1, points2;
        for (int i = 0; i < length; ++i) {
            points1.push_back(dp[i][0]);
            points2.push_back(*(dp[length - 1 - i].end() - 1));
        }
        //dp.clear();
        BezierDrawCurve(map, points1);
        BezierDrawCurve(map, points2);
    }
}

void Canvas::BezierDrawCurve(QPixmap *map, vector<Point> &points) {
    double a, b, c, sqavg_ab;//ab的平方平均数，根号下a2+b2
    int length = points.size();
    a = points[length - 1].yp - points[0].yp;
    b = points[0].xp - points[length - 1].xp;
    c = points[length - 1].xp * points[0].yp
      - points[0].xp * points[length - 1].yp;
    sqavg_ab = sqrt(a * a + b * b);
    bool flag = true;//flag表示是否已经足够接近
    for (int i = 1; i < length - 1; ++i) {
        //cout << abs(a * points[i].x() + b * points[i].y() + c) / sqavg_ab << endl;
        if ((sqavg_ab == 0 && pow(points[i].xp - points[0].xp, 2) + pow(points[i].yp - points[0].yp, 2) > 0.1)
                ||abs((a * points[i].xp + b * points[i].yp + c) / sqavg_ab) > 0.1) {
            flag = false; //不够接近
            break;
        }
    }
    if (flag == true) { //足够接近，可以直接绘制多边形
        for (int i = 0; i < length - 1; ++i) {
            this->BREDrawLine(map, (int)(points[i].xp + 0.5), (int)(points[i].yp + 0.5),
                              (int)(points[i + 1].xp + 0.5), (int)(points[i + 1].yp + 0.5));
        }
        return;
    }
    else {
        vector<vector<Point>> dp(length);
        dp[0] = points;
        for (int i = 1; i < length; ++i) {
            for (int j = 0; j < dp[i - 1].size() - 1; ++j) {
                Point a;
                a.xp = (dp[i - 1][j].xp + dp[i - 1][j + 1].xp) / 2.0;
                a.yp = (dp[i - 1][j].yp + dp[i - 1][j + 1].yp) / 2.0;
                dp[i].push_back(a);
            }
        }
        vector<Point> points1, points2;
        for (int i = 0; i < length; ++i) {
            points1.push_back(dp[i][0]);
            points2.push_back(*(dp[length - 1 - i].end() - 1));
        }
        dp.clear();
        BezierDrawCurve(map, points1);
        BezierDrawCurve(map, points2);
    }
}

int Canvas::getCurveNode(const QPoint &p) {
    for (int i = 0; i < transpoints.size(); ++i) {
        double dx = transpoints[i].xp - p.x();
        double dy = transpoints[i].yp - p.y();
        if(dx * dx + dy * dy < 1600) {
            return i;
        }
    }
    return -1;
}
void Canvas::completeDraw() {
    if (!isPolygonDrawDone || !isBezierDrawDone) {
        isPolygonDrawDone = true;
        isBezierDrawDone = true;
        bezierReadyDone = true;
        lineDrawed = 0;
        startPoint = endPoint;
        if (graphClass == POLYGON) {
            if (this->lineClass == DDALINE)
                DDADrawPolygon(&pixmap, vb);
            else BREDrawPolygon(&pixmap, vb);
            vb.clear();
        }
        else if (graphClass == FILLPOLYGON) {
            ETFillPolygon(&pixmap, vb);
            vb.clear();
        }
        else if (graphClass == BEZIERCURVE) {
            BezierDrawCurve(&pixmap, vb);
            vb.clear();
        }
        else if (graphClass == BSPLINECURVE) {
            BSplineDrawCurve(&pixmap, vb, kBSpline);
            vb.clear();
        }
    }
    update();
}
void Canvas::changecolor(){
    QColorDialog dlg;
    penColor = dlg.getColor(penColor);
    update();
}
void Canvas::chooseLine() {
    Transforming = -1;
    completeDraw();
    this->graphClass = LINE;
}
void Canvas::chooseEllipse() {
    Transforming = -1;
    completeDraw();
    this->graphClass = ELLIPSE;
}
void Canvas::choosePencil() {
    Transforming = -1;
    completeDraw();
    this->graphClass = PENCIL;
}
void Canvas::choosePolygon() {
    Transforming = -1;
    completeDraw();
    this->graphClass = POLYGON;
}
void Canvas::chooseFillPolygon() {
    Transforming = -1;
    completeDraw();
    this->graphClass = FILLPOLYGON;
}
void Canvas::chooseFill() {
    Transforming = -1;
    completeDraw();
    this->graphClass = FILL;
}
void Canvas::chooseBezier() {
    Transforming = -1;
    completeDraw();
    this->graphClass = BEZIERCURVE;
}
void Canvas::chooseBSpline() {
    Transforming = -1;
    completeDraw();
    this->graphClass = BSPLINECURVE;
}
void Canvas::chooseTranslate() {
    Transforming = TRANSLATE;
    startPoint = endPoint;
    update();
}
void Canvas::chooseRotate() {
    Transforming = ROTATE;
    startPoint = endPoint;
    update();
}
void Canvas::chooseScale() {
    Transforming = SCALE;
    startPoint = endPoint;
    update();
}
void Canvas::chooseClip() {
    Transforming = CLIP;
    startPoint = endPoint;
    update();
}
void Canvas::chooseAdjust() {
    Transforming = ADJUST;
    startPoint = endPoint;
    update();
}

void Canvas::setSurround(const vector<Point> &v) {
    xl = 1000000, yd = 1000000, xr = -1000000, yu = -1000000;
    for (int i = 0; i < v.size(); ++i) {
        int xp = v[i].xp, yp = v[i].yp;
        xl = min(xl, xp);
        xr = max(xr, xp);
        yd = min(yd, yp);
        yu = max(yu, yp);
    }
}
/*
 *     for (int i = 0; i < NET.size(); ++i) {
        if (!NET[i].empty()){
            for (int j = 0; j < NET[i].size(); ++j) {
                cout << i + ymin<< ',' << NET[i][j];
            }
            cout << endl;
        }

    }
    */
void Canvas::redraw() {
    pixmap.fill(Qt::white);
    QColor tmpcolor = penColor;
    /*计数*/
    for (int i = 0; i < graphseq.size(); ++i) {
        if (graphseq[i].graphclass == DDALINE) {
            penColor = ((DDALine*)graphseq[i].g)->penColor;
            DDADrawLine(&pixmap, ((DDALine*)graphseq[i].g)->x0,
                               ((DDALine*)graphseq[i].g)->y0,
                               ((DDALine*)graphseq[i].g)->x1,
                               ((DDALine*)graphseq[i].g)->y1);
        }
        else if (graphseq[i].graphclass == BRELINE) {
            penColor = ((BRELine*)graphseq[i].g)->penColor;
            BREDrawLine(&pixmap, ((BRELine*)graphseq[i].g)->x0,
                               ((BRELine*)graphseq[i].g)->y0,
                               ((BRELine*)graphseq[i].g)->x1,
                               ((BRELine*)graphseq[i].g)->y1);
        }
        else if (graphseq[i].graphclass == DDAPOLYGON) {
            penColor = ((DDAPolygon*)graphseq[i].g)->penColor;
            DDADrawPolygon(&pixmap, ((DDAPolygon*)graphseq[i].g)->v);
        }
        else if (graphseq[i].graphclass == BREPOLYGON) {
            penColor = ((BREPolygon*)graphseq[i].g)->penColor;
            BREDrawPolygon(&pixmap, ((BREPolygon*)graphseq[i].g)->v);
        }
        else if (graphseq[i].graphclass == ELLIPSE) {
            penColor = ((Ellipse*)graphseq[i].g)->penColor;
            BREDrawEllipse(&pixmap, ((Ellipse*)graphseq[i].g)->xc, ((Ellipse*)graphseq[i].g)->yc,
                           ((Ellipse*)graphseq[i].g)->rx, ((Ellipse*)graphseq[i].g)->ry);
        }
        else if (graphseq[i].graphclass == FILLPOLYGON) {
            penColor = ((FillPolygon*)graphseq[i].g)->penColor;
            ETFillPolygon(&pixmap, ((FillPolygon*)graphseq[i].g)->v);
        }
        else if (graphseq[i].graphclass == BEZIERCURVE) {
            penColor = ((BezierCurve*)graphseq[i].g)->penColor;
            vector<Point> v;
            for (int i = 0; i < ((BezierCurve*)graphseq[i].g)->v.size(); ++i) {
                v.push_back(((BezierCurve*)graphseq[i].g)->v[i]);
            }
            BezierDrawCurve(&pixmap, v);
        }
        else if (graphseq[i].graphclass == BSPLINECURVE) {
            penColor = ((B_SplineCurve*)graphseq[i].g)->penColor;
            vector<Point> v;
            for (int i = 0; i < ((B_SplineCurve*)graphseq[i].g)->v.size(); ++i) {
                v.push_back(((B_SplineCurve*)graphseq[i].g)->v[i]);
            }
            BSplineDrawCurve(&pixmap, v, 4);//4阶3次B样条
        }
    }
    penColor = tmpcolor;
}
