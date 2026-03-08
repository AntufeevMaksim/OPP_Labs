#include "IProducer.hpp"

void* IProducer::thread_func(void* arg)
{
    IProducer* self = static_cast<IProducer*>(arg);
    self->produce();
    return 0;
}