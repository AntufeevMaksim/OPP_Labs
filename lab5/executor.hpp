#pragma once

#include "ITask.hpp"
#include "resources.hpp"

class Executor
{
public:
    static void* thread_func(void* arg);
    Executor(Resources& resources);
    void execute();

private:
    Resources& res_;
};
