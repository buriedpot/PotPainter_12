#include "command.h"

Command::Command()
{
    this->picsavedir = ".";
}

Command::Command(QString picsavedir) {
    this->picsavedir = picsavedir.toStdString();
    if (this->picsavedir == "") this->picsavedir = ".";
    QDir dir;
    if (!dir.exists(picsavedir)) {
        dir.mkpath(picsavedir);
    }
}

string Command::lower(const string& str) {
    string result = str;
    for (int i = 0; i < result.size(); ++i) {
        if (result[i] >= 'A' && result[i] <= 'Z') {
            result[i] += 0x20;
        }
    }
    return result;
}

vector<string> Command::split(string cmd, char tag) {
    vector<string> result;
    string substring("");
    for (unsigned int i = 0; i < cmd.size(); ++i) {
        if (cmd[i] != tag) {
            substring += cmd[i];
            continue;
        }
        else {
            if (!substring.empty()){
                result.push_back(substring);
                substring.clear();
            }
        }
    }
    if (!substring.empty()) {
        result.push_back(substring);
    }
    return result;
}

int Command::cmdAnalyze(string cmd, Canvas& canvas, QFile& fin) {
    vector<string> command = split(cmd, ' ');
    if (command.empty()) return -1;
    if (lower(command[0]) == "resetcanvas") {  //重置画布
        if (command.size() < 3) return -1;
        QString wstr(command[1].c_str()), hstr(command[2].c_str());
        int w = wstr.toInt(), h = hstr.toInt();
        if (w < 100 || w > 1000 || h < 100 || h > 1000) {
            cout << "画布长宽限制：100<=width,height<=1000" << endl;
            return -1;
        }
        canvas.pixmap = QPixmap(w, h);
        canvas.resize(w, h);
        canvas.pixmap.fill(Qt::white);
        canvas.update();
        /*删除所有图元*/
        canvas.graphseq.clear();
    }
    else if (lower(command[0]) == "savecanvas") { //保存画布
        if (command.size() < 2) return -1;
        string f = this->picsavedir + "/" + command[1] + ".bmp";
        QString filename(f.c_str());
        canvas.pixmap.save(filename);
        cout << "save" << endl;
    }
    else if (lower(command[0]) == "setcolor") {
        if (command.size() < 4) return -1;
        QString rstr(command[1].c_str()), gstr(command[2].c_str()), bstr(command[3].c_str());
        int r = rstr.toInt(), g = gstr.toInt(), b = bstr.toInt();
        canvas.penColor = QColor(r,g,b);
    }
    else if (lower(command[0]) == "drawline") {
        if (command.size() < 7) return -1;
        QString idstr(command[1].c_str()), x1str(command[2].c_str()), y1str(command[3].c_str())\
                , x2str(command[4].c_str()), y2str(command[5].c_str());
        int id = idstr.toInt(), x1 = x1str.toInt(), y1 = y1str.toInt(), x2 = x2str.toInt(), \
                y2 = y2str.toInt();
        if (Command::lower(command[6]) == "dda") {
            canvas.DDADrawLine(&canvas.pixmap, x1, y1, x2, y2);
            canvas.graphseq.push_back({id, DDALINE,
                                      new DDALine(x1, y1, x2, y2, canvas.penColor, canvas.penWidth)});
        }
        else if (Command::lower(command[6]) == "bresenham") {
            canvas.BREDrawLine(&canvas.pixmap, x1, y1, x2, y2);
            canvas.graphseq.push_back({id, BRELINE,
                                       new BRELine(x1, y1, x2, y2, canvas.penColor, canvas.penWidth)});
        }
    }
    else if (lower(command[0]) == "drawellipse") {
        if (command.size() < 6) return -1;
        QString idstr(command[1].c_str()), xcstr(command[2].c_str()), ycstr(command[3].c_str())\
                , rxstr(command[4].c_str()), rystr(command[5].c_str());
        int id = idstr.toInt(); 
        long long xc = xcstr.toLongLong(), yc = ycstr.toLongLong(), rx = rxstr.toLongLong(), \
                ry = rystr.toLongLong();
        canvas.BREDrawEllipse(&canvas.pixmap, xc, yc, rx, ry);
        canvas.graphseq.push_back({id, ELLIPSE,
                                  new Ellipse(xc, yc, rx, ry, canvas.penColor, canvas.penWidth)});
    }
    else if (lower(command[0]) == "drawpolygon" ||
             lower(command[0]) == "drawfillpolygon") {
        if (command.size() < 4) return -1;
        vector<QPoint> v;
        int id = QString(command[1].c_str()).toInt();
        int n = QString(command[2].c_str()).toInt();

        QByteArray line = fin.readLine();
        QString tmp(line);
        if (tmp[tmp.size() - 1] == '\n')
            tmp = tmp.left(tmp.size() - 1);
        string datastr = tmp.toStdString();
        vector<string> data = split(datastr, ' ');

        for (int i = 0; i < n; ++i) {
            QPoint p(QString(data[2 * i].c_str()).toInt(),
                    QString(data[1 + 2 * i].c_str()).toInt());
            v.push_back(p);
        }
        if (Command::lower(command[0]) == "drawpolygon") {
            if (lower(command[3]) == "dda") {
                canvas.DDADrawPolygon(&canvas.pixmap, v);
                canvas.graphseq.push_back({id, DDAPOLYGON,
                                          new DDAPolygon(v, canvas.penColor, canvas.penWidth)});
            }
            else if (lower(command[3]) == "bresenham") {
                canvas.BREDrawPolygon(&canvas.pixmap, v);
                canvas.graphseq.push_back({id, BREPOLYGON,
                                          new BREPolygon(v, canvas.penColor, canvas.penWidth)});
            }
        }
        else {
            canvas.graphseq.push_back({id, FILLPOLYGON,
                                      new FillPolygon(v, canvas.penColor, canvas.penWidth)});
            canvas.ETFillPolygon(&canvas.pixmap, v);
        }
    }
    else if (lower(command[0]) == "clip") {
        if (command.size() < 7) return -1;
        QString idstr(command[1].c_str()), x1str(command[2].c_str()), y1str(command[3].c_str()),
                x2str(command[4].c_str()), y2str(command[5].c_str());
        int id = idstr.toInt(), x1 = x1str.toInt(), y1 = y1str.toInt(), x2 = x2str.toInt(),
                y2 = y2str.toInt();
        vector<Graphseq>::iterator it = find(canvas.graphseq.begin(),
                                             canvas.graphseq.end(), id);
        if (it == canvas.graphseq.end()) {
            //cout << "No such id!" << endl;
            return -1;
        }
        if (it->graphclass != DDALINE && it->graphclass != BRELINE) {
            //cout << "Not a line!" << it->graphclass << endl;
            return -1;
        }
        QPoint p1(((Line*)(it->g))->x0, ((Line*)(it->g))->y0),
                p2(((Line*)(it->g))->x1, ((Line*)(it->g))->y1),
                w1(x1, y1), w2(x2, y2);
        int result = 0;
        if (lower(command[6]) == "cohen-sutherland") {
            result = canvas.Cohen_Sutherland(p1, p2, w1, w2);
        }
        else if (lower(command[6]) == "liang-barsky") {
            result = canvas.Liang_Barsky(p1, p2, w1, w2);
        }
        else return -1;
        if (result == 2) canvas.graphseq.erase(it);
        else {
            ((Line*)(it->g))->x0 = p1.x(), ((Line*)(it->g))->y0 = p1.y();
            ((Line*)(it->g))->x1 = p2.x(), ((Line*)(it->g))->y1 = p2.y();
        }
        canvas.redraw();
    }
    else if (lower(command[0]) == "translate") {
        if (command.size() < 4) return -1;
        QString idstr(command[1].c_str()), dxstr(command[2].c_str()), dystr(command[3].c_str());
        int id = idstr.toInt(), dx = dxstr.toInt(), dy = dystr.toInt();
        vector<Graphseq>::iterator it = find(canvas.graphseq.begin(),
                                             canvas.graphseq.end(), id);
        if (it == canvas.graphseq.end()) {
            //cout << "No such id!" << endl;
            return -1;
        }
        if (it->graphclass == ELLIPSE) {
            canvas.translate(((Ellipse*)(it->g))->xc,
                             ((Ellipse*)(it->g))->yc, dx, dy);
        }
        else if (it->graphclass == DDALINE || it->graphclass == BRELINE) {
            canvas.translate(((Line*)(it->g))->x0, ((Line*)(it->g))->y0, dx, dy);
            canvas.translate(((Line*)(it->g))->x1, ((Line*)(it->g))->y1, dx, dy);
        }
        else if (it->graphclass == DDAPOLYGON || it->graphclass == BREPOLYGON
                 || it->graphclass == FILLPOLYGON) {
            int length = ((Polygon*)(it->g))->v.size();
            for (int i = 0; i < length; ++i) {
                canvas.translate(((Polygon*)(it->g))->v[i], dx, dy);
            }
        }
        else if (it->graphclass == BEZIERCURVE || it->graphclass == BSPLINECURVE) {
            int length = ((BezierCurve*)(it->g))->v.size();
            for (int i = 0; i < length; ++i) {
                canvas.translate(((BezierCurve*)(it->g))->v[i], dx, dy);
            }
        }
        canvas.redraw();
    }
    else if (lower(command[0]) == "rotate") {
        if (command.size() < 5) return -1;
        QString idstr(command[1].c_str()), cxstr(command[2].c_str()), cystr(command[3].c_str()),
                rstr(command[4].c_str());
        int id = idstr.toInt(), cx = cxstr.toInt(), cy = cystr.toInt(), r = rstr.toInt();
        QPoint center(cx, cy);
        vector<Graphseq>::iterator it = find(canvas.graphseq.begin(),
                                             canvas.graphseq.end(), id);
        if (it == canvas.graphseq.end()) {
            //cout << "No such id!" << endl;
            return -1;
        }
        if (it->graphclass == ELLIPSE) {
            //cout << "Ellipse cannot be rotated!" << endl;
            return -1;
        }
        else if (it->graphclass == DDALINE || it->graphclass == BRELINE) {
            canvas.rotate(((Line*)(it->g))->x0, ((Line*)(it->g))->y0, center, (double)r);
            canvas.rotate(((Line*)(it->g))->x1, ((Line*)(it->g))->y1, center, (double)r);
        }
        else if (it->graphclass == DDAPOLYGON || it->graphclass == BREPOLYGON
                 || it->graphclass == FILLPOLYGON) {
            int length = ((Polygon*)(it->g))->v.size();
            for (int i = 0; i < length; ++i) {
                canvas.rotate(((Polygon*)(it->g))->v[i], center, (double)r);
            }
        }
        else if (it->graphclass == BEZIERCURVE || it->graphclass == BSPLINECURVE) {
            int length = ((BezierCurve*)(it->g))->v.size();
            for (int i = 0; i < length; ++i) {
                canvas.rotate(((BezierCurve*)(it->g))->v[i], center, (double)r);
            }
        }
        canvas.redraw();
    }
    else if (lower(command[0]) == "drawcurve") {
        if (command.size() < 4) return -1;
        QString idstr(command[1].c_str()), nstr(command[2].c_str());
        int id = idstr.toInt(), n =nstr.toInt();
        QByteArray line = fin.readLine();
        QString tmp(line);
        if (tmp[tmp.size() - 1] == '\n')
            tmp = tmp.left(tmp.size() - 1);
        string datastr = tmp.toStdString();
        vector<string> data = split(datastr, ' ');
        if (data.size() != 2 * n) return -1;
        vector<QPoint> points;
        vector<Point> pointsp;
        for (int i = 0; i < n; ++i) {
            QPoint p(QString(data[2 * i].c_str()).toInt(), QString(data[2 * i + 1].c_str()).toInt());
            points.push_back(p);
            pointsp.push_back(p);
        }
        if (lower(command[3]) == "bezier") {
            canvas.BezierDrawCurve(&canvas.pixmap, pointsp);
            canvas.graphseq.push_back({id, BEZIERCURVE,
                                      new BezierCurve(points, canvas.penColor, canvas.penWidth)});
        }
        else if (lower(command[3]) == "b-spline" || lower(command[3]) == "bspline") {
            canvas.BSplineDrawCurve(&canvas.pixmap, pointsp, 4);
            canvas.graphseq.push_back({id, BSPLINECURVE,
                                      new B_SplineCurve(points, canvas.penColor, canvas.penWidth)});
        }
    }
    else if (lower(command[0]) == "scale") {
        if (command.size() < 5) return -1;
        QString idstr(command[1].c_str()), xstr(command[2].c_str()), ystr(command[3].c_str()), sstr(command[4].c_str());
        //qDebug() << sstr;
        int id = idstr.toInt(), x = xstr.toInt(), y = ystr.toInt();
        double s = sstr.toDouble();
        //qDebug() << s;
        vector<Graphseq>::iterator it = find(canvas.graphseq.begin(),
                                             canvas.graphseq.end(), id);
        QPoint center(x, y);
        QPoint center2(0,0);
        if (it == canvas.graphseq.end()) {
            //cout << "No such id!" << endl;
            return -1;
        }
        if (it->graphclass == ELLIPSE) {
            canvas.scale(((Ellipse*)(it->g))->xc,
                             ((Ellipse*)(it->g))->yc, center, s);
            canvas.scale(((Ellipse*)(it->g))->rx,
                             ((Ellipse*)(it->g))->ry, center2, s);
        }
        else if (it->graphclass == DDALINE || it->graphclass == BRELINE) {
            canvas.scale(((Line*)(it->g))->x0, ((Line*)(it->g))->y0, center, s);
            canvas.scale(((Line*)(it->g))->x1, ((Line*)(it->g))->y1, center, s);
        }
        else if (it->graphclass == DDAPOLYGON || it->graphclass == BREPOLYGON
                 || it->graphclass == FILLPOLYGON) {
            int length = ((Polygon*)(it->g))->v.size();
            for (int i = 0; i < length; ++i) {
                canvas.scale(((Polygon*)(it->g))->v[i], center, s);
            }
        }
        else if (it->graphclass == BEZIERCURVE) {
            int length = ((BezierCurve*)(it->g))->v.size();
            for (int i = 0; i < length; ++i) {
                canvas.scale(((BezierCurve*)(it->g))->v[i], center, s);
            }
        }
        else if (it->graphclass == BSPLINECURVE) {
            int length = ((B_SplineCurve*)(it->g))->v.size();
            for (int i = 0; i < length; ++i) {
                canvas.scale(((B_SplineCurve*)(it->g))->v[i], center, s);
            }
        }
        canvas.redraw();
    }
    return 0;
}
