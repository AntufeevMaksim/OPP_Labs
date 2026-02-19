#pragma once

#include <vector>
#include <string>
#include "matrix_getter.hpp"

class Matrix
{
private:
    int st_h_;
    int st_w_;
    int width_;
    int height_;
    std::vector<double> data;
public:
    Matrix() = default;
    Matrix(int st_h,
         int w, int h,
        IMatrixGetter& getter);

    double get(int i, int j);

    std::vector<double> MulFullVec(std::vector<double> &x,
                                       const std::string& schedule_type,
                                       int chunk_size = 0);
};
