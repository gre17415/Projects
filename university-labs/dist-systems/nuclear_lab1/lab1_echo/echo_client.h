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
 *
 * YOUR TASK:
 * Implement the calculate_retry_delay() method to use exponential backoff with jitter.
 *
 * Requirements:
 * - Exponential backoff
 * - Jitter: add random variation
 *
 * Example:
 * - Attempt 0: 100 + jitter(0-10) = 100-110
 * - Attempt 1: 200 + jitter(0-20) = 200-220
 * - Attempt 2: 400 + jitter(0-40) = 400-440
 * - Attempt 3: 800 + jitter(0-80) = 800-880
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
    // Protected so test class can override it
    virtual SimTime calculate_retry_delay(int attempt);

private:
    NodeId server_id_;

    static constexpr int MAX_RETRIES = 5;
};

} // namespace nuclear::labs
