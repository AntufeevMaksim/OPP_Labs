#include "IProducer.hpp"

IProducer::IProducer(Resources &resources)
    : res_(resources)
{
}

void *IProducer::thread_func(void *arg)
{
    IProducer *self = static_cast<IProducer *>(arg);
    self->produce();
    return 0;
}