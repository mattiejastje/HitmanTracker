#include "game.hpp"

#include <windows.h>
#include <tlhelp32.h>

#include <memory>
#include <optional>

#include "logger.hpp"

enum GameId {
    HITMAN_CODENAME_47,
    HITMAN2_SILENT_ASSASSIN,
    HITMAN_CONTRACTS,
    HITMAN_BLOOD_MONEY,
};

struct GameProcess {
    GameId game_id;
    DWORD process_id;
};

const std::map<GameId, std::string> game_name
    = {{HITMAN_CODENAME_47, "Hitman: Codename 47"},
       {HITMAN2_SILENT_ASSASSIN, "Hitman 2: Silent Assassin"},
       {HITMAN_CONTRACTS, "Hitman: Contracts"},
       {HITMAN_BLOOD_MONEY, "Hitman: Blood Money"}};

static std::optional<GameId> find_game_id(const char* exe_file) {
    if (stricmp("hitman.exe", exe_file) == 0) {
        return HITMAN_CODENAME_47;
    } else if (stricmp("hitman2.exe", exe_file) == 0) {
        return HITMAN2_SILENT_ASSASSIN;
    } else if (stricmp("hitmancontracts.exe", exe_file) == 0) {
        return HITMAN_CONTRACTS;
    } else if (stricmp("hitmanbloodmoney.exe", exe_file) == 0) {
        return HITMAN_BLOOD_MONEY;
    }
    return {};
}

static std::optional<GameProcess> find_game_process(HANDLE snapshot_handle) {
    std::optional<GameProcess> game_process{};
    PROCESSENTRY32 process_entry{};
    process_entry.dwSize = sizeof(PROCESSENTRY32);
    BOOL hResult = Process32First(snapshot_handle, &process_entry);
    while (hResult) {
        spdlog::trace("Checking process {}", process_entry.szExeFile);
        auto game_id = find_game_id(process_entry.szExeFile);
        if (game_id.has_value()) {
            spdlog::info(
                "Found {} (process id {})",
                game_name.at(game_id.value()).c_str(),
                process_entry.th32ProcessID
            );
            return GameProcess{game_id.value(), process_entry.th32ProcessID};
        }
        hResult = Process32Next(snapshot_handle, &process_entry);
    }
    return {};
}

static std::optional<GameProcess> find_game_process() {
    HANDLE snapshot_handle;
    snapshot_handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (INVALID_HANDLE_VALUE == snapshot_handle) {
        spdlog::error("Unable to list processes (invalid snapshot handle)");
    } else {
        auto game_process = find_game_process(snapshot_handle);
        CloseHandle(snapshot_handle);
        return game_process;
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
    HANDLE process_handle = OpenProcess(PROCESS_ALL_ACCESS, 0, process_id);
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

std::optional<Game> find_game() {
    auto game_process = find_game_process();
    if (game_process) {
        auto process_handle = open_process_handle(game_process->process_id);
        if (process_handle) {
            return Game{
                game_name.at(game_process->game_id), std::move(process_handle)
            };
        }
    };
    return {};
}

bool game_is_running(const ProcessHandlePtr& process_handle) {
    if (process_handle) {
        UINT32 first_bytes = 0;
        SIZE_T bytes_read;
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
