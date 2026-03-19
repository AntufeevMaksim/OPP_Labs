#include "stop_program_message.hpp"

#include "memory.h"

StopProgramMessage::StopProgramMessage(uint32_t thread_id)
    : Message(MessageType::STOP_PROGRAM, thread_id)
{
}



std::vector<uint8_t> StopProgramMessage::serialize() const 
{
    std::vector<uint8_t> data(sizeof(MessageType) + sizeof(uint32_t));

    data[0] = static_cast<uint8_t>(type_);

    memcpy(
        data.data() + sizeof(MessageType),
        &thread_id_,
        sizeof(uint32_t));

    return data;
}    
