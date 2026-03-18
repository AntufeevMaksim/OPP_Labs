#pragma once

#include "IProducer.hpp"

class TestProducer : public IProducer
{

public:
    TestProducer(const std::shared_ptr<ThreadSafeQueue<std::unique_ptr<ITask>>>& tasks, int count);
    void produce() override;

private:
    int count_;
    std::shared_ptr<ThreadSafeQueue<std::unique_ptr<ITask>>> tasks_;
};
