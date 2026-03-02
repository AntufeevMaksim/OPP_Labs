#include <mpi.h>
#include "common.hpp"
#include "main.h"

int rank, size;

struct ProcessInfo
{
    int id;
    int x, y;
};

struct ProgramInfo
{
    int root;
    int n1, n2, n3;
    int sx, sy;
    int col_per_proc, row_per_proc;
    int col_per_root, row_per_root;
};

double *init_mat_A(int h, int w)
{
    double *A = (double *)malloc(h * w * sizeof(double /*  */));

    for (int i = 0; i < h; i++)
        for (int j = 0; j < w; j++)
            A[i * w + j] = i + 1;
    return A;
}

double *init_mat_B(int h, int w)
{
    double *B = (double *)malloc(h * w * sizeof(double));

    for (int i = 0; i < h; i++)
        for (int j = 0; j < w; j++)
            B[i * w + j] = j + 1;

    return B;
}

double *mul_part_mat(ProgramInfo *info,
                     ProcessInfo *proc_info,
                     double *local_A,
                     double *local_B)
{
    int cols, rows;
    if (proc_info->x == 0)
    {
        cols = info->col_per_root;
    }
    else
    {
        cols = info->col_per_proc;
    }

    if (proc_info->y == 0)
    {
        rows = info->row_per_root;
    }
    else
    {
        rows = info->row_per_proc;
    }

    double *local_C = (double *)malloc(cols * rows * sizeof(double));

    for (int r = 0; r < rows; ++r)
    {
        for (int c = 0; c < cols; ++c)
        {
            double sum = 0.0;

            for (int k = 0; k < info->n2; ++k)
            {
                sum +=
                    local_A[r * info->n2 + k] *
                    local_B[k * cols + c];
            }

            local_C[r * cols + c] = sum;
        }
    }

    return local_C;
}
void send_B(ProcessInfo &proc, ProgramInfo &info, int root, double *B, int root_cols, double *local_B, MPI_Comm row_comm, MPI_Comm col_comm)
{
    if (proc.y == 0)
    {
        MPI_Datatype tmp_type, send_cols_type;

        MPI_Type_vector(
            info.n2,
            info.col_per_proc,
            info.n3,
            MPI_DOUBLE,
            &tmp_type);

        MPI_Type_create_resized(
            tmp_type,
            0,
            info.col_per_proc * sizeof(double),
            &send_cols_type);

        MPI_Type_commit(&send_cols_type);
        MPI_Type_free(&tmp_type);

        int *counts;
        int *displs;
        counts = (int *)malloc(info.sx * sizeof(int));
        displs = (int *)malloc(info.sx * sizeof(int));
        counts[0] = 0;
        displs[0] = 0;
        for (int i = 1; i < info.sx; ++i)
        {
            counts[i] = 1;
            displs[i] = i;
        }
        if (rank == root)
        {
            MPI_Scatterv(B, counts, displs, send_cols_type, local_B, 0, MPI_DOUBLE, root, row_comm);
        }
        else
        {
            MPI_Scatterv(B, counts, displs, send_cols_type, local_B, info.col_per_proc * info.n2, MPI_DOUBLE, root, row_comm);
        }
        MPI_Type_free(&send_cols_type);
    }
    else
    {
        if (proc.x == 0)
        {
            MPI_Bcast(local_B, info.col_per_root * info.n2, MPI_DOUBLE, root, col_comm);
        }
        else
        {
            MPI_Bcast(local_B, info.col_per_proc * info.n2, MPI_DOUBLE, root, col_comm);
        }
    }
}

void send_A(ProcessInfo &proc, int root, ProgramInfo &info, double *A, double *local_A, MPI_Comm col_comm, MPI_Comm row_comm)
{
    if (proc.x == 0)
    {
        int *counts;
        int *displs;
        counts = (int *)malloc(info.sy * sizeof(int));
        displs = (int *)malloc(info.sy * sizeof(int));
        counts[0] = 0;
        displs[0] = 0;
        for (int i = 1; i < info.sy; ++i)
        {
            counts[i] = info.row_per_proc * info.n2;
            displs[i] = info.row_per_proc * info.n2 + displs[i - 1];
        }
        if (rank == root)
        {

            MPI_Scatterv(A + info.row_per_root * info.n2, counts, displs, MPI_DOUBLE, local_A, 0, MPI_DOUBLE, root, col_comm);

            memcpy(local_A, A, info.row_per_root * info.n2 * sizeof(double));
        }
        else
        {
            MPI_Scatterv(A + info.row_per_root * info.n2, counts, displs, MPI_DOUBLE, local_A, info.row_per_proc * info.n2, MPI_DOUBLE, root, col_comm);
        }
    }
    else
    {
        if (proc.y == 0)
        {
            MPI_Bcast(local_A, info.n2 * info.col_per_root, MPI_DOUBLE, root, row_comm);
        }
        else
        {
            MPI_Bcast(local_A, info.n2 * info.col_per_proc, MPI_DOUBLE, root, row_comm);
        }
    }
}

void send_C(ProgramInfo &info,
            int root,
            double *local_C,
            double *C,
            MPI_Comm comm_2d,
            ProcessInfo &proc)
{
    int world_size;
    MPI_Comm_size(comm_2d, &world_size);

    int local_rows = (proc.y == 0)
                         ? info.row_per_root
                         : info.row_per_proc;

    int local_cols = (proc.x == 0)
                         ? info.col_per_root
                         : info.col_per_proc;

    if (rank == root)
    {
        for (int p = 0; p < world_size; ++p)
        {
            int coords[2];
            MPI_Cart_coords(comm_2d, p, 2, coords);

            int rows = (coords[1] == 0)
                           ? info.row_per_root
                           : info.row_per_proc;

            int cols = (coords[0] == 0)
                           ? info.col_per_root
                           : info.col_per_proc;

            int row_start = 0;
            for (int i = 0; i < coords[1]; ++i)
                row_start += (i == 0)
                                 ? info.row_per_root
                                 : info.row_per_proc;

            int col_start = 0;
            for (int i = 0; i < coords[0]; ++i)
                col_start += (i == 0)
                                 ? info.col_per_root
                                 : info.col_per_proc;

            MPI_Datatype subarray;
            MPI_Type_vector(rows, cols, info.n2, MPI_DOUBLE, &subarray);
            MPI_Type_commit(&subarray);

            if (p == root)
            {
                memcpy(&C[row_start * info.n3 + col_start],
                       local_C,
                       rows * cols * sizeof(double));
            }
            else
            {
                MPI_Recv(C + row_start*info.n3 + col_start, 1, subarray, p, 0, comm_2d, MPI_STATUS_IGNORE);
            }

            MPI_Type_free(&subarray);
        }
    }
    else
    {
        MPI_Send(local_C,
                 local_rows * local_cols,
                 MPI_DOUBLE,
                 root,
                 0,
                 comm_2d);
    }
}

int main(int argc, char **argv)
{

    int root_cols, root_rows;
    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    ProgramInfo info;
    int N = init_N(argv[1]);
    auto [sx, sy] = computeGridSize(N);
    int dims[2] = {sx, sy};
    int periods[2] = {0, 0};

    info.sx = sx;
    info.sy = sy;

    MPI_Comm comm_2d;
    MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, 0, &comm_2d);

    int coords[2];
    MPI_Cart_coords(comm_2d, rank, 2, coords);

    ProcessInfo proc;
    proc.x = coords[0];
    proc.y = coords[1];

    MPI_Comm row_comm;
    MPI_Comm_split(comm_2d, proc.x, proc.y, &row_comm);

    MPI_Comm col_comm;
    MPI_Comm_split(comm_2d, proc.y, proc.x, &col_comm);

    int root_coords[2] = {0, 0};
    int root;
    MPI_Cart_rank(comm_2d, root_coords, &root);
    info.n1 = 10;
    info.n2 = 10;
    info.n3 = 10;

    info.row_per_proc = info.n1 / info.sy;
    info.col_per_proc = info.n3 / info.sx;

    info.row_per_root = info.n1 - info.row_per_proc * (info.sy - 1);
    info.col_per_root = info.n3 - info.col_per_proc * (info.sx - 1);

    double *A; // n1 x n2
    double *B; // n2 x n3
    double *C; // n1 x n3
    if (rank == root)
    {
        A = init_mat_A(info.n1, info.n2);
        B = init_mat_B(info.n2, info.n3);
        C = (double *)malloc(info.n1 * info.n3 * sizeof(double));
    }
    printf("init proc: %d\n", rank);
    int A_size = proc.y == 0 ? info.row_per_root * info.n2 : info.row_per_proc * info.n2;
    double *local_A = (double *)malloc(A_size * sizeof(double));
    send_A(proc, root, info, A, local_A, col_comm, row_comm);
    if (rank == 1)
    {
        write_mat(local_A, 5, 10);
    }
    fflush(stdout);
    int B_size = proc.y == 0 ? info.col_per_root * info.n2 : info.col_per_proc * info.n2;
    double *local_B = (double *)malloc(B_size * sizeof(double));
    send_B(proc, info, root, B, root_cols, local_B, row_comm, col_comm);
    printf("recv B proc: %d\n", rank);
    fflush(stdout);
    double *local_C = mul_part_mat(&info, &proc, local_A, local_B);

    send_C(info, root, local_C, C, comm_2d, proc);
    printf("recv C proc: %d\n", rank);
    fflush(stdout);
    if (rank == 3)
    {
//        write_mat(C, info.n1, info.n3);
//        write_mat(local_C, info.row_per_proc, info.col_per_proc);
    }

    fflush(stdout);
    MPI_Finalize();
}
