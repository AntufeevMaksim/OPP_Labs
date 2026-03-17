#include "message.hpp"


Message::Message(MessageType type, uint32_t thread_id)
        : type_(type), thread_id_(thread_id) {}


        
MessageType Message::type() const
{
        return type_;
}

uint32_t Message::threadId() const {
        return thread_id_;
}