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

#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "advapi32.lib")
using namespace std;
namespace fs = std::filesystem;

constexpr INTERNET_PORT DEBUG_PORT = 9222;

const wchar_t* APP_NAME = L"Zalo.exe";
wstring ToLower(wstring value) {
    transform(
        value.begin(),
        value.end(),
        value.begin(),
        towlower
    );

    return value;
}

wstring Trim(wstring value) {
    size_t first = value.find_first_not_of(L" \t\r\n");

    if (first == wstring::npos)
        return L"";

    size_t last = value.find_last_not_of(L" \t\r\n");

    return value.substr(first, last - first + 1);
}

string WideToUtf8(const wstring& input) {
    if (input.empty())
        return {};

    int size = WideCharToMultiByte(
        CP_UTF8, 0,
        input.c_str(), -1,
        nullptr, 0, nullptr, nullptr
    );

    if (size <= 1)
        return {};

    string output(size, '\0');

    WideCharToMultiByte(
        CP_UTF8, 0,
        input.c_str(), -1,
        output.data(), size,
        nullptr, nullptr
    );

    output.pop_back();

    return output;
}

wstring Utf8ToWide(const string& input) {
    if (input.empty())
        return {};

    int size = MultiByteToWideChar(
        CP_UTF8, 0,
        input.c_str(), -1,
        nullptr, 0
    );

    if (size <= 1)
        return {};

    wstring output(size, L'\0');

    MultiByteToWideChar(
        CP_UTF8, 0,
        input.c_str(), -1,
        output.data(), size
    );

    output.pop_back();

    return output;
}

bool IsValidZaloPath(const wstring& path) {
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

wstring FindZaloFromPath() {
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

wstring FindZaloFromRegistry(HKEY root, const wchar_t* subkey) {
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

wstring FindZaloInCommonFolders() {
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

wstring FindZaloPath() {
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

bool IsZaloRunning() {
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

void ForceStopZalo() {
    HANDLE snapshot = CreateToolhelp32Snapshot(
        TH32CS_SNAPPROCESS,
        0
    );

    if (snapshot == INVALID_HANDLE_VALUE) {
        cout << "Cannot enumerate processes.\n";
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



string HttpGet(const wstring& path) {
    HINTERNET session = WinHttpOpen(
        L"ZaloPrivacyController/1.0",
        WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS,
        0
    );

    if (!session)
        return {};

    HINTERNET connection = WinHttpConnect(
        session,
        L"127.0.0.1",
        DEBUG_PORT,
        0
    );

    if (!connection) {
        WinHttpCloseHandle(session);
        return {};
    }

    HINTERNET request = WinHttpOpenRequest(
        connection,
        L"GET",
        path.c_str(),
        nullptr,
        WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES,
        0
    );

    string result;

    if (request &&
        WinHttpSendRequest(
            request,
            WINHTTP_NO_ADDITIONAL_HEADERS,
            0,
            WINHTTP_NO_REQUEST_DATA,
            0,
            0,
            0
        ) &&
        WinHttpReceiveResponse(request, nullptr)) {

        DWORD available = 0;

        while (WinHttpQueryDataAvailable(
            request,
            &available
        ) && available > 0) {

            vector<char> buffer(available);

            DWORD downloaded = 0;

            if (!WinHttpReadData(
                request,
                buffer.data(),
                available,
                &downloaded
            )) {
                break;
            }

            result.append(buffer.data(), downloaded);
        }
    }

    if (request)
        WinHttpCloseHandle(request);

    WinHttpCloseHandle(connection);
    WinHttpCloseHandle(session);

    return result;
}

string ExtractJsonString(
    const string& json,
    const string& key
) {
    string search = "\"" + key + "\"";

    size_t pos = json.find(search);

    if (pos == string::npos)
        return {};

    pos = json.find(':', pos);

    if (pos == string::npos)
        return {};

    pos = json.find('"', pos);

    if (pos == string::npos)
        return {};

    size_t end = json.find('"', pos + 1);

    if (end == string::npos)
        return {};

    return json.substr(pos + 1, end - pos - 1);
}

string GetWebSocketUrl() {
    string json = HttpGet(L"/json/list");

    if (json.empty())
        return {};

    size_t pos = 0;

    while ((pos = json.find("\"type\"", pos)) != string::npos) {
        size_t end = json.find('}', pos);

        if (end == string::npos)
            break;

        string object = json.substr(pos, end - pos + 1);

        string type = ExtractJsonString(object, "type");

        if (type == "page") {
            string url = ExtractJsonString(
                object,
                "webSocketDebuggerUrl"
            );

            if (!url.empty())
                return url;
        }

        pos = end + 1;
    }

    return {};
}

bool ExecuteJavaScript(const string& code, bool rawCdp = false) {
    string websocketUrl = GetWebSocketUrl();

    if (websocketUrl.empty()) {
        cout << "No debuggable page found.\n";
        return false;
    }

    bool secure = false;

    if (websocketUrl.rfind("ws://", 0) == 0) {
        websocketUrl.erase(0, 5);
    }
    else if (websocketUrl.rfind("wss://", 0) == 0) {
        websocketUrl.erase(0, 6);
        secure = true;
    }
    else {
        cout << "Unsupported WebSocket protocol.\n";
        return false;
    }

    size_t slash = websocketUrl.find('/');
    if (slash == string::npos) {
        cout << "Invalid WebSocket URL.\n";
        return false;
    }

    string authority = websocketUrl.substr(0, slash);
    string path = websocketUrl.substr(slash);

    string host;
    INTERNET_PORT port = secure ? 443 : 80;

    size_t colon = authority.rfind(':');
    if (colon != string::npos) {
        host = authority.substr(0, colon);

        try {
            port = static_cast<INTERNET_PORT>(
                stoi(authority.substr(colon + 1))
                );
        }
        catch (...) {
            cout << "Invalid WebSocket port.\n";
            return false;
        }
    }
    else {
        host = authority;
    }

    if (host.empty() || path.empty()) {
        cout << "Invalid WebSocket URL.\n";
        return false;
    }

    HINTERNET session = WinHttpOpen(
        L"ZaloPrivacyController/1.0",
        WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS,
        0
    );

    if (!session) {
        cout << "WinHttpOpen failed: " << GetLastError() << "\n";
        return false;
    }

    wstring wideHost = Utf8ToWide(host);
    wstring widePath = Utf8ToWide(path);

    HINTERNET connection = WinHttpConnect(
        session, wideHost.c_str(), port, 0
    );

    if (!connection) {
        cout << "WinHttpConnect failed: " << GetLastError() << "\n";
        WinHttpCloseHandle(session);
        return false;
    }

    HINTERNET request = WinHttpOpenRequest(
        connection,
        L"GET",
        widePath.c_str(),
        nullptr,
        WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES,
        secure ? WINHTTP_FLAG_SECURE : 0
    );

    if (!request) {
        cout << "WinHttpOpenRequest failed: " << GetLastError() << "\n";
        WinHttpCloseHandle(connection);
        WinHttpCloseHandle(session);
        return false;
    }

    if (!WinHttpSetOption(
        request,
        WINHTTP_OPTION_UPGRADE_TO_WEB_SOCKET,
        nullptr,
        0
    )) {
        cout << "WebSocket upgrade option failed: "
            << GetLastError() << "\n";
        WinHttpCloseHandle(request);
        WinHttpCloseHandle(connection);
        WinHttpCloseHandle(session);
        return false;
    }

    if (!WinHttpSendRequest(
        request,
        WINHTTP_NO_ADDITIONAL_HEADERS,
        0,
        WINHTTP_NO_REQUEST_DATA,
        0,
        0,
        0
    )) {
        cout << "WinHttpSendRequest failed: " << GetLastError() << "\n";
        WinHttpCloseHandle(request);
        WinHttpCloseHandle(connection);
        WinHttpCloseHandle(session);
        return false;
    }

    if (!WinHttpReceiveResponse(request, nullptr)) {
        cout << "WinHttpReceiveResponse failed: "
            << GetLastError() << "\n";
        WinHttpCloseHandle(request);
        WinHttpCloseHandle(connection);
        WinHttpCloseHandle(session);
        return false;
    }

    HINTERNET websocket = WinHttpWebSocketCompleteUpgrade(request, 0);

    if (!websocket) {
        cout << "WebSocket handshake failed: " << GetLastError() << "\n";
        WinHttpCloseHandle(request);
        WinHttpCloseHandle(connection);
        WinHttpCloseHandle(session);
        return false;
    }

    WinHttpCloseHandle(request);

    string message;

    if (rawCdp) {
        message = code;
    }
    else {
        auto JsonEscape = [](const string& input) {
            string output;

            for (unsigned char c : input) {
                switch (c) {
                case '"':  output += "\\\""; break;
                case '\\': output += "\\\\"; break;
                case '\b': output += "\\b";  break;
                case '\f': output += "\\f";  break;
                case '\n': output += "\\n";  break;
                case '\r': output += "\\r";  break;
                case '\t': output += "\\t";  break;

                default:
                    if (c < 0x20) {
                        char buffer[7];
                        sprintf_s(
                            buffer,
                            "\\u%04x",
                            static_cast<unsigned int>(c)
                        );
                        output += buffer;
                    }
                    else {
                        output += static_cast<char>(c);
                    }
                }
            }

            return output;
            };

        message =
            "{\"id\":1,"
            "\"method\":\"Runtime.evaluate\","
            "\"params\":{"
            "\"expression\":\"" + JsonEscape(code) + "\","
            "\"awaitPromise\":true,"
            "\"returnByValue\":true"
            "}}";
    }

    DWORD sendResult = WinHttpWebSocketSend(
        websocket,
        WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE,
        const_cast<char*>(message.data()),
        static_cast<DWORD>(message.size())
    );

    if (sendResult != NO_ERROR) {
        cout << "WebSocket send failed: " << sendResult << "\n";
        WinHttpWebSocketClose(
            websocket,
            WINHTTP_WEB_SOCKET_SUCCESS_CLOSE_STATUS,
            nullptr,
            0
        );
        WinHttpCloseHandle(websocket);
        WinHttpCloseHandle(connection);
        WinHttpCloseHandle(session);
        return false;
    }

    string response;
    vector<char> buffer(8192);
    bool received = false;

    while (true) {
        DWORD bytesRead = 0;
        WINHTTP_WEB_SOCKET_BUFFER_TYPE bufferType;

        DWORD result = WinHttpWebSocketReceive(
            websocket,
            buffer.data(),
            static_cast<DWORD>(buffer.size()),
            &bytesRead,
            &bufferType
        );

        if (result != NO_ERROR) {
            cout << "WebSocket receive failed: " << result << "\n";
            break;
        }

        if (bufferType == WINHTTP_WEB_SOCKET_CLOSE_BUFFER_TYPE) {
            cout << "WebSocket closed by CDP.\n";
            break;
        }

        response.append(buffer.data(), bytesRead);

        if (bufferType == WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE) {
            received = true;
            break;
        }
    }

    WinHttpWebSocketClose(
        websocket,
        WINHTTP_WEB_SOCKET_SUCCESS_CLOSE_STATUS,
        nullptr,
        0
    );

    WinHttpCloseHandle(websocket);
    WinHttpCloseHandle(connection);
    WinHttpCloseHandle(session);

    if (!received) {
        cout << "Failed to receive CDP response.\n";
        return false;
    }

    cout << "CDP Response:\n" << response << "\n";

    if (response.find("\"error\"") != string::npos ||
        response.find("\"exceptionDetails\"") != string::npos) {
        cout << "CDP command failed.\n";
        return false;
    }

    cout << (rawCdp
        ? "CDP command executed.\n"
        : "JavaScript executed successfully.\n");

    return true;
}
void OpenZaloDebugging() {
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
