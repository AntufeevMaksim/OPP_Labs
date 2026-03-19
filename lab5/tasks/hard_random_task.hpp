#pragma once

#include "ITask.hpp"
#include <memory>

class HardRandomTask : public ITask
{
public:
    static std::unique_ptr<HardRandomTask> deserialize(std::vector<uint8_t>::iterator begin,
         std::vector<uint8_t>::iterator end);
    HardRandomTask(int id);
    void execute() override;
    TaskID getId() const override;
    std::vector<uint8_t> serialize() const override;
private:
    int id_;

};
