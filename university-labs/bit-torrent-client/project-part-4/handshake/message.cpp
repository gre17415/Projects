#include "message.h"

Message Message::Parse(const std::string& messageString) {
    if (messageString.empty())
        return {MessageId::KeepAlive, 0, ""};

    MessageId id = static_cast<MessageId>(messageString[0]);
    std::string payload = messageString.substr(1);
    return {id, messageString.size(), payload};
}

Message Message::Init(MessageId id, const std::string& payload) {
    if (id == MessageId::KeepAlive)
        return {MessageId::KeepAlive, 0, ""};
    return {id, payload.size() + 1, payload};
}

std::string Message::ToString() const {
    if (id == MessageId::KeepAlive)
        return std::string(4, '\0');
    std::string res = IntToBytes(messageLength);
    res += static_cast<unsigned char>(id);
    res += payload;
    return res;
}