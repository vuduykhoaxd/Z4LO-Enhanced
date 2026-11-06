#include "ThemeTab.h"
#include "../Widgets.h"
#include "../Drawing.h"
#include <string>
#include <vector>
using namespace Widgets;
#include "../../Command/JavaScript.h"
#include "../../Core/Actions.hpp"
#include "../../Core/ExecuteJS.hpp"
#include "../../Utils/Zalo_Utils.hpp"
#include "../../Utils/FileDialog.hpp"

namespace {
    std::string g_themeStatus;
    char        g_themeUrl[512] = "";
    char        g_themePath[MAX_PATH * 2] = "";
    bool        g_themeUrlLoaded = false;
}

namespace Tabs {
    void RenderThemeTab()
    {
        ImDrawList* dl = ImGui::GetWindowDrawList();

        BeginCard(Drawing::T("preset_title"), Drawing::T("preset_desc"));
        {
            const char* keys[5] = { "p_dark", "p_light", "p_neon", "p_glass", "p_custom" };
            const float gap = 10.f;
            const float tileW = (g_innerW - gap * 4.f) / 5.f;
            const float prevH = 50.f;
            const ImVec2 origin = ImGui::GetCursorScreenPos();

            for (int i = 0; i < 5; ++i)
            {
                const ImVec2 tp = P(origin.x + (tileW + gap) * i, origin.y);
                ImGui::SetCursorScreenPos(tp);
                ImGui::PushID(i);
                if (ImGui::InvisibleButton("##preset", P(tileW, prevH + 22.f))) Drawing::ApplyPreset(i);
                const bool hov = ImGui::IsItemHovered();
                ImGui::PopID();
                if (hov) ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

                const ImVec4 bg = (i < 4) ? kPresets[i].bg : g_cBg;
                const ImVec4 ac = (i < 4) ? kPresets[i].accent : g_cAccent;
                const ImVec4 tx = (i < 4) ? kPresets[i].text : g_cText;
                const bool dark = Luma(bg) < 0.5f;
                const ImVec4 side = dark ? Mix(bg, ImVec4(0, 0, 0, 1), 0.3f) : Mix(bg, tx, 0.05f);

                const ImVec2 a = tp, b = P(tp.x + tileW, tp.y + prevH);
                dl->AddRectFilled(a, b, U32(Alpha(bg, 1.f)), 8.f);
                dl->AddRectFilled(a, P(a.x + tileW * 0.28f, b.y), U32(Alpha(side, 1.f)), 8.f, ImDrawFlags_RoundCornersLeft);
                dl->AddRectFilled(P(a.x + tileW * 0.36f, a.y + 10.f), P(a.x + tileW * 0.80f, a.y + 18.f), U32(ac), 3.f);
                dl->AddRectFilled(P(a.x + tileW * 0.36f, a.y + 26.f), P(a.x + tileW * 0.88f, a.y + 30.f), U32(Alpha(tx, 0.5f)), 2.f);
                dl->AddRectFilled(P(a.x + tileW * 0.36f, a.y + 35.f), P(a.x + tileW * 0.70f, a.y + 39.f), U32(Alpha(tx, 0.3f)), 2.f);

                const bool active = (Drawing::nActiveTheme == i);
                if (active)       dl->AddRect(a, b, U32(Drawing::colAccent), 8.f, 0, 2.f);
                else if (hov)     dl->AddRect(a, b, U32(Alpha(Drawing::colText, 0.35f)), 8.f);
                else              dl->AddRect(a, b, U32(Drawing::colBorder), 8.f);

                const char* name = Drawing::T(keys[i]);
                Txt(dl, P(tp.x + (tileW - TxtW(name, 0.86f)) * 0.5f, b.y + 6.f),
                    U32(active ? Drawing::colText : Drawing::colTextDim), name, 0.86f);
            }
            ImGui::SetCursorScreenPos(origin);
            ImGui::Dummy(P(g_innerW, prevH + 22.f));
        }
        EndCard();

        BeginCard(Drawing::T("custom_title"), Drawing::T("custom_desc"));
        {
            bool changed = false;
            changed |= ColorRow(Drawing::T("c_accent"), &Drawing::colAccent);
            changed |= ColorRow(Drawing::T("c_bg"), &Drawing::colWindowBg);
            changed |= ColorRow(Drawing::T("c_text"), &Drawing::colText);

            float r = Drawing::fFrameRounding;
            if (SliderRow(Drawing::T("c_round"), &r, 0.f, 16.f, "%.0f"))
            {
                Drawing::fFrameRounding = r;
                Drawing::fWindowRounding = r + 4.f;
                changed = true;
            }
            SliderRow(Drawing::T("c_scale"), &Drawing::fUiScale, 0.8f, 1.3f, "%.2f");

            if (changed)
            {
                g_cBg = Drawing::colWindowBg; g_cAccent = Drawing::colAccent; g_cText = Drawing::colText;
                Drawing::nActiveTheme = 4;
                Drawing::ApplyCurrentStyle();
            }
        }
        EndCard();

        BeginCard(Drawing::T("zalo_theme_title"), Drawing::T("zalo_theme_desc"));

        if (ImGui::Button(Drawing::T("apply_liquid_glass"), P(210.f, 32.f)))
        {
            ApplyLiquidGlassTheme(&g_themeStatus);
        }
        ImGui::SameLine();
        if (ImGui::Button(Drawing::T("remove_zalo_theme"), P(170.f, 32.f)))
        {
            RemoveZaloTheme(&g_themeStatus);
        }

        if (!g_themeUrlLoaded)
        {
            const std::string saved = ReadSavedThemeUrl();
            if (!saved.empty()) {
                strncpy_s(g_themeUrl, saved.c_str(), _TRUNCATE);
            }
            g_themeUrlLoaded = true;
        }

        ImGui::Spacing();
        ImGui::TextUnformatted(Drawing::T("theme_url_label"));
        ImGui::SetNextItemWidth(g_innerW - 136.f);
        ImGui::InputText("##theme_url", g_themeUrl, sizeof(g_themeUrl));
        ImGui::SameLine();
        if (ImGui::Button(Drawing::T("fetch_apply"), P(126.f, 0.f)))
        {
            FetchAndApplyStyleCss(g_themeUrl, &g_themeStatus);
        }

        ImGui::Spacing();
        ImGui::TextUnformatted(Drawing::T("theme_file_label"));
        ImGui::SetNextItemWidth(g_innerW - 136.f);
        ImGui::InputText("##theme_path", g_themePath, sizeof(g_themePath));
        ImGui::SameLine();
        if (ImGui::Button(Drawing::T("browse"), P(126.f, 0.f)))
        {
            const std::wstring file = OpenFileDialogW(
                nullptr,
                L"Select a Zalo style.css",
                L"CSS files (*.css)\0*.css\0All files (*.*)\0*.*\0"
            );
            if (!file.empty())
            {
                WideCharToMultiByte(CP_UTF8, 0, file.c_str(), -1,
                    g_themePath, sizeof(g_themePath), nullptr, nullptr);
            }
        }

        if (ImGui::Button(Drawing::T("apply_local_css"), P(190.f, 30.f)))
        {
            if (g_themePath[0] == '\0') {
                g_themeStatus = "Choose a local .css file first.";
            }
            else {
                ApplyStyleFile(fs::path(Utf8ToWide(g_themePath)), &g_themeStatus);
            }
        }

        StatusLine(g_themeStatus);
        EndCard();
    }
} // namespace Tabs

