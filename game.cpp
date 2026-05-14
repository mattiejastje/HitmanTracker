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
#include "fnv1a.hpp"
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

struct ModuleInfo {
    std::string name;
    uint64_t hash;
};

struct GameInfo {
    std::string name;
    GameMethods methods;
    // first module name is always exe name
    std::array<ModuleInfo, 5> module_infos;
    uint64_t hash;
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
    // non-null hook pointer means success so return stub
    return HookPtr{new Hook{}};
}

static bool hook_immediately_ready(void* handle, const BasePtrs& base_ptrs) {
    return true;
}

static const std::vector<GameInfo> game_infos = {
    GameInfo{
        hitman_codename_47::GAME_NAME,
        GameMethods{
            hitman_codename_47::gui,
            hitman_codename_47::hook,
            hook_immediately_ready,
            hitman_codename_47::update_slow,
            hitman_codename_47::update_fast,
        },
        {{
            {"hitman.exe", 0xD6739CF25081C0F5ULL},
            {"hitmandlc.dlc", 0xCC2D12E73040901FULL},
            {"enginedata.dll", 0xA0C506C5C1D98559ULL},
        }},
    },
    GameInfo{
        hitman_2016::GAME_NAME,
        GameMethods{
            hitman_2016::gui,
            hook_nothing,
            hook_immediately_ready,
            stats_nothing,
            stats_nothing,
        },
        {{
            {"hitman.exe", 0x9019923E9B36C383ULL},
            {"tobii.gameintegration.dll", 0xB36F82D72789C260ULL},
        }},
    },
    GameInfo{
        hitman2_silent_assassin::GAME_NAME,
        GameMethods{
            hitman2_silent_assassin::gui,
            hitman2_silent_assassin::hook,
            hook_immediately_ready,
            hitman2_silent_assassin::update_slow,
            hitman2_silent_assassin::update_fast
        },
        {{{"hitman2.exe", 0xB68C2F1042BD339DULL}}},

    },
    GameInfo{
        hitman_contracts::GAME_NAME,
        GameMethods{
            hitman_contracts::gui,
            hitman_contracts::hook,
            hook_immediately_ready,
            hitman_contracts::update_slow,
            hitman_contracts::update_fast
        },
        {{{"hitmancontracts.exe", 0xA7AD9FC9AF91F8CBULL}}},
    },
    GameInfo{
        hitman_blood_money::GAME_NAME,
        GameMethods{
            hitman_blood_money::gui,
            hitman_blood_money::hook,
            hook_immediately_ready,
            hitman_blood_money::update_slow,
            hitman_blood_money::update_fast,
        },
        {{{"hitmanbloodmoney.exe", 0xD31C7C7A7C311D9BULL}}},
    },
    GameInfo{
        hitman_absolution::GAME_NAME,
        GameMethods{
            hitman_absolution::gui,
            hitman_absolution::hook,
            hitman_absolution::hook_ready,
            hitman_absolution::update_slow,
            hitman_absolution::update_fast
        },
        {{{"hma.exe", 0x3618C80C35CA45F1ULL}}},
    },
};

struct Module {
    intptr_t base_ptr{};
    std::string exe_path{};
};

static std::unordered_map<std::string, Module> get_all_modules(
    HANDLE process_handle, DWORD process_id
) {
    logging::debug("Finding modules of process id {:#x}", process_id);
    std::unordered_map<std::string, Module> modules{};
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
                modules[name] = {base_ptr, module_entry.szExePath};
            } while (Module32Next(snapshot_handle.get(), &module_entry));
        }
    }
    return modules;
}

static std::optional<std::array<Module, 5>> get_modules(
    const std::unordered_map<std::string, Module>& all_modules,
    const std::array<ModuleInfo, 5>& module_infos
) {
    std::array<Module, 5> modules{};
    std::array<uint64_t, 5> module_hashes;
    for (int i = 0; i < 5; i++) {
        const auto& module_info = module_infos[i];
        if (module_info.name.empty()) continue;
        auto module = all_modules.find(module_info.name);
        if (module == all_modules.end()) {
            logging::debug("Cannot find module {}", module_info.name);
            return {};
        }
        const auto& exe_path = module->second.exe_path;
        auto hash = fnv1a::fnv1a(exe_path);
        if (!hash) {
            logging::error("Unable to calculate checksum of {}", exe_path);
            return {};
        }
        module_hashes[i] = *hash;
        logging::debug(
            "Found required module {} at {:#x}",
            module_info.name,
            module->second.base_ptr
        );
        modules[i] = module->second;
    }
    // check hashes only once all modules processed
    // this prevents spurious hash errors for games with same exe
    for (int i = 0; i < 5; i++) {
        const auto& module_info = module_infos[i];
        if (module_info.name.empty()) continue;
        auto hash = module_hashes[i];
        if (hash != module_info.hash) {
            logging::error(
                "{} has checksum {:#x} but expected {:#x}; "
                "perhaps not running steam version?",
                module_info.name,
                hash,
                module_info.hash
            );
            return {};
        }
    }
    return modules;
}

static BasePtrs get_base_ptrs(const std::array<Module, 5>& modules) {
    BasePtrs base_ptrs{};
    for (int i = 0; i < 5; i++) {
        base_ptrs[i] = modules[i].base_ptr;
    };
    return base_ptrs;
}

static std::optional<Game> get_game_for_process(
    const char* exe_file, DWORD process_id
) {
    logging::trace("Inspecting process {} with id {:#x}", exe_file, process_id);
    for (auto& info : game_infos) {
        if (stricmp(info.module_infos[0].name.c_str(), exe_file) != 0) continue;
        auto process_handle = open_process_handle(process_id);
        if (!process_handle) continue;
        auto modules = get_modules(
            get_all_modules(process_handle.get(), process_id), info.module_infos
        );
        if (!modules) continue;
        std::shared_ptr<void> handle = std::move(process_handle);
        auto base_ptrs = get_base_ptrs(*modules);
        logging::info("Found process for {}", info.name);
        return Game{
            handle,
            base_ptrs,
            info.methods,
            nullptr,  // installed later
        };
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
