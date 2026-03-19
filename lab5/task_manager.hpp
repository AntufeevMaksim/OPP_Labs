#pragma once

#include "request_tasks_message.hpp"
#include "message_protocol_tools.hpp"
#include "tasks_stat_message.hpp"
#include "IProducer.hpp"
#include "resources.hpp"

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
    int fails_count = 0;
    std::chrono::steady_clock::time_point time_of_last_request;
    bool communication_;
    bool need_to_request();

    Resources& res_;
    std::vector<size_t> total_tasks_;
    std::vector<size_t> completed_tasks_;
    std::chrono::steady_clock::time_point time_of_last_stat_send;
    bool need_send_stat();
    void send_stat();
    bool can_stop_ = false;
    void stop();
    
    std::random_device rd;
    std::mt19937 gen;
    std::uniform_int_distribution<int> dist;

    void request_tasks();
    void send_tasks(std::unique_ptr<RequestTasksMessage>& message, int process);
    void recv_tasks(std::unique_ptr<SendTasksMessage>& message);
    void update_stat(std::unique_ptr<TasksStatMessage>& message, int process);
    std::pair<int, std::unique_ptr<Message>> recv();

public:
    TaskManager(Resources& resources, int process_count, int process_id, int thread_id, bool communication = true);
    void run();
};
