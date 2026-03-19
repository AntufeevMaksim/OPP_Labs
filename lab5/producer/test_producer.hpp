#pragma once

#include "IProducer.hpp"
#include "resources.hpp"

class TestProducer : public IProducer
{

public:
    TestProducer(Resources& resources, int count);
    void produce() override;
    virtual uint64_t totalTasks() override;

private:
    int count_;
};
