#pragma once

#include "ITask.hpp"

#include <vector>
#include <stdint.h>
#include <memory>

#define THREAD_RECV_TASKS 0

enum class MessageType:uint8_t
{
    REQUEST_TASKS,
    SEND_TASKS,
    GET_TASKS
};


struct Message
{
    MessageType type;
    uint64_t thread_id;
    bool has_tasks;
    uint64_t stride_size;
    std::vector<uint64_t> stride;
    std::vector<uint8_t> data;
};



class MessageProtocolTools
{
public:
    static std::vector<std::unique_ptr<ITask>> GetTasks(int src_thread_id, int dest_process_id);
    static void SendTasks(const std::vector<std::unique_ptr<ITask>>&,
     int dest_process_id,
     int dest_thread_id);
    static std::vector<uint8_t> TryRead(int process, int thread);
    static MessageType GetType(const std::vector<uint8_t>& message);
};
