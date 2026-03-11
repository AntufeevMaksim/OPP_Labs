#pragma once

#include "ITask.hpp"

#include <memory>
#include <stdint.h>
#include <vector>

class Serializer
{
public:
    static std::unique_ptr<ITask> Deserialize(int id, std::vector<uint8_t>::iterator begin, std::vector<uint8_t>::iterator end);
    static std::vector<uint8_t> Serialize(const ITask& task);
};
