#include "tasks_stat_message.hpp"

#include "memory.h"

TasksStatMessage::TasksStatMessage(uint32_t thread_id, uint64_t total_tasks, uint64_t completed_tasks)
    : Message(MessageType::TASKS_STAT, thread_id),
      total_tasks_(total_tasks),
      completed_tasks_(completed_tasks)
{
}

uint64_t TasksStatMessage::getTotalTasks() {
    return total_tasks_;
}

uint64_t TasksStatMessage::getCompletedTasks() {
    return completed_tasks_;
}


std::vector<uint8_t> TasksStatMessage::serialize() const
{
    std::vector<uint8_t> data(sizeof(MessageType) + sizeof(uint32_t) +
                              sizeof(uint64_t) + sizeof(uint64_t));

    data[0] = static_cast<uint8_t>(type_);

    memcpy(
        data.data() + sizeof(MessageType),
        &thread_id_,
        sizeof(uint32_t));

    memcpy(
        data.data() + sizeof(MessageType) + sizeof(uint32_t),
        &total_tasks_,
        sizeof(uint64_t));

    memcpy(
        data.data() + sizeof(MessageType) + sizeof(uint32_t) + sizeof(uint64_t),
        &completed_tasks_,
        sizeof(uint64_t));

    return data;
}