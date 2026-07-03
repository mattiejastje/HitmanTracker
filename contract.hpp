#pragma once

#include <spdlog/spdlog.h>

#include <stdexcept>

namespace contract {

// stops execution
[[noreturn]] inline void fail_require(const char* expr) {
    spdlog::critical("REQUIRE FAILED ! {}", expr);
    throw std::logic_error(expr);
}

// just log error, continue execution
inline void fail_check(const char* expr) {
    spdlog::error("CHECK FAILED ! {}", expr);
}

}  // namespace contract

#define APP_REQUIRE(expr)                           \
    do {                                            \
        if (!(expr)) contract::fail_require(#expr); \
    } while (false)

#define APP_CHECK(expr)                           \
    do {                                          \
        if (!(expr)) contract::fail_check(#expr); \
    } while (false)
