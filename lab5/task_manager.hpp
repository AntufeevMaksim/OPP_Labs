#pragma once

#include "request_tasks_message.hpp"
#include "message_protocol_tools.hpp"
#include "IProducer.hpp"

#include <memory>
#include <atomic>
#include <chrono>
#include <random>

class TaskManager
{
private:
    int process_id_;
    int thread_id_;
    int process_count_;

    MessageProtocolTools protocol_tools_;

    bool tasks_requested_ = false;
    bool last_fail = false;
    std::chrono::steady_clock::time_point time_of_last_request;
    bool need_to_request();

    IProducer& producer_;
    inline bool can_stop();

    std::random_device rd;
    std::mt19937 gen;
    std::uniform_int_distribution<int> dist; // distribution in range [1, 6]

    void request_tasks();
    void send_tasks(std::unique_ptr<RequestTasksMessage>& message, int process);
    void recv_tasks(std::unique_ptr<SendTasksMessage>& message);
    std::pair<int, std::unique_ptr<Message>> recv();

public:
    TaskManager(IProducer& producer, int process_count, int process_id, int thread_id);
    void run();
    std::shared_ptr<ThreadSafeQueue<std::unique_ptr<ITask>>> queue;
    std::atomic<int> tasks_count;
};
