#include "game.hpp"

#include <windows.h>
#include <tlhelp32.h>

#include <memory>
#include <optional>

#include "hitman2_silent_assassin/gui.hpp"
#include "hitman2_silent_assassin/stats.hpp"
#include "hitman_blood_money/gui.hpp"
#include "hitman_blood_money/stats.hpp"
#include "hitman_codename_47/gui.hpp"
#include "hitman_codename_47/stats.hpp"
#include "hitman_contracts/gui.hpp"
#include "hitman_contracts/stats.hpp"
#include "logger.hpp"
#include "read_process_memory.hpp"

static std::optional<std::tuple<GameGui, GameStats>> find_game_gui_stats(
    const char* exe_file
) {
    if (stricmp("hitman.exe", exe_file) == 0) {
        return std::make_tuple(
            gui_hitman_codename_47, stats_hitman_codename_47
        );
    } else if (stricmp("hitman2.exe", exe_file) == 0) {
        return std::make_tuple(
            gui_hitman2_silent_assassin, stats_hitman2_silent_assassin
        );
    } else if (stricmp("hitmancontracts.exe", exe_file) == 0) {
        return std::make_tuple(gui_hitman_contracts, stats_hitman_contracts);
    } else if (stricmp("hitmanbloodmoney.exe", exe_file) == 0) {
        return std::make_tuple(
            gui_hitman_blood_money, stats_hitman_blood_money
        );
    }
    return {};
}

static std::optional<Game> find_game(HANDLE snapshot_handle) {
    PROCESSENTRY32 process_entry{};
    process_entry.dwSize = sizeof(PROCESSENTRY32);
    auto found = Process32First(snapshot_handle, &process_entry);
    while (found) {
        spdlog::trace("Checking process {}", process_entry.szExeFile);
        auto game_gui_stats = find_game_gui_stats(process_entry.szExeFile);
        if (game_gui_stats.has_value()) {
            spdlog::info(
                "Found game (process id {})", process_entry.th32ProcessID
            );
            auto process_handle
                = open_process_handle(process_entry.th32ProcessID);
            if (process_handle) {
                return Game{
                    std::move(process_handle),
                    std::get<0>(game_gui_stats.value()),
                    std::get<1>(game_gui_stats.value())
                };
            }
        }
        found = Process32Next(snapshot_handle, &process_entry);
    }
    return {};
}

std::optional<Game> find_game() {
    auto snapshot_handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (INVALID_HANDLE_VALUE == snapshot_handle) {
        spdlog::error("Unable to list processes (invalid snapshot handle)");
    } else {
        auto game = find_game(snapshot_handle);
        CloseHandle(snapshot_handle);
        return game;
    }
    return {};
}

bool game_is_running(const ProcessHandlePtr& process_handle) {
    return read_uint32(process_handle, (LPCVOID)0x00400000) == 0x00905A4D;
}
