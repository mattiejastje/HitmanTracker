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
#include "hitman2_silent_assassin/hook.hpp"
#include "hitman2_silent_assassin/stats.hpp"
#include "hitman_2016/gui.hpp"
#include "hitman_absolution/gui.hpp"
#include "hitman_absolution/hook.hpp"
#include "hitman_absolution/stats.hpp"
#include "hitman_blood_money/gui.hpp"
#include "hitman_blood_money/hook.hpp"
#include "hitman_blood_money/stats.hpp"
#include "hitman_codename_47/gui.hpp"
#include "hitman_codename_47/hook.hpp"
#include "hitman_codename_47/stats.hpp"
#include "hitman_contracts/gui.hpp"
#include "hitman_contracts/hook.hpp"
#include "hitman_contracts/stats.hpp"
#include "logging.hpp"
#include "mem/handle.hpp"
#include "mem/read_write.hpp"

using GameHook = std::function<HookPtr(std::shared_ptr<void>, const BasePtrs&)>;

struct GameInfo {
    GameMethods methods;
    // first module name is always exe name
    std::array<std::string, 5> module_names;
    GameHook hook;
};

static bool stats_nothing(
    void* handle,
    const BasePtrs& base_ptrs,
    const LabelPtrs& label_ptrs,
    Stats& stats
) {
    return true;
}

static HookPtr hook_nothing(
    std::shared_ptr<void> handle, const BasePtrs& base_ptrs
) {
    return HookPtr{};
}

static const std::vector<GameInfo> game_infos = {
    GameInfo{
        GameMethods{
            hitman_codename_47::gui,
            hitman_codename_47::update_slow,
            hitman_codename_47::update_fast,
        },
        {{"hitman.exe", "hitmandlc.dlc", "enginedata.dll"}},
        hitman_codename_47::hook,
    },
    GameInfo{
        GameMethods{hitman_2016::gui, stats_nothing, stats_nothing},
        {{"hitman.exe", "tobii.gameintegration.dll"}},
        hook_nothing,
    },
    GameInfo{
        GameMethods{
            hitman2_silent_assassin::gui,
            hitman2_silent_assassin::update_slow,
            hitman2_silent_assassin::update_fast
        },
        {{"hitman2.exe"}},
        hitman2_silent_assassin::hook,
    },
    GameInfo{
        GameMethods{
            hitman_contracts::gui,
            hitman_contracts::update_slow,
            hitman_contracts::update_fast
        },
        {{"hitmancontracts.exe"}},
        hitman_contracts::hook,
    },
    GameInfo{
        GameMethods{
            hitman_blood_money::gui,
            hitman_blood_money::update_slow,
            hitman_blood_money::update_fast,
        },
        {{"hitmanbloodmoney.exe"}},
        hitman_blood_money::hook,
    },
    GameInfo{
        GameMethods{
            hitman_absolution::gui,
            hitman_absolution::update_slow,
            hitman_absolution::update_fast
        },
        {{"hma.exe"}},
        hitman_absolution::hook,
    },
};

static std::unordered_map<std::string, intptr_t> get_all_base_ptrs(
    HANDLE process_handle, DWORD process_id
) {
    logging::debug("Finding modules of process id {:#x}", process_id);
    BasePtrs base_ptrs{};
    std::unordered_map<std::string, intptr_t> all_base_ptrs{};
    WaitForSingleObject(process_handle, 1000);  // wait until dlls are loaded
    auto snapshot_handle = open_snapshot_handle(
        TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, process_id
    );
    if (snapshot_handle) {
        MODULEENTRY32 module_entry = {0};
        module_entry.dwSize = sizeof(MODULEENTRY32);
        if (Module32First(snapshot_handle.get(), &module_entry)) {
            do {
                std::string name{module_entry.szModule};
                std::transform(
                    name.begin(), name.end(), name.begin(), [](char& c) {
                        return std::tolower(c);
                    }
                );
                auto base_ptr
                    = reinterpret_cast<intptr_t>(module_entry.modBaseAddr);
                logging::trace("Found module {} at {:#x}", name, base_ptr);
                all_base_ptrs[name] = base_ptr;
            } while (Module32Next(snapshot_handle.get(), &module_entry));
        }
    }
    return all_base_ptrs;
}

static std::optional<BasePtrs> get_base_ptrs(
    const std::unordered_map<std::string, intptr_t>& all_base_ptrs,
    const std::array<std::string, 5>& module_names
) {
    BasePtrs base_ptrs{};
    for (int i = 0; i < 5; i++) {
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
            base_ptrs[i] = base_ptr->second;
        }
    }
    return base_ptrs;
}

static std::optional<Game> get_game_for_process(
    const char* exe_file, DWORD process_id
) {
    logging::trace("Inspecting process {} with id {:#x}", exe_file, process_id);
    for (auto& info : game_infos) {
        if (stricmp(info.module_names[0].c_str(), exe_file) != 0) continue;
        logging::info("Found game {}", exe_file);
        auto process_handle = open_process_handle(process_id);
        if (process_handle) {
            auto base_ptrs = get_base_ptrs(
                get_all_base_ptrs(process_handle.get(), process_id),
                info.module_names
            );
            if (base_ptrs) {
                std::shared_ptr<void> handle = std::move(process_handle);
                auto hook_ptr = info.hook(handle, base_ptrs.value());
                return Game{
                    handle,
                    base_ptrs.value(),
                    info.methods,
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
