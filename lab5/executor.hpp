#pragma once

#include "ITask.hpp"
#include "thread_safe_queue.tpp"
#include "ICommunication_strategy.hpp"

class Executor
{
public:
    static void* thread_func(void* arg);
    Executor(const std::shared_ptr<ThreadSafeQueue<std::unique_ptr<ITask>>>& tasks);
    void execute();

private:
    std::shared_ptr<ThreadSafeQueue<std::unique_ptr<ITask>>> tasks_;
};
