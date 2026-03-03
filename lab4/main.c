
#include <mpi.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <memory.h>
#include <time.h>

typedef struct
{
    int rows;
    int rem;
    int root;
    int size;
    int N;
} ProgramData;

void write_info(int num_proc, double time)
{
    FILE *f = fopen("info.txt", "w");
    if (f == NULL)
    {
        perror("fopen");
        return;
    }  

    fprintf(f, "Num proc: %d\n Total time: %lf\n", num_proc, time);
}

void write_mat(bool *mat, int h, int w)
{
    FILE *f = fopen("output.txt", "w");
    if (f == NULL)
    {
        perror("fopen");
        return;
    }

    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            char c = mat[y * w + x] ? '#' : '.';
            fputc(c, f);
        }
        fputc('\n', f);
    }

    fclose(f);
}

static inline void swap(bool **a, bool **b)
{
    bool *tmp = *a;
    *a = *b;
    *b = tmp;
}

static inline int calc_start_h(int coord, ProgramData *data)
{
    return coord * data->rows + (coord < data->rem ? coord : data->rem);
}

static inline int calc_h(int coord, ProgramData *data)
{
    return data->rows + (coord < data->rem ? 1 : 0);
}

static inline int calc_real_h(int h, int coord, ProgramData *data)
{
    h += (coord == 0 ? 0 : 1);
    h += (coord == data->size - 1 ? 0 : 1);
    return h;
}

bool *init_empty(int N)
{
    bool *field = (bool *)malloc(N * N * sizeof(bool));
    for (int y = 0; y < N; ++y)
    {
        for (int x = 0; x < N; ++x)
        {
            field[N * y + x] = 0;
        }
    }
    return field;
}

bool *init_glider(int N)
{
    bool *field = init_empty(N);

    int x = 1;
    int y = 1;

    field[y * N + (x + 1)] = true;
    field[(y + 1) * N + (x + 2)] = true;
    field[(y + 2) * N + x] = true;
    field[(y + 2) * N + (x + 1)] = true;
    field[(y + 2) * N + (x + 2)] = true;

    return field;
}

bool *init_line(int N)
{
    bool *field = init_empty(N);

    int x = N / 2;

    for (int y = 0; y < N; ++y)
    {
        field[y * N + x] = 1;
    }
    return field;
}

bool *init_blinker(int N)
{
    bool *field = init_empty(N);

    int cx = N / 2;
    int cy = N / 2;

    field[cy * N + (cx - 1)] = true;
    field[cy * N + cx] = true;
    field[cy * N + (cx + 1)] = true;

    return field;
}

bool *init_random(int N, unsigned int seed)
{
    srand(seed);

    bool *field = (bool *)malloc(N * N * sizeof(bool));
    if (field == NULL)
        return NULL;

    for (int y = 0; y < N; ++y)
    {
        for (int x = 0; x < N; ++x)
        {
            field[N * y + x] = rand() % 2;
        }
    }

    return field;
}

bool *init_field(char* arg, int N)
{
    bool *field;
    if (strcmp(arg, "line") == 0)
    {
        field = init_line(N);
    }
    else if (strcmp(arg, "glider") == 0)
    {
        field = init_glider(N);
    }
    else if (strcmp(arg, "blinker") == 0)
    {
        field = init_blinker(N);
    }
    else if (strcmp(arg, "random") == 0)
    {
        field = init_random(N, 137);
    }
    return field;
}


bool calc_next_state(bool *field, int x, int y, int local_sx, int local_sy)
{
    bool current_state = field[y * local_sx + x];
    int live_neighbors = 0;

    for (int dy = -1; dy <= 1; dy++)
    {
        for (int dx = -1; dx <= 1; dx++)
        {
            if (dx == 0 && dy == 0)
                continue;

            int nx = x + dx;
            int ny = y + dy;

            if (nx >= 0 && nx < local_sx && ny >= 0 && ny < local_sy)
            {
                live_neighbors += field[ny * local_sx + nx];
            }
        }
    }

    bool new_state;
    if (current_state)
    {
        new_state = (live_neighbors == 2 || live_neighbors == 3) ? 1 : 0;
    }
    else
    {
        new_state = live_neighbors == 3 ? 1 : 0;
    }

    return new_state;
}

void send_field(bool *field, bool *local_field, int coord, ProgramData *data, MPI_Comm comm_1d)
{
    int *counts = (int *)malloc(data->size * sizeof(int));
    int *displs = (int *)malloc(data->size * sizeof(int));

    int disp = 0;
    for (int i = 0; i < data->size; ++i)
    {
        displs[i] = disp;

        counts[i] = calc_h(i, data);
        counts[i] = calc_real_h(counts[i], i, data) * data->N;

        disp = (calc_start_h(i + 1, data) - 1) * data->N;
    }

    int recv_count = calc_real_h(calc_h(coord, data), coord, data) * data->N;
    MPI_Scatterv(field, counts, displs, MPI_C_BOOL, local_field, recv_count, MPI_C_BOOL, 0, comm_1d);
}

bool *run_game(bool *local_field, int iters, ProgramData *data, int coord, MPI_Comm comm_1d)
{
    const int SEND_UP = 0;
    const int RECV_DOWN = SEND_UP;
    const int SEND_DOWN = 1;
    const int RECV_UP = SEND_DOWN;

    MPI_Request send_up, send_down, recv_up, recv_down;

    int proc_h = calc_h(coord, data);
    int h = calc_real_h(proc_h, coord, data);
    bool *tmp = (bool *)malloc(data->N * h * sizeof(bool));

    for (int i = 0; i < iters; ++i)
    {

        if (coord != 0)
        {
            for (int x = 0; x < data->N; ++x) // calc up border
            {
                tmp[data->N + x] = calc_next_state(local_field, x, 1, data->N, h);
            }

            MPI_Isend(tmp + data->N, data->N, MPI_C_BOOL, coord - 1, SEND_UP, comm_1d, &send_up);
            MPI_Irecv(tmp, data->N, MPI_C_BOOL, coord - 1, RECV_UP, comm_1d, &recv_up);
        }
        if (coord != data->size - 1)
        {
            for (int x = 0; x < data->N; ++x) // calc down border
            {
                tmp[data->N * (h - 2) + x] = calc_next_state(local_field, x, h - 2, data->N, h);
            }

            MPI_Isend(tmp + data->N * (h - 2), data->N, MPI_C_BOOL, coord + 1, SEND_DOWN, comm_1d, &send_down);
            MPI_Irecv(tmp + data->N * (h - 1), data->N, MPI_C_BOOL, coord + 1, RECV_DOWN, comm_1d, &recv_down);
        }

        int st = coord == 0 ? 0 : 1;
        for (int y = st; y < proc_h + st; ++y)
        {
            for (int x = 0; x < data->N; ++x)
            {
                tmp[data->N * y + x] = calc_next_state(local_field, x, y, data->N, h);
            }
        }

        if (coord != 0)
        {
            MPI_Wait(&send_up, MPI_STATUS_IGNORE);
            MPI_Wait(&recv_up, MPI_STATUS_IGNORE);
        }
        if (coord != data->size - 1)
        {
            MPI_Wait(&send_down, MPI_STATUS_IGNORE);
            MPI_Wait(&recv_down, MPI_STATUS_IGNORE);
        }

        swap(&tmp, &local_field);
    }
    free(tmp);
    return local_field;
}

void recv_field(bool *field, bool *local_field, int coord, ProgramData *data, MPI_Comm comm_1d)
{
    int *recv_counts = (int *)malloc(data->size * sizeof(int));
    int *displs = (int *)malloc(data->size * sizeof(int));

    for (int i = 0; i < data->size; ++i)
    {
        displs[i] = calc_start_h(i, data) * data->N;
        recv_counts[i] = calc_h(i, data) * data->N;
    }

    int send_count = calc_h(coord, data) * data->N;
    int start = (coord == 0) ? 0 : data->N;
    MPI_Gatherv(local_field + start, send_count, MPI_C_BOOL, field, recv_counts, displs, MPI_C_BOOL, 0, comm_1d);
}

int main(int argc, char **argv)
{

    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc != 4)
    {
        if (rank == 0) printf("Wrong amount of args");
        return 0;
    }

    int N = atoi(argv[2]);
    int iters = atoi(argv[3]);
    int dims[1] = {size};
    int periods[1] = {0};

    MPI_Comm comm_1d;
    MPI_Cart_create(MPI_COMM_WORLD, 1, dims, periods, 0, &comm_1d);

    int coords[1];
    MPI_Cart_coords(comm_1d, rank, 1, coords);
    int coord = coords[0];

    ProgramData data;
    data.size = size;
    data.N = N;
    int root_coords[1] = {0};
    MPI_Cart_rank(comm_1d, root_coords, &(data.root));

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    data.rows = N / size;
    data.rem = N % size;
    int start_h = calc_start_h(coord, &data);
    int height = calc_h(coord, &data);

    bool *field;

    if (coord == 0)
    {
        field = init_field(argv[1], N);
    }
    int local_size_y = height;
    local_size_y += (coord == 0 ? 0 : 1);
    local_size_y += (coord == size - 1 ? 0 : 1);

    bool *local_field = (bool *)malloc(N * local_size_y * sizeof(bool));

    send_field(field, local_field, coord, &data, comm_1d);

    local_field = run_game(local_field, iters, &data, coord, comm_1d);

    recv_field(field, local_field, coord, &data, comm_1d);

    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed = (end.tv_sec - start.tv_sec) + 
                     (end.tv_nsec - start.tv_nsec) / 1e9;
    if (coord == 0)
    {
        write_mat(field, N, N);
        write_info(size, elapsed);
    }

    MPI_Finalize();
}
