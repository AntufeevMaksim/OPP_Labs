#include "task_manager.hpp"

#include "message_protocol_tools.hpp"
#include <mpi.h>

#include <stdio.h> //

TaskManager::TaskManager(int process_count,int process_id, int thread_id)
    : process_id_{process_id},
      thread_id_{thread_id},
      process_count_{process_count},
      queue{std::make_shared<ThreadSafeQueue<std::unique_ptr<ITask>>>()}
{

}

void TaskManager::run()
{
  while (true)
  {
    
    int recv_size;
    int has_message;
    MPI_Status status;
    for (int src_proc = 0; src_proc < process_count_; ++src_proc)
    {
      printf("iter: %d\n", src_proc);
      fflush(stdout);
      
      std::vector<uint8_t> recv_message = MessageProtocolTools::TryRead(src_proc, THREAD_RECV_TASKS);

      if (recv_message.empty()) continue;

      MessageType type = MessageProtocolTools::GetType(recv_message);

      switch (type)
      {
      case MessageType::REQUEST_TASKS:
        answer_request_tasks(recv_message, src_proc);
        break;
      
      default:
        throw std::runtime_error("invalid message type in task manager");
        break;
      }
    }
  }
}


void TaskManager::answer_request_tasks(std::vector<uint8_t> message, int process)
{
  uint64_t thread_id;
  memcpy(&thread_id, message.data() + 1, sizeof(uint64_t));

  std::vector<std::unique_ptr<ITask>> tasks;
  
  auto [task, success] = queue->try_pop();

  if (success)
  {
    tasks.push_back(std::move(task));
  }

  MessageProtocolTools::SendTasks(tasks, process, thread_id);
}