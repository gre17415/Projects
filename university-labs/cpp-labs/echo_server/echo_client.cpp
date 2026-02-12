#include "echo_client.h"

namespace nuclear::labs {

EchoClient::EchoClient(NodeId id, NodeId server_id)
    : NetworkNode(id), server_id_(server_id) {}

void EchoClient::on_start() {
    info("Echo client starting, server is node {}", server_id_);
}

void EchoClient::on_message(const Message& msg) {
    info("Received unexpected message: {}", msg.payload);
}

void EchoClient::on_stop() {
    info("Echo client shutting down");
}

void EchoClient::send_echo_request(const std::string& message) {
    info("Sending echo request: {}", message);
    int attempt = 0;
    bool success = false;
    while (!success) {
        auto response = send_request(server_id_, "echo_request", message, calculate_retry_delay(attempt)).get();
        if (response.has_value() && response->is_ok()) {
            info("Success! Received echo: {}", response->payload);
            success = true;
        }
        attempt++;
    }
}

SimTime EchoClient::calculate_retry_delay(int attempt) {
    constexpr SimTime BASE_DELAY = 100;
    SimTime base_delay = BASE_DELAY * (1 << attempt);
    SimTime max_jitter = base_delay / 10;
    SimTime jitter = random_number() % (max_jitter + 1);
    SimTime total_delay = base_delay + jitter;
    info("Retry attempt {}: base_delay={}, jitter={}, total_delay={}", attempt, base_delay, jitter, total_delay);
    return total_delay;
}

} // namespace nuclear::labs
