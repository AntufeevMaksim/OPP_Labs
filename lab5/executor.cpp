#include "executor.hpp"

#include "stdio.h" //

void *Executor::thread_func(void *arg)
{
    Executor *self = static_cast<Executor *>(arg);
    self->execute();
    return 0;
}

Executor::Executor(const std::shared_ptr<ThreadSafeQueue<std::unique_ptr<ITask>>> &tasks)
    : tasks_{tasks}
{
}

void Executor::execute()
{
    while (true)
    {
        std::unique_ptr<ITask> task = tasks_->pop();
        if (task != nullptr)
        {
            task->execute();
        }
        else
        {
            break;
        }
    }
    
}