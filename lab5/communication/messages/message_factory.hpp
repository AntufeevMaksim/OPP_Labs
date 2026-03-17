#pragma once

#include "message.hpp"

#include <memory>

class MessageFactory
{
public:
    static std::unique_ptr<Message> deserialize(std::vector<uint8_t>& data);

private:
    static const size_t HEADER_SIZE = sizeof(MessageType) + sizeof(uint32_t);
    static std::unique_ptr<Message> deserializeSendTasks(
        uint32_t thread_id,
        std::vector<uint8_t>& data);
};