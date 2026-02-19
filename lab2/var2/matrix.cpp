#include "matrix.hpp"

#include <iostream>
#include <fstream>
#include <vector>
#include <stdexcept>

Matrix::Matrix(int w, int h, IMatrixGetter& getter)
    : width_(w),
      height_(h),
      data(height_ * width_, 0.0f)
      {
        for (int i = 0; i < height_; ++i)
        {
            for(int j = 0; j < width_; ++j)
            {
                data[i * width_ + j] = getter.get(i, j);
            }
        }
      }

double Matrix::get(int i, int j)
{
    return data[i * width_ + j];
}
std::vector<double> Matrix::MulVec(std::vector<double> &res, std::vector<double> &x)
{
    #pragma omp for
    for (int i = 0; i < height_; ++i)
    {
        double sum = 0.0;
        for (int j = 0; j < width_; ++j)
            sum += data[i*width_ + j] * x[j];
        res[i] = sum;
    }
    return res;
}

std::vector<double> Matrix::MulVecAdd(std::vector<double> &res, std::vector<double> &x, std::vector<double> &add, double cof)
{
    #pragma omp for
    for (int i = 0; i < height_; ++i)
    {
        double sum = 0.0;
        for (int j = 0; j < width_; ++j)
            sum += data[i*width_ + j] * x[j];
        res[i] = sum + add[i]*cof;
    }
    return res;
}