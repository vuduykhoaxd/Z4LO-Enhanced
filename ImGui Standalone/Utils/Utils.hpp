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

inline wstring ToLower(wstring value) {
    transform(
        value.begin(),
        value.end(),
        value.begin(),
        towlower
    );

    return value;
}

inline wstring Trim(wstring value) {
    size_t first = value.find_first_not_of(L" \t\r\n");

    if (first == wstring::npos)
        return L"";

    size_t last = value.find_last_not_of(L" \t\r\n");

    return value.substr(first, last - first + 1);
}

inline string WideToUtf8(const wstring& input) {
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

inline wstring Utf8ToWide(const string& input) {
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