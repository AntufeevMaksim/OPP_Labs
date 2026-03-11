#include "random_backoff_strategy.hpp"

#include "message_protocol_tools.hpp"
#include <random>
#include <chrono>
#include <thread>

RandomBackoffStrategy::RandomBackoffStrategy(int size, int process_rank, int thread_rank)
:   size_{size},
    process_rank_{process_rank},
    thread_rank_{thread_rank}
{

}

std::vector<std::unique_ptr<ITask>> RandomBackoffStrategy::GetTasks()
{
    printf("in get tasks\n");
    fflush(stdout);
    const int MAX_ATTEMPTS = size_ * 2;
    const int BASE_BACKOFF_US = 100;

    std::vector<std::unique_ptr<ITask>> tasks;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, size_ - 1);

    int attempts = 0;

    while (attempts < MAX_ATTEMPTS)
    {
        int victim = dist(gen);

        // не воруем у себя
        if (victim == process_rank_)
            continue;

        tasks = MessageProtocolTools::GetTasks(thread_rank_, victim);

        if (!tasks.empty())
        {
            return tasks;
        }

        // backoff
        int sleep_time = BASE_BACKOFF_US * (1 << std::min(attempts, 10));
        std::this_thread::sleep_for(std::chrono::microseconds(sleep_time));

        attempts++;
    }

    return {};
}

