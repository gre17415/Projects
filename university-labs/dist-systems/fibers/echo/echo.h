#pragma once

#include "state.h"

#include <userver/concurrent/mpsc_queue.hpp>

#include <userver/engine/future.hpp>

#include <userver/utest/utest.hpp>

#include <memory>
#include <string>

using namespace USERVER_NAMESPACE;

using DataPart = std::unique_ptr<std::string>;

struct Connection {
    concurrent::MpscQueue<DataPart>::Consumer Input;
    concurrent::MpscQueue<DataPart>::Producer Output;
};

using ConnectionPtr = std::unique_ptr<Connection>;

class Channel {
public:
    explicit Channel(concurrent::MpscQueue<ConnectionPtr>::Producer server)
        : Writer_(concurrent::MpscQueue<DataPart>::Create()->GetProducer())
        , Reader_(concurrent::MpscQueue<DataPart>::Create()->GetConsumer())
    {
        auto mutRef = []<typename T>(const T& q) -> T& {
            return const_cast<T&>(q);
        };

        EXPECT_TRUE(server.Push(std::make_unique<Connection>(Connection{
            .Input = mutRef(*Writer_.Queue()).GetConsumer(),
            .Output = mutRef(*Reader_.Queue()).GetProducer(),
        })));
    }

    void Write(std::string msg);

    engine::Future<std::string> Read(int n);

    void StopWrite() {
        std::move(Writer_).Reset();
    }

private:
    concurrent::MpscQueue<DataPart>::Producer Writer_;
    concurrent::MpscQueue<DataPart>::Consumer Reader_;
    ChannelState State_;
};

class EchoClient {
public:
    explicit EchoClient(Channel channel);

    void Write(std::string msg);
    engine::Future<std::string> Read(int n);
    engine::Future<std::string> ReadAll();

    void StopWrite() {
        Channel_.StopWrite();
    }

private:
    Channel Channel_;
};

void ServeForever(concurrent::MpscQueue<ConnectionPtr>::Consumer connections);
