#pragma once

#include <vector>
#include "matrix_getter.hpp"
#include <string>
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

    void MulVec(std::vector<double> &res, 
                                   std::vector<double> &x,
                                   const std::string& schedule_type,
                                   int chunk_size = 0);
    void MulVecAdd(std::vector<double> &res, 
                                      std::vector<double> &x, 
                                      std::vector<double> &add, 
                                      double cof,
                                      const std::string& schedule_type,
                                      int chunk_size = 0);
};
