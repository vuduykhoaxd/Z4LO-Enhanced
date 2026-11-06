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

#include "../Utils/Zalo_Utils.hpp"
#include "../Utils/Utils.hpp"
#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "advapi32.lib")

using namespace std;

inline string HttpGet(const wstring& path) {
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

inline string ExtractJsonString(
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

inline string GetWebSocketUrl() {
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

inline string JsonEscapeForCdp(const string& input) {
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
                sprintf_s(buffer, "\\u%04x", static_cast<unsigned int>(c));
                output += buffer;
            } else {
                output += static_cast<char>(c);
            }
        }
    }
    return output;
}

inline string ParseCdpEvaluateValue(const string& response) {
    // CDP: {"id":1,"result":{"result":{"type":"string","value":"..."}}}
    // Our JS returns JSON.stringify(...) as a string, so value is a JSON string.
    size_t pos = response.find("\"value\"");
    if (pos == string::npos) return "";
    pos = response.find(':', pos);
    if (pos == string::npos) return "";
    ++pos;
    while (pos < response.size() && isspace((unsigned char)response[pos])) ++pos;
    if (pos >= response.size()) return "";
    if (response[pos] == '"') {
        ++pos;
        string out;
        bool esc = false;
        for (; pos < response.size(); ++pos) {
            char c = response[pos];
            if (esc) {
                switch (c) {
                case 'n': out += '\n'; break;
                case 'r': out += '\r'; break;
                case 't': out += '\t'; break;
                case 'b': out += '\b'; break;
                case 'f': out += '\f'; break;
                case '/': out += '/'; break;
                case '\\': out += '\\'; break;
                case '"': out += '"'; break;
                case 'u': {
                    // \uXXXX
                    if (pos + 4 < response.size()) {
                        string hex = response.substr(pos + 1, 4);
                        try {
                            unsigned int code = (unsigned int)stoi(hex, nullptr, 16);
                            // UTF-8 encode (BMP only, enough for config keys)
                            if (code < 0x80) out += (char)code;
                            else if (code < 0x800) {
                                out += (char)(0xC0 | (code >> 6));
                                out += (char)(0x80 | (code & 0x3F));
                            } else {
                                out += (char)(0xE0 | (code >> 12));
                                out += (char)(0x80 | ((code >> 6) & 0x3F));
                                out += (char)(0x80 | (code & 0x3F));
                            }
                        } catch (...) {}
                        pos += 4;
                    }
                    break;
                }
                default: out += c; break;
                }
                esc = false;
            } else if (c == '\\') {
                esc = true;
            } else if (c == '"') {
                break;
            } else {
                out += c;
            }
        }
        return out;
    }
    size_t end = pos;
    while (end < response.size() && response[end] != ',' && response[end] != '}' && response[end] != ']' ) ++end;
    string v = response.substr(pos, end - pos);
    while (!v.empty() && isspace((unsigned char)v.back())) v.pop_back();
    return v;
}

inline bool ExecuteJavaScriptInternal(const string& code, bool rawCdp, string* outValue) {
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
        message =
            "{\"id\":1,"
            "\"method\":\"Runtime.evaluate\","
            "\"params\":{"
            "\"expression\":\"" + JsonEscapeForCdp(code) + "\","
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

    if (outValue) {
        *outValue = ParseCdpEvaluateValue(response);
    }

    cout << (rawCdp
        ? "CDP command executed.\n"
        : "JavaScript executed successfully.\n");

    return true;
}

inline bool ExecuteJavaScript(const string& code, bool rawCdp = false) {
    return ExecuteJavaScriptInternal(code, rawCdp, nullptr);
}

// Evaluate JS and return its stringified result (JSON.stringify(...) expected).
inline string EvaluateJavaScript(const string& code, bool* ok = nullptr) {
    string value;
    bool success = ExecuteJavaScriptInternal(code, false, &value);
    if (ok) *ok = success;
    return success ? value : "";
}


