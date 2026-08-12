#include "format_duration.hpp"

#include <fmt/format.h>

#include <chrono>

std::string format_duration(float seconds) {
    using namespace std::chrono;
    hh_mm_ss hms{round<milliseconds>(duration<float>(seconds))};
    return fmt::format(
        "{}{}:{:02}.{:03}",
        hms.is_negative() ? "-" : "",
        hms.hours().count() * 60 + hms.minutes().count(),
        hms.seconds().count(),
        hms.subseconds().count()
    );
}
