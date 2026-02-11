#include "matrix.hpp"

#include <iostream>
#include <fstream>
#include <vector>
#include <stdexcept>

Matrix::Matrix(int st_h, int w, int h, IMatrixGetter& getter)
    : st_h_(st_h),
      width_(w),
      height_(h),
      data(height_, std::vector<double>(width_, 0.0f))
      {
        for (int i = 0; i < height_; ++i)
        {
            for(int j = 0; j < width_; ++j)
            {
                data[i][j] = getter.get(i + st_h_, j);
            }
        }
      }

double Matrix::get(int i, int j)
{
    i -= st_h_;
    return data[i][j];
}
std::vector<double> Matrix::MulFullVec(std::vector<double> &x)
{
    std::vector<double> res(x.size(), 0.0);
    for (int i = 0; i < height_; ++i)
    {
        double sum = 0.0;
        for (int j = 0; j < width_; ++j)
            sum += data[i][j] * x[j];
        res[i + st_h_] = sum;
    }
    return res;
}

std::vector<double> Matrix::MulPartVec(std::vector<double> &x, int st, int real_size)
{
    std::vector<double> res(x.size(), 0.0);
    for (int i = 0; i < height_; ++i)
    {
        double sum = 0.0;
        for (int j = 0; j < real_size; ++j)
            sum += data[i][j + st] * x[j];
        res[i] = sum;
    }
    return res;
}
