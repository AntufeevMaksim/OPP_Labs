#pragma once

#include "ITask.hpp"

class ActualTask : public ITask
{
public:
    ActualTask(int id);
    void execute() override;
private:
    int id_;
};
