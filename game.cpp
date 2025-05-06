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

static std::unordered_map<std::string, int32_t> get_module_base(DWORD process_id
) {
    spdlog::debug("Finding modules of process id {:#x}", process_id);
    std::unordered_map<std::string, int32_t> module_base{};
    auto snapshot_handle = open_snapshot_handle(TH32CS_SNAPMODULE, process_id);
    if (snapshot_handle) {
        MODULEENTRY32 module_entry = {0};
        module_entry.dwSize = sizeof(MODULEENTRY32);
        if (Module32First(snapshot_handle.get(), &module_entry)) {
            do {
                std::string name{module_entry.szModule};
                std::transform(
                    name.begin(),
                    name.end(),
                    name.begin(),
                    [](char& c) { return std::tolower(c); }
                );
                auto base_ptr
                    = reinterpret_cast<int32_t>(module_entry.modBaseAddr);
                spdlog::trace("Found module {} at {:#x}", name, base_ptr);
                module_base[name] = base_ptr;
            } while (Module32Next(snapshot_handle.get(), &module_entry));
        }
    }
    return module_base;
}

static std::optional<Game> get_game_for_process(
    const char* exe_file, DWORD process_id
) {
    spdlog::trace("Inspecting process {} with id {:#x}", exe_file, process_id);
    auto methods = get_game_methods(exe_file);
    if (methods) {
        spdlog::info("Found game {}", exe_file);
        auto process_handle = open_process_handle(process_id);
        if (process_handle) {
            return Game{
                std::move(process_handle),
                get_module_base(process_id),
                methods.value(),
            };
        }
    }
    return {};
}

std::optional<Game> find_game() {
    spdlog::debug("Inspecting all processes");
    std::optional<Game> game{};
    auto snapshot_handle = open_snapshot_handle(TH32CS_SNAPPROCESS, 0);
    if (snapshot_handle) {
        PROCESSENTRY32 process_entry = {0};
        process_entry.dwSize = sizeof(PROCESSENTRY32);
        if (Process32First(snapshot_handle.get(), &process_entry)) {
            do {
                game = get_game_for_process(
                    process_entry.szExeFile, process_entry.th32ProcessID
                );
                if (game) break;
            } while (Process32Next(snapshot_handle.get(), &process_entry));
        }
    }
    return game;
}

bool game_is_running(void* process_handle) {
    auto ret = WaitForSingleObject(process_handle, 0);
    return ret == WAIT_TIMEOUT;
}
