#include <mpi.h>
#include <vector>
#include <cmath>
#include <iostream>
#include <algorithm>
#include <iostream>
#include "matrix.hpp"
#include "first_matrix.hpp"
#include "common.hpp"

double dot(const std::vector<double> &a,
           const std::vector<double> &b,
           int start, int count)
{
    double s = 0.0;
    for (int i = start; i < start + count; ++i)
        s += a[i] * b[i];
    return s;
}

static inline void vec_sum(std::vector<double> &result, std::vector<double> &vec1, std::vector<double> &vec2, int start_h, int height, double cof = 1)
{
    std::transform(vec1.begin() + start_h, vec1.begin() + start_h + height, vec2.begin() + start_h,
                   result.begin() + start_h, [cof](double a, double b)
                   { return a + cof * b; });
}
static inline double calc_dot(std::vector<double> &vec1, std::vector<double> &vec2, int start, int count)
{
    double norm_local = dot(vec1, vec2, start, count);
    double norm;
    MPI_Allreduce(&norm_local, &norm, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
    return norm;
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int rank, total;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &total);

    int N;
    init_N(N, argv[1]);
    const double eps = 1e-5;

    std::vector<double> xn(N, 0.0);
    std::vector<double> yn(N, 0.0);
    std::vector<double> Ayn(N, 0.0);



    int rows = N / total;
    int rem = N % total;
    int start_h = rank * rows + std::min(rank, rem);
    int height = rows + (rank < rem ? 1 : 0);

    Matrix A;
    std::vector<double> b;
    init_mat_vec1(A, b, argv[2], N, start_h, height, rows);


    double norm_b = std::sqrt(calc_dot(b, b, start_h, height));

    double t0 = MPI_Wtime();
    int iter = 0;

    while (iter < 1000)
    {
        // y = A*x - b
        yn = A.MulFullVec(xn);
        vec_sum(yn, yn, b, start_h, height, -1.0);
        MPI_Allreduce(MPI_IN_PLACE, yn.data(), N, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);


        double norm_y = std::sqrt(calc_dot(yn, yn, start_h, height));

        if (norm_y / norm_b < eps)
            break;

        // tau
        Ayn = A.MulFullVec(yn); // Ayn = A*yn
        MPI_Allreduce(MPI_IN_PLACE, Ayn.data(), N, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);

        double num = calc_dot(Ayn, yn, start_h, height);
        double den = calc_dot(Ayn, Ayn, start_h, height);
        double tau = num / den;

        // x(n+1) = xn - tau*yn
        vec_sum(xn, xn, yn, start_h, height, -tau);
        MPI_Allreduce(MPI_IN_PLACE, xn.data(), N, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);

        iter++;
    }

    double t1 = MPI_Wtime();

    if (rank == 0)
    {
        std::cout << "Iterations: " << iter << "\n";
        std::cout << "Time: " << (t1 - t0) << " s\n";
        std::cout << "x[0] = " << xn[0] << " "<< xn[1] << " " << xn[2] << "\n";
    }

    MPI_Finalize();
    return 0;
}
