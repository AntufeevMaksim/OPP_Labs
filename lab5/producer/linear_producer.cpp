#include "linear_producer.hpp"

#include "actual_task.hpp"

LinearProducer::LinearProducer(const std::shared_ptr<ThreadSafeQueue<std::unique_ptr<ITask>>>& tasks)
: tasks_{tasks}
{}

void LinearProducer::produce()
{
    for (int i = 0; i < 100; i++)
    {
        std::unique_ptr<ITask> task = std::make_unique<ActualTask>(i);
        tasks_->push(std::move(task));
    }    
}