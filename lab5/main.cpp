#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "thread_safe_queue.tpp"
#include <memory>
#include <mpi.h>
#include "actual_task.hpp"
#include "linear_producer.hpp"
#include "executor.hpp"
#include "task_manager.hpp"
#include "message_protocol_tools.hpp"
#include <thread>

int main(int argc, char **argv)
{
    int provided;
    MPI_Init_thread(&argc, &argv,
                    MPI_THREAD_MULTIPLE,
                    &provided);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc != 2)
    {
        throw std::invalid_argument("invalid argument count");
    }
    int nt = atoi(argv[1]);

    pthread_t prod_thread;
    pthread_t exec_threads[nt];
    std::vector<std::unique_ptr<Executor>> executors(nt);

    TaskManager task_manager{size, rank, THREAD_RECV_TASKS};

    std::unique_ptr<IProducer> producer = std::make_unique<LinearProducer>(task_manager.queue);
    pthread_create(&prod_thread, NULL,
                   &IProducer::thread_func, producer.get());

    for (int i = 0; i < nt; i++)
    {
        executors[i] = std::make_unique<Executor>(task_manager.queue);
        pthread_create(&exec_threads[i], NULL,
                       &Executor::thread_func, executors[i].get());
    }

    task_manager.run();

    for (int i = 0; i < nt; i++)
    {
        pthread_join(exec_threads[i], NULL);
    }

    pthread_join(prod_thread, NULL);
    // std::thread::id thread_id = std::this_thread::get_id();
    // std::cout << thread_id << std::endl;
    // fflush(stdout);

    MPI_Finalize();

    return 0;
}
