#include "heat_matrix.hpp"

HeatMatrix::HeatMatrix(int Nx, int Ny)
    : Nx_(Nx), Ny_(Ny), N_(Nx * Ny)
{
}

double HeatMatrix::get(int i, int j)
{
    if (i < 0 || i >= N_ || j < 0 || j >= N_)
        return 0.0;

    // координаты узла i
    int iy = i / Nx_;
    int ix = i % Nx_;

    // координаты узла j
    int jy = j / Nx_;
    int jx = j % Nx_;

    // диагональ
    if (i == j)
        return -4.0;

    // левый сосед
    if (iy == jy && jx == ix - 1)
        return 1.0;

    // правый сосед
    if (iy == jy && jx == ix + 1)
        return 1.0;

    // верхний сосед
    if (jx == ix && jy == iy - 1)
        return 1.0;

    // нижний сосед
    if (jx == ix && jy == iy + 1)
        return 1.0;

    return 0.0;
}