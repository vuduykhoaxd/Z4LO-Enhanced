#pragma once

// High-level actions shared by the ImGui tabs. Everything here talks to the
// Zalo PC web layer through CDP (port 9222) or touches the local sticker
// library. Ported from the Z4LOEnhanced console build.

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "../Utils/Utils.hpp"
#include "../Utils/Http.hpp"
#include "../Utils/Zalo_Utils.hpp"
#include "ExecuteJS.hpp"

#include "../Command/JavaScript.h"
#include "../Command/ThemeCss.h"
#include "../Command/Sticker.hpp"

inline constexpr const char* Z4_THEME_STYLE_ID = "custom-zalo-theme";

// ---------------------------------------------------------------
//  Blur / privacy
// ---------------------------------------------------------------
inline bool ApplyBlurState(
    bool message,
    bool image,
    bool sticker,
    bool aiSticker,
    bool url,
    bool embed,
    bool conversationList,
    std::string* status = nullptr
) {
    std::ostringstream json;
    json << "{message:" << (message ? "true" : "false")
         << ",image:" << (image ? "true" : "false")
         << ",sticker:" << (sticker ? "true" : "false")
         << ",aiSticker:" << (aiSticker ? "true" : "false")
         << ",url:" << (url ? "true" : "false")
         << ",embed:" << (embed ? "true" : "false")
         << ",conversationList:" << (conversationList ? "true" : "false")
         << "}";

    std::string script = BLUR_API_JS;
    script += "\nwindow.__z4applyState(" + json.str() + ");";

    const bool ok = ExecuteJavaScript(script);

    if (status) {
        *status = ok
            ? "Privacy settings applied to Zalo."
            : "Failed. Start Zalo with --remote-debugging-port=9222.";
    }

    return ok;
}

// ---------------------------------------------------------------
//  Zalo theme (CSS injected into the Zalo web view)
// ---------------------------------------------------------------
inline bool ApplyThemeCss(
    const std::string& css,
    const std::string& styleId = Z4_THEME_STYLE_ID
) {
    if (css.empty()) return false;

    std::string script = THEME_API_JS;
    script += "\nwindow.apply_theme_base64(\"";
    script += Base64Encode(css);
    script += "\", \"";
    script += JsonEscapeValue(styleId);
    script += "\");";

    return ExecuteJavaScript(script);
}

inline bool ApplyLiquidGlassTheme(std::string* status = nullptr) {
    const bool ok = ApplyThemeCss(THEME_CSS, Z4_THEME_STYLE_ID);
    if (status) {
        *status = ok
            ? "Liquid Glass applied to Zalo."
            : "Apply failed. Is Zalo running with debug port 9222?";
    }
    return ok;
}

inline bool ApplyStyleFile(const fs::path& path, std::string* status = nullptr) {
    std::string css;
    if (!ReadTextFile(path, css)) {
        if (status) *status = "Cannot read: " + WideToUtf8(path.wstring());
        return false;
    }

    const bool ok = ApplyThemeCss(css, Z4_THEME_STYLE_ID);
    if (status) {
        if (ok) {
            std::ostringstream ss;
            ss << "Applied " << css.size() << " bytes from "
               << WideToUtf8(path.filename().wstring()) << ".";
            *status = ss.str();
        }
        else {
            *status = "Apply failed. Is Zalo running with debug port 9222?";
        }
    }
    return ok;
}

// Fetch a remote .css, store it next to the exe and apply it to Zalo.
inline bool FetchAndApplyStyleCss(
    const std::string& url,
    std::string* status = nullptr
) {
    if (url.empty()) {
        if (status) *status = "Enter a CSS URL first.";
        return false;
    }

    std::string css;
    if (!HttpGetUrl(Utf8ToWide(url), css)) {
        if (status) *status = "Download failed (check URL / network).";
        return false;
    }

    const fs::path styleFile = GetAppDirectory() / L"style.css";
    const fs::path urlFile = GetAppDirectory() / L"theme_url.txt";

    WriteTextFile(styleFile, css);
    WriteTextFile(urlFile, url);

    const bool ok = ApplyThemeCss(css, Z4_THEME_STYLE_ID);

    if (status) {
        if (ok) {
            std::ostringstream ss;
            ss << "Fetched and applied " << css.size() << " bytes.";
            *status = ss.str();
        }
        else {
            *status = "Downloaded, but applying failed. Is Zalo on port 9222?";
        }
    }

    return ok;
}

inline bool RemoveZaloTheme(std::string* status = nullptr) {
    std::string script = THEME_API_JS;
    script += "\nwindow.remove_theme(\"";
    script += JsonEscapeValue(Z4_THEME_STYLE_ID);
    script += "\");";

    const bool ok = ExecuteJavaScript(script);
    if (status) {
        *status = ok ? "Zalo theme removed." : "Remove failed (Zalo not reachable).";
    }
    return ok;
}

inline std::string ReadSavedThemeUrl() {
    std::string url;
    ReadTextFile(GetAppDirectory() / L"theme_url.txt", url);
    return url;
}

// ---------------------------------------------------------------
//  Custom sticker injection
// ---------------------------------------------------------------
inline bool EnableCustomSticker(std::string* status = nullptr) {
    const std::vector<fs::path> files = CollectStickerFiles();

    std::ostringstream json;
    json << "[";

    for (size_t i = 0; i < files.size(); ++i) {
        if (i > 0) json << ",";
        json << "{\"name\":\""
             << JsonEscapeValue(WideToUtf8(files[i].filename().wstring()))
             << "\",\"path\":\""
             << JsonEscapeValue(WideToUtf8(files[i].wstring()))
             << "\"}";
    }

    json << "]";

    std::string script =
        "window.__z4customDisabled = false;\n"
        "window.__z4customStickers = " + json.str() + ";\n" +
        CUSTOM_STICKER_JS +
        "\nif (window.__z4customPanelRefresh) "
        "window.__z4customPanelRefresh();";

    const bool ok = ExecuteJavaScript(script);

    if (status) {
        if (ok) {
            std::ostringstream ss;
            ss << "Custom Sticker enabled (" << files.size() << " sticker"
               << (files.size() == 1 ? "" : "s") << ").";
            *status = ss.str();
        }
        else {
            *status = "Enable failed. Is Zalo running with debug port 9222?";
        }
    }

    return ok;
}

inline bool RemoveCustomSticker(std::string* status = nullptr) {
    const std::string script = R"JS(
(function () {
    window.__z4customDisabled = true;

    if (window.__z4customObserver) {
        window.__z4customObserver.disconnect();
        window.__z4customObserver = null;
    }

    var panel = document.getElementById("z4-custom-panel");
    var tab = document.getElementById("z4-custom-tab");
    var style = document.getElementById("z4-custom-style");

    if (panel) panel.remove();
    if (tab) tab.remove();
    if (style) style.remove();

    return "CUSTOM_STICKER_REMOVED";
})();
)JS";

    const bool ok = ExecuteJavaScript(script);
    if (status) {
        *status = ok ? "Custom Sticker removed." : "Remove failed (Zalo not reachable).";
    }
    return ok;
}

// ---------------------------------------------------------------
//  Misc: live Zalo config flags
// ---------------------------------------------------------------
struct MiscFlag {
    std::string key;
    std::string type;   // bool | number | string | array
    std::string text;   // last applied value as text
    bool b = false;     // pending bool (edited in UI, sent on Apply)
    double num = 0.0;   // pending number
    char strBuf[512] = {}; // pending string edit buffer
    bool dirty = false;
    bool appliedB = false;
    double appliedNum = 0.0;
    std::string appliedText;
};

inline bool FetchMiscConfig(
    std::vector<MiscFlag>& flags,
    std::string& status
) {
    const std::string result = EvaluateJavaScript(MISC_FETCH_JS);

    if (result.empty()) {
        status = "Fetch failed. Start Zalo with --remote-debugging-port=9222.";
        return false;
    }

    if (result.rfind("ERR\t", 0) == 0) {
        status = result.substr(4);
        return false;
    }

    flags.clear();

    std::istringstream stream(result);
    std::string line;

    while (std::getline(stream, line)) {
        if (line == "OK" || line.empty()) continue;

        const size_t first = line.find('\t');
        if (first == std::string::npos) continue;

        const size_t second = line.find('\t', first + 1);
        if (second == std::string::npos) continue;

        MiscFlag flag;
        flag.key = line.substr(0, first);
        flag.type = line.substr(first + 1, second - first - 1);
        flag.text = line.substr(second + 1);

        // The JS side reports typeof() === "boolean"; normalize it.
        if (flag.type == "boolean") flag.type = "bool";

        if (flag.type == "bool") {
            flag.b = (flag.text == "true");
        }
        else if (flag.type == "number") {
            try { flag.num = std::stod(flag.text); } catch (...) { flag.num = 0.0; }
        }

        flag.appliedB = flag.b;
        flag.appliedNum = flag.num;
        flag.appliedText = flag.text;
        flag.dirty = false;
        strncpy_s(flag.strBuf, flag.text.c_str(), _TRUNCATE);

        flags.push_back(std::move(flag));
    }

    std::sort(flags.begin(), flags.end(),
        [](const MiscFlag& a, const MiscFlag& b) { return a.key < b.key; });

    std::ostringstream ss;
    ss << "Fetched " << flags.size() << " live flags.";
    status = ss.str();

    return !flags.empty();
}

// `literal` must be a raw JavaScript literal: true, false, 42, "text".
inline bool SetMiscFlag(
    const MiscFlag& flag,
    const std::string& literal,
    std::string& status
) {
    std::string script = MISC_SET_JS;

    size_t keyPos = script.find("__Z4KEY__");
    if (keyPos != std::string::npos) {
        script.replace(keyPos, 9, JsonEscapeValue(flag.key));
    }

    size_t valuePos = script.find("__Z4VALUE__");
    if (valuePos != std::string::npos) {
        script.replace(valuePos, 11, literal);
    }

    const std::string result = EvaluateJavaScript(script);

    if (result.empty()) {
        status = "Set failed (Zalo not reachable).";
        return false;
    }

    if (result.rfind("ERR\t", 0) == 0) {
        status = result.substr(4);
        return false;
    }

    status = flag.key + " = " + result.substr(std::min<size_t>(3, result.size()));
    return true;
}
