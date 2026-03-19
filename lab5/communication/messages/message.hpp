#pragma once


#include <vector>
#include <cstdint>

enum class MessageType : uint8_t
{
    REQUEST_TASKS,
    SEND_TASKS,
    TASKS_STAT,
    STOP_PROGRAM
};

class Message
{
public:
    Message(MessageType type, uint32_t thread_id);

    virtual ~Message() = default;

    MessageType type() const;
    uint32_t threadId() const;

    virtual std::vector<uint8_t> serialize() const = 0;

protected:
    MessageType type_;
    uint32_t thread_id_;
};