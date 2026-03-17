#pragma once

#include "message.hpp"
#include "ITask.hpp"
#include "serializer.hpp"

#include <memory>
#include <memory.h>

class SendTasksMessage : public Message
{
public:
    SendTasksMessage(uint32_t thread_id, size_t init_tasks_count = 0);
    SendTasksMessage(uint32_t thread_id, std::vector<std::unique_ptr<ITask>>&& tasks);
    void addTask(std::unique_ptr<ITask>& task);
    
    std::vector<std::unique_ptr<ITask>>& tasks();

    std::vector<uint8_t> serialize() const override;

private:
    std::vector<std::unique_ptr<ITask>> tasks_;
};