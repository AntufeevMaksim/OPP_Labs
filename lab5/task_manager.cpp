#include "task_manager.hpp"

#include "message_protocol_tools.hpp"

#include <mpi.h>
#include <algorithm>
#include <ranges>

TaskManager::TaskManager(Resources& resources, int process_count, int process_id, int thread_id, bool communication)
    : process_id_{process_id},
      thread_id_{thread_id},
      process_count_{process_count},
      communication_{communication},
      res_{resources},
      total_tasks_(process_count, 0),
      completed_tasks_(process_count, 0)
{
    gen = std::mt19937(rd());
    dist = std::uniform_int_distribution<int>(0, process_count - 1);
}

void TaskManager::run()
{
    while (true)
    {

        if (can_stop_)
        {
            stop();
            break;
        }

        if (need_send_stat())
        {
            send_stat();
        }

        if (need_to_request())
        {
            request_tasks();
        }

        for (int src_proc = 0; src_proc < process_count_; ++src_proc)
        {

            auto recv = protocol_tools_.TryRead(src_proc, THREAD_RECV_TASKS);

            if (recv == nullptr)
                continue;

            MessageType type = recv->type();

            if (type == MessageType::REQUEST_TASKS)
            {
                std::unique_ptr<RequestTasksMessage> message(static_cast<RequestTasksMessage*>(recv.release()));
                send_tasks(message, src_proc);
            }
            else if (type == MessageType::SEND_TASKS)
            {
                std::unique_ptr<SendTasksMessage> message(static_cast<SendTasksMessage*>(recv.release()));
                recv_tasks(message);
            }
            else if (type == MessageType::TASKS_STAT)
            {
                std::unique_ptr<TasksStatMessage> message(static_cast<TasksStatMessage*>(recv.release()));
                update_stat(message, src_proc);           
            }
            else if (type == MessageType::STOP_PROGRAM)
            {
                can_stop_ = true;
            }
            else
            {
                throw std::runtime_error("invalid message type in task manager");
            }
        }
    }
}

void TaskManager::send_tasks(std::unique_ptr<RequestTasksMessage>& message, int process)
{
    std::vector<std::unique_ptr<ITask>> tasks;

    size_t i = 0;
    while (i < res_.queue.size() / 2)
    {
        auto [task, success] = res_.queue.try_pop();

        if (!success)
            break;

        tasks.push_back(std::move(task));
    }
    protocol_tools_.SendTasks(tasks, process, message->threadId());
}

void TaskManager::recv_tasks(std::unique_ptr<SendTasksMessage>& message)
{
    if (protocol_tools_.RecvTasks(message, res_.queue))
        --fails_count;  
    tasks_requested_ = false;
}

bool TaskManager::need_to_request() 
{
    if (communication_ && !tasks_requested_ && res_.queue.size() == 0)
    {
        auto now = std::chrono::steady_clock::now();
        bool need_to_send = (fails_count == 0) || (now - time_of_last_request >= std::chrono::milliseconds(100));

        return need_to_send;
    }
    return false;
}

void TaskManager::request_tasks() 
{
    int dest_proc;
    while ((dest_proc = dist(gen)) == process_id_)
    ++fails_count;
    tasks_requested_ = true;
    protocol_tools_.RequestTasks(thread_id_, dest_proc,THREAD_RECV_TASKS);
}



void TaskManager::stop() 
{
    if (process_id_ == 0)
    {
        for (int proc = 1; proc < process_count_; ++proc)
        {
            protocol_tools_.SendStopProgram(proc, THREAD_RECV_TASKS);
        }
    }
    for (int i = 0; i < res_.num_threads; ++i)
    {
        res_.queue.push(nullptr);
    }
}

void TaskManager::update_stat(std::unique_ptr<TasksStatMessage>& message, int process) 
{
    total_tasks_[process] = message->getTotalTasks();
    completed_tasks_[process] = message->getCompletedTasks();

    bool all_given = std::all_of(total_tasks_.begin(), total_tasks_.end(), 
                               [](uint64_t x) { return x > 0; });
    if (!all_given)
        return;

    uint64_t sum_total = std::accumulate(total_tasks_.begin(), total_tasks_.end(), 0);
    uint64_t sum_completed = std::accumulate(completed_tasks_.begin(), completed_tasks_.end(), 0);

    can_stop_ = (sum_total == sum_completed);
}

bool TaskManager::need_send_stat() 
{
    if (res_.producer_end_job)
    {
        auto now = std::chrono::steady_clock::now();
        if (now - time_of_last_request >= std::chrono::milliseconds(100))
        {
            return true;
        }
    }  
    return false;  
}

void TaskManager::send_stat() 
{
    time_of_last_request = std::chrono::steady_clock::now();
    protocol_tools_.SendStat(0, THREAD_RECV_TASKS, res_.total_tasks, res_.tasks_completed);
}
