#pragma once

#include "ICommunication_strategy.hpp"

class RandomBackoffStrategy : public ICommunicationStrategy
{
private:
    int size_;
    int process_rank_;
    int thread_rank_;
public:
    RandomBackoffStrategy(int size, int process_rank, int thread_rank);
    virtual std::vector<std::unique_ptr<ITask>> GetTasks() override;
};
