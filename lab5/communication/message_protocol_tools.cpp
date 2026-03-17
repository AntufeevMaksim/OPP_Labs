#include "message_protocol_tools.hpp"

#include "serializer.hpp"
#include "request_tasks_message.hpp"
#include "send_tasks_message.hpp"
#include "message_factory.hpp"

#include <stdio.h> //

#include <exception>
#include <mpi.h>

void MessageProtocolTools::RecvTasks(std::unique_ptr<SendTasksMessage>& message, ThreadSafeQueue<std::unique_ptr<ITask>>& queue) 
{
    printf("recived\n");
    fflush(stdout);
    for (auto& task : message->tasks())
    {
        queue.push(std::move(task));
    }
}



void MessageProtocolTools::SendTasks(
    std::vector<std::unique_ptr<ITask>> &tasks,
    int dest_process_id,
    uint32_t dest_thread_id)
{
    printf("sended\n");
    fflush(stdout);
    if (send_tasks_status != MPI_REQUEST_NULL)
    {
        MPI_Wait(&send_tasks_status, MPI_STATUS_IGNORE);
    }

    SendTasksMessage message(dest_thread_id, std::move(tasks));
    send_tasks_data = message.serialize();

    MPI_Isend(send_tasks_data.data(),
              send_tasks_data.size(),
              MPI_BYTE,
              dest_process_id,
              dest_thread_id,
              MPI_COMM_WORLD,
              &send_tasks_status
            );
}

void MessageProtocolTools::RequestTasks(uint32_t src_thread_id, int dest_process_id, uint32_t dest_thread_id) 
{
    RequestTasksMessage message(src_thread_id);
    std::vector<uint8_t> data = message.serialize();
    MPI_Send(data.data(), data.size(), MPI_BYTE, dest_process_id, dest_thread_id, MPI_COMM_WORLD);
}


std::unique_ptr<Message> MessageProtocolTools::TryRead(int process, uint32_t thread)
{
    MPI_Status status;
    int has_message;
    MPI_Iprobe(process, thread, MPI_COMM_WORLD, &has_message, &status);

    if (!has_message)
        return nullptr;

    int recv_size;
    MPI_Get_count(&status, MPI_BYTE, &recv_size);

    std::vector<uint8_t> recv_data(recv_size);
    MPI_Recv(recv_data.data(), recv_size, MPI_BYTE, process, thread, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    std::unique_ptr<Message> message = MessageFactory::deserialize(recv_data);
    return message;
}
