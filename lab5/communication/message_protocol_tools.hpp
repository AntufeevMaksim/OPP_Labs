#pragma once

#include "ITask.hpp"
#include "send_tasks_message.hpp"
#include "thread_safe_queue.tpp"

#include <mpi.h>
#include <vector>
#include <stdint.h>
#include <memory>
#include <optional>

#define THREAD_RECV_TASKS 0

class MessageProtocolTools
{
private:
    MPI_Request send_tasks_status = MPI_REQUEST_NULL;
    std::vector<uint8_t> send_tasks_data;
    int process_id_;

public:

    void SendTasks(std::vector<std::unique_ptr<ITask>> &tasks,
                   int dest_process_id,
                   uint32_t dest_thread_id);
    void RequestTasks(uint32_t src_thread_id, int dest_process_id, uint32_t dest_thread_id);
    void SendStat(int dest_process_id, uint32_t dest_thread_id, uint64_t total_tasks, uint64_t completed_tasks);
    void SendStopProgram(int dest_process_id, uint32_t dest_thread_id);

    std::unique_ptr<Message> TryRead(int process, uint32_t thread);
    int RecvTasks(std::unique_ptr<SendTasksMessage>& message, ThreadSafeQueue<std::unique_ptr<ITask>>& queue);

    bool hasActiveCommunication();
};
