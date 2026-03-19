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
#include "test_producer.hpp"
#include "resources.hpp"
#include <thread>
#include <iostream>

using namespace std::chrono_literals;

int main(int argc, char **argv)
{
    int provided;
    MPI_Init_thread(&argc, &argv,
                    MPI_THREAD_MULTIPLE,
                    &provided);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc != 4)
    {
        throw std::invalid_argument("invalid argument count");
    }
    Resources resources;
    resources.num_threads = atoi(argv[1]);
    int task_count = atoi(argv[2]);
    bool need_communication = !strcmp(argv[3], "comm");
    
    pthread_t prod_thread;
    pthread_t exec_threads[resources.num_threads];
    std::vector<std::unique_ptr<Executor>> executors(resources.num_threads);

    TaskManager task_manager(resources, size, rank, THREAD_RECV_TASKS, need_communication);

    int count = rank == 0 ? 5 * task_count : 100 * task_count;
    std::unique_ptr<IProducer> producer = std::make_unique<TestProducer>(resources, count);
    pthread_create(&prod_thread, NULL,
                   &IProducer::thread_func, producer.get());

    for (int i = 0; i < resources.num_threads; i++)
    {
        executors[i] = std::make_unique<Executor>(resources);
        pthread_create(&exec_threads[i], NULL,
                       &Executor::thread_func, executors[i].get());
    }

    auto start = std::chrono::high_resolution_clock::now();
    task_manager.run();

    for (int i = 0; i < resources.num_threads; i++)
    {
        pthread_join(exec_threads[i], NULL);
    }

    pthread_join(prod_thread, NULL);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "total time: " << duration.count() / 1000.0 << std::endl;
    MPI_Finalize();

    return 0;
}
