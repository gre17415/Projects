# Lab 1: Echo Client with Exponential Backoff

## Overview

In this lab, you will implement a network client that communicates with an unreliable echo server. The server randomly fails to respond to requests, so your client must implement a **retry mechanism with exponential backoff and jitter**.

## Learning Objectives

- Understand exponential backoff as a retry strategy
- Learn why jitter is important in distributed systems
- Practice using the Nuclear framework's deterministic simulation
- Implement robust error handling for network failures

## Background

### What is Exponential Backoff?

When a network request fails, immediately retrying often doesn't help - the server might still be overloaded or unavailable. **Exponential backoff** is a strategy where you wait progressively longer between retries:

- 1st retry: wait 100ms
- 2nd retry: wait 200ms
- 3rd retry: wait 400ms
- 4th retry: wait 800ms
- etc.

The delay doubles with each attempt: `delay = base_delay * 2^attempt`

### What is Jitter?

If many clients all fail at the same time and use the same retry schedule, they'll all retry simultaneously - causing a **thundering herd** that overwhelms the server again.

**Jitter** adds randomness to the delay to spread out retries:

```
delay = base_delay * 2^attempt + random(0, base_delay * 2^attempt * 0.1)
```

This adds up to 10% random variation to each delay.

## Your Task

You need to implement the `calculate_retry_delay()` method in `echo_client.cpp`.

### Files You Can Edit

- `echo_client.h` - Client class header (you can modify if needed)
- `echo_client.cpp` - Client implementation (contains TODO)

### Files You Cannot See

The test infrastructure (server, test harness) is hidden from you. Your code will be tested against a server with random failures.

## Implementation Requirements

1. **Exponential Backoff**: Each retry should wait progressively longer (delays should grow)
2. **Reasonable Delays**: Delays should be in a reasonable range (not too short, not too long)
3. **Base Delay**: Use the `BASE_DELAY` constant (100 time units)
4. **Suggested Formula**: `delay = BASE_DELAY * 2^attempt + jitter`
   - Where `jitter` is a random value to add variation
   - But you can use other exponential backoff strategies too!

## Code Structure

The `EchoClient` class is already implemented with retry logic. You only need to fix the `calculate_retry_delay()` method:

```cpp
SimTime calculate_retry_delay(int attempt) {
    // TODO: Implement exponential backoff with jitter
    // Suggested formula: BASE_DELAY * 2^attempt + jitter
    // But you can use other strategies as long as delays grow exponentially
    return BASE_DELAY;  // WRONG - students must fix
}
```

### Available Methods

You can use these methods from the `NetworkNode` base class:

- `random_number()` - Returns a random uint64_t value
- `info(format, ...)` - Log informational messages
- `warning(format, ...)` - Log warnings
- `error(format, ...)` - Log errors

## Building and Testing

### Local Build

```bash
cd homeworks/nuclear_lab1/lab1_echo
make
```

This will compile your code and run the test.

## Resources

- [Exponential Backoff on Wikipedia](https://en.wikipedia.org/wiki/Exponential_backoff)
- [AWS Architecture Blog: Exponential Backoff and Jitter](https://aws.amazon.com/blogs/architecture/exponential-backoff-and-jitter/)
- Nuclear Framework documentation: `nuclear/include/nuclear_student.h`

## Questions?

If you have questions about:
- The Nuclear framework API: check `nuclear/include/nuclear_student.h`
- The assignment requirements: re-read this README
- Build errors: check that you're using C++23 and linking against libnuclear.a

Good luck! 🚀
