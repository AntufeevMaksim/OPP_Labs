
#include <mpi.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <memory.h>
#include <time.h>

int EVENT_COMP_START, EVENT_COMP_END;
int EVENT_COMM_START, EVENT_COMM_END;
int EVENT_ITER_START, EVENT_ITER_END;

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

void write_mat(FILE *f, bool *mat, int h, int w)
{
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

void init_empty(bool *field, int st_h, int h, int w)
{
    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            field[w * y + x] = 0;
        }
    }
}

void init_line(bool *field, int st_h, int h, int w)
{
    init_empty(field, st_h, h, w);

    int x = w / 2;

    for (int y = 0; y < h; ++y)
    {
        field[y * w + x] = 1;
    }
}

void init_field(char *arg, bool *field, int st_h, int h, int w)
{
    if (strcmp(arg, "line") == 0)
    {
        init_line(field, st_h, h, w);
    }
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
        // MPE_Log_event(EVENT_ITER_START, 0, NULL);
        // MPE_Log_event(EVENT_COMP_START, 0, NULL);

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
        // MPE_Log_event(EVENT_COMM_START, 0, NULL);

        int st = coord == 0 ? 0 : 1;
        for (int y = st; y < proc_h + st; ++y)
        {
            for (int x = 0; x < data->N; ++x)
            {
                tmp[data->N * y + x] = calc_next_state(local_field, x, y, data->N, h);
            }
        }
        // MPE_Log_event(EVENT_COMP_END, 0, NULL);

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
        // MPE_Log_event(EVENT_COMM_END, 0, NULL);
        swap(&tmp, &local_field);

        // MPE_Log_event(EVENT_ITER_END, 0, NULL);
    }
    free(tmp);
    return local_field;
}

void recv_field(bool *field, int coord, ProgramData *data, MPI_Comm comm_1d)
{

    if (coord == 0)
    {
        FILE *f = fopen("result.txt", "w");
        if (f == NULL)
        {
            perror("fopen");
            return;
        }

        int h = calc_h(coord, data);
        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < data->N; ++x)
            {
                char c = field[y * data->N + x] ? '#' : '.';
                fputc(c, f);
            }
            fputc('\n', f);
        }
        fclose(f);
    }

    for (int i = 1; i < data->size; ++i)
    {
        if (coord == i)
        {
            MPI_Recv(NULL, 0, MPI_INT, i - 1, 0,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            FILE *f = fopen("result.txt", "a");
            if (f == NULL)
            {
                perror("fopen");
                return;
            }

            int h = calc_h(coord, data);
            for (int y = 0; y < h; ++y)
            {
                for (int x = 0; x < data->N; ++x)
                {
                    char c = field[y * data->N + x] ? '#' : '.';
                    fputc(c, f);
                }
                fputc('\n', f);
            }
            fclose(f);
        }
        else if (coord == i - 1)
        {
            MPI_Send(NULL, 0, MPI_INT, i, 0, MPI_COMM_WORLD);
        }
    }
}

int main(int argc, char **argv)
{

    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // MPE_Log_get_state_eventIDs(&EVENT_COMP_START, &EVENT_COMP_END);
    // MPE_Describe_state(EVENT_COMP_START, EVENT_COMP_END,
    //                    "COMPUTE", "red");

    // MPE_Log_get_state_eventIDs(&EVENT_COMM_START, &EVENT_COMM_END);
    // MPE_Describe_state(EVENT_COMM_START, EVENT_COMM_END,
    //                    "COMM", "blue");

    // MPE_Log_get_state_eventIDs(&EVENT_ITER_START, &EVENT_ITER_END);
    // MPE_Describe_state(EVENT_ITER_START, EVENT_ITER_END,
    //                    "ITER", "green");

    if (argc != 4)
    {
        if (rank == 0)
            printf("Wrong amount of args");
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

    int h = calc_real_h(calc_h(coord, &data), coord, &data);
    int st_h = calc_h(coord, &data);
    bool *field = (bool *)malloc(h * N * sizeof(bool));
    init_field(argv[1], field, st_h, h, N);

    int local_size_y = height;
    local_size_y += (coord == 0 ? 0 : 1);
    local_size_y += (coord == size - 1 ? 0 : 1);

    field = run_game(field, iters, &data, coord, comm_1d);

    recv_field(field, coord, &data, comm_1d);

    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed = (end.tv_sec - start.tv_sec) +
                     (end.tv_nsec - start.tv_nsec) / 1e9;
    if (coord == 0)
    {
        write_info(size, elapsed);
    }

    MPI_Finalize();
}
