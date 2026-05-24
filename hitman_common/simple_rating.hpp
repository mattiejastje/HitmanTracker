#pragma once

#include <string>

#include "../stats.hpp"

inline std::string get_simple_rating_value(Status status) {
    return status == Status::GREEN ? "Silent Assassin" : "No Silent Assassin";
};
