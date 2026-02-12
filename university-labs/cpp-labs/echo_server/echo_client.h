#pragma once

#include <nuclear/student/api.h>
#include <string>
#include <optional>

namespace nuclear::labs {

/**
 * Echo Client with Exponential Backoff and Jitter
 *
 * Sends messages to a server and receives echo responses.
 * Implements retry logic with exponential backoff and jitter
 * to handle server failures gracefully.
 */
class EchoClient : public student::NetworkNode {
public:
    explicit EchoClient(NodeId id, NodeId server_id);

    void on_start() override;
    void on_message(const Message& msg) override;
    void on_stop() override;

    // Public method for sending echo requests (can be called from tests)
    void send_echo_request(const std::string& message);

protected:
    // Protected to allow overriding in tests
    virtual SimTime calculate_retry_delay(int attempt);

private:
    NodeId server_id_;

    static constexpr int MAX_RETRIES = 5;
};

} // namespace nuclear::labs