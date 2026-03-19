#include "hard_random_task.hpp"

#include <random>
#include <memory.h>

HardRandomTask::HardRandomTask(int id) : id_(id) 
{}


void HardRandomTask::execute() 
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(1, 100);

    int sum = 0;
    for (int i = 0; i < 10e6; ++i)
    {
        sum += dist(gen);
    }
}

std::unique_ptr<HardRandomTask> HardRandomTask::deserialize(std::vector<uint8_t>::iterator begin,
         std::vector<uint8_t>::iterator end) 
{
    int id;
    memcpy(&id, &(*begin), sizeof(int));

    return std::make_unique<HardRandomTask>(id);
}

TaskID HardRandomTask::getId() const {
    return TaskID::HARD_RANDOM_TASK;
}

std::vector<uint8_t> HardRandomTask::serialize() const 
{
    std::vector<uint8_t> data(sizeof(int));

    memcpy(data.data(), &id_, sizeof(int));  
    
    return data;
}