#include "game.hpp"

#include <synchapi.h>
#include <tlhelp32.h>

#include <memory>
#include <optional>

#include "hitman2_silent_assassin/gui.hpp"
#include "hitman2_silent_assassin/stats.hpp"
#include "hitman_blood_money/gui.hpp"
#include "hitman_codename_47/gui.hpp"
#include "hitman_contracts/gui.hpp"
#include "logger.hpp"
#include "read_process_memory.hpp"

static void stats_nothing(void *handle, Stats& stats) {}

static std::optional<GameMethods> get_game_methods(const char* exe_file) {
    if (stricmp("hitman.exe", exe_file) == 0) {
        return GameMethods{
            hitman_codename_47::gui,
            stats_nothing,
            stats_nothing,
            stats_nothing,
        };
    } else if (stricmp("hitman2.exe", exe_file) == 0) {
        return GameMethods{
            hitman2_silent_assassin::gui,
            stats_nothing,
            hitman2_silent_assassin::update_slow,
            hitman2_silent_assassin::update_fast
        };
    } else if (stricmp("hitmancontracts.exe", exe_file) == 0) {
        return GameMethods{
            hitman_contracts::gui,
            stats_nothing,
            stats_nothing,
            stats_nothing
        };
    } else if (stricmp("hitmanbloodmoney.exe", exe_file) == 0) {
        return GameMethods{
        };
    }
    return {};
}

static std::optional<Game> get_game_for_process(
    const char* exe_file, DWORD process_id
) {
    auto methods = get_game_methods(exe_file);
    if (methods) {
        spdlog::info("Found game {}", exe_file);
        auto process_handle = open_process_handle(process_id);
        if (process_handle) {
            return Game{
                std::move(process_handle),
                methods.value(),
            };
        }
    }
    return {};
}

std::optional<Game> find_game() {
    spdlog::debug("Inspecting all processes");
    std::optional<Game> game{};
    if (snapshot_handle) {
        PROCESSENTRY32 process_entry{};
        process_entry.dwSize = sizeof(PROCESSENTRY32);
        auto found = Process32First(snapshot_handle.get(), &process_entry);
        while (found) {
            game = get_game_for_process(
                process_entry.szExeFile, process_entry.th32ProcessID
            );
            if (game) break;
            found = Process32Next(snapshot_handle.get(), &process_entry);
        }
    }
    return game;
}

bool game_is_running(void* process_handle) {
    auto ret = WaitForSingleObject(process_handle, 0);
    return ret == WAIT_TIMEOUT;
}
