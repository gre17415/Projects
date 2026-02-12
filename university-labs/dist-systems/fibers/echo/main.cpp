#include "echo.h"

#include <userver/concurrent/background_task_storage.hpp>

#include <userver/engine/async.hpp>

#include <gtest/gtest.h>

#include <memory>

using namespace std::literals;

class Echo : public ::testing::Test {
protected:
    void SetUp() override {
        Connection_ = concurrent::MpscQueue<ConnectionPtr>::Create();

        Bts_.CriticalAsyncDetach("server", [queue = Connection_] {
            auto p = queue->GetProducer();
            ServeForever(queue->GetConsumer());
        });
    }

    void TearDown() override {
        Bts_.CancelAndWait();
    }

    EchoClient MakeEchoClient() {
        Channel channel(Connection_->GetProducer());
        return EchoClient(std::move(channel));
    }

private:
    std::shared_ptr<concurrent::MpscQueue<ConnectionPtr>> Connection_;
    concurrent::BackgroundTaskStorage Bts_;
};

UTEST_F_MT(Echo, Simple, 2) {
    auto client = MakeEchoClient();

    std::string message = "hello world";
    client.Write(message);

    auto response = client.Read(message.size()).get();
    ASSERT_EQ(response, message);

    message = "value";
    client.Write(message);

    auto resp = client.ReadAll();
    client.StopWrite();

    ASSERT_EQ(resp.get(), message);
}

UTEST_F_MT(Echo, Stop, 2) {
    const std::string message = "test stop writer";

    auto client = MakeEchoClient();

    client.Write(message);
    client.StopWrite();

    ASSERT_EQ(client.ReadAll().get(), message);
}

UTEST_F_MT(Echo, Clients, 3) {
    auto run = [this](std::string msg) {
        auto client = MakeEchoClient();

        std::string resp;

        client.Write(msg);
        client.Write(" ");

        resp += client.Read(5).get();
        resp += client.Read(5).get();

        client.Write("message");

        resp += client.Read(5).get();
        resp += client.Read(5).get();

        ASSERT_EQ(resp, msg + " message");
    };

    auto first = engine::AsyncNoSpan(run, "first"s);
    auto second = engine::AsyncNoSpan(run, "second"s);

    second.Get();
    first.Get();
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
