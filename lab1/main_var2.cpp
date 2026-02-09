#include <mpi.h>
#include <vector>
#include <cmath>
#include <iostream>
#include <algorithm>
#include <iostream>
#include "matrix.hpp"
#include "first_matrix.hpp"
#include "heat_matrix.hpp"
#include "common.hpp"
#include <fstream>
#include <cstring>

int rank, size, N;
const double eps = 1e-5;
int rows, rem;

double dot(const std::vector<double> &a,
           const std::vector<double> &b,
           int count)
{
    double s = 0.0;
    for (int i = 0; i < count; ++i)
        s += a[i] * b[i];
    return s;
}

static inline void vec_sum(std::vector<double> &result, std::vector<double> &vec1, std::vector<double> &vec2, int height, double cof = 1)
{
    std::transform(vec1.begin(), vec1.begin() + height, vec2.begin(),
                   result.begin(), [cof](double a, double b)
                   { return a + cof * b; });
}
static inline double calc_dot(std::vector<double> &vec1, std::vector<double> &vec2, int height)
{
    double norm_local = dot(vec1, vec2, height);
    double norm;
    MPI_Allreduce(&norm_local, &norm, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
    return norm;
}

static inline void mul_part_vec_mat(std::vector<double> &res, Matrix &mat, std::vector<double> &vec, int height)
{
    int curr = rank;
    res.assign(res.size(), 0.0);
    for (int i = 0; i < size; ++i)
    {
        // y = A*x - b
        int start_h = curr * rows + std::min(curr, rem);
        int height = rows + (curr < rem ? 1 : 0);

        std::vector<double> tmp = mat.MulPartVec(vec, start_h, height);
        vec_sum(res, res, tmp, res.size());

        MPI_Sendrecv_replace(vec.data(), vec.size(), MPI_DOUBLE,
                             (rank - 1 + size) % size, 0, // source
                             (rank + 1) % size, 0,        // dest
                             MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        curr = (curr + 1) % size;
    }
}
static inline int calc_norm(std::vector<double> &vec, int height)
{
    return std::sqrt(calc_dot(vec, vec, height));
}

static inline int get_height(int rank)
{
    return rows + (rank < rem ? 1 : 0);
}

void write_res(std::vector<double> &data, int data_size)
{
    if (rank == 0)
    {
        std::fstream out_file("result_vec.txt", std::ios::out | std::ios::trunc);

        for (int i = 0; i < data_size; ++i)
        {
            out_file << data[i] << '\n';
        }
        out_file.close();
    }
    for (int i = 1; i < size; ++i)
    {
        if (rank == i)
        {
            MPI_Recv(NULL, 0, MPI_INT, i - 1, 0,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            std::fstream out_file("result_vec.txt", std::ios::out | std::ios::app);

            for (int i = 0; i < data_size; ++i)
            {
                out_file << data[i] << '\n';
            }
            out_file.close();
        }
        else if (rank == i - 1)
        {
            MPI_Send(NULL, 0, MPI_INT, i, 0, MPI_COMM_WORLD);
        }
    }
    MPI_Barrier(MPI_COMM_WORLD);
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        std::cerr << "Wrong amount of args: " << argc << " argc must be 2" << std::endl;
    }

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    init_N(N, argv[1]);

    rows = N / size;
    rem = N % size;
    int start_h = rank * rows + std::min(rank, rem);

    std::vector<double> xn(rows + 1, 0.0);
    std::vector<double> yn(rows + 1, 0.0);
    std::vector<double> Ayn(rows + 1, 0.0);

    Matrix A;
    std::vector<double> b;
    init_mat_vec2(A, b, argv[2], N, start_h, get_height(rank), rows);




    double norm_b = calc_norm(b, get_height(rank));
    double t0 = MPI_Wtime();
    int iter = 0;

    while (iter < 1000000)
    {
        // yn = A*xn
        mul_part_vec_mat(yn, A, xn, get_height(rank));

        // yn = A*xn - b
        vec_sum(yn, yn, b, get_height(rank), -1.0);

        double norm_y = calc_norm(yn, get_height(rank));

        if (norm_y / norm_b < eps)
            break;

        // tau
        mul_part_vec_mat(Ayn, A, yn, get_height(rank));

        double num = calc_dot(Ayn, yn, get_height(rank));
        double den = calc_dot(Ayn, Ayn, get_height(rank));
        double tau = num / den;

        // x(n+1) = xn - tau*yn
        vec_sum(xn, xn, yn, get_height(rank), -tau);

        iter++;
    }

    double t1 = MPI_Wtime();

    if (rank == 0)
    {
        std::cout << "Iterations: " << iter << "\n";
        std::cout << "Time: " << (t1 - t0) << " s\n";
    }

    write_res(xn, get_height(rank));
    MPI_Finalize();
    return 0;
}
