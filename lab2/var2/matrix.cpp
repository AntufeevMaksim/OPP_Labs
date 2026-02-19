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

void  Matrix::MulVec(std::vector<double> &res, 
                                   std::vector<double> &x,
                                   const std::string& schedule_type,
                                   int chunk_size)
{
    if (schedule_type == "static") {
        if (chunk_size > 0) {
            #pragma omp for schedule(static, chunk_size)
            for (int i = 0; i < height_; ++i)
            {
                double sum = 0.0;
                for (int j = 0; j < width_; ++j)
                    sum += data[i*width_ + j] * x[j];
                res[i] = sum;
            }
        } else {
            #pragma omp for schedule(static)
            for (int i = 0; i < height_; ++i)
            {
                double sum = 0.0;
                for (int j = 0; j < width_; ++j)
                    sum += data[i*width_ + j] * x[j];
                res[i] = sum;
            }
        }
    }
    else if (schedule_type == "dynamic") {
        if (chunk_size > 0) {
            #pragma omp for schedule(dynamic, chunk_size)
            for (int i = 0; i < height_; ++i)
            {
                double sum = 0.0;
                for (int j = 0; j < width_; ++j)
                    sum += data[i*width_ + j] * x[j];
                res[i] = sum;
            }
        } else {
            #pragma omp for schedule(dynamic)
            for (int i = 0; i < height_; ++i)
            {
                double sum = 0.0;
                for (int j = 0; j < width_; ++j)
                    sum += data[i*width_ + j] * x[j];
                res[i] = sum;
            }
        }
    }
    else if (schedule_type == "guided") {
        if (chunk_size > 0) {
            #pragma omp for schedule(guided, chunk_size)
            for (int i = 0; i < height_; ++i)
            {
                double sum = 0.0;
                for (int j = 0; j < width_; ++j)
                    sum += data[i*width_ + j] * x[j];
                res[i] = sum;
            }
        } else {
            #pragma omp for schedule(guided)
            for (int i = 0; i < height_; ++i)
            {
                double sum = 0.0;
                for (int j = 0; j < width_; ++j)
                    sum += data[i*width_ + j] * x[j];
                res[i] = sum;
            }
        }
    }
    else if (schedule_type == "auto") {
        #pragma omp for schedule(auto)
        for (int i = 0; i < height_; ++i)
        {
            double sum = 0.0;
            for (int j = 0; j < width_; ++j)
                sum += data[i*width_ + j] * x[j];
            res[i] = sum;
        }
    }
    else if (schedule_type == "runtime") {
        #pragma omp for schedule(runtime)
        for (int i = 0; i < height_; ++i)
        {
            double sum = 0.0;
            for (int j = 0; j < width_; ++j)
                sum += data[i*width_ + j] * x[j];
            res[i] = sum;
        }
    }
    else { // По умолчанию - static
        #pragma omp for schedule(static)
        for (int i = 0; i < height_; ++i)
        {
            double sum = 0.0;
            for (int j = 0; j < width_; ++j)
                sum += data[i*width_ + j] * x[j];
            res[i] = sum;
        }
    }
    
}
void Matrix::MulVecAdd(std::vector<double> &res, 
                                      std::vector<double> &x, 
                                      std::vector<double> &add, 
                                      double cof,
                                      const std::string& schedule_type,
                                      int chunk_size)
{
    if (schedule_type == "static") {
        if (chunk_size > 0) {
            #pragma omp for schedule(static, chunk_size)
            for (int i = 0; i < height_; ++i)
            {
                double sum = 0.0;
                for (int j = 0; j < width_; ++j)
                    sum += data[i*width_ + j] * x[j];
                res[i] = sum + add[i] * cof;
            }
        } else {
            #pragma omp for schedule(static)
            for (int i = 0; i < height_; ++i)
            {
                double sum = 0.0;
                for (int j = 0; j < width_; ++j)
                    sum += data[i*width_ + j] * x[j];
                res[i] = sum + add[i] * cof;
            }
        }
    }
    else if (schedule_type == "dynamic") {
        if (chunk_size > 0) {
            #pragma omp for schedule(dynamic, chunk_size)
            for (int i = 0; i < height_; ++i)
            {
                double sum = 0.0;
                for (int j = 0; j < width_; ++j)
                    sum += data[i*width_ + j] * x[j];
                res[i] = sum + add[i] * cof;
            }
        } else {
            #pragma omp for schedule(dynamic)
            for (int i = 0; i < height_; ++i)
            {
                double sum = 0.0;
                for (int j = 0; j < width_; ++j)
                    sum += data[i*width_ + j] * x[j];
                res[i] = sum + add[i] * cof;
            }
        }
    }
    else if (schedule_type == "guided") {
        if (chunk_size > 0) {
            #pragma omp for schedule(guided, chunk_size)
            for (int i = 0; i < height_; ++i)
            {
                double sum = 0.0;
                for (int j = 0; j < width_; ++j)
                    sum += data[i*width_ + j] * x[j];
                res[i] = sum + add[i] * cof;
            }
        } else {
            #pragma omp for schedule(guided)
            for (int i = 0; i < height_; ++i)
            {
                double sum = 0.0;
                for (int j = 0; j < width_; ++j)
                    sum += data[i*width_ + j] * x[j];
                res[i] = sum + add[i] * cof;
            }
        }
    }
    else if (schedule_type == "auto") {
        #pragma omp for schedule(auto)
        for (int i = 0; i < height_; ++i)
        {
            double sum = 0.0;
            for (int j = 0; j < width_; ++j)
                sum += data[i*width_ + j] * x[j];
            res[i] = sum + add[i] * cof;
        }
    }
    else if (schedule_type == "runtime") {
        #pragma omp for schedule(runtime)
        for (int i = 0; i < height_; ++i)
        {
            double sum = 0.0;
            for (int j = 0; j < width_; ++j)
                sum += data[i*width_ + j] * x[j];
            res[i] = sum + add[i] * cof;
        }
    }
    else { // По умолчанию - static
        #pragma omp for schedule(static)
        for (int i = 0; i < height_; ++i)
        {
            double sum = 0.0;
            for (int j = 0; j < width_; ++j)
                sum += data[i*width_ + j] * x[j];
            res[i] = sum + add[i] * cof;
        }
    }
}