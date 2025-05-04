#include "game.hpp"

#include <fmt/format.h>
#include <imgui.h>
#include <windows.h>
#include <tlhelp32.h>

#include <chrono>
#include <memory>
#include <optional>

#include "logger.hpp"

struct GameProcess {
    DWORD process_id;
    GameGui gui;
};

static std::string format_duration(float seconds) {
    std::chrono::duration<float> duration(seconds);
    auto mins = std::chrono::duration_cast<std::chrono::minutes>(duration);
    duration -= mins;
    auto secs = std::chrono::duration_cast<std::chrono::seconds>(duration);
    duration -= secs;
    auto millisecs
        = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
    return fmt::format(
        "{}:{:02}.{:03}", mins.count(), secs.count(), millisecs.count()
    );
}

static void gui_hitman_codename_47(const Stats& stats) {
    ImGui::Text("Hitman: Codename 47");
    ImGui::Separator();
    ImGui::Text(format_duration(stats.time).c_str());
}

static void gui_hitman2_silent_assassin(const Stats& stats) {
    ImGui::Text("Hitman 2: Silent Assassin");
    ImGui::Separator();
    ImGui::Text(format_duration(stats.time).c_str());
}

static void gui_hitman_contracts(const Stats& stats) {
    ImGui::Text("Hitman: Contracts");
    ImGui::Separator();
    ImGui::Text(format_duration(stats.time).c_str());
}

static void gui_hitman_blood_money(const Stats& stats) {
    ImGui::Text("Hitman: Blood Money");
    ImGui::Separator();
    ImGui::Text(format_duration(stats.time).c_str());
}

static void stats_hitman_codename_47(
    const ProcessHandlePtr& handle, Stats& stats
) {}

static void stats_hitman2_silent_assassin(
    const ProcessHandlePtr& handle, Stats& stats
) {}

static void stats_hitman_contracts(
    const ProcessHandlePtr& handle, Stats& stats
) {}

static void stats_hitman_blood_money(
    const ProcessHandlePtr& handle, Stats& stats
) {}

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

void ProcessHandleDeleter::operator()(void* process_handle) const {
    if (process_handle) {
        CloseHandle(process_handle);
        spdlog::debug("Process handle {} closed", process_handle);
    }
};

static ProcessHandlePtr open_process_handle(DWORD process_id) {
    auto process_handle = OpenProcess(PROCESS_ALL_ACCESS, 0, process_id);
    if (process_handle) {
        spdlog::debug(
            "Process handle {} opened for process id {}",
            process_handle,
            process_id
        );
    } else {
        spdlog::error("Cannot open handle for process id {}", process_id);
    }
    return ProcessHandlePtr{process_handle};
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
    if (process_handle) {
        UINT32 first_bytes = 0;
        SIZE_T bytes_read = 0;
        ReadProcessMemory(
            process_handle.get(),
            (LPCVOID)0x00400000,
            &first_bytes,
            4,
            &bytes_read
        );
        if (first_bytes == 0x00905A4D) return true;
    }
    return false;
}
