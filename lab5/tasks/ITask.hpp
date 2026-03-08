#pragma once

class ITask
{
public:
    virtual void execute() = 0;
    virtual ~ITask() = default;
};
