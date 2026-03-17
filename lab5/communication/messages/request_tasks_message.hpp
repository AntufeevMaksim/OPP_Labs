#pragma once

#include "message.hpp"

class RequestTasksMessage : public Message
{
public:
    RequestTasksMessage(uint32_t thread_id);

    std::vector<uint8_t> serialize() const override;
};