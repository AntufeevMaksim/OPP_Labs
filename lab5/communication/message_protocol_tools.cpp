#include "message_protocol_tools.hpp"

#include "serializer.hpp"

#include <exception>
#include <mpi.h>

std::vector<std::unique_ptr<ITask>> MessageProtocolTools::GetTasks(int src_thread_id, int dest_process_id)
{
    printf("run get tasks\n");
    fflush(stdout);
    std::vector<uint8_t> send_data = std::vector<uint8_t>(sizeof(uint8_t) + sizeof(uint64_t));
    send_data[0] = static_cast<uint8_t>(MessageType::REQUEST_TASKS);
    memcpy(send_data.data() + 1, &dest_process_id, sizeof(int));

    MPI_Send(send_data.data(), send_data.size(), MPI_BYTE, dest_process_id, THREAD_RECV_TASKS, MPI_COMM_WORLD);

    int recv_size;
    MPI_Status status;
    MPI_Probe(dest_process_id, src_thread_id, MPI_COMM_WORLD, &status);
    MPI_Get_count(&status, MPI_BYTE, &recv_size);

    std::vector<uint8_t> recv_data(recv_size);
    MPI_Recv(recv_data.data(), recv_size, MPI_BYTE, dest_process_id, src_thread_id, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    // |-----0byte------|------1byte-----|-----2:10byte-----|----'   8byte      ' 4byte   '     task_data_size bytes    '--|
    // |--message_type--|--has_tasks--|-----tasks_count--|----'Task_data_size' task_id '       task_data             '--|
    MessageType recv_type = static_cast<MessageType>(recv_data[0]);

    if (recv_type != MessageType::SEND_TASKS)
    {
        throw std::runtime_error("invalid message type in " + std::to_string(src_thread_id));
    }

    std::vector<std::unique_ptr<ITask>> res;

    if (recv_data[1])
    {
        uint64_t tasks_count;
        memcpy(&tasks_count, recv_data.data() + 2, sizeof(uint64_t));

        auto it = recv_data.begin() + 2 + sizeof(uint64_t);

        for (uint64_t i = 0; i < tasks_count; ++i)
        {
            uint64_t task_data_size;
            memcpy(&task_data_size, &(*it), sizeof(uint64_t));
            it += sizeof(uint64_t);

            int task_id;
            memcpy(&task_id, &(*it), sizeof(int));
            it += sizeof(int);

            auto task_begin = it;
            auto task_end = it + task_data_size;

            if (task_end > recv_data.end())
            {
                throw std::runtime_error("corrupted task data");
            }

            auto task = Serializer::Deserialize(task_id, task_begin, task_end);
            res.push_back(std::move(task));

            it += task_data_size;
        }
    }

    return res;
}

void MessageProtocolTools::SendTasks(
    const std::vector<std::unique_ptr<ITask>> &tasks,
    int dest_process_id,
    int dest_thread_id)
{
    printf("run send tasks\n");
    fflush(stdout);
    std::vector<uint8_t> send_data(10);

    send_data[0] = static_cast<uint8_t>(MessageType::SEND_TASKS);
    send_data[1] = tasks.empty() ? 0 : 1;

    uint64_t tasks_count = tasks.size();
    memcpy(send_data.data() + 2, &tasks_count, sizeof(uint64_t));

    for (const auto &task_ptr : tasks)
    {
        const ITask &task = *task_ptr;

        std::vector<uint8_t> task_data = Serializer::Serialize(task);

        uint64_t task_data_size = task_data.size();
        int task_id = static_cast<int>(task.getId());

        size_t old_size = send_data.size();
        send_data.resize(
            old_size +
            sizeof(uint64_t) +
            sizeof(int) +
            task_data_size);

        uint8_t *ptr = send_data.data() + old_size;

        memcpy(ptr, &task_data_size, sizeof(uint64_t));
        ptr += sizeof(uint64_t);

        memcpy(ptr, &task_id, sizeof(int));
        ptr += sizeof(int);

        memcpy(ptr, task_data.data(), task_data_size);
    }

    MPI_Send(
        send_data.data(),
        send_data.size(),
        MPI_BYTE,
        dest_process_id,
        dest_thread_id,
        MPI_COMM_WORLD);
}

std::vector<uint8_t> MessageProtocolTools::TryRead(int process, int thread)
{
    MPI_Status status;
    int has_message;
    MPI_Iprobe(process, thread, MPI_COMM_WORLD, &has_message, &status);

    if (!has_message)
        return std::vector<uint8_t>();


    int recv_size;
    MPI_Get_count(&status, MPI_BYTE, &recv_size);

    std::vector<uint8_t>recv_message(recv_size);

    MPI_Recv(recv_message.data(), recv_size, MPI_BYTE, process, thread, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    return recv_message;
}

MessageType MessageProtocolTools::GetType(const std::vector<uint8_t>& message) 
{
    return static_cast<MessageType>(message[0]);
}