#pragma once

#include "thread_safe_queue.tpp"
#include "ITask.hpp"

#include <atomic>

class IProducer
{
public:
    static void* thread_func(void* arg);
    bool endJob();
    virtual void produce() = 0;
    virtual ~IProducer() = default;
protected:
    std::atomic<bool> end_job_;
};

