#include "message_factory.hpp"

#include "request_tasks_message.hpp"
#include "send_tasks_message.hpp"
#include "tasks_stat_message.hpp"
#include "stop_program_message.hpp"
#include "serializer.hpp"

#include <stdexcept>
#include <memory.h>

std::unique_ptr<Message> MessageFactory::deserialize(std::vector<uint8_t> &data)
{
    MessageType type = static_cast<MessageType>(data[0]);

    uint32_t thread_id;
    memcpy(
        &thread_id,
        data.data() + sizeof(MessageType),
        sizeof(uint32_t));

    switch (type)
    {
    case MessageType::REQUEST_TASKS:
        return std::make_unique<RequestTasksMessage>(thread_id);

    case MessageType::SEND_TASKS:
        return deserializeSendTasks(thread_id, data);

    case MessageType::TASKS_STAT:
        return deserializeTasksStat(thread_id, data);
        
    case MessageType::STOP_PROGRAM:
        return std::make_unique<StopProgramMessage>(thread_id);        
    default:
        throw std::runtime_error("Unknown message type");
    }
}

std::unique_ptr<Message> MessageFactory::deserializeSendTasks(
    uint32_t thread_id,
    std::vector<uint8_t> &data)
{

    uint32_t tasks_count;
    memcpy(&tasks_count, data.data() + HEADER_SIZE, sizeof(uint32_t));

    auto message = std::make_unique<SendTasksMessage>(thread_id, tasks_count);

    auto it = data.begin() + HEADER_SIZE + sizeof(uint32_t);

    for (uint32_t i = 0; i < tasks_count; ++i)
    {
        uint64_t task_data_size;
        memcpy(&task_data_size, &(*it), sizeof(uint64_t));
        it += sizeof(uint64_t);

        int task_id;
        memcpy(&task_id, &(*it), sizeof(int));
        it += sizeof(int);

        auto task_begin = it;
        auto task_end = it + task_data_size;

        if (task_end > data.end())
        {
            throw std::runtime_error("corrupted task data");
        }

        auto task = Serializer::Deserialize(task_id, task_begin, task_end);
        message->addTask(task);

        it += task_data_size;
    }

    return message;
}

std::unique_ptr<Message> MessageFactory::deserializeTasksStat(
        uint32_t thread_id,
        std::vector<uint8_t> &data) 
{
    uint64_t total_tasks;
    memcpy(&total_tasks, data.data() + HEADER_SIZE, sizeof(uint64_t));

    uint64_t comleted_tasks;
    memcpy(&comleted_tasks, data.data() + HEADER_SIZE + sizeof(uint64_t), sizeof(uint64_t));
    
    std::unique_ptr<Message> message = std::make_unique<TasksStatMessage>(thread_id, total_tasks, comleted_tasks);
    return message;
}