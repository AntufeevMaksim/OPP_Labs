#include "task_manager.hpp"

#include "message_protocol_tools.hpp"

#include <mpi.h>

#include <stdio.h> //

TaskManager::TaskManager(IProducer& producer, int process_count, int process_id, int thread_id)
    : process_id_{process_id},
      thread_id_{thread_id},
      process_count_{process_count},
      producer_{producer},
      queue{std::make_shared<ThreadSafeQueue<std::unique_ptr<ITask>>>()}
{
    gen = std::mt19937(rd());
    dist = std::uniform_int_distribution<int>(0, process_count - 1);
}

void TaskManager::run()
{
    while (true)
    {
        fflush(stdout);
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
                printf("type req\n");
                std::unique_ptr<RequestTasksMessage> message(static_cast<RequestTasksMessage*>(recv.release()));
                send_tasks(message, src_proc);
            }
            else if (type == MessageType::SEND_TASKS)
            {
                printf("type send\n");
                std::unique_ptr<SendTasksMessage> message(static_cast<SendTasksMessage*>(recv.release()));
                recv_tasks(message);
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

    auto [task, success] = queue->try_pop();

    if (success)
    {
        if (task == nullptr)
        {
            printf("NULLPTR\n");
        }
        tasks.push_back(std::move(task));
    }

    printf("try to send\n");
    protocol_tools_.SendTasks(tasks, process, message->threadId());
}

void TaskManager::recv_tasks(std::unique_ptr<SendTasksMessage>& message)
{
    printf("try recv\n");
    fflush(stdout);
    protocol_tools_.RecvTasks(message, *queue.get());
    tasks_requested_ = false;    
}

bool TaskManager::need_to_request() 
{
    if (!tasks_requested_ && queue->size() == 0)
    {
        auto now = std::chrono::steady_clock::now();
        bool need_to_send = !last_fail || (now - time_of_last_request >= std::chrono::milliseconds(100));
        if (need_to_send)
        {
            printf("need to send\n");
            fflush(stdout);
        }
        return need_to_send;
    }
    return false;
}

void TaskManager::request_tasks() 
{
    int dest_proc;
    while ((dest_proc = dist(gen)) == process_id_)
    tasks_requested_ = true;
    protocol_tools_.RequestTasks(thread_id_, dest_proc,THREAD_RECV_TASKS);
}

inline bool TaskManager::can_stop() 
{
    return producer_.endJob() && !tasks_requested_;
}