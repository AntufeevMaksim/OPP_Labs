#include "send_tasks_message.hpp"

SendTasksMessage::SendTasksMessage(
    uint32_t thread_id, size_t init_tasks_count)
    : Message(MessageType::SEND_TASKS, thread_id),
      tasks_(init_tasks_count)
{
}

SendTasksMessage::SendTasksMessage(uint32_t thread_id, std::vector<std::unique_ptr<ITask>> &&tasks)
    : Message(MessageType::SEND_TASKS, thread_id),
      tasks_(std::move(tasks))
{
}

std::vector<std::unique_ptr<ITask>> &SendTasksMessage::tasks()
{
    return tasks_;
}

std::vector<uint8_t> SendTasksMessage::serialize() const
{
    std::vector<uint8_t> data;

    data.resize(sizeof(MessageType) + sizeof(uint32_t));

    data[0] = static_cast<uint8_t>(type_);

    memcpy(
        data.data() + sizeof(MessageType),
        &thread_id_,
        sizeof(uint32_t));

    uint64_t count = tasks_.size();

    size_t old = data.size();
    data.resize(old + sizeof(uint64_t));

    memcpy(data.data() + old, &count, sizeof(uint64_t));

    for (auto &task : tasks_)
    {
        auto task_data = Serializer::Serialize(*task);

        uint64_t size = task_data.size();
        int id = static_cast<int>(task->getId());

        old = data.size();

        data.resize(old + sizeof(uint64_t) + sizeof(int) + size);

        uint8_t *ptr = data.data() + old;

        memcpy(ptr, &size, sizeof(uint64_t));
        ptr += sizeof(uint64_t);

        memcpy(ptr, &id, sizeof(int));
        ptr += sizeof(int);

        memcpy(ptr, task_data.data(), size);
    }

    return data;
}

void SendTasksMessage::addTask(std::unique_ptr<ITask> &task)
{
    tasks_.push_back(std::move(task));
}