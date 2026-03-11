#pragma once

#include "ITask.hpp"

#include <vector>
#include <memory>

class ICommunicationStrategy
{
public:
    virtual std::vector<std::unique_ptr<ITask>> GetTasks() = 0;
};
