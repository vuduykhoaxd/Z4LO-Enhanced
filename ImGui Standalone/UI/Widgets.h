#pragma once
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <string>
#include "Drawing.h"

namespace Widgets {
    constexpr float kSidebarW = 204.f;
    constexpr float kHeaderH  = 68.f;
    constexpr float kFooterH  = 30.f;
    constexpr float kMargin   = 24.f;
    constexpr float kCardPad  = 18.f;

    inline ImVec2 P(float x, float y) { return ImVec2(x, y); }
    inline ImU32  U32(const ImVec4& c) { return ImGui::ColorConvertFloat4ToU32(c); }
    inline float  Luma(const ImVec4& c) { return 0.299f * c.x + 0.587f * c.y + 0.114f * c.z; }

    ImVec4 Mix(const ImVec4& a, const ImVec4& b, float t);
    ImVec4 Alpha(ImVec4 c, float a);
    void  Txt(ImDrawList* dl, ImVec2 p, ImU32 col, const char* t, float scale = 1.f);
    float TxtW(const char* t, float scale = 1.f);
    float TxtH(float scale = 1.f);
    float Anim(ImGuiID id, float target, float speed = 14.f);

    struct PresetDef { ImVec4 bg, accent, text; float alpha; };
    extern const PresetDef kPresets[4];
    extern ImVec4 g_cBg;
    extern ImVec4 g_cAccent;
    extern ImVec4 g_cText;

    extern ImDrawListSplitter g_split;
    extern ImVec2 g_cardPos;
    extern float  g_cardW;
    extern float  g_innerW;

    void DrawIcon(ImDrawList* dl, int type, ImVec2 c, float r, ImU32 col);
    void DrawLogo(ImDrawList* dl, ImVec2 p, float size);
    bool PrimaryButton(const char* label, ImVec2 size);
    bool NavItem(const char* id, const char* label, int icon, bool active, bool enabled);
    void LanguageSwitch(ImVec2 p, float w);
    void BeginCard(const char* title, const char* sub = nullptr, const char* chip = nullptr, bool chipOn = false);
    void EndCard();
    bool SettingRow(const char* label, const char* desc, bool* v, bool last);
    bool ColorRow(const char* label, ImVec4* c);
    bool SliderRow(const char* label, float* v, float lo, float hi, const char* fmt);
    void StatusLine(const std::string& text);
    bool ToggleSwitch(const char* id, bool* v);
} // namespace Widgets
