#include "executor.hpp"

#include "stdio.h" //

void *Executor::thread_func(void *arg)
{
    Executor *self = static_cast<Executor *>(arg);
    self->execute();
    return 0;
}

Executor::Executor(Resources& resources)
    : res_{resources}
{
}

void Executor::execute()
{
    while (true)
    {
        std::unique_ptr<ITask> task = res_.queue.pop();
        if (task != nullptr)
        {
            task->execute();
            res_.tasks_completed++;
        }
        else
        {
            return;
        }
    }
    
}