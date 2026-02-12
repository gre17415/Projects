#include "echo.h"
#include <userver/engine/async.hpp>
#include <algorithm>
#include <limits>
#include <cstdint>

void Channel::Write(std::string message) 
{
    auto chunk = std::make_unique<std::string>(std::move(message));
    if (!Writer_.PushNoblock(std::move(chunk)))
        throw std::runtime_error("Failed to send message");
}

engine::Future<std::string> Channel::Read(int count) 
{
    engine::Promise<std::string> promise;
    auto ftr = promise.get_future();
    engine::AsyncNoSpan([this, count, promise = std::move(promise)]() mutable {
        std::string result;
        if (count <= 0) 
        {
            promise.set_value(std::move(result));
            return;
        }
        size_t remaining = count;
        if (!State_.buffer.empty()) 
        {
            size_t take = std::min(State_.buffer.size(), remaining);
            result.append(State_.buffer, 0, take);
            State_.buffer.erase(0, take);
            remaining -= take;
        }
        bool has_data = !result.empty();
        while (remaining > 0) 
        {
            DataPart chunk;
            bool success = has_data ? Reader_.PopNoblock(chunk) : Reader_.Pop(chunk);
            if (!success) break;
            size_t take = std::min(chunk->size(), remaining);
            result.append(*chunk, 0, take);
            if (take < chunk->size()) 
            {
                State_.buffer = chunk->substr(take);
            }
            remaining -= take;
            has_data = true;
        }
        promise.set_value(std::move(result));
    }).Detach();
    
    return ftr;
}

EchoClient::EchoClient(Channel ch): Channel_(std::move(ch)) {}

void EchoClient::Write(std::string msg) {
    Channel_.Write(std::move(msg));
}

engine::Future<std::string> EchoClient::Read(int n) {
    return Channel_.Read(n);
}

engine::Future<std::string> EchoClient::ReadAll() {
    return Channel_.Read(1024);
}

void ServeForever(concurrent::MpscQueue<ConnectionPtr>::Consumer queue) 
{
    while (true)
    {
        ConnectionPtr ptr_cnnct;
        if (!queue.Pop(ptr_cnnct)) break;
        engine::AsyncNoSpan([ptr = std::move(ptr_cnnct)]() mutable {
            auto &input = ptr->Input;
            auto &output = ptr->Output;
            while (true) 
            {
                DataPart chunk;
                if (!input.Pop(chunk)) break;
                if (!output.PushNoblock(std::move(chunk))) break;
            }
            std::move(output).Reset();
        }).Detach();
    }
}