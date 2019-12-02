#ifndef STRUCTURE_H
#define STRUCTURE_H
#include <iostream>
#include <QColor>
#include <vector>
#include <QPoint>
using namespace std;

typedef long long int ll;

typedef struct ETunit{
    int ymax;
    double x;
    double k_1;//斜率倒数
    friend ostream& operator<<(ostream &out, const ETunit &x);
} ETunit;

bool ETcmpl(const ETunit &u1, const ETunit &u2) {
    return u1.x < u2.x;
}

ostream  &operator<<(ostream &out, const ETunit &x)    //进来后又出去
{
    cout << x.x << ' ' << x.ymax << ' ' << x.k_1 << ' ';
}




#endif // STRUCTURE_H
