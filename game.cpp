#include "game.hpp"

#include <synchapi.h>
#include <tlhelp32.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

#include "base_ptrs.hpp"
#include "hitman2_silent_assassin/gui.hpp"
#include "hitman2_silent_assassin/stats.hpp"
#include "hitman_blood_money/gui.hpp"
#include "hitman_codename_47/gui.hpp"
#include "hitman_contracts/gui.hpp"
#include "logging.hpp"
#include "mem/read_write.hpp"

using GameHook = std::function<HookPtr(std::shared_ptr<void>)>;

struct GameInfo {
    GameMethods methods;
    std::array<std::string, 4> module_names;
    GameHook hook;
};

static void stats_nothing(
    void* handle, const BasePtrs& base_ptrs, Stats& stats
) {}

static HookPtr hook_nothing(std::shared_ptr<void> handle) { return HookPtr{}; }

static std::optional<GameInfo> get_game_info(const char* exe_file) {
    if (stricmp("hitman.exe", exe_file) == 0) {
        return GameInfo{
            GameMethods{
                hitman_codename_47::gui,
                stats_nothing,
                stats_nothing,
            },
            {{"hitman.exe"}},
            hook_nothing,
        };
    } else if (stricmp("hitman2.exe", exe_file) == 0) {
        return GameInfo{
            GameMethods{
                hitman2_silent_assassin::gui,
                hitman2_silent_assassin::update_slow,
                hitman2_silent_assassin::update_fast
            },
            {{"hitman2.exe"}},
            hook_nothing,

        };
    } else if (stricmp("hitmancontracts.exe", exe_file) == 0) {
        return GameInfo{
            GameMethods{hitman_contracts::gui, stats_nothing, stats_nothing},
            {{"hitmancontracts.exe"}},
            hook_nothing,

        };
    } else if (stricmp("hitmanbloodmoney.exe", exe_file) == 0) {
        return GameInfo{
            GameMethods{hitman_blood_money::gui, stats_nothing, stats_nothing},
            {{"hitmanbloodmoney.exe"}},
            hook_nothing,

        };
    }
    return {};
}

static std::unordered_map<std::string, int32_t> get_all_base_ptrs(
    HANDLE process_handle, DWORD process_id
) {
    logging::debug("Finding modules of process id {:#x}", process_id);
    BasePtrs base_ptrs{};
    std::unordered_map<std::string, int32_t> all_base_ptrs{};
    WaitForSingleObject(process_handle, 1000);  // wait until dlls are loaded
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
                logging::trace("Found module {} at {:#x}", name, base_ptr);
                all_base_ptrs[name] = base_ptr;
            } while (Module32Next(snapshot_handle.get(), &module_entry));
        }
    }
    return all_base_ptrs;
}

static std::optional<BasePtrs> get_base_ptrs(
    const std::unordered_map<std::string, int32_t>& all_base_ptrs,
    const std::array<std::string, 4>& module_names
) {
    BasePtrs base_ptrs{};
    for (int i = 0; i < 4; i++) {
        if (!module_names[i].empty()) {
            auto base_ptr = all_base_ptrs.find(module_names[i]);
            if (base_ptr == all_base_ptrs.end()) {
                logging::error("Cannot find module {}", module_names[i]);
                return {};
            } else {
                logging::debug(
                    "Found required module {} at {:#x}",
                    module_names[i],
                    base_ptr->second
                );
            }
            base_ptrs[i + 1] = base_ptr->second;
        }
    }
    return base_ptrs;
}

static std::optional<Game> get_game_for_process(
    const char* exe_file, DWORD process_id
) {
    logging::trace("Inspecting process {} with id {:#x}", exe_file, process_id);
    auto info = get_game_info(exe_file);
    if (info) {
        logging::info("Found game {}", exe_file);
        auto process_handle = open_process_handle(process_id);
        if (process_handle) {
            auto base_ptrs = get_base_ptrs(
                get_all_base_ptrs(process_handle.get(), process_id),
                info.value().module_names
            );
            if (base_ptrs) {
                std::shared_ptr<void> handle = std::move(process_handle);
                auto hook_ptr = info.value().hook(handle);
                if (hook_ptr && hook_ptr->target_alloc) {
                    base_ptrs.value()[0] = hook_ptr->target_alloc->ptr;
                }
                return Game{
                    handle,
                    base_ptrs.value(),
                    info.value().methods,
                    std::move(hook_ptr),
                };
            }
        }
    }
    return {};
}

std::optional<Game> find_game() {
    logging::debug("Inspecting all processes");
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
