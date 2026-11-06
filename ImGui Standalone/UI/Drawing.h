#pragma once
#include <Windows.h>
#include <tchar.h>
#include "d3d9.h"
#include "d3d11.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"


class Drawing
{
public:
    // ---- Window ----
    static LPCSTR           lpWindowName;
    static ImVec2           vWindowSize;
    static ImGuiWindowFlags WindowFlags;
    static bool             bDraw;
    static HWND             hwnd;  // hidden D3D11 host (set by UI::Render); visible window is the ImGui viewport
    static void             AttachHwnd(HWND h);

    // ---- State ----
    static int  nActiveTab;      // 0 = Privacy, 1 = Theme, 2 = Custom Sticker, 3 = Misc, 4 = Debug
    static int  nLanguage;       // 0 = EN, 1 = VI
    static int  nActiveTheme;    // 0..3 = preset, 4 = custom

    // ---- Theme ----
    static float  fWindowRounding;
    static float  fFrameRounding;
    static float  fWindowAlpha;
    static float  fItemSpacing;
    static float  fUiScale;
    static ImVec4 colWindowBg;
    static ImVec4 colAccent;
    static ImVec4 colText;
    static ImVec4 colFrameBg;
    static ImVec4 colBorder;
    static ImVec4 colSidebar;    // derived
    static ImVec4 colCard;       // derived
    static ImVec4 colTextDim;    // derived

    // ---- Features ----
    static bool auth;

    static bool blur_msg;
    static bool blur_image;
    static bool blur_sticker;
    static bool blur_ai_sticker;
    static bool blur_url;
    static bool blur_embed;
    static bool blur_conversation_list;
    static bool blur_notify;

    static bool feat_auto_update;
    static bool feat_startup;
    static bool feat_low_cpu;

    // ---- API ----
    static const char* T(const char* key);
    static void DeriveColors();
    static void ApplyCurrentStyle();
    static void ApplyPreset(int idx);

    static void Active();
    static bool isActive();
    static void Draw();
};