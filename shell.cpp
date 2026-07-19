#include "shell.hpp"

#include <windows.h>

// Must be included after windows.h
#include <shellapi.h>

#include <filesystem>

void shell_open_url(const char* url) {
    ShellExecuteA(nullptr, "open", url, nullptr, nullptr, SW_SHOWNORMAL);
}

void shell_open_file(const wchar_t* filename) {
    static wchar_t buffer[MAX_PATH];
    DWORD length = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
    auto path = std::filesystem::path(buffer).parent_path().wstring();
    ShellExecuteW(
        nullptr, L"open", filename, nullptr, path.c_str(), SW_SHOWNORMAL
    );
}
