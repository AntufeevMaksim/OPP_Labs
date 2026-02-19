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

#include <mpe.h>
#include <sys/resource.h>

int EVENT_COMP_START, EVENT_COMP_END;
int EVENT_COMM_START, EVENT_COMM_END;
int EVENT_ITER_START, EVENT_ITER_END;

int rank, size, N;
const double eps = 1e-7;
int rows, rem;

long get_memory_usage()
{
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    return usage.ru_maxrss; // KB
}

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
    MPE_Log_event(EVENT_COMP_END, 0, NULL);
    MPE_Log_event(EVENT_COMM_START, 0, NULL);
    MPI_Allreduce(&norm_local, &norm, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
    MPE_Log_event(EVENT_COMM_END, 0, NULL);
    MPE_Log_event(EVENT_COMP_START, 0, NULL);
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

        MPE_Log_event(EVENT_COMP_END, 0, NULL);
        MPE_Log_event(EVENT_COMM_START, 0, NULL);
        MPI_Sendrecv_replace(vec.data(), vec.size(), MPI_DOUBLE,
                             (rank - 1 + size) % size, 0, // source
                             (rank + 1) % size, 0,        // dest
                             MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPE_Log_event(EVENT_COMM_END, 0, NULL);
        MPE_Log_event(EVENT_COMP_START, 0, NULL);
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

    MPE_Log_get_state_eventIDs(&EVENT_COMP_START, &EVENT_COMP_END);
    MPE_Describe_state(EVENT_COMP_START, EVENT_COMP_END,
                       "COMPUTE", "red");

    MPE_Log_get_state_eventIDs(&EVENT_COMM_START, &EVENT_COMM_END);
    MPE_Describe_state(EVENT_COMM_START, EVENT_COMM_END,
                       "COMM", "blue");

    MPE_Log_get_state_eventIDs(&EVENT_ITER_START, &EVENT_ITER_END);
    MPE_Describe_state(EVENT_ITER_START, EVENT_ITER_END,
                       "ITER", "green");

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    init_num(N, argv[1]);

    rows = N / size;
    rem = N % size;
    int start_h = rank * rows + std::min(rank, rem);

    std::vector<double> xn(rows + 1, 0.0);
    std::vector<double> yn(rows + 1, 0.0);
    std::vector<double> Ayn(rows + 1, 0.0);

    Matrix A;
    std::vector<double> b;
    init_mat_vec2(A, b, argv[2], N, start_h, get_height(rank), rows);

    double norm_b = calc_dot(b, b, get_height(rank));
    double t0 = MPI_Wtime();
    int iter = 0;

    while (iter < 10000)
    {
        MPE_Log_event(EVENT_ITER_START, 0, NULL);
        MPE_Log_event(EVENT_COMP_START, 0, NULL);
        // yn = A*xn
        mul_part_vec_mat(yn, A, xn, get_height(rank));

        // yn = A*xn - b
        vec_sum(yn, yn, b, get_height(rank), -1.0);

        double norm_y = calc_dot(yn, yn, get_height(rank));

        if (norm_y / norm_b < eps * eps)
            MPE_Log_event(EVENT_COMP_END, 0, NULL);
            MPE_Log_event(EVENT_ITER_END, 0, NULL);
            break;

        // tau
        mul_part_vec_mat(Ayn, A, yn, get_height(rank));

        double num = calc_dot(Ayn, yn, get_height(rank));
        double den = calc_dot(Ayn, Ayn, get_height(rank));
        double tau = num / den;

        // x(n+1) = xn - tau*yn
        vec_sum(xn, xn, yn, get_height(rank), -tau);

        iter++;
        MPE_Log_event(EVENT_COMP_END, 0, NULL);
        MPE_Log_event(EVENT_ITER_END, 0, NULL);
    }
    long local_mem = get_memory_usage();
    long total_mem = 0;

    MPI_Reduce(&local_mem, &total_mem,
               1, MPI_LONG, MPI_SUM,
               0, MPI_COMM_WORLD);
    double t1 = MPI_Wtime();

    if (rank == 0)
    {
        write_info(iter, t1 - t0, 2, size, total_mem);
    }

    write_res(xn, get_height(rank));
    MPI_Finalize();
    return 0;
}
