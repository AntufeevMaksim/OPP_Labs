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

double norm_y;

void write_res(std::vector<double> &data)
{
    std::fstream out_file("result_vec.txt", std::ios::out | std::ios::trunc);

    for (int i = 0; i < data.size(); ++i)
    {
        out_file << data[i] << '\n';
    }
    out_file.close();
}

double sum; // глобальная переменная (лучше сделать локальной для параллельной области)

void dot(double *res,
         const std::vector<double> &a,
         const std::vector<double> &b,
         int count,
         const std::string &schedule_type = "static",
         int chunk_size = 0)
{
#pragma omp single
    sum = 0.0;
#pragma omp barrier

    if (schedule_type == "static")
    {
        if (chunk_size > 0)
        {
#pragma omp for reduction(+ : sum) schedule(static, chunk_size)
            for (int i = 0; i < count; ++i)
                sum += a[i] * b[i];
        }
        else
        {
#pragma omp for reduction(+ : sum) schedule(static)
            for (int i = 0; i < count; ++i)
                sum += a[i] * b[i];
        }
    }
    else if (schedule_type == "dynamic")
    {
        if (chunk_size > 0)
        {
#pragma omp for reduction(+ : sum) schedule(dynamic, chunk_size)
            for (int i = 0; i < count; ++i)
                sum += a[i] * b[i];
        }
        else
        {
#pragma omp for reduction(+ : sum) schedule(dynamic)
            for (int i = 0; i < count; ++i)
                sum += a[i] * b[i];
        }
    }
    else if (schedule_type == "guided")
    {
        if (chunk_size > 0)
        {
#pragma omp for reduction(+ : sum) schedule(guided, chunk_size)
            for (int i = 0; i < count; ++i)
                sum += a[i] * b[i];
        }
        else
        {
#pragma omp for reduction(+ : sum) schedule(guided)
            for (int i = 0; i < count; ++i)
                sum += a[i] * b[i];
        }
    }
    else if (schedule_type == "auto")
    {
#pragma omp for reduction(+ : sum) schedule(auto)
        for (int i = 0; i < count; ++i)
            sum += a[i] * b[i];
    }
    else if (schedule_type == "runtime")
    {
#pragma omp for reduction(+ : sum) schedule(runtime)
        for (int i = 0; i < count; ++i)
            sum += a[i] * b[i];
    }
    else
    { // По умолчанию - static
#pragma omp for reduction(+ : sum) schedule(static)
        for (int i = 0; i < count; ++i)
            sum += a[i] * b[i];
    }

#pragma omp single
    *res = sum;
}

static inline void vec_sum(std::vector<double> &result,
                           std::vector<double> &vec1,
                           std::vector<double> &vec2,
                           int height,
                           double cof = 1,
                           const std::string &schedule_type = "static",
                           int chunk_size = 0)
{
    if (schedule_type == "static")
    {
        if (chunk_size > 0)
        {
#pragma omp for schedule(static, chunk_size)
            for (int i = 0; i < height; ++i)
                result[i] = vec1[i] + cof * vec2[i];
        }
        else
        {
#pragma omp for schedule(static)
            for (int i = 0; i < height; ++i)
                result[i] = vec1[i] + cof * vec2[i];
        }
    }
    else if (schedule_type == "dynamic")
    {
        if (chunk_size > 0)
        {
#pragma omp for schedule(dynamic, chunk_size)
            for (int i = 0; i < height; ++i)
                result[i] = vec1[i] + cof * vec2[i];
        }
        else
        {
#pragma omp for schedule(dynamic)
            for (int i = 0; i < height; ++i)
                result[i] = vec1[i] + cof * vec2[i];
        }
    }
    else if (schedule_type == "guided")
    {
        if (chunk_size > 0)
        {
#pragma omp for schedule(guided, chunk_size)
            for (int i = 0; i < height; ++i)
                result[i] = vec1[i] + cof * vec2[i];
        }
        else
        {
#pragma omp for schedule(guided)
            for (int i = 0; i < height; ++i)
                result[i] = vec1[i] + cof * vec2[i];
        }
    }
    else if (schedule_type == "auto")
    {
#pragma omp for schedule(auto)
        for (int i = 0; i < height; ++i)
            result[i] = vec1[i] + cof * vec2[i];
    }
    else if (schedule_type == "runtime")
    {
#pragma omp for schedule(runtime)
        for (int i = 0; i < height; ++i)
            result[i] = vec1[i] + cof * vec2[i];
    }
    else
    { // По умолчанию - static
#pragma omp for schedule(static)
        for (int i = 0; i < height; ++i)
            result[i] = vec1[i] + cof * vec2[i];
    }
}
int main(int argc, char **argv)
{
    std::string shedule_type = "guided";
    int chunk_size = 1024;
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
    init_mat_vec1(A, b, argv[3], N, N, N);

    double norm_b;
    dot(&norm_b, b, b, N);

    int iter = 0;

    double norm_y;
    double num, den;
    auto start = std::chrono::high_resolution_clock::now();
#pragma omp parallel
    {
        while (true)
        {

            A.MulVecAdd(yn, xn, b, -1.0, shedule_type, chunk_size); // yn = A*xn - b

            dot(&norm_y, yn, yn, N, shedule_type, chunk_size);

            bool stop = std::sqrt(norm_y / norm_b) < eps;

#pragma omp barrier
            if (stop)
                break;

            A.MulVec(Ayn, yn, shedule_type, chunk_size); // Ayn = A*yn

            dot(&num, Ayn, yn, N, shedule_type, chunk_size);
            dot(&den, Ayn, Ayn, N, shedule_type, chunk_size);

            double tau = num / den;

            // x(n+1) = xn - tau*yn
            vec_sum(xn, xn, yn, N, -tau, shedule_type, chunk_size);

#pragma omp single
            iter++;
#pragma omp barrier
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    write_info(iter, (double)duration.count() / 1e6, 2, num_threads, -1);
    write_res(xn);
    return 0;
}
