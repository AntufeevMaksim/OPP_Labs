#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "thread_safe_queue.tpp"
#include <memory>
#include <mpi.h>
#include "actual_task.hpp"
#include "linear_producer.hpp"
#include "executor.hpp"

int main(int argc, char **argv)
{
    int provided;
    MPI_Init_thread(&argc, &argv,
                    MPI_THREAD_MULTIPLE,
                    &provided);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int nt = atoi(argv[1]);

    pthread_t prod_thread;
    pthread_t exec_threads[nt];

    auto tasks = std::make_shared<ThreadSafeQueue<std::unique_ptr<ITask>>>();
    std::unique_ptr<IProducer> producer = std::make_unique<LinearProducer>(tasks);
    pthread_create(&prod_thread, NULL,
                   &IProducer::thread_func, producer.get());

    std::unique_ptr<Executor> executor = std::make_unique<Executor>(tasks);
    for (int i = 0; i < nt; i++)
    {
        pthread_create(&exec_threads[i], NULL,
                       &Executor::thread_func, executor.get());
    }

    // Wait for prod thread to finish
    pthread_join(prod_thread, NULL);
    // Sending NULL to stop each exec thread
    for (int i = 0; i < nt; i++)
    {
        tasks->push(nullptr);
    }

    for (int i = 0; i < nt; i++)
    {
        pthread_join(exec_threads[i], NULL);
    }

    MPI_Finalize();

    return 0;
}
