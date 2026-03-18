#include "test_producer.hpp"

#include "actual_task.hpp"

#include <stdio.h>

TestProducer::TestProducer(const std::shared_ptr<ThreadSafeQueue<std::unique_ptr<ITask>>>& tasks, int count)
: count_{count},
tasks_{tasks}
{
    end_job_ = false;
}

void TestProducer::produce()
{
    for (int i = 0; i < count_; i++)
    {
        std::unique_ptr<ITask> task = std::make_unique<ActualTask>(i);
        tasks_->push(std::move(task));
    }
    end_job_ = true;
}