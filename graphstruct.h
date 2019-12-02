#ifndef GRAPHSTRUCT_H
#define GRAPHSTRUCT_H

#include <iostream>
#include <QColor>
#include <vector>
#include <QPoint>
using namespace std;

typedef long long int ll;

typedef struct Graph {

} Graph;

typedef struct Graphseq{
    int id;
    int graphclass;
    Graph *g;
    bool operator ==(const Graphseq& gsq) {
        return gsq.id == id;
    }
    bool operator ==(const int& id) {
        return this->id == id;
    }

    ~Graphseq() {
        //delete g;
    }
} Graphseq;

typedef struct Point {
    double xp, yp;
    Point(){}
    Point(const QPoint& p) {
        xp = p.x();
        yp = p.y();
    }
} Point;

typedef struct Line: public Graph{
    int x0, y0;
    int x1, y1;
    QColor penColor;
    int penWidth;
    Line(int x0, int y0, int x1, int y1, QColor penColor, int penWidth) {
        this->x0 = x0;
        this->y0 = y0;
        this->x1 = x1;
        this->y1 = y1;
        this->penColor = penColor;
        this->penWidth = penWidth;
    }
} DDALine, BRELine;

typedef struct Ellipse: public Graph {
    ll xc, yc;
    ll rx, ry;
    QColor penColor;
    int penWidth;
    Ellipse(ll xc, ll yc, ll rx, ll ry, QColor penColor, int penWidth) {
        this->xc = xc;
        this->yc = yc;
        this->rx = rx;
        this->ry = ry;
        this->penColor = penColor;
        this->penWidth = penWidth;
    }
} Ellipse;

typedef struct Polygon: public Graph {
    vector<QPoint> v;
    QColor penColor;
    int penWidth;
    Polygon(vector<QPoint> v, QColor penColor, int penWidth){
        this->v = v;
        this->penColor = penColor;
        this->penWidth = penWidth;
    }
} DDAPolygon, BREPolygon, FillPolygon;//填充或边界多边形

typedef struct Curve: public Graph {
    vector<QPoint> v;
    QColor penColor;
    int penWidth;
    Curve(vector<QPoint> v, QColor penColor, int penWidth) {
        this->v = v;
        this->penColor = penColor;
        this->penWidth = penWidth;
    }
} BezierCurve, B_SplineCurve;
#endif // GRAPHSTRUCT_H
