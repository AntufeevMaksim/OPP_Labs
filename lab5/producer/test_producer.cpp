#include "test_producer.hpp"

#include "actual_task.hpp"

#include <stdio.h>

TestProducer::TestProducer(Resources& resources, int count)
: IProducer(resources),
count_{count}
{
    res_.producer_end_job = false;
}

void TestProducer::produce()
{
    for (int i = 0; i < count_; i++)
    {
        std::unique_ptr<ITask> task = std::make_unique<ActualTask>(i);
        res_.queue.push(std::move(task));
    }
    res_.producer_end_job = true;
    res_.total_tasks = count_;
}

uint64_t TestProducer::totalTasks() {
    return count_;
}