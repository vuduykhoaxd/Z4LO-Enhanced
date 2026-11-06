#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include "Drawing.h"
#include <cmath>
#include <cstring>
#include <cstdio>
#include <string>
#include <vector>
#include <algorithm>
LPCSTR Drawing::lpWindowName = "Zalo Enhanced";
ImVec2 Drawing::vWindowSize = { 760, 470 };
ImGuiWindowFlags Drawing::WindowFlags =
ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings;
bool Drawing::bDraw = true;
HWND Drawing::hwnd = nullptr;
void Drawing::AttachHwnd(HWND h) { hwnd = h; }

int Drawing::nActiveTab = 0;
int Drawing::nLanguage = 1;
int Drawing::nActiveTheme = 0;

float  Drawing::fWindowRounding = 12.f;
float  Drawing::fFrameRounding = 8.f;
float  Drawing::fWindowAlpha = 1.f;
float  Drawing::fItemSpacing = 10.f;
float  Drawing::fUiScale = 1.0f;
ImVec4 Drawing::colWindowBg = ImVec4(0.09f, 0.10f, 0.13f, 1.f);
ImVec4 Drawing::colAccent = ImVec4(0.35f, 0.45f, 0.95f, 1.f);
ImVec4 Drawing::colText = ImVec4(0.92f, 0.94f, 1.f, 1.f);
ImVec4 Drawing::colFrameBg = ImVec4(0.15f, 0.17f, 0.22f, 1.f);
ImVec4 Drawing::colBorder = ImVec4(0.20f, 0.22f, 0.28f, 1.f);
ImVec4 Drawing::colSidebar = ImVec4(0.06f, 0.07f, 0.09f, 1.f);
ImVec4 Drawing::colCard = ImVec4(0.12f, 0.13f, 0.17f, 1.f);
ImVec4 Drawing::colTextDim = ImVec4(0.55f, 0.58f, 0.66f, 1.f);

bool Drawing::auth = false;

bool Drawing::blur_msg = false;
bool Drawing::blur_image = false;
bool Drawing::blur_sticker = false;
bool Drawing::blur_ai_sticker = false;
bool Drawing::blur_url = false;
bool Drawing::blur_embed = false;
bool Drawing::blur_conversation_list = false;
bool Drawing::blur_notify = false;

bool Drawing::feat_auto_update = true;
bool Drawing::feat_startup = false;
bool Drawing::feat_low_cpu = true;


#include "Widgets.h"
#include "Tabs/PrivacyTab.h"
#include "Tabs/ThemeTab.h"
#include "Tabs/StickerTab.h"
#include "Tabs/MiscTab.h"
#include "Tabs/DebugTab.h"
using namespace Widgets;
using namespace Tabs;

namespace {
    void RenderWelcome()
    {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        const ImVec2 p = ImGui::GetCursorScreenPos();
        const float w = ImGui::GetContentRegionAvail().x;
        const float cx = p.x + w * 0.5f;
        const float top = p.y + 34.f;

        DrawLogo(dl, P(cx - 32.f, top), 64.f);

        const char* title = Drawing::T("welcome_title");
        Txt(dl, P(cx - TxtW(title, 1.6f) * 0.5f, top + 84.f), U32(Drawing::colText), title, 1.6f);

        const char* desc = Drawing::T("open_desc");
        Txt(dl, P(cx - TxtW(desc) * 0.5f, top + 118.f), U32(Drawing::colTextDim), desc);

        ImGui::SetCursorScreenPos(P(cx - 110.f, top + 156.f));
        if (PrimaryButton(Drawing::T("open_btn"), P(220.f, 40.f))) {
            /*ForceStopZalo();
            OpenZaloDebugging();
            ExecuteJavaScript(SIMULATE_TOUCH_JS, true);
            ExecuteJavaScript(SIMULATE_MOUSE_JS, true);
            ExecuteJavaScript(SIMULATETOUCH_JS, true);*/
            Drawing::auth = true;
        }
            
    }
    void DrawSidebar(const ImVec2& ws)
    {
        ImGui::SetCursorPos(P(0, 0));
        ImGui::BeginChild("##side", P(kSidebarW, ws.y), false,
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBackground);
        ImGui::SetWindowFontScale(Drawing::fUiScale);

        ImDrawList* dl = ImGui::GetWindowDrawList();
        const ImVec2 o = ImGui::GetWindowPos();

        DrawLogo(dl, P(o.x + 20.f, o.y + 22.f), 36.f);
        Txt(dl, P(o.x + 66.f, o.y + 24.f), U32(Drawing::colText), Drawing::T("app_title"), 1.05f);
        Txt(dl, P(o.x + 66.f, o.y + 43.f), U32(Drawing::colTextDim), Drawing::T("app_sub"), 0.84f);

        Txt(dl, P(o.x + 24.f, o.y + 92.f), U32(Alpha(Drawing::colTextDim, 0.8f)), Drawing::T("nav_menu"), 0.76f);

        const char* ids[5] = { "nav_p", "nav_t", "nav_s", "nav_m", "nav_d" };
        const char* keys[5] = { "tab_privacy", "tab_theme", "tab_sticker", "tab_misc", "tab_debug" };
        for (int i = 0; i < 5; ++i)
        {
            ImGui::SetCursorScreenPos(P(o.x + 12.f, o.y + 114.f + i * 44.f));
            if (NavItem(ids[i], Drawing::T(keys[i]), i % 3, Drawing::nActiveTab == i && Drawing::auth, Drawing::auth))
                Drawing::nActiveTab = i;
        }

        LanguageSwitch(P(o.x + 14.f, o.y + ws.y - 84.f), kSidebarW - 28.f);
        Txt(dl, P(o.x + 18.f, o.y + ws.y - 38.f), U32(Alpha(Drawing::colTextDim, 0.7f)), "v1.0", 0.82f);

        ImGui::EndChild();
    }
    void DrawMain(const ImVec2& ws)
    {
        ImGui::SetCursorPos(P(kSidebarW, 0));
        ImGui::BeginChild("##main", P(ws.x - kSidebarW, ws.y), false,
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBackground);
        ImGui::SetWindowFontScale(Drawing::fUiScale);

        ImDrawList* dl = ImGui::GetWindowDrawList();
        const ImVec2 o = ImGui::GetWindowPos();
        const float w = ImGui::GetWindowWidth();
        const float h = ImGui::GetWindowHeight();

        const char* title = Drawing::T("app_title");
        const char* sub = Drawing::T("app_sub");
        if (Drawing::auth)
        {
            static const char* tk[5] = { "privacy_title", "theme_title", "sticker_title", "misc_title", "debug_title" };
            static const char* sk[5] = { "privacy_desc", "theme_desc", "sticker_desc", "misc_desc", "debug_desc" };
            title = Drawing::T(tk[Drawing::nActiveTab]);
            sub = Drawing::T(sk[Drawing::nActiveTab]);
        }
        Txt(dl, P(o.x + kMargin, o.y + 14.f), U32(Drawing::colText), title, 1.4f);
        Txt(dl, P(o.x + kMargin, o.y + 14.f + TxtH(1.4f) + 4.f), U32(Drawing::colTextDim), sub, 0.88f);

        {
            ImGui::SetCursorPos(P(w - 26.f - 16.f, 21.f));
            if (ImGui::InvisibleButton("##close", P(26.f, 26.f))) Drawing::bDraw = false;
            const bool hov = ImGui::IsItemHovered();
            const ImVec2 a = ImGui::GetItemRectMin(), b = ImGui::GetItemRectMax();
            if (hov) dl->AddRectFilled(a, b, IM_COL32(232, 68, 68, 230), 7.f);
            const ImVec2 c = P((a.x + b.x) * 0.5f, (a.y + b.y) * 0.5f);
            const ImU32 xc = hov ? IM_COL32(255, 255, 255, 255) : U32(Drawing::colTextDim);
            dl->AddLine(P(c.x - 5, c.y - 5), P(c.x + 5, c.y + 5), xc, 1.6f);
            dl->AddLine(P(c.x + 5, c.y - 5), P(c.x - 5, c.y + 5), xc, 1.6f);
        }
        // Borderless window: drag the OS window from the header strip
        // (the close button above takes precedence on its own rect).
        {
            ImGui::SetCursorPos(P(0, 0));
            ImGui::InvisibleButton("##drag", P(w - 42.f, kHeaderH));
            if (ImGui::IsItemActive() && ImGui::IsMouseDragging(0, 0.0f))
            {
                const ImVec2 d = ImGui::GetIO().MouseDelta;
                if (Drawing::hwnd && (d.x != 0.f || d.y != 0.f))
                {
                    RECT r = {};
                    GetWindowRect(Drawing::hwnd, &r);
                    SetWindowPos(Drawing::hwnd, nullptr,
                        r.left + static_cast<int>(d.x),
                        r.top + static_cast<int>(d.y),
                        0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
                }
            }
            if (ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        }
        dl->AddLine(P(o.x, o.y + kHeaderH), P(o.x + w, o.y + kHeaderH), U32(Drawing::colBorder));

        ImGui::SetCursorPos(P(0, kHeaderH + 1.f));
        ImGui::BeginChild("##body", P(w, h - kHeaderH - 1.f - kFooterH), false, ImGuiWindowFlags_NoBackground);
        ImGui::SetWindowFontScale(Drawing::fUiScale);
        ImGui::SetCursorPosY(18.f);

        if (!Drawing::auth) RenderWelcome();
        else
        {
            switch (Drawing::nActiveTab)
            {
            case 0: RenderPrivacyTab(); break;
            case 1: RenderThemeTab();   break;
            case 2: RenderStickerTab(); break;
            case 3: RenderMiscTab();    break;
            case 4: RenderDebugTab();   break;
            }
            ImGui::Dummy(P(0, 8));
        }
        ImGui::EndChild();

        const float fy = o.y + h - kFooterH;
        const float cy = fy + kFooterH * 0.5f;
        dl->AddLine(P(o.x, fy), P(o.x + w, fy), U32(Drawing::colBorder));
        dl->AddCircleFilled(P(o.x + kMargin + 4.f, cy),
            4.f, Drawing::auth ? IM_COL32(77, 217, 128, 255) : U32(Drawing::colTextDim));
        Txt(dl, P(o.x + kMargin + 16.f, cy - TxtH(0.85f) * 0.5f), U32(Drawing::colTextDim),
            Drawing::auth ? Drawing::T("activated") : Drawing::T("locked"), 0.85f);
        const char* ver = "v1.0";
        Txt(dl, P(o.x + w - kMargin - TxtW(ver, 0.85f), cy - TxtH(0.85f) * 0.5f),
            U32(Alpha(Drawing::colTextDim, 0.7f)), ver, 0.85f);

        ImGui::EndChild();
    }
}

const char* Drawing::T(const char* key)
{
    struct Entry { const char* key; const char* en; const char* vi; };
    static const Entry table[] = {
        { "app_title",     "Zalo Enhanced",       "Zalo Enhanced" },
        { "app_sub",       "Privacy Control",     "Kiá»ƒm soÃ¡t riÃªng tÆ°" },
        { "nav_menu",      "MENU",                "MENU" },

        { "tab_privacy",   "Privacy",             "RiÃªng tÆ°" },
        { "tab_theme",     "Theme",               "Giao diá»‡n" },
        { "tab_sticker",   "Sticker",             "Sticker" },
        { "tab_misc",      "Misc",                "Misc" },
        { "tab_debug",     "Debug",               "Debug" },

        { "sticker_title", "Custom Sticker",      "Custom Sticker" },
        { "sticker_desc",  "Convert and send WebP stickers", "Chuyá»ƒn vÃ  gá»­i sticker WebP" },
        { "debug_title",   "Debug",               "Debug" },
        { "debug_desc",    "Zalo process control", "Äiá»u khiá»ƒn tiáº¿n trÃ¬nh Zalo" },

        { "welcome_title", "Welcome",             "ChÃ o má»«ng" },
        { "open_btn",      "Open",                "Má»Ÿ" },
        { "open_desc",     "Activate the module to begin", "KÃ­ch hoáº¡t module Ä‘á»ƒ báº¯t Ä‘áº§u" },
        { "activated",     "Activated",           "ÄÃ£ kÃ­ch hoáº¡t" },
        { "locked",        "Not activated",       "ChÆ°a kÃ­ch hoáº¡t" },
        { "on",            "On",                  "Báº­t" },
        { "off",           "Off",                 "Táº¯t" },

        { "privacy_title", "Privacy Controls",    "Kiá»ƒm soÃ¡t riÃªng tÆ°" },
        { "privacy_desc",  "Manage what is visible on screen", "Quáº£n lÃ½ ná»™i dung hiá»ƒn thá»‹ trÃªn mÃ n hÃ¬nh" },
        { "blur_label",    "Blur Message",        "LÃ m má» tin nháº¯n" },
        { "blur_label_desc",     "Blur message content in chat window", "LÃ m má» ná»™i dung tin nháº¯n trong cá»­a sá»• chat" },
        { "blur_notify_label", "Blur Notifications",  "LÃ m má» thÃ´ng bÃ¡o" },
		{ "blur_notify_desc", "Blur message content in notifications", "LÃ m má» ná»™i dung tin nháº¯n trong thÃ´ng bÃ¡o" },
		{ "blur_conversation_list_label", "Blur Conversation List", "LÃ m má» danh sÃ¡ch há»™i thoáº¡i" },
		{ "blur_conversation_list_desc",  "Blur message content in conversation list", "LÃ m má» ná»™i dung tin nháº¯n trong danh sÃ¡ch há»™i thoáº¡i" },
        { "blur_image_label", "Blur Image", "LÃ m má» áº£nh" },
        { "blur_image_desc", "Blur image content in chat window", "LÃ m má» áº£nh trong cá»­a sá»• chat" },
        { "blur_sticker_label", "Blur Sticker", "LÃ m má» sticker" },
        { "blur_sticker_desc", "Blur sticker content in chat window", "LÃ m má» sticker trong cá»­a sá»• chat" },
        { "blur_ai_label", "Blur AI Sticker", "LÃ m má» AI Sticker" },
        { "blur_ai_desc", "Blur AI sticker content in chat window", "LÃ m má» AI Sticker trong cá»­a sá»• chat" },
        { "blur_url_label", "Blur URL", "LÃ m má» link" },
        { "blur_url_desc", "Blur URL/link content in chat window", "LÃ m má» link trong cá»­a sá»• chat" },
        { "blur_embed_label", "Blur Embed", "LÃ m má» embed" },
        { "blur_embed_desc", "Blur embed content in chat window", "LÃ m má» embed trong cá»­a sá»• chat" },

        { "theme_title",   "Appearance",          "Giao diá»‡n" },
        { "theme_desc",    "Choose a preset or build your own", "Chá»n preset cÃ³ sáºµn hoáº·c tá»± tuá»³ chá»‰nh" },
        { "preset_title",  "Presets",             "Preset cÃ³ sáºµn" },
        { "preset_desc",   "Applied immediately, no save needed", "Ãp dá»¥ng ngay, khÃ´ng cáº§n lÆ°u" },
        { "p_dark",        "Dark",                "Tá»‘i" },
        { "p_light",       "Light",               "SÃ¡ng" },
        { "p_neon",        "Neon",                "Neon" },
        { "p_glass",       "Glass",               "Glass" },
        { "p_custom",      "Custom",              "Tuá»³ chá»‰nh" },
        { "custom_title",  "Customize",           "Tuá»³ chá»‰nh" },
        { "custom_desc",   "Fine-tune colors and shapes", "Tinh chá»‰nh mÃ u sáº¯c vÃ  hÃ¬nh dáº¡ng" },
        { "c_accent",      "Accent color",        "MÃ u nháº¥n" },
        { "c_bg",          "Background",          "MÃ u ná»n" },
        { "c_text",        "Text color",          "MÃ u chá»¯" },
        { "c_round",       "Corner radius",       "Äá»™ bo gÃ³c" },
        { "c_scale",       "UI scale",            "Tá»‰ lá»‡ giao diá»‡n" },

        { "enable_all",    "Enable all",          "Báº­t táº¥t cáº£" },
        { "disable_all",   "Disable all",         "Táº¯t táº¥t cáº£" },

        { "zalo_theme_title", "Zalo Theme",       "Giao diá»‡n Zalo" },
        { "zalo_theme_desc",  "Apply CSS directly inside Zalo (not the tool UI)", "Ãp dá»¥ng CSS trá»±c tiáº¿p vÃ o Zalo (khÃ´ng pháº£i UI)" },
        { "apply_liquid_glass", "Apply Liquid Glass", "Ãp dá»¥ng Liquid Glass" },
        { "remove_zalo_theme",  "Remove theme",       "Gá»¡ giao diá»‡n" },
        { "theme_url_label",    "CSS URL",            "URL file CSS" },
        { "fetch_apply",        "Fetch & apply",      "Táº£i & Ã¡p dá»¥ng" },
        { "theme_file_label",   "Local style.css",    "File style.css" },
        { "browse",             "Browse...",          "Chá»n file..." },
        { "apply_local_css",    "Apply local CSS",    "Ãp dá»¥ng CSS local" },

        { "sticker_create_title", "Create sticker",   "Táº¡o sticker" },
        { "sticker_create_desc",  "Turn an existing image into a transparent WebP sticker", "Biáº¿n áº£nh cÃ³ sáºµn thÃ nh sticker WebP trong suá»‘t" },
        { "choose_image",         "Choose image...",  "Chá»n áº£nh..." },
        { "no_file",              "No image selected", "ChÆ°a chá»n áº£nh" },
        { "cutout_mode",          "Background removal", "TÃ¡ch ná»n" },
        { "mode_auto",            "Auto",             "Tá»± Ä‘á»™ng" },
        { "mode_rembg",           "rembg (AI)",       "rembg (AI)" },
        { "mode_keep",            "Keep as-is",       "Giá»¯ nguyÃªn" },
        { "border_px",            "Border (px)",      "Viá»n (px)" },
        { "add_shadow",           "Add shadow",       "ThÃªm bÃ³ng Ä‘á»•" },
        { "max_size",             "Max size (px)",    "KÃ­ch thÆ°á»›c tá»‘i Ä‘a (px)" },
        { "convert_now",          "Convert to sticker", "Chuyá»ƒn thÃ nh sticker" },
        { "sticker_lib_title",    "Sticker library",  "ThÆ° viá»‡n sticker" },
        { "sticker_lib_desc",     "Stored outside Zalo, synced by sending as WebP", "LÆ°u ngoÃ i Zalo, Ä‘á»“ng bá»™ nhá» gá»­i dáº¡ng WebP" },
        { "sticker_count",        "Stickers on disk: %d", "Sticker trÃªn Ä‘Ä©a: %d" },
        { "open_folder",          "Open folder",      "Má»Ÿ thÆ° má»¥c" },
        { "copy_last",            "Copy last sticker", "Copy sticker má»›i nháº¥t" },
        { "enable_sticker",       "Enable Custom Sticker", "Báº­t Custom Sticker" },
        { "remove_sticker",       "Remove Custom Sticker", "Gá»¡ Custom Sticker" },

        { "misc_title",           "Misc Features",    "TÃ­nh nÄƒng Misc" },
        { "misc_desc",            "Fetch Zalo's live config, then search / toggle flags", "Äá»c live config cá»§a Zalo, tÃ¬m kiáº¿m / báº­t táº¯t" },
        { "fetch_all",            "Fetch all flags",  "Táº£i toÃ n bá»™ flags" },
        { "misc_fetch_hint",      "Click Fetch to load Zalo's live config", "Báº¥m Táº£i Ä‘á»ƒ Ä‘á»c live config cá»§a Zalo" },
        { "misc_count",           "%d live flags",    "%d flag trá»±c tiáº¿p" },
        { "misc_list_title",      "Live flags",       "Flags trá»±c tiáº¿p" },
        { "misc_list_desc",       "Edit the value / toggle, then press Apply to send it to Zalo", "Sá»­a giÃ¡ trá»‹ / báº­t táº¯t, rá»“i báº¥m Ãp dá»¥ng Ä‘á»ƒ gá»­i vÃ o Zalo" },
        { "apply",                "Apply",             "Ãp dá»¥ng" },
        { "search_hint",          "Search flags...",  "TÃ¬m flag..." },
        { "misc_showing",         "Showing %d / %d",  "Hiá»ƒn thá»‹ %d / %d" },
        { "load_more",            "Load 80 more",     "Táº£i thÃªm 80" },

        { "zalo_status",          "Zalo process: %s", "Tiáº¿n trÃ¬nh Zalo: %s" },
        { "running",              "running",          "Ä‘ang cháº¡y" },
        { "not_running",          "not running",      "khÃ´ng cháº¡y" },
        { "force_stop",           "Force stop Zalo",  "Dá»«ng Zalo" },
        { "open_debug",           "Open Zalo (9222)", "Má»Ÿ Zalo (9222)" },
        { "disable_touch",        "Disable touch emulation", "Táº¯t giáº£ láº­p cáº£m á»©ng" },
        { "test_cdp",             "Test CDP connection", "Kiá»ƒm tra káº¿t ná»‘i CDP" },

        { "feat_title",    "Feature Toggles",     "Báº­t / táº¯t tÃ­nh nÄƒng" },


    };
    for (auto& e : table)
        if (strcmp(e.key, key) == 0)
            return (nLanguage == 1) ? e.vi : e.en;
    return key;
}

void Drawing::DeriveColors()
{
    const bool dark = Luma(colWindowBg) < 0.5f;
    const ImVec4 black(0, 0, 0, 1), white(1, 1, 1, 1);

    colSidebar = dark ? Mix(colWindowBg, black, 0.30f) : Mix(colWindowBg, colText, 0.04f);
    colCard = dark ? Mix(colWindowBg, colText, 0.045f) : white;
    colFrameBg = Mix(colWindowBg, colText, dark ? 0.10f : 0.08f);
    colBorder = Mix(colWindowBg, colText, dark ? 0.13f : 0.12f);
    colTextDim = Mix(colWindowBg, colText, 0.58f);

    colSidebar.w = colCard.w = fWindowAlpha;
    colFrameBg.w = colBorder.w = colTextDim.w = 1.f;
}

void Drawing::ApplyCurrentStyle()
{
    DeriveColors();
    ImGuiStyle& s = ImGui::GetStyle();
    ImVec4* c = s.Colors;

    c[ImGuiCol_WindowBg] = colWindowBg;
    c[ImGuiCol_ChildBg] = ImVec4(0, 0, 0, 0);
    c[ImGuiCol_PopupBg] = Mix(colCard, colWindowBg, 0.f);
    c[ImGuiCol_Border] = colBorder;
    c[ImGuiCol_Text] = colText;
    c[ImGuiCol_TextDisabled] = colTextDim;

    c[ImGuiCol_Button] = colAccent;
    c[ImGuiCol_ButtonHovered] = Mix(colAccent, ImVec4(1, 1, 1, 1), 0.15f);
    c[ImGuiCol_ButtonActive] = Mix(colAccent, ImVec4(0, 0, 0, 1), 0.15f);

    c[ImGuiCol_FrameBg] = colFrameBg;
    c[ImGuiCol_FrameBgHovered] = Mix(colFrameBg, colText, 0.06f);
    c[ImGuiCol_FrameBgActive] = Mix(colFrameBg, colText, 0.10f);

    c[ImGuiCol_SliderGrab] = colAccent;
    c[ImGuiCol_SliderGrabActive] = Mix(colAccent, ImVec4(1, 1, 1, 1), 0.2f);
    c[ImGuiCol_CheckMark] = colAccent;

    c[ImGuiCol_Separator] = colBorder;
    c[ImGuiCol_ScrollbarBg] = ImVec4(0, 0, 0, 0);
    c[ImGuiCol_ScrollbarGrab] = Mix(colFrameBg, colText, 0.15f);
    c[ImGuiCol_ScrollbarGrabHovered] = Mix(colFrameBg, colText, 0.30f);
    c[ImGuiCol_ScrollbarGrabActive] = colAccent;
}

void Drawing::ApplyPreset(int idx)
{
    nActiveTheme = idx;
    ApplyCurrentStyle();
}

void Drawing::Active() { bDraw = true; }
bool Drawing::isActive() { return bDraw; }

void Drawing::Draw()
{
    if (!isActive()) return;

    static bool inited = false;
    if (!inited) { ApplyPreset(0); inited = true; }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, fWindowRounding);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, P(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.f);
    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 6.f);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, P(10.f, fItemSpacing));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, P(10.f, 6.f));

    ImGui::SetNextWindowPos(P(0, 0), ImGuiCond_Once);
    ImGui::SetNextWindowSize(vWindowSize, ImGuiCond_Once);
    ImGui::SetNextWindowBgAlpha(fWindowAlpha);
    ImGui::Begin(lpWindowName, &bDraw, WindowFlags);

    ImGui::SetWindowFontScale(fUiScale);
    const ImVec2 wp = ImGui::GetWindowPos();
    const ImVec2 ws = ImGui::GetWindowSize();
    ImDrawList* dl = ImGui::GetWindowDrawList();

    dl->AddRectFilled(wp, P(wp.x + kSidebarW, wp.y + ws.y), U32(colSidebar),
        fWindowRounding, ImDrawFlags_RoundCornersLeft);
    dl->AddLine(P(wp.x + kSidebarW, wp.y + 1.f), P(wp.x + kSidebarW, wp.y + ws.y - 1.f), U32(colBorder));

    DrawSidebar(ws);
    DrawMain(ws);

    ImGui::End();
    ImGui::PopStyleVar(6);
}

