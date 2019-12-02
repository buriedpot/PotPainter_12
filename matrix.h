#ifndef MATRIX_H
#define MATRIX_H
#include <QDebug>
#include <iostream>
using namespace std;
class Matrix {
    int row, col;
    qreal **matrix;
public:
    Matrix(int row, int col);
    ~Matrix();
    void setValue(int r, int c, qreal value);
    qreal*& operator[] (int i);
    qreal& element(int i, int j);
    Matrix operator*(const Matrix& m);
    friend ostream& operator<<(ostream &out, const Matrix &m);
};


#endif // MATRIX_H
