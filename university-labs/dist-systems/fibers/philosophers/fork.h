#pragma once

#include <userver/engine/mutex.hpp>

struct Fork {
    userver::engine::Mutex mtx;
};