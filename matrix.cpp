#include "matrix.h"

Matrix::Matrix(int row, int col){
    this->row = row;
    this->col = col;
    matrix = new qreal*[row];
    for (int i = 0; i < row; ++i) {
        matrix[i] = new qreal[col];
        for (int j = 0; j < col; ++j) {
            matrix[i][j] = 0;
        }
    }
}
Matrix::~Matrix() {
    for (int i = 0; i < row; ++i) {
        delete[] matrix[i];
    }
    delete[] matrix;
}
void Matrix::setValue(int r, int c, qreal value) {
    (*this)[r][c] = value;
}
qreal*& Matrix::operator[] (int i) {
    return matrix[i];
}
qreal& Matrix::element(int i, int j) {
    return matrix[i][j];
}
Matrix Matrix::operator*(const Matrix& m) {
    try {
        if (this->col != m.row) {
            throw -1;
        }
        Matrix result(this->row, m.col);
        for (int i = 0; i < this->row; ++i) {
            for (int j = 0; j < m.col; ++j) {
                result[i][j] = 0;
                for (int k = 0; k < this->col; ++k) {
                    //cout << i << j << k;
                    result[i][j] += matrix[i][k] * m.matrix[k][j];

                }
            }
        }
        //cout << result[0][0] << ' ' << result[0][1] << ' ' << result[0][2] << endl;
        return result;
    }
    catch (int e){
        qDebug() << "Illegal Multiplication! Return code: " << e;
    }
}
