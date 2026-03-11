#pragma once

#include "thread_safe_queue.tpp"
#include "ITask.hpp"

#include <memory>
#include <atomic>

class TaskManager
{
private:
    int process_id_;
    int thread_id_;
    int process_count_;
public:
    TaskManager(int process_count, int process_id, int thread_id);
    void run();
    void answer_request_tasks(std::vector<uint8_t> message, int process);
    std::shared_ptr<ThreadSafeQueue<std::unique_ptr<ITask>>> queue;
    std::atomic<int> tasks_count;
};
