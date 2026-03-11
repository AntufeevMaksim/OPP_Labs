#pragma once

#include "ITask.hpp"

#include <memory>

class ActualTask : public ITask
{
public:
    static std::unique_ptr<ActualTask> deserialize(std::vector<uint8_t>::iterator begin,
         std::vector<uint8_t>::iterator end);
    ActualTask(int id);
    void execute() override;
    TaskID getId() const override;
    std::vector<uint8_t> serialize() const override;
private:
    int id_;
};
