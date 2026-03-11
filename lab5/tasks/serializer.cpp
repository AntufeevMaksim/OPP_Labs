#include "serializer.hpp"

#include "actual_task.hpp"

#include <stdexcept>
#include <cstring>

std::vector<uint8_t> Serializer::Serialize(const ITask& task)
{
    return task.serialize();
}

std::unique_ptr<ITask> Serializer::Deserialize(
    int id,
    std::vector<uint8_t>::iterator begin,
    std::vector<uint8_t>::iterator end)
{
    switch(id)
    {
        case static_cast<int>(TaskID::ACTUAL_TASK):
            return ActualTask::deserialize(begin, end);

        default:
            throw std::runtime_error("Unknown task id");
    }
}
