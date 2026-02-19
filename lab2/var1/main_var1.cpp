#include <vector>
#include <cmath>
#include <iostream>
#include <algorithm>
#include <iostream>
#include "matrix.hpp"
#include "first_matrix.hpp"
#include "common.hpp"
#include <fstream>
#include <chrono>
#include <omp.h>
void write_res(std::vector<double> &data)
{
    std::fstream out_file("result_vec.txt", std::ios::out | std::ios::trunc);

    for (int i = 0; i < data.size(); ++i)
    {
        out_file << data[i] << '\n';
    }
    out_file.close();
}

double dot(const std::vector<double> &a,
           const std::vector<double> &b,
           int start, int count)
{
    double s = 0.0;
    #pragma omp parallel for reduction(+:s)
    for (int i = start; i < start + count; ++i)
        s += a[i] * b[i];
    return s;
}

static inline void vec_sum(std::vector<double> &result, std::vector<double> &vec1, std::vector<double> &vec2, int start_h, int height, double cof = 1)
{
    #pragma omp parallel for
    for (int i = start_h; i < start_h + height; ++i)
        result[i] = vec1[i] + cof * vec2[i];
}

int main(int argc, char **argv)
{
    int num_threads;
    init_num(num_threads, argv[1]);
    omp_set_num_threads(num_threads);
    int N;
    init_num(N, argv[2]);
    const double eps = 1e-5;

    std::vector<double> xn(N, 0.0);
    std::vector<double> yn(N, 0.0);
    std::vector<double> Ayn(N, 0.0);

    Matrix A;
    std::vector<double> b;
    init_mat_vec1(A, b, argv[3], N, 0, N, N);

    double norm_b = dot(b, b, 0, N);

    int iter = 0;
    auto start = std::chrono::high_resolution_clock::now();
    while (iter < 100000)
    {

        yn = A.MulFullVec(xn);
        vec_sum(yn, yn, b, 0, N, -1.0);

        double norm_y = dot(yn, yn, 0, N);

        if (norm_y / norm_b < eps * eps)
            break;

        Ayn = A.MulFullVec(yn); // Ayn = A*yn


        double num = dot(Ayn, yn, 0, N);
        double den = dot(Ayn, Ayn, 0, N);
        double tau = num / den;

        // x(n+1) = xn - tau*yn
        vec_sum(xn, xn, yn, 0, N, -tau);

        iter++;
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    write_info(iter, (double) duration.count() / 1e6, 1, num_threads, -1);
    write_res(xn);
    return 0;
}
