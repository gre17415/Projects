#pragma once

#include <string>

struct ChannelState {
    std::string buffer;
    bool closed = false;
};