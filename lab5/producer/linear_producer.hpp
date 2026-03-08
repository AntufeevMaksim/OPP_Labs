#pragma once

#include "IProducer.hpp"

class LinearProducer : public IProducer
{

public:
    LinearProducer(const std::shared_ptr<ThreadSafeQueue<std::unique_ptr<ITask>>>& tasks);
    void produce() override;

private:
    std::shared_ptr<ThreadSafeQueue<std::unique_ptr<ITask>>> tasks_;
};
