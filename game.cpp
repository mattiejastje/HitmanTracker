#include "game.hpp"

#include <spdlog/spdlog.h>
#include <synchapi.h>
#include <tlhelp32.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <ranges>
#include <string>
#include <unordered_map>

#include "base_ptrs.hpp"
#include "mem/handle.hpp"
#include "mem/read_write.hpp"
#include "pe.hpp"

struct Module {
    intptr_t base_ptr{};
    std::string exe_path{};
};

static std::unordered_map<std::string, Module> get_all_modules(
    HANDLE process_handle, DWORD process_id
) {
    spdlog::debug("Finding modules of process id {:#x}", process_id);
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
                spdlog::trace("Found module {} at {:#x}", name, base_ptr);
                modules[name] = {base_ptr, module_entry.szExePath};
            } while (Module32Next(snapshot_handle.get(), &module_entry));
        }
    }
    return modules;
}

static std::optional<std::vector<Module>> get_modules(
    const std::unordered_map<std::string, Module>& all_modules,
    const std::vector<ModuleInfo>& module_infos
) {
    std::vector<Module> modules{};
    std::vector<PeId> pe_ids{};
    for (const auto& module_info : module_infos) {
        auto module = all_modules.find(module_info.name);
        if (module == all_modules.end()) {
            spdlog::debug("Cannot find module {}", module_info.name);
            return {};
        }
        const auto& exe_path = module->second.exe_path;
        auto pe_id = get_pe_id(exe_path);
        if (!pe_id) return {};
        if (pe_id->time_date_stamp != module_info.pe_id.time_date_stamp) {
            spdlog::debug(
                "{} time date stamp {:#x} does not match {:#x}",
                module_info.name,
                pe_id->time_date_stamp,
                module_info.pe_id.time_date_stamp
            );
            return {};
        }
        modules.push_back(module->second);
        spdlog::debug(
            "Found required module {} at {:#x}",
            module_info.name,
            module->second.base_ptr
        );
    }
    return modules;
}

static BasePtrs get_base_ptrs(const std::vector<Module>& modules) {
    BasePtrs base_ptrs{};
    for (const auto& module : modules) {
        base_ptrs.push_back(module.base_ptr);
    };
    return base_ptrs;
}

static std::optional<Game> get_game_for_process(
    const std::vector<GameInfo>& registry, const char* exe_file, DWORD process_id
) {
    spdlog::trace("Inspecting process {} with id {:#x}", exe_file, process_id);
    for (auto& info : registry) {
        if (stricmp(info.module_infos.at(0).name.c_str(), exe_file) != 0)
            continue;
        auto process_handle = open_process_handle(process_id);
        if (!process_handle) continue;
        auto modules = get_modules(
            get_all_modules(process_handle.get(), process_id), info.module_infos
        );
        if (!modules) continue;
        std::shared_ptr<void> handle = std::move(process_handle);
        auto base_ptrs = get_base_ptrs(*modules);
        spdlog::info("Found process for {}", info.name);
        return Game{
            (*modules)[0].exe_path,
            handle,
            base_ptrs,
            info.methods,
            nullptr,  // installed later
            info.make_remote_state(),
            info.make_stats(),
        };
    }
    return {};
}

std::optional<Game> find_game(const std::vector<GameInfo>& registry) {
    spdlog::debug("Inspecting all processes");
    std::optional<Game> game{};
    auto snapshot_handle = open_snapshot_handle(TH32CS_SNAPPROCESS, 0);
    if (snapshot_handle) {
        PROCESSENTRY32 process_entry = {0};
        process_entry.dwSize = sizeof(PROCESSENTRY32);
        if (Process32First(snapshot_handle.get(), &process_entry)) {
            do {
                game = get_game_for_process(
                    registry, process_entry.szExeFile, process_entry.th32ProcessID
                );
                if (game) break;
            } while (Process32Next(snapshot_handle.get(), &process_entry));
        }
    }
    return game;
}
