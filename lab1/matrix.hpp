#pragma once

#include <vector>
#include "matrix_getter.hpp"

class Matrix
{
private:
    int st_h_;
    int st_w_;
    int width_;
    int height_;
    std::vector<std::vector<double>> data;
public:
    Matrix() = default;
    Matrix(int st_h,
         int w, int h,
        IMatrixGetter& getter);

    double get(int i, int j);

    std::vector<double> MulFullVec(std::vector<double> &x);
    std::vector<double> MulPartVec(std::vector<double> &x, int st, int size);
};
