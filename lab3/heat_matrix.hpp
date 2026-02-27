#pragma once

#include "matrix_getter.hpp"

class HeatMatrix : public IMatrixGetter
{
public:
    HeatMatrix(int Nx, int Ny);

    double get(int i, int j) override;
    
private:
    int Nx_;
    int Ny_;
    int N_;
};