#pragma once

#include <vector>
#include "matrix_getter.hpp"

class Matrix
{
private:
    int width_;
    int height_;
    std::vector<double> data;

public:
    Matrix() = default;
    Matrix(int w,
           int h,
           IMatrixGetter &getter);

    double get(int i, int j);

    std::vector<double> MulVec(std::vector<double> &res, std::vector<double> &x);
    std::vector<double> MulVecAdd(std::vector<double> &res, std::vector<double> &x, std::vector<double> &add, double cof = 1.0);
};
