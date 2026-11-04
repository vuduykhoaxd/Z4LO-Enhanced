#include "Drawing.h"

LPCSTR Drawing::lpWindowName = "Zalo Enhanced";
ImVec2 Drawing::vWindowSize = { 460, 400 };
ImGuiWindowFlags Drawing::WindowFlags =
ImGuiWindowFlags_NoResize |
ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse |
ImGuiWindowFlags_NoSavedSettings;
bool Drawing::bDraw = true;

int  Drawing::nActiveTab = 0;
int  Drawing::nLanguage = 1;     

bool Drawing::bOpenThemeEditor = false;
int  Drawing::nActiveTheme = 0;

float  Drawing::fWindowRounding = 12.f;
float  Drawing::fFrameRounding = 8.f;
float  Drawing::fWindowAlpha = 1.f;
float  Drawing::fItemSpacing = 12.f;
ImVec4 Drawing::colWindowBg = ImVec4(0.09f, 0.10f, 0.13f, 1.f);
ImVec4 Drawing::colAccent = ImVec4(0.35f, 0.45f, 0.95f, 1.f);
ImVec4 Drawing::colText = ImVec4(0.92f, 0.94f, 1.f, 1.f);
ImVec4 Drawing::colFrameBg = ImVec4(0.15f, 0.17f, 0.22f, 1.f);
ImVec4 Drawing::colBorder = ImVec4(0.20f, 0.22f, 0.28f, 1.f);

float Drawing::fUiScale = 0.85f;

bool Drawing::auth = false;
bool Drawing::blur_msg = false;
bool Drawing::feat_auto_update = true;
bool Drawing::feat_startup = false;
bool Drawing::feat_low_cpu = true;

const char* Drawing::T(const char* key)
{
    struct Entry { const char* key; const char* en; const char* vi; };
    static const Entry table[] = {
        { "app_title",      "Zalo Enhanced",          "Zalo Enhanced" },
        { "app_sub",        "Privacy Control",           "Kiểm soát riêng tư" },
        { "lang_btn",       "EN",                       "VI" },

        { "tab_privacy",    "Privacy",                  "Riêng tư" },
        { "tab_theme",      "Theme",                    "Giao diện" },
        { "tab_feature",    "Feature",                  "Tính năng" },

        { "open_btn",       "Open",                     "Mở" },
        { "open_desc",      "Activate the module to begin", "Kích hoạt module để bắt đầu" },
        { "activated",      "Activated",                "Đã kích hoạt" },

        { "privacy_title",  "Privacy Controls",         "Kiểm soát riêng tư" },
        { "blur_label",     "Blur Message",             "Làm mờ tin nhắn" },
        { "blur_desc",      "Blur message content in chat window", "Làm mờ nội dung tin nhắn trong cửa sổ chat" },

        { "theme_title",    "Appearance",               "Giao diện" },
        { "theme_open",     "Open Theme Editor",        "Mở trình chỉnh theme" },
        { "theme_open_desc","Choose from 5 presets or customize", "Chọn từ 5 preset có sẵn hoặc tuỳ chỉnh" },
        { "theme_active",   "Active theme",             "Theme đang dùng" },

        { "feat_title",     "Feature Toggles",          "Bật / tắt tính năng" },
        { "feat_auto",      "Auto Update",              "Tự động cập nhật" },
        { "feat_auto_desc", "Check for updates on startup", "Kiểm tra bản mới khi khởi động" },
        { "feat_startup",   "Run at Startup",           "Chạy cùng Windows" },
        { "feat_startup_desc","Launch app when Windows starts","Khởi động app cùng Windows" },
        { "feat_cpu",       "Low CPU Mode",             "Chế độ tiết kiệm CPU" },
        { "feat_cpu_desc",  "Reduce refresh rate to save CPU", "Giảm tần suất render để tiết kiệm CPU" },

        { "te_title",       "Theme Editor",             "Trình chỉnh theme" },
        { "te_hint",        "Preset applies immediately, no save needed.",
                            "Preset áp dụng ngay, không cần lưu." },
        { "te_dark",        "Apply SaaS 2026 Dark Theme",  "Áp dụng SaaS 2026 Tối" },
        { "te_light",       "Apply SaaS 2026 Light Theme", "Áp dụng SaaS 2026 Sáng" },
        { "te_neon",        "Apply SaaS 2026 Neon Theme",  "Áp dụng SaaS 2026 Neon" },
        { "te_glass",       "Apply Glass Theme",           "Áp dụng Glass Theme" },
        { "te_custom",      "Custom Theme",                "Theme tuỳ chỉnh" },
    };
    for (auto& e : table)
        if (strcmp(e.key, key) == 0)
            return (nLanguage == 1) ? e.vi : e.en;
    return key;
}

void Drawing::ApplyCurrentStyle()
{
    ImGuiStyle& s = ImGui::GetStyle();
    s.WindowRounding = 0.0f;
    s.ChildRounding = 0.0f;
    s.FrameRounding = 0.0f;
    s.PopupRounding = 0.0f;
    s.ScrollbarRounding = 0.0f;
    s.GrabRounding = 0.0f;
    s.TabRounding = 0.0f;
    s.ItemSpacing = ImVec2(10, fItemSpacing);
    s.WindowPadding = ImVec2(20, 20);
    s.FramePadding = ImVec2(8, 4);

    s.Colors[ImGuiCol_WindowBg] = colWindowBg;
    s.Colors[ImGuiCol_Text] = colText;
    s.Colors[ImGuiCol_Button] = colAccent;
    s.Colors[ImGuiCol_ButtonHovered] = ImVec4(colAccent.x + 0.10f, colAccent.y + 0.10f, colAccent.z + 0.05f, 1.f);
    s.Colors[ImGuiCol_ButtonActive] = ImVec4(colAccent.x - 0.05f, colAccent.y - 0.05f, colAccent.z - 0.10f, 1.f);
    s.Colors[ImGuiCol_CheckMark] = colAccent;
    s.Colors[ImGuiCol_FrameBg] = colFrameBg;
    s.Colors[ImGuiCol_Border] = colBorder;
}

void Drawing::ApplyPreset(int idx)
{
    nActiveTheme = idx;
    switch (idx)
    {
    case 0:
        fWindowRounding = 0.f; fFrameRounding = 0.f; fWindowAlpha = 1.f; fItemSpacing = 10.f;
        colWindowBg = ImVec4(0.09f, 0.10f, 0.13f, 1.f);
        colAccent = ImVec4(0.35f, 0.45f, 0.95f, 1.f);
        colText = ImVec4(0.92f, 0.94f, 1.00f, 1.f);
        colFrameBg = ImVec4(0.15f, 0.17f, 0.22f, 1.f);
        colBorder = ImVec4(0.20f, 0.22f, 0.28f, 1.f);
        break;
    case 1:
        fWindowRounding = 0.f; fFrameRounding = 0.f; fWindowAlpha = 1.f; fItemSpacing = 10.f;
        colWindowBg = ImVec4(0.98f, 0.98f, 1.00f, 1.f);
        colAccent = ImVec4(0.20f, 0.40f, 0.90f, 1.f);
        colText = ImVec4(0.10f, 0.12f, 0.18f, 1.f);
        colFrameBg = ImVec4(0.92f, 0.93f, 0.96f, 1.f);
        colBorder = ImVec4(0.80f, 0.82f, 0.88f, 1.f);
        break;
    case 2:
        fWindowRounding = 0.f; fFrameRounding = 0.f; fWindowAlpha = 1.f; fItemSpacing = 8.f;
        colWindowBg = ImVec4(0.04f, 0.02f, 0.10f, 1.f);
        colAccent = ImVec4(0.95f, 0.20f, 0.85f, 1.f);
        colText = ImVec4(0.65f, 1.00f, 0.95f, 1.f);
        colFrameBg = ImVec4(0.10f, 0.04f, 0.20f, 1.f);
        colBorder = ImVec4(0.55f, 0.15f, 0.85f, 1.f);
        break;
    case 3:
        fWindowRounding = 0.f; fFrameRounding = 0.f; fWindowAlpha = 0.75f; fItemSpacing = 10.f;
        colWindowBg = ImVec4(0.18f, 0.20f, 0.28f, 0.65f);
        colAccent = ImVec4(0.55f, 0.75f, 1.00f, 1.f);
        colText = ImVec4(1.00f, 1.00f, 1.00f, 1.f);
        colFrameBg = ImVec4(1.00f, 1.00f, 1.00f, 0.10f);
        colBorder = ImVec4(1.00f, 1.00f, 1.00f, 0.25f);
        break;
    default: break;
    }
    ApplyCurrentStyle();
}

void Drawing::PushCompactStyle()
{
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 6));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 5));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14, 8));
    ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 14.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 10.0f);

    ImGui::SetWindowFontScale(fUiScale);
}

void Drawing::PopCompactStyle()
{
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopStyleVar(5);
}

void Drawing::SectionTitle(const char* text)
{
    ImGui::PushStyleColor(ImGuiCol_Text, colAccent);
    ImGui::TextUnformatted(text);
    ImGui::PopStyleColor();
    ImGui::Separator();
    ImGui::Spacing();
}

void Drawing::DrawStatusBar()
{
    const float statusHeight = ImGui::GetTextLineHeightWithSpacing() + 2.0f;
    ImGui::SetCursorPosY(ImGui::GetWindowHeight() - statusHeight - ImGui::GetStyle().WindowPadding.y);
    ImGui::Separator();
    ImGui::TextDisabled("v1.0  •  %s", T("app_sub"));
}

bool Drawing::ButtonWithDesc(const char* label, const char* desc, ImVec2 size)
{
    ImGui::PushID(label);
    bool clicked = ImGui::Button(label, size);

    if (ImGui::IsItemHovered() && desc && desc[0])
        ImGui::SetTooltip("%s", desc);

    if (desc && desc[0])
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.55f, 0.60f, 0.70f, 1.0f));
        ImGui::SetWindowFontScale(fUiScale * 0.82f);
        ImGui::TextWrapped("%s", desc);
        ImGui::SetWindowFontScale(fUiScale);
        ImGui::PopStyleColor();
    }
    ImGui::PopID();
    return clicked;
}

bool Drawing::CheckboxWithDesc(const char* label, const char* desc, bool* v)
{
    ImGui::PushID(label);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6.0f, 2.0f));
    bool changed = ImGui::Checkbox(label, v);

    if (ImGui::IsItemHovered() && desc && desc[0])
        ImGui::SetTooltip("%s", desc);

    if (desc && desc[0])
    {
        ImGui::Indent(ImGui::GetFrameHeight() + 4.0f);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.55f, 0.60f, 0.70f, 1.0f));
        ImGui::SetWindowFontScale(fUiScale * 0.82f);
        ImGui::TextWrapped("%s", desc);
        ImGui::SetWindowFontScale(fUiScale);
        ImGui::PopStyleColor();
        ImGui::Unindent(ImGui::GetFrameHeight() + 4.0f);
    }
    ImGui::PopStyleVar();
    ImGui::PopID();
    return changed;
}

int Drawing::RenderTabBarFullWidth(const char** labels, int count, int current)
{
    ImGuiStyle& s = ImGui::GetStyle();
    float avail = ImGui::GetContentRegionAvail().x;
    float tabW = avail / (float)count;
    float h = ImGui::GetFrameHeight();

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

    int clicked = current;
    for (int i = 0; i < count; ++i)
    {
        ImGui::PushID(i);
        bool active = (i == current);

        if (active)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(colAccent.x, colAccent.y, colAccent.z, 0.85f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(colAccent.x, colAccent.y, colAccent.z, 0.85f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(colAccent.x, colAccent.y, colAccent.z, 1.00f));
        }
        else
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1, 1, 1, 0.06f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1, 1, 1, 0.10f));
        }

        if (ImGui::Button(labels[i], ImVec2(tabW - 1, h)))
            clicked = i;

        ImGui::PopStyleColor(3);
        ImGui::PopID();
        if (i < count - 1) ImGui::SameLine();
    }

    {
        ImVec2 wp = ImGui::GetWindowPos();
        float y = ImGui::GetCursorScreenPos().y + 1.0f;
        float x0 = wp.x + s.WindowPadding.x + tabW * current;
        ImGui::GetWindowDrawList()->AddLine(
            ImVec2(x0, y), ImVec2(x0 + tabW, y),
            ImGui::ColorConvertFloat4ToU32(colAccent), 2.0f);
    }

    ImGui::PopStyleVar();
    ImGui::Separator();
    return clicked;
}

void Drawing::RenderThemeEditorPopup()
{
    if (bOpenThemeEditor)
    {
        ImGui::OpenPopup("Theme Editor");
        bOpenThemeEditor = false;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12, 10));
    if (ImGui::BeginPopup("Theme Editor", ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::TextColored(colAccent, "%s", T("te_title"));
        ImGui::Separator();
        ImGui::Spacing();

        const char* keys[] = { "te_dark", "te_light", "te_neon", "te_glass", "te_custom" };
        for (int i = 0; i < 5; ++i)
        {
            bool active = (nActiveTheme == i);
            if (active)
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(colAccent.x, colAccent.y, colAccent.z, 0.85f));

            if (ImGui::Button(T(keys[i]), ImVec2(240, 28)))
                ApplyPreset(i);

            if (active)
                ImGui::PopStyleColor();

            ImGui::Spacing();
        }

        ImGui::Separator();
        ImGui::TextDisabled("%s", T("te_hint"));
        ImGui::EndPopup();
    }
    ImGui::PopStyleVar();
}

void Drawing::RenderPrivacyTab(bool& blur)
{
    SectionTitle(T("privacy_title"));
    CheckboxWithDesc(T("blur_label"), T("blur_desc"), &blur);

    ImGui::Dummy(ImVec2(0.0f, 2.0f));
    ImGui::TextColored(ImVec4(0.55f, 0.60f, 0.70f, 1.f),
        blur ? "● On" : "○ Off");
}

void Drawing::RenderThemeTab()
{
    SectionTitle(T("theme_title"));

    if (ButtonWithDesc(T("theme_open"), T("theme_open_desc"), ImVec2(-1, 30)))
        bOpenThemeEditor = true;

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    const char* names[] = {
        "SaaS 2026 Dark", "SaaS 2026 Light",
        "SaaS 2026 Neon", "Glass", "Custom"
    };
    ImGui::TextDisabled("%s:", T("theme_active"));
    ImGui::SameLine();
    ImGui::TextColored(colAccent, "%s", names[nActiveTheme]);
}

void Drawing::RenderFeatureTab()
{
    SectionTitle(T("feat_title"));
    CheckboxWithDesc(T("feat_auto"), T("feat_auto_desc"), &feat_auto_update);
    ImGui::Spacing();
    CheckboxWithDesc(T("feat_startup"), T("feat_startup_desc"), &feat_startup);
    ImGui::Spacing();
    CheckboxWithDesc(T("feat_cpu"), T("feat_cpu_desc"), &feat_low_cpu);
}

void Drawing::Active() { bDraw = true; }
bool Drawing::isActive() { return bDraw; }

void Drawing::Draw()
{
    if (!isActive()) return;

    static bool inited = false;
    if (!inited) { ApplyPreset(0); inited = true; }

    ImGui::SetNextWindowSize(vWindowSize, ImGuiCond_Once);
    ImGui::SetNextWindowBgAlpha(fWindowAlpha);
    ImGui::Begin(lpWindowName, &bDraw, WindowFlags);

    PushCompactStyle();
    if (!auth)
    {
        const float btnW = 40.0f;
        if (ImGui::Button(T("lang_btn"), ImVec2(btnW, 22.0f))) nLanguage = (nLanguage == 0) ? 1 : 0;
        if (ImGui::IsItemHovered()) ImGui::SetTooltip(nLanguage == 0 ? "Switch to Tiếng Việt" : "Chuyển sang English");
        ImGui::TextWrapped("%s", T("open_desc"));
        ImGui::Spacing();
        if (ButtonWithDesc(T("open_btn"), T("open_desc"), ImVec2(-1, 30)))
            auth = true;

        RenderThemeEditorPopup();
        DrawStatusBar();
        PopCompactStyle();
        ImGui::End();
        return;
    }

    ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.6f, 1.0f), "● %s", T("activated"));
    ImGui::Spacing();

    const char* tabLabels[] = { T("tab_privacy"), T("tab_theme"), T("tab_feature") };
    nActiveTab = RenderTabBarFullWidth(tabLabels, 3, nActiveTab);

    ImGui::Spacing();

    switch (nActiveTab)
    {
    case 0: RenderPrivacyTab(blur_msg); break;
    case 1: RenderThemeTab();           break;
    case 2: RenderFeatureTab();         break;
    }

    RenderThemeEditorPopup();
    DrawStatusBar();

    PopCompactStyle();
    ImGui::End();
}