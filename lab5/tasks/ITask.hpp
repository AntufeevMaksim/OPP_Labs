#pragma once

#include <vector>
#include <stdint.h>

enum class TaskID:int
{
    ACTUAL_TASK,
    HARD_RANDOM_TASK
};


class ITask
{
public:
    virtual void execute() = 0;
    virtual TaskID getId() const = 0;
    virtual std::vector<uint8_t> serialize() const = 0;
    virtual ~ITask() = default;
};
