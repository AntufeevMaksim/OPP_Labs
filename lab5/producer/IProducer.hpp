#pragma once

#include "thread_safe_queue.tpp"
#include "ITask.hpp"

class IProducer
{
public:
    static void* thread_func(void* arg);
    virtual void produce() = 0;
    virtual ~IProducer() = default;
};

