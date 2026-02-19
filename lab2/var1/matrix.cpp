#include "matrix.hpp"

#include <iostream>
#include <fstream>
#include <vector>
#include <stdexcept>

Matrix::Matrix(int st_h, int w, int h, IMatrixGetter& getter)
    : st_h_(st_h),
      width_(w),
      height_(h),
      data(height_ * width_, 0.0f)
      {
        for (int i = 0; i < height_; ++i)
        {
            for(int j = 0; j < width_; ++j)
            {
                data[i * width_ + j] = getter.get(i + st_h_, j);
            }
        }
      }

double Matrix::get(int i, int j)
{
    i -= st_h_;
    return data[i * width_ + j];
}
std::vector<double> Matrix::MulFullVec(std::vector<double> &x,
                                       const std::string& schedule_type,
                                       int chunk_size)
{
    std::vector<double> res(x.size(), 0.0);
    
    if (schedule_type == "static") {
        if (chunk_size > 0) {
            #pragma omp parallel for schedule(static, chunk_size)
            for (int i = 0; i < height_; ++i)
            {
                double sum = 0.0;
                for (int j = 0; j < width_; ++j)
                    sum += data[i*width_ + j] * x[j];
                res[i + st_h_] = sum;
            }
        } else {
            #pragma omp parallel for schedule(static)
            for (int i = 0; i < height_; ++i)
            {
                double sum = 0.0;
                for (int j = 0; j < width_; ++j)
                    sum += data[i*width_ + j] * x[j];
                res[i + st_h_] = sum;
            }
        }
    }
    else if (schedule_type == "dynamic") {
        if (chunk_size > 0) {
            #pragma omp parallel for schedule(dynamic, chunk_size)
            for (int i = 0; i < height_; ++i)
            {
                double sum = 0.0;
                for (int j = 0; j < width_; ++j)
                    sum += data[i*width_ + j] * x[j];
                res[i + st_h_] = sum;
            }
        } else {
            #pragma omp parallel for schedule(dynamic)
            for (int i = 0; i < height_; ++i)
            {
                double sum = 0.0;
                for (int j = 0; j < width_; ++j)
                    sum += data[i*width_ + j] * x[j];
                res[i + st_h_] = sum;
            }
        }
    }
    else if (schedule_type == "guided") {
        if (chunk_size > 0) {
            #pragma omp parallel for schedule(guided, chunk_size)
            for (int i = 0; i < height_; ++i)
            {
                double sum = 0.0;
                for (int j = 0; j < width_; ++j)
                    sum += data[i*width_ + j] * x[j];
                res[i + st_h_] = sum;
            }
        } else {
            #pragma omp parallel for schedule(guided)
            for (int i = 0; i < height_; ++i)
            {
                double sum = 0.0;
                for (int j = 0; j < width_; ++j)
                    sum += data[i*width_ + j] * x[j];
                res[i + st_h_] = sum;
            }
        }
    }
    else if (schedule_type == "auto") {
        #pragma omp parallel for schedule(auto)
        for (int i = 0; i < height_; ++i)
        {
            double sum = 0.0;
            for (int j = 0; j < width_; ++j)
                sum += data[i*width_ + j] * x[j];
            res[i + st_h_] = sum;
        }
    }
    else if (schedule_type == "runtime") {
        #pragma omp parallel for schedule(runtime)
        for (int i = 0; i < height_; ++i)
        {
            double sum = 0.0;
            for (int j = 0; j < width_; ++j)
                sum += data[i*width_ + j] * x[j];
            res[i + st_h_] = sum;
        }
    }
    else { // По умолчанию - static
        #pragma omp parallel for schedule(static)
        for (int i = 0; i < height_; ++i)
        {
            double sum = 0.0;
            for (int j = 0; j < width_; ++j)
                sum += data[i*width_ + j] * x[j];
            res[i + st_h_] = sum;
        }
    }
    
    return res;
}

