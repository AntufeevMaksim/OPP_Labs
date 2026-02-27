#include <mpi.h>
#include "common.hpp"
#include "main.h"

int rank, size;

struct ProcessDataInfo
{
    int start; // все размеры в терминах строк, столбцов
    int size;
    int capacity;
};

void init_process_data(int data_size, int total_proc, int idx, ProcessDataInfo *data)
{
    int rows = data_size / total_proc;
    int rem = data_size % total_proc;
    data->start = idx * rows + std::min(idx, rem);
    data->size = rows + (idx < rem ? 1 : 0);
    data->capacity = rows + 1;
}

struct ProgramInfo
{
    int n1, n2, n3;
    int sx, sy;
    int x, y;
};

double *init_mat_A(int h, int w)
{
    double *A = (double *)malloc(h * w * sizeof(double));

    for (int i = 0; i < h; i++)
        for (int j = 0; j < w; j++)
            A[i * h + j] = i + 1;

    return A;
}

double *init_mat_B(int h, int w)
{
    double *B = (double *)malloc(h * w * sizeof(double));

    for (int i = 0; i < h; i++)
        for (int j = 0; j < w; j++)
            B[i * h + j] = j + 1;

    return B;
}

void send_A_row(double *A, double *local_A, ProgramInfo *info, MPI_Comm &comm_2d)
{
    for (int r = 0; r < info->sy; r++)
    {

        int coords_send[2] = {r, 0};
        int dest;
        MPI_Cart_rank(comm_2d, coords_send, &dest);

        ProcessDataInfo dest_info;
        init_process_data(info->n1, info->sy, r, &dest_info);
        if (dest == 0)
        {
            //write_mat(A, dest_info.size, info->n2);
            memcpy(local_A,
                   &A[dest_info.start * info->n2],
                   dest_info.size * info->n2 * sizeof(double));
            //write_mat(local_A, dest_info.size , info->n2);
            
        }
        else
        {
            //printf("send A to rank: %d\n", dest);
            MPI_Send(&A[dest_info.start * info->n2],
                     dest_info.capacity * info->n2,
                     MPI_DOUBLE,
                     dest,
                     0,
                     comm_2d);
        }
    }
}

void send_B_col(double *B, double *local_B, ProgramInfo *info, ProcessDataInfo *col_info, MPI_Comm comm_2d, MPI_Datatype col_block)
{
    for (int c = 0; c < info->sx; c++)
    {

        int coords_send[2] = {0, c};
        int dest;
        MPI_Cart_rank(comm_2d, coords_send, &dest);

        ProcessDataInfo dest_col_info;
        init_process_data(info->n3, info->sx, c, &dest_col_info);

        MPI_Type_vector(info->n2,
                        dest_col_info.size,
                        info->n3,
                        MPI_DOUBLE,
                        &col_block);

        MPI_Type_commit(&col_block);

        if (dest == 0)
        {
            write_mat(B, 10, 10);
            int idx = 0;
            for (int offset = col_info->start; offset < col_info->start + col_info->size; ++offset)
            {
                for (int line = 0; line < info->n2; ++line)
                {
                    local_B[idx] = B[line * info->n3 + offset];
                    ++idx;
                }
            }
        }
        else
        {

            MPI_Send(&B[dest_col_info.start],
                     1,
                     col_block,
                     dest,
                     1,
                     comm_2d);
        }
    }
}

void collect_c(ProcessDataInfo *rows_info,
               ProcessDataInfo *cols_info,
               ProgramInfo *info,
               int rank,
               double *local_C,
               double *C,
               MPI_Comm comm_2d)
{
    int world_size;
    MPI_Comm_size(comm_2d, &world_size);

    int *sendcounts  = (int*)calloc(world_size, sizeof(int));
    int *recvcounts  = (int*)calloc(world_size, sizeof(int));
    int *sdispls     = (int*)calloc(world_size, sizeof(int));
    int *rdispls     = (int*)calloc(world_size, sizeof(int));
    MPI_Datatype *sendtypes = (MPI_Datatype*)malloc(world_size * sizeof(MPI_Datatype));
    MPI_Datatype *recvtypes = (MPI_Datatype*)malloc(world_size * sizeof(MPI_Datatype));

    for (int i = 0; i < world_size; ++i)
    {
        sendtypes[i] = MPI_DOUBLE;
        recvtypes[i] = MPI_DOUBLE;
    }

    // -------------------------------------------------
    // Каждый процесс отправляет только root
    // -------------------------------------------------
    sendcounts[0] = rows_info->size * cols_info->size;
    sdispls[0]    = 0;

    // -------------------------------------------------
    // Root готовит subarray-типы
    // -------------------------------------------------
    if (rank == 0)
    {
        for (int r = 0; r < info->sx; ++r)
        {
            ProcessDataInfo row_info;
            init_process_data(info->n1, info->sx, r, &row_info);

            for (int c = 0; c < info->sy; ++c)
            {
                ProcessDataInfo col_info;
                init_process_data(info->n3, info->sy, c, &col_info);

                int coords[2] = {r, c};
                int src_rank;
                MPI_Cart_rank(comm_2d, coords, &src_rank);

                int sizes[2]    = {info->n1, info->n3};
                int subsizes[2] = {row_info.size, col_info.size};
                int starts[2]   = {row_info.start, col_info.start};

                MPI_Type_create_subarray(
                    2,
                    sizes,
                    subsizes,
                    starts,
                    MPI_ORDER_C,
                    MPI_DOUBLE,
                    &recvtypes[src_rank]);

                MPI_Type_commit(&recvtypes[src_rank]);

                recvcounts[src_rank] = 1;
            }
        }
    }

    MPI_Alltoallw(
        local_C,
        sendcounts,
        sdispls,
        sendtypes,
        C,
        recvcounts,
        rdispls,
        recvtypes,
        comm_2d
    );

    if (rank == 0)
    {
        for (int i = 0; i < world_size; ++i)
        {
            if (recvcounts[i] == 1)
                MPI_Type_free(&recvtypes[i]);
        }
    }

    free(sendcounts);
    free(recvcounts);
    free(sdispls);
    free(rdispls);
    free(sendtypes);
    free(recvtypes);
}


void mul_part_mat(ProgramInfo *info,
                  ProcessDataInfo *cols_info,
                  ProcessDataInfo *rows_info,
                  double *local_C,
                  double *local_A,
                  double *local_B)
{
    for (int c = 0; c < cols_info->size; ++c)
    {
        for (int r = 0; r < rows_info->size; ++r)
        {
            double sum = 0.0;

            for (int k = 0; k < info->n2; ++k)
            {
                sum +=
                    local_A[r * info->n2 + k] *
                    local_B[c * info->n2 + k];
            }

            local_C[c * rows_info->size + r] = sum;
        }
    }
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);


    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    ProgramInfo info;
    int N = init_N(argv[1]);
    auto [sx, sy] = computeGridSize(N);
    int dims[2] = {sx, sy};
    int periods[2] = {0, 0};

    if (rank == 0) printf("sx: %d sy: %d\n", sx, sy);
    info.sx = sx;
    info.sy = sy;

    MPI_Comm comm_2d;

    MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, 0, &comm_2d);

    int coords[2];
    MPI_Cart_coords(comm_2d, rank, 2, coords);

    info.x = coords[0];
    info.y = coords[1];

    MPI_Comm row_comm;
    MPI_Comm_split(comm_2d, info.x, info.y, &row_comm);

    MPI_Comm col_comm;
    MPI_Comm_split(comm_2d, info.y, info.x, &col_comm);

    info.n1 = 10;
    info.n2 = 10;
    info.n3 = 10;
    double *A; // n1 x n2
    double *B; // n2 x n3
    double *C; // n1 x n3

    ProcessDataInfo rows_info;
    init_process_data(info.n1, info.sx, info.x, &rows_info);

    ProcessDataInfo cols_info;
    init_process_data(info.n3, info.sy, info.y, &cols_info);
    // if (rank == 0)
    // {
    //     printf("size: %d\n", size);
    // }
    //printf("init rank: %d\n", rank);
    double *local_A = (double *)malloc(rows_info.capacity * info.n2 * sizeof(double));
    if (info.y == 0)
    {

        if (rank == 0)
        {
            A = init_mat_A(info.n1, info.n2);
            B = init_mat_B(info.n2, info.n3);
            C = (double *)malloc(info.n1 * info.n3 * sizeof(double));
            send_A_row(A, local_A, &info, comm_2d);
        }
        else
        {
            //printf("Recieve A by rank: %d\n", rank);
            MPI_Recv(local_A,
                     rows_info.capacity * info.n2,
                     MPI_DOUBLE,
                     0,
                     0,
                     comm_2d,
                     MPI_STATUS_IGNORE);
        }
    }

    MPI_Bcast(local_A,
              rows_info.capacity * info.n2,
              MPI_DOUBLE,
              0,
              row_comm);
    printf("complete send A rank: %d\n", rank);
    double *local_B = (double *)malloc(info.n2 * cols_info.capacity * sizeof(double));

    MPI_Datatype col_block;
    if (info.x == 0)
    {

        if (rank == 0)
        {
            send_B_col(B, local_B, &info, &cols_info, comm_2d, col_block);
        }
        else
        {
            MPI_Datatype recv_type;

MPI_Type_vector(info.n2,           // количество строк
                    cols_info.size,    // сколько элементов в строке
                    cols_info.size,    // шаг (ставим вплотную)
                    MPI_DOUBLE, 
                    &recv_type);

            MPI_Type_commit(&recv_type);
            MPI_Recv(local_B, 1, recv_type, 0, 1, comm_2d, MPI_STATUS_IGNORE);
        }
    }

    MPI_Bcast(local_B,
              info.n2 * cols_info.capacity,
              MPI_DOUBLE,
              0,
              col_comm);
    printf("complete send B rank: %d\n", rank);
    double *local_C = (double *)calloc(cols_info.capacity * rows_info.capacity,
                                       sizeof(double));

    mul_part_mat(&info, &cols_info, &rows_info, local_C, local_A, local_B);

    if (rank == 2) printf("B[0][0]=%f B[0][1]=%f\n", B[2], B[3]);

    collect_c(&rows_info, &cols_info, &info, rank, local_C, C, comm_2d);
    printf("complete collect C rank: %d\n", rank);
    // if (rank == 0)
    // {
    //     write_mat(C, info.n3, info.n1);
    //     printf("Result: %d", (int) check_res(C, info.n1, info.n2));
    // }
    if (rank == 1)
    {
        //write_mat(A, info.n1, info.n2);
        //write_mat(local_A, rows_info.size, info.n2);
        write_mat_colmajor(local_B, info.n2, cols_info.size);
        //write_mat(local_C, rows_info.size, cols_info.size);
        printf("Result: %d", (int) check_res(C, info.n1, info.n2));
    }
    MPI_Finalize();
}
