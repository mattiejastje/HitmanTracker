#pragma once

#include <memory>
#include <optional>
#include <string>

struct ProcessHandleDeleter {
    void operator()(void* process_handle) const;
};

using ProcessHandlePtr = std::unique_ptr<void, ProcessHandleDeleter>;

struct Game {
    std::string name;
    ProcessHandlePtr handle;
};

std::optional<Game> find_game();

bool game_is_running(const ProcessHandlePtr& process_handle);
