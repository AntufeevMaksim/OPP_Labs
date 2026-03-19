#pragma once

#include "resources.hpp"

class IProducer
{
public:
    IProducer(Resources& resources);
    static void* thread_func(void* arg);
    bool endJob();
    virtual uint64_t totalTasks() = 0;
    virtual void produce() = 0;
    virtual ~IProducer() = default;
protected:
    Resources& res_;
};

