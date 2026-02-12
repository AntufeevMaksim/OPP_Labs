#include <mpi.h>
#include <vector>
#include <cmath>
#include <iostream>
#include <algorithm>
#include <iostream>
#include "matrix.hpp"
#include "first_matrix.hpp"
#include "common.hpp"
#include <fstream>
#include <mpe.h>
#include <sys/resource.h>

long get_memory_usage()
{
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    return usage.ru_maxrss; // KB
}

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
    for (int i = start; i < start + count; ++i)
        s += a[i] * b[i];
    return s;
}

static inline void vec_sum(std::vector<double> &result, std::vector<double> &vec1, std::vector<double> &vec2, int start_h, int height, double cof = 1)
{
    for (int i = start_h; i < start_h + height; ++i)
        result[i] = vec1[i] + cof * vec2[i];
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int EVENT_COMP_START, EVENT_COMP_END;
    int EVENT_COMM_START, EVENT_COMM_END;
    int EVENT_ITER_START, EVENT_ITER_END;

    MPE_Log_get_state_eventIDs(&EVENT_COMP_START, &EVENT_COMP_END);
    MPE_Describe_state(EVENT_COMP_START, EVENT_COMP_END,
                       "COMPUTE", "red");

    MPE_Log_get_state_eventIDs(&EVENT_COMM_START, &EVENT_COMM_END);
    MPE_Describe_state(EVENT_COMM_START, EVENT_COMM_END,
                       "COMM", "blue");

    MPE_Log_get_state_eventIDs(&EVENT_ITER_START, &EVENT_ITER_END);
    MPE_Describe_state(EVENT_ITER_START, EVENT_ITER_END,
                       "ITER", "green");
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int N;
    init_N(N, argv[1]);
    const double eps = 1e-5;

    std::vector<double> xn(N, 0.0);
    std::vector<double> yn(N, 0.0);
    std::vector<double> Ayn(N, 0.0);

    int rows = N / size;
    int rem = N % size;
    int start_h = rank * rows + std::min(rank, rem);
    int height = rows + (rank < rem ? 1 : 0);

    Matrix A;
    std::vector<double> b;
    init_mat_vec1(A, b, argv[2], N, start_h, height, rows);

    double norm_b = dot(b, b, 0, N);

    double t0 = MPI_Wtime();
    int iter = 0;

    while (iter < 100000)
    {

        MPE_Log_event(EVENT_ITER_START, 0, NULL);

        // y = A*x - b
        MPE_Log_event(EVENT_COMP_START, 0, NULL);

        yn = A.MulFullVec(xn);
        vec_sum(yn, yn, b, start_h, height, -1.0);

        MPE_Log_event(EVENT_COMP_END, 0, NULL);

        // COMMUNICATION
        MPE_Log_event(EVENT_COMM_START, 0, NULL);

        MPI_Allreduce(MPI_IN_PLACE, yn.data(), N,
                      MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);

        MPE_Log_event(EVENT_COMM_END, 0, NULL);

        MPE_Log_event(EVENT_COMP_START, 0, NULL);
        double norm_y = dot(yn, yn, 0, N);
        MPE_Log_event(EVENT_COMP_END, 0, NULL);

        if (norm_y / norm_b < eps * eps)
            break;

        MPE_Log_event(EVENT_COMP_START, 0, NULL);
        // tau
        Ayn = A.MulFullVec(yn); // Ayn = A*yn
        MPE_Log_event(EVENT_COMP_END, 0, NULL);

        MPE_Log_event(EVENT_COMM_START, 0, NULL);
        MPI_Allreduce(MPI_IN_PLACE, Ayn.data(), N, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
        MPE_Log_event(EVENT_COMM_END, 0, NULL);

        MPE_Log_event(EVENT_COMP_START, 0, NULL);
        double num = dot(Ayn, yn, 0, N);
        double den = dot(Ayn, Ayn, 0, N);
        double tau = num / den;

        // x(n+1) = xn - tau*yn
        vec_sum(xn, xn, yn, 0, N, -tau);

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
        write_info(iter, t1 - t0, 1, size, total_mem);
    }
    write_res(xn);
    MPI_Finalize();
    return 0;
}
