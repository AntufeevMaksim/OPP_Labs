#include "executor.hpp"

#include "stdio.h" //

void *Executor::thread_func(void *arg)
{
    Executor *self = static_cast<Executor *>(arg);
    self->execute();
    return 0;
}

Executor::Executor(const std::shared_ptr<ThreadSafeQueue<std::unique_ptr<ITask>>> &tasks,
                   std::unique_ptr<ICommunicationStrategy> communication)
    : tasks_{tasks},
      communication_{std::move(communication)}
{
}

void Executor::execute()
{
    while (true)
    {
        printf("in execute\n");
        fflush(stdout);
        std::unique_ptr<ITask> task = tasks_->pop();
        if (task != nullptr)
        {
            printf("address %p\n", (void*) task.get());
            task->execute();
        }
        else
        {
            // auto new_tasks = communication_->GetTasks();
            // for (auto& task : new_tasks)
            // {
            //     tasks_->push(std::move(task));
            // }
        }
    }
    
}