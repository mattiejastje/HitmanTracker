#include "stats.hpp"

std::string get_simple_rating_value(Status status) {
    return status == Status::GREEN ? "Silent Assassin" : "No Silent Assassin";
};
