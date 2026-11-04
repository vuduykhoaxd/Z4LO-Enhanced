#ifndef DRAWING_H
#define DRAWING_H

#include "pch.h"

class Drawing
{
public:
    static LPCSTR lpWindowName;
    static ImVec2 vWindowSize;
    static ImGuiWindowFlags WindowFlags;
    static bool bDraw;

    static void Active();
    static bool isActive();
    static void Draw();

private:
    // --- Tabs ---
    static int  nActiveTab;        // 0=Privacy, 1=Theme, 2=Feature

    // --- Language ---
    static int  nLanguage;         // 0=EN, 1=VI
    static const char* T(const char* key);

    // --- Theme Editor ---
    static bool bOpenThemeEditor;
    static int  nActiveTheme;

    // Theme state
    static float  fWindowRounding;
    static float  fFrameRounding;
    static float  fWindowAlpha;
    static float  fItemSpacing;
    static ImVec4 colWindowBg;
    static ImVec4 colAccent;
    static ImVec4 colText;
    static ImVec4 colFrameBg;
    static ImVec4 colBorder;

    // Compact style
    static float fUiScale;
    static void  PushCompactStyle();
    static void  PopCompactStyle();

    // Helpers
    static void ApplyCurrentStyle();
    static void ApplyPreset(int idx);
    static void RenderThemeEditorPopup();
    static void DrawStatusBar();

    // UI building blocks
    static bool ButtonWithDesc(const char* label, const char* desc, ImVec2 size = ImVec2(-1, 34));
    static bool CheckboxWithDesc(const char* label, const char* desc, bool* v);
    static void SectionTitle(const char* text);
    static int  RenderTabBarFullWidth(const char** labels, int count, int current);

    // Tab renderers
    static void RenderPrivacyTab(bool& blur);
    static void RenderThemeTab();
    static void RenderFeatureTab();

    // State
    static bool auth;
    static bool blur_msg;
    static bool feat_auto_update;
    static bool feat_startup;
    static bool feat_low_cpu;
};

#endif
