#include <gtest/gtest.h>
#include "echo_client.h"
#include <vector>
#include <cmath>
#include <memory>

namespace nuclear::labs {

// Global data storage for test results
struct TestResults {
    std::vector<SimTime> retry_delays;
    std::vector<SimTime> timeout_values;  // Client-side timeout values from single client tests
    std::map<NodeId, std::vector<SimTime>> client_timeouts;  // Per-client timeout values for jitter test
    size_t request_count = 0;
};
static TestResults g_test_results;

// ============================================================================
// Echo Server with Message Tracking
// ============================================================================

class EchoServerWithTracking : public student::NetworkNode {
public:
    explicit EchoServerWithTracking(NodeId id, double failure_rate = 0.5)
        : NetworkNode(id), failure_rate_(failure_rate) {}

    void on_start() override {
        info("Echo server starting (failure rate: {}%)", failure_rate_ * 100);
    }

    void on_message(const Message& msg) override {
        if (msg.type == "echo_request") {
            SimTime current = current_time();

            // Track requests by sender and payload to detect retries
            // A retry is when we see the same (sender, payload) combination again
            std::string request_key = std::to_string(msg.from) + ":" + msg.payload;

            // Check if this is a retry
            auto it = last_request_times_.find(request_key);
            if (it != last_request_times_.end()) {
                SimTime delay = current - it->second;
                if (delay > 50) {  // Filter out spurious duplicates
                    retry_delays_.push_back(delay);
                    info("RETRY detected! Request from node {}, Delay since last attempt: {} time units",
                         msg.from, delay);
                }
            }

            // Update last request time for this key
            last_request_times_[request_key] = current;
            request_timestamps_.push_back(current);

            // Simulate server being overloaded/slow
            if (should_fail()) {
                warning("Simulating server overload - delaying response");
                // Sleep long enough that client timeout will trigger
                sleep(1500).get();
                // By the time we try to respond, client has already timed out
                // But we still try to send response (it will be ignored by client)
            }

            // Send response - echo back the payload as-is
            std::string response_payload = msg.payload + " (echoed)";
            send_response(msg, Response::ok(response_payload)).get();
            info("Sent echo response");
        }
    }

    void on_stop() override {
        info("Echo server shutting down");
        info("Total requests received: {}", request_timestamps_.size());
        info("Retry delays observed: {}", format_delays());

        // Save results to global storage BEFORE destruction
        g_test_results.retry_delays = retry_delays_;
        g_test_results.request_count = request_timestamps_.size();
    }

    const std::vector<SimTime>& get_retry_delays() const { return retry_delays_; }
    size_t get_request_count() const { return request_timestamps_.size(); }

private:
    double failure_rate_;
    std::vector<SimTime> request_timestamps_;
    std::vector<SimTime> retry_delays_;
    std::unordered_map<std::string, SimTime> last_request_times_;  // Track by (sender:payload)

    bool should_fail() {
        if (failure_rate_ <= 0.0) return false;
        uint64_t rand_val = random_number();
        double rand_prob = (rand_val % 10000) / 10000.0;
        return rand_prob < failure_rate_;
    }

    std::string format_delays() {
        std::string result = "[";
        for (size_t i = 0; i < retry_delays_.size(); i++) {
            if (i > 0) result += ", ";
            result += std::to_string(retry_delays_[i]);
        }
        result += "]";
        return result;
    }
};

} // namespace nuclear::labs

// ============================================================================
// GTest Test Cases
// ============================================================================

using namespace nuclear;
using namespace nuclear::labs;

// Test client wrapper that sends specific messages and tracks timeout values
class TestEchoClient : public EchoClient {
public:
    TestEchoClient(NodeId id, NodeId server_id, int num_messages = 3, bool track_per_client = false)
        : EchoClient(id, server_id), num_messages_(num_messages), track_per_client_(track_per_client), my_id_(id) {}

    void on_start() override {
        EchoClient::on_start();  // Call parent on_start

        // Send unique messages with delays between them
        for (int i = 0; i < num_messages_; i++) {
            if (i > 0) {
                sleep(500).get();  // Delay between messages
            }
            std::string msg = "msg_" + std::to_string(i);
            send_echo_request(msg);
        }
    }

    void on_stop() override {
        // Save timeout values to global storage before destruction
        if (track_per_client_) {
            g_test_results.client_timeouts[my_id_] = timeout_history_;
        } else {
            g_test_results.timeout_values = timeout_history_;
        }
        EchoClient::on_stop();  // Call parent on_stop
    }

protected:
    // Override to track timeout values for testing
    SimTime calculate_retry_delay(int attempt) override {
        SimTime timeout = EchoClient::calculate_retry_delay(attempt);
        timeout_history_.push_back(timeout);
        return timeout;
    }

private:
    int num_messages_;
    bool track_per_client_;
    NodeId my_id_;  // Store node ID for later use
    std::vector<SimTime> timeout_history_;  // Track timeouts in test client
};

class EchoLabTest : public ::testing::Test {
protected:
    std::vector<SimTime> retry_delays_;
    std::vector<SimTime> timeout_values_;  // Client-side timeout values
    size_t request_count_ = 0;

    void SetUp() override {
        g_test_results.retry_delays.clear();
        g_test_results.timeout_values.clear();
        g_test_results.request_count = 0;
        retry_delays_.clear();
        timeout_values_.clear();
        request_count_ = 0;
    }

    void RunSimulation(double failure_rate, SimTime duration, int num_messages = 3) {
        std::vector<std::function<std::unique_ptr<student::NetworkNode>()>> factories;

        // Server with message tracking
        factories.push_back([failure_rate]() {
            return std::make_unique<EchoServerWithTracking>(1, failure_rate);
        });

        // Test client that sends specific messages
        factories.push_back([num_messages]() {
            return std::make_unique<TestEchoClient>(2, 1, num_messages);
        });

        student::run_simulation(std::move(factories), duration);

        // Copy data from global storage (saved in on_stop methods)
        retry_delays_ = g_test_results.retry_delays;
        timeout_values_ = g_test_results.timeout_values;
        request_count_ = g_test_results.request_count;
    }
};

TEST_F(EchoLabTest, ClientSucceedsWithNoFailures) {
    // With 0% failure rate, client should succeed immediately
    ASSERT_NO_THROW(RunSimulation(0.0, 5000));

    // Should have received multiple requests (no retries needed)
    EXPECT_GE(request_count_, 1UL);
}

TEST_F(EchoLabTest, ExponentialBackoffPattern) {
    // Run simulation with 100% failure rate and 1 message to get all retry attempts
    // This ensures we see the full exponential backoff pattern: 100, 200, 400, 800, 1600
    ASSERT_NO_THROW(RunSimulation(1.0, 20000, 1));

    // Should have at least 5 timeout values (initial + 4 retries minimum)
    // With 100% failure rate and eventual success, we typically get 5 values
    ASSERT_GE(timeout_values_.size(), 5UL)
        << "Expected at least 5 timeout values, got " << timeout_values_.size();

    // Verify the exponential pattern with jitter
    // Expected base values: 100, 200, 400, 800, 1600 (and possibly 3200 if all fail)
    // With 0-10% jitter: [100-110], [200-220], [400-440], [800-880], [1600-1760]
    std::vector<SimTime> expected_base = {100, 200, 400, 800, 1600, 3200};

    for (size_t i = 0; i < std::min(timeout_values_.size(), expected_base.size()); i++) {
        SimTime base = expected_base[i];
        SimTime min_val = base;  // Base value (no jitter)
        SimTime max_val = base + base / 10;  // Base + 10% jitter

        EXPECT_GE(timeout_values_[i], min_val)
            << "Timeout " << i << " (" << timeout_values_[i]
            << ") should be at least " << min_val;

        EXPECT_LE(timeout_values_[i], max_val)
            << "Timeout " << i << " (" << timeout_values_[i]
            << ") should be at most " << max_val;
    }

    // Also check exponential growth between consecutive values
    for (size_t i = 1; i < timeout_values_.size(); i++) {
        double ratio = static_cast<double>(timeout_values_[i]) / timeout_values_[i-1];

        EXPECT_GE(ratio, 1.8)
            << "Timeout " << i << " (" << timeout_values_[i]
            << ") should be at least 1.8x previous (" << timeout_values_[i-1] << "). "
            << "Ratio: " << ratio << ". This suggests missing exponential backoff.";

        EXPECT_LE(ratio, 2.2)
            << "Timeout " << i << " (" << timeout_values_[i]
            << ") should be at most 2.2x previous (" << timeout_values_[i-1] << "). "
            << "Ratio: " << ratio << ". This suggests incorrect exponential growth.";
    }
}

TEST_F(EchoLabTest, DelaysWithinReasonableRange) {
    // Run simulation with 80% failure rate
    ASSERT_NO_THROW(RunSimulation(0.8, 20000));

    ASSERT_GE(timeout_values_.size(), 1UL) << "No timeout values observed";

    // Check that timeout values are within reasonable range
    // First timeout should be around BASE_DELAY (100), allow 50-200
    // Later timeouts grow exponentially but shouldn't exceed 5000
    for (size_t i = 0; i < timeout_values_.size(); i++) {
        EXPECT_GE(timeout_values_[i], SimTime(50))
            << "Timeout " << i << " (" << timeout_values_[i] << ") is too small. "
            << "Should have reasonable base delay.";
        EXPECT_LE(timeout_values_[i], SimTime(5000))
            << "Timeout " << i << " (" << timeout_values_[i] << ") is too large. "
            << "Backoff should not wait excessively long.";
    }

    // First timeout should be close to BASE_DELAY (100)
    if (timeout_values_.size() > 0) {
        EXPECT_GE(timeout_values_[0], SimTime(80))
            << "First timeout should be around BASE_DELAY (100), got " << timeout_values_[0];
        EXPECT_LE(timeout_values_[0], SimTime(150))
            << "First timeout should be around BASE_DELAY (100), got " << timeout_values_[0];
    }
}

TEST_F(EchoLabTest, BackoffHasVariation) {
    // Run simulation with multiple clients and high failure rate
    // This ensures we get enough retries to compare timeout values across clients
    const double failure_rate = 0.9;  // 90% failure rate
    const int num_clients = 3;
    const int num_messages = 3;
    const SimTime duration = 30000;

    std::vector<std::function<std::unique_ptr<student::NetworkNode>()>> factories;

    // Create server with high failure rate
    factories.push_back([failure_rate]() {
        return std::make_unique<EchoServerWithTracking>(1, failure_rate);
    });

    // Create multiple clients that track timeouts per-client
    for (int i = 0; i < num_clients; i++) {
        factories.push_back([i, num_messages]() {
            return std::make_unique<TestEchoClient>(2 + i, 1, num_messages, true);  // track_per_client=true
        });
    }

    ASSERT_NO_THROW(student::run_simulation(std::move(factories), duration));

    // Verify we have timeout data from multiple clients
    ASSERT_GE(g_test_results.client_timeouts.size(), 2UL)
        << "Need at least 2 clients with timeout data to verify jitter";

    // Collect timeout values for each attempt number across all clients
    std::map<int, std::vector<SimTime>> timeouts_by_attempt;

    for (const auto& [client_id, timeout_values] : g_test_results.client_timeouts) {
        for (size_t i = 0; i < timeout_values.size(); i++) {
            timeouts_by_attempt[i].push_back(timeout_values[i]);
        }
    }

    // Check ONLY Attempt 0 - the very first timeout value from each client
    // This is the only attempt where we can be certain all clients are at the same stage
    bool has_variation = false;

    if (timeouts_by_attempt.count(0) > 0) {
        const auto& attempt0_timeouts = timeouts_by_attempt[0];

        // Need at least 2 clients to compare
        if (attempt0_timeouts.size() >= 2) {
            // Check if all values are identical
            SimTime first_value = attempt0_timeouts[0];

            for (size_t i = 1; i < attempt0_timeouts.size(); i++) {
                if (attempt0_timeouts[i] != first_value) {
                    has_variation = true;
                    break;
                }
            }
        }
    }

    // Jitter is REQUIRED to prevent thundering herd
    EXPECT_TRUE(has_variation)
        << "All clients use identical timeout values for attempt 0, indicating no jitter. "
        << "Without jitter, all clients retry at exactly the same time, causing thundering herd. "
        << "Add random variation (e.g., 0-10% of backoff delay) to each timeout.";
}

// Global tracker for message overload test
namespace {
    struct MessageTracker {
        size_t total_messages_sent = 0;
        size_t messages_handled = 0;
        size_t messages_dropped = 0;
    };

    static MessageTracker g_message_tracker;

    // Server that tracks all incoming messages
    class TrackingServer : public student::NetworkNode {
    public:
        explicit TrackingServer(NodeId id, double failure_rate)
            : NetworkNode(id), failure_rate_(failure_rate) {}

        void on_start() override {
            info("Tracking server starting (failure rate: {}%)", failure_rate_ * 100);
        }

        void on_message(const Message& msg) override {
            if (msg.type == "echo_request") {
                g_message_tracker.total_messages_sent++;

                // Simulate server being overloaded/slow
                if (should_fail()) {
                    g_message_tracker.messages_dropped++;
                    warning("Simulating server overload - delaying response");
                    // Sleep long enough that client timeout (1000) will trigger
                    sleep(1500).get();
                    // Client has timed out, but we still send response (will be ignored)
                } else {
                    g_message_tracker.messages_handled++;
                }

                // Extract actual payload
                std::string actual_payload;
                size_t payload_pos = msg.payload.find("|payload:");
                if (payload_pos != std::string::npos) {
                    actual_payload = msg.payload.substr(payload_pos + 9);
                } else {
                    actual_payload = msg.payload;
                }

                std::string response_payload = actual_payload + " (echoed)";
                send_response(msg, Response::ok(response_payload)).get();
            }
        }

        void on_stop() override {
            info("Tracking server shutting down");
            info("Total messages received: {}", g_message_tracker.total_messages_sent);
            info("Messages handled: {}", g_message_tracker.messages_handled);
            info("Messages dropped: {}", g_message_tracker.messages_dropped);
        }

    private:
        double failure_rate_;

        bool should_fail() {
            if (failure_rate_ <= 0.0) return false;
            uint64_t rand_val = random_number();
            double rand_prob = (rand_val % 10000) / 10000.0;
            return rand_prob < failure_rate_;
        }
    };
}

// Test for message overload - checks that clients don't overwhelm the server
TEST_F(EchoLabTest, NoMessageOverload) {
    // Reset tracker
    g_message_tracker = MessageTracker{};

    // Run simulation with multiple clients to stress test
    std::vector<std::function<std::unique_ptr<student::NetworkNode>()>> factories;

    // One server with 70% failure rate
    factories.push_back([]() {
        return std::make_unique<TrackingServer>(1, 0.7);
    });

    // Three test clients - each sends 3 unique messages
    for (int i = 0; i < 3; i++) {
        factories.push_back([i]() {
            return std::make_unique<TestEchoClient>(2 + i, 1, 3);
        });
    }

    ASSERT_NO_THROW(student::run_simulation(std::move(factories), 30000));

    // Validate results
    ASSERT_GT(g_message_tracker.total_messages_sent, 0UL) << "No messages were sent";
    ASSERT_GT(g_message_tracker.messages_handled, 0UL) << "No messages were handled successfully";

    // Key validation: Check that clients don't generate excessive messages
    // With 3 clients, each trying to send ~3 messages with up to 5 retries:
    // - Theoretical minimum: 3 clients * 3 messages = 9 messages (if all succeed first try)
    // - With exponential backoff and 70% failure rate: ~15-20 messages
    // - Without exponential backoff (constant retry): ~23+ messages
    // - Set limit to 20 to catch implementations without proper backoff

    EXPECT_LE(g_message_tracker.total_messages_sent, 20UL)
        << "Too many messages sent (" << g_message_tracker.total_messages_sent
        << "). Clients may be retrying too aggressively or not implementing exponential backoff correctly.";

    // Check that we're not sending too few messages either (clients should retry on failure)
    EXPECT_GE(g_message_tracker.total_messages_sent, 9UL)
        << "Too few messages sent (" << g_message_tracker.total_messages_sent
        << "). Clients should retry on failures.";

    // Calculate retry efficiency: ratio of handled to total
    double efficiency = static_cast<double>(g_message_tracker.messages_handled) / g_message_tracker.total_messages_sent;

    // With 70% failure rate and proper backoff, we expect:
    // - Some messages to succeed on first try (30% success rate)
    // - Others to require retries
    // - Efficiency should be reasonable (not too many wasted retries)
    // Allow efficiency between 10% and 90% (very generous range)
    EXPECT_GE(efficiency, 0.10)
        << "Efficiency too low (" << (efficiency * 100) << "%). "
        << "Too many messages are being dropped. Check retry logic.";

    EXPECT_LE(efficiency, 0.90)
        << "Efficiency suspiciously high (" << (efficiency * 100) << "%). "
        << "With 70% failure rate, not all messages should succeed immediately.";

    // Log statistics for debugging
    std::cout << "\n=== Message Overload Test Statistics ===\n";
    std::cout << "Total messages sent: " << g_message_tracker.total_messages_sent << "\n";
    std::cout << "Messages handled: " << g_message_tracker.messages_handled << "\n";
    std::cout << "Messages dropped: " << g_message_tracker.messages_dropped << "\n";
    std::cout << "Efficiency: " << (efficiency * 100) << "%\n";
    std::cout << "Average retries per handled message: "
              << (static_cast<double>(g_message_tracker.total_messages_sent) / g_message_tracker.messages_handled - 1.0) << "\n";
}

// Main function for GTest
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
