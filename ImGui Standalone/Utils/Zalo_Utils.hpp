#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>
#include <tlhelp32.h>
#include <winhttp.h>
#include <shlobj.h>

#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <memory>
#include <cctype>

#include "Utils.hpp"
#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "advapi32.lib")

namespace fs = std::filesystem;

constexpr INTERNET_PORT DEBUG_PORT = 9222;
inline constexpr const wchar_t* APP_NAME = L"Zalo.exe";

inline bool IsZaloRunning() {
    HANDLE snapshot = CreateToolhelp32Snapshot(
        TH32CS_SNAPPROCESS,
        0
    );

    if (snapshot == INVALID_HANDLE_VALUE)
        return false;

    PROCESSENTRY32W entry{};
    entry.dwSize = sizeof(entry);

    bool found = false;

    if (Process32FirstW(snapshot, &entry)) {
        do {
            if (_wcsicmp(entry.szExeFile, APP_NAME) == 0) {
                found = true;
                break;
            }
        } while (Process32NextW(snapshot, &entry));
    }

    CloseHandle(snapshot);

    return found;
}
inline bool IsValidZaloPath(const std::wstring& path) {
    if (path.empty())
        return false;

    try {
        return fs::exists(path) &&
            fs::is_regular_file(path) &&
            ToLower(fs::path(path).filename().wstring()) ==
            L"zalo.exe";
    }
    catch (...) {
        return false;
    }
}
inline std::wstring FindZaloFromPath() {
    wchar_t buffer[MAX_PATH]{};

    DWORD length = SearchPathW(
        nullptr,
        APP_NAME,
        nullptr,
        MAX_PATH,
        buffer,
        nullptr
    );

    if (length > 0 && length < MAX_PATH) {
        wstring path(buffer);

        if (IsValidZaloPath(path))
            return path;
    }

    return L"";
}
inline std::wstring FindZaloFromRegistry(HKEY root, const wchar_t* subkey) {
    HKEY key = nullptr;

    if (RegOpenKeyExW(
        root,
        subkey,
        0,
        KEY_READ,
        &key
    ) != ERROR_SUCCESS) {
        return L"";
    }

    wchar_t value[MAX_PATH]{};
    DWORD size = sizeof(value);
    DWORD type = 0;

    LONG result = RegQueryValueExW(
        key,
        L"InstallLocation",
        nullptr,
        &type,
        reinterpret_cast<LPBYTE>(value),
        &size
    );

    RegCloseKey(key);

    if (result == ERROR_SUCCESS && type == REG_SZ) {
        fs::path candidate = fs::path(value) / L"Zalo.exe";

        if (IsValidZaloPath(candidate.wstring()))
            return candidate.wstring();
    }

    return L"";
}
inline std::wstring FindZaloInCommonFolders() {
    wchar_t localAppData[MAX_PATH]{};
    wchar_t programFiles[MAX_PATH]{};
    wchar_t programFilesX86[MAX_PATH]{};

    SHGetFolderPathW(
        nullptr,
        CSIDL_LOCAL_APPDATA,
        nullptr,
        SHGFP_TYPE_CURRENT,
        localAppData
    );

    SHGetFolderPathW(
        nullptr,
        CSIDL_PROGRAM_FILES,
        nullptr,
        SHGFP_TYPE_CURRENT,
        programFiles
    );

    SHGetFolderPathW(
        nullptr,
        CSIDL_PROGRAM_FILESX86,
        nullptr,
        SHGFP_TYPE_CURRENT,
        programFilesX86
    );

    vector<fs::path> candidates = {
        fs::path(localAppData) / L"Programs" / L"Zalo" / L"Zalo.exe",
        fs::path(localAppData) / L"Zalo" / L"Zalo.exe",
        fs::path(programFiles) / L"Zalo" / L"Zalo.exe",
        fs::path(programFilesX86) / L"Zalo" / L"Zalo.exe"
    };

    for (const auto& candidate : candidates) {
        if (IsValidZaloPath(candidate.wstring()))
            return candidate.wstring();
    }

    return L"";
}
inline std::wstring FindZaloPath() {
    wstring path;

    path = FindZaloFromPath();

    if (!path.empty())
        return path;

    path = FindZaloFromRegistry(
        HKEY_CURRENT_USER,
        L"Software\\Zalo\\Zalo"
    );

    if (!path.empty())
        return path;

    path = FindZaloFromRegistry(
        HKEY_LOCAL_MACHINE,
        L"Software\\Zalo\\Zalo"
    );

    if (!path.empty())
        return path;

    path = FindZaloInCommonFolders();

    if (!path.empty())
        return path;

    return L"";
}
inline void ForceStopZalo() {
    HANDLE snapshot = CreateToolhelp32Snapshot(
        TH32CS_SNAPPROCESS,
        0
    );

    if (snapshot == INVALID_HANDLE_VALUE) {
		MessageBoxA(nullptr, "Cannot enumerate processes.", "Error", MB_OK | MB_ICONERROR);
        return;
    }

    PROCESSENTRY32W entry{};
    entry.dwSize = sizeof(entry);

    int killed = 0;

    if (Process32FirstW(snapshot, &entry)) {
        do {
            if (_wcsicmp(entry.szExeFile, APP_NAME) == 0) {
                HANDLE process = OpenProcess(
                    PROCESS_TERMINATE,
                    FALSE,
                    entry.th32ProcessID
                );

                if (process) {
                    if (TerminateProcess(process, 0))
                        ++killed;

                    CloseHandle(process);
                }
            }
        } while (Process32NextW(snapshot, &entry));
    }

    CloseHandle(snapshot);

    if (killed > 0) {
        cout << "Terminated " << killed << " Zalo process(es).\n";
    }
    else {
        cout << "No Zalo process found or access denied.\n";
    }
}
inline void OpenZaloDebugging() {
    wstring zaloPath = FindZaloPath();

    if (zaloPath.empty()) {
        cout << "Cannot find Zalo.exe.\n";
        cout << "Install Zalo or add its directory to PATH.\n";
        return;
    }

    wcout << L"Found Zalo: " << zaloPath << L"\n";

    wstring command =
        L"\"" + zaloPath + L"\""
        L" --remote-debugging-port=9222";

    STARTUPINFOW si{};
    si.cb = sizeof(si);

    PROCESS_INFORMATION pi{};

    if (CreateProcessW(
        zaloPath.c_str(),
        command.data(),
        nullptr,
        nullptr,
        FALSE,
        0,
        nullptr,
        nullptr,
        &si,
        &pi
    )) {
        cout << "Zalo launched with debugging arguments.\n";

        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    }
    else {
        cout << "Failed to launch Zalo. Error: "
            << GetLastError() << "\n";
    }
}