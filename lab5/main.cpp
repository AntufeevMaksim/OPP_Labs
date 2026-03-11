#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "thread_safe_queue.tpp"
#include <memory>
#include <mpi.h>
#include "actual_task.hpp"
#include "linear_producer.hpp"
#include "executor.hpp"
#include "random_backoff_strategy.hpp"
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

    int nt = atoi(argv[1]);

    pthread_t prod_thread;
    pthread_t task_sender;
    pthread_t exec_threads[nt];

    TaskManager task_manager{size, rank, THREAD_RECV_TASKS};

    std::unique_ptr<IProducer> producer = std::make_unique<LinearProducer>(task_manager.queue);
    pthread_create(&prod_thread, NULL,
                   &IProducer::thread_func, producer.get());

    for (int i = 0; i < nt; i++)
    {
        std::unique_ptr<ICommunicationStrategy> communication =
            std::make_unique<RandomBackoffStrategy>(size, rank, i + 1);
        auto executor = std::make_unique<Executor>(task_manager.queue, std::move(communication));
        pthread_create(&exec_threads[i], NULL,
                       &Executor::thread_func, executor.get());
    }

    // printf("123\n");
    // fflush(stdout);
    std::thread::id thread_id = std::this_thread::get_id();
    std::cout << thread_id << std::endl;
    fflush(stdout);
    task_manager.run();

    MPI_Finalize();

    return 0;
}
