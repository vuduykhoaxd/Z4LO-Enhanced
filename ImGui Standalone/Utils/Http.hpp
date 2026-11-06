#pragma once

// Small HTTP / text / encoding helpers used by the Theme tab.
// Ported from Z4LOEnhanced (console).

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>
#include <winhttp.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <filesystem>

#include "Utils.hpp"

#pragma comment(lib, "winhttp.lib")

// Downloads an http:// or https:// URL into `body`. Follows redirects and
// rejects non-2xx responses and bodies larger than 5 MB.
inline bool HttpGetUrl(const std::wstring& url, std::string& body) {
    URL_COMPONENTS components{};
    components.dwStructSize = sizeof(components);

    wchar_t hostName[256]{};
    wchar_t urlPath[4096]{};
    wchar_t extraInfo[4096]{};

    components.lpszHostName = hostName;
    components.dwHostNameLength = 255;
    components.lpszUrlPath = urlPath;
    components.dwUrlPathLength = 4095;
    components.lpszExtraInfo = extraInfo;
    components.dwExtraInfoLength = 4095;

    if (!WinHttpCrackUrl(
        url.c_str(),
        static_cast<DWORD>(url.size()),
        0,
        &components
    )) {
        return false;
    }

    if (components.nScheme != INTERNET_SCHEME_HTTP &&
        components.nScheme != INTERNET_SCHEME_HTTPS) {
        return false;
    }

    const bool secure = components.nScheme == INTERNET_SCHEME_HTTPS;

    std::wstring path = urlPath;
    path += extraInfo;

    if (path.empty()) {
        path = L"/";
    }

    HINTERNET session = WinHttpOpen(
        L"ZaloEnhanced/1.0",
        WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS,
        0
    );

    if (!session) return false;

    HINTERNET connection = WinHttpConnect(
        session,
        hostName,
        components.nPort,
        0
    );

    if (!connection) {
        WinHttpCloseHandle(session);
        return false;
    }

    HINTERNET request = WinHttpOpenRequest(
        connection,
        L"GET",
        path.c_str(),
        nullptr,
        WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES,
        secure ? WINHTTP_FLAG_SECURE : 0
    );

    if (!request) {
        WinHttpCloseHandle(connection);
        WinHttpCloseHandle(session);
        return false;
    }

    DWORD redirectPolicy = WINHTTP_OPTION_REDIRECT_POLICY_ALWAYS;
    WinHttpSetOption(
        request,
        WINHTTP_OPTION_REDIRECT_POLICY,
        &redirectPolicy,
        sizeof(redirectPolicy)
    );

    bool success = false;

    if (WinHttpSendRequest(
        request,
        WINHTTP_NO_ADDITIONAL_HEADERS,
        0,
        WINHTTP_NO_REQUEST_DATA,
        0,
        0,
        0
    ) && WinHttpReceiveResponse(request, nullptr)) {

        DWORD statusCode = 0;
        DWORD statusSize = sizeof(statusCode);

        const bool isSuccessStatus =
            WinHttpQueryHeaders(
                request,
                WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                WINHTTP_HEADER_NAME_BY_INDEX,
                &statusCode,
                &statusSize,
                WINHTTP_NO_HEADER_INDEX
            ) &&
            statusCode >= 200 && statusCode < 300;

        if (isSuccessStatus) {
            constexpr size_t kMaxBytes = 5 * 1024 * 1024;
            bool tooLarge = false;
            DWORD available = 0;

            while (WinHttpQueryDataAvailable(request, &available) &&
                   available > 0) {
                std::vector<char> buffer(available);
                DWORD downloaded = 0;

                if (!WinHttpReadData(
                    request,
                    buffer.data(),
                    available,
                    &downloaded
                )) {
                    break;
                }

                body.append(buffer.data(), downloaded);
                if (body.size() > kMaxBytes) {
                    tooLarge = true;
                    body.clear();
                    break;
                }
            }

            success = !tooLarge;
        }
    }

    WinHttpCloseHandle(request);
    WinHttpCloseHandle(connection);
    WinHttpCloseHandle(session);

    return success;
}

inline bool ReadTextFile(const std::filesystem::path& path, std::string& content) {
    std::ifstream file(path, std::ios::binary);
    if (!file) return false;

    std::ostringstream buffer;
    buffer << file.rdbuf();
    content = buffer.str();
    return true;
}

inline bool WriteTextFile(const std::filesystem::path& path, const std::string& content) {
    std::ofstream file(path, std::ios::binary);
    if (!file) return false;

    file.write(content.data(), static_cast<std::streamsize>(content.size()));
    return file.good();
}

inline std::string Base64Encode(const std::string& input) {
    static const char table[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    std::string output;
    unsigned int value = 0;
    int bits = -6;

    for (unsigned char c : input) {
        value = (value << 8) | c;
        bits += 8;

        while (bits >= 0) {
            output += table[(value >> bits) & 0x3F];
            bits -= 6;
        }
    }

    if (bits > -6) {
        output += table[((value << 8) >> (bits + 8)) & 0x3F];
    }

    while (output.size() % 4 != 0) {
        output += '=';
    }

    return output;
}

inline std::string JsonEscapeValue(const std::string& input) {
    std::string output;

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
            }
            else {
                output += static_cast<char>(c);
            }
        }
    }

    return output;
}
