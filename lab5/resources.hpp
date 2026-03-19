#pragma once

#include "ITask.hpp"
#include "thread_safe_queue.tpp"
#include <atomic>

class Resources
{
public:
    ThreadSafeQueue<std::unique_ptr<ITask>> queue;
    std::atomic<bool> producer_end_job = false;
    int num_threads;
    std::atomic<uint64_t> tasks_completed{0};
    std::atomic<uint64_t> total_tasks{0};


};
