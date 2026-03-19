#pragma once

#include "message.hpp"

class TasksStatMessage : public Message
{
public:
    TasksStatMessage(uint32_t thread_id, uint64_t total_tasks, uint64_t completed_tasks);

    uint64_t getTotalTasks();
    uint64_t getCompletedTasks();

    std::vector<uint8_t> serialize() const override;
private:
    uint64_t total_tasks_;
    uint64_t completed_tasks_;
};