#pragma once

#include "ITask.hpp"
#include "thread_safe_queue.tpp"
#include "ICommunication_strategy.hpp"

class Executor
{
public:
    static void* thread_func(void* arg);
    Executor(const std::shared_ptr<ThreadSafeQueue<std::unique_ptr<ITask>>>& tasks,
    std::unique_ptr<ICommunicationStrategy> communication);
    void execute();

private:
    std::shared_ptr<ThreadSafeQueue<std::unique_ptr<ITask>>> tasks_;
    std::unique_ptr<ICommunicationStrategy> communication_;
};
