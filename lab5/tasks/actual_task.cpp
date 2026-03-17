#include "actual_task.hpp"

#include <thread>
#include <chrono>

#include <stdio.h>

using namespace std::chrono_literals;

ActualTask::ActualTask(int id) : id_{id} {}

void ActualTask::execute()
{
    std::this_thread::sleep_for(100ms);
    printf("Task % d\n", id_);
    fflush(stdout);
}

TaskID ActualTask::getId() const
{
    return TaskID::ACTUAL_TASK;
}

std::unique_ptr<ActualTask> ActualTask::deserialize(std::vector<uint8_t>::iterator begin,
                                                    std::vector<uint8_t>::iterator end)
{
    return std::make_unique<ActualTask>(777);
}

std::vector<uint8_t> ActualTask::serialize() const
{
    return std::vector<uint8_t>();
}