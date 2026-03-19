#pragma once

#include "message.hpp"

class StopProgramMessage : public Message
{
public:
    StopProgramMessage(uint32_t thread_id);
    std::vector<uint8_t> serialize() const override;
};