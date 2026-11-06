#include "Widgets.h"
#include <algorithm>

namespace Widgets {


    ImVec4 Mix(const ImVec4& a, const ImVec4& b, float t)
    {
        return ImVec4(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t,
            a.z + (b.z - a.z) * t, a.w + (b.w - a.w) * t);
    }
    ImVec4 Alpha(ImVec4 c, float a) { c.w = a; return c; }

    void Txt(ImDrawList* dl, ImVec2 p, ImU32 col, const char* t, float scale)
    {
        dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, p, col, t);
    }
    float TxtW(const char* t, float scale) { return ImGui::CalcTextSize(t).x * scale; }
    float TxtH(float scale) { return ImGui::GetFontSize() * scale; }

    float Anim(ImGuiID id, float target, float speed)
    {
        ImGuiStorage* st = ImGui::GetStateStorage();
        float v = st->GetFloat(id, target);
        v += (target - v) * std::min(1.f, ImGui::GetIO().DeltaTime * speed);
        st->SetFloat(id, v);
        return v;
    }

    const PresetDef kPresets[4] = {
        { ImVec4(0.09f, 0.10f, 0.13f, 1.f), ImVec4(0.35f, 0.45f, 0.95f, 1.f), ImVec4(0.92f, 0.94f, 1.00f, 1.f), 1.00f },  
        { ImVec4(0.95f, 0.96f, 0.98f, 1.f), ImVec4(0.20f, 0.40f, 0.90f, 1.f), ImVec4(0.10f, 0.12f, 0.18f, 1.f), 1.00f },  
        { ImVec4(0.05f, 0.03f, 0.11f, 1.f), ImVec4(0.95f, 0.25f, 0.85f, 1.f), ImVec4(0.75f, 1.00f, 0.96f, 1.f), 1.00f },  
        { ImVec4(0.16f, 0.19f, 0.28f, 1.f), ImVec4(0.55f, 0.75f, 1.00f, 1.f), ImVec4(1.00f, 1.00f, 1.00f, 1.f), 0.80f },  
    };
    ImVec4 g_cBg = kPresets[0].bg;
    ImVec4 g_cAccent = kPresets[0].accent;
    ImVec4 g_cText = kPresets[0].text;

    ImDrawListSplitter g_split;
    ImVec2 g_cardPos;
    float  g_cardW = 0.f;
    float  g_innerW = 0.f;

    void DrawIcon(ImDrawList* dl, int type, ImVec2 c, float r, ImU32 col)
    {
        const float th = 1.7f;
        if (type == 0)  
        {
            ImVec2 pts[6] = {
                P(c.x, c.y - r), P(c.x + r * 0.85f, c.y - r * 0.55f), P(c.x + r * 0.75f, c.y + r * 0.25f),
                P(c.x, c.y + r), P(c.x - r * 0.75f, c.y + r * 0.25f), P(c.x - r * 0.85f, c.y - r * 0.55f) };
            dl->AddPolyline(pts, 6, col, ImDrawFlags_Closed, th);
        }
        else if (type == 1)  
        {
            dl->AddCircle(c, r, col, 0, th);
            dl->AddCircleFilled(P(c.x - r * 0.40f, c.y - r * 0.10f), r * 0.16f, col);
            dl->AddCircleFilled(P(c.x - r * 0.05f, c.y - r * 0.50f), r * 0.16f, col);
            dl->AddCircleFilled(P(c.x + r * 0.40f, c.y - r * 0.10f), r * 0.16f, col);
        }
        else  
        {
            const float ys[3] = { -0.6f, 0.f, 0.6f };
            const float ks[3] = { -0.35f, 0.40f, -0.05f };
            for (int i = 0; i < 3; ++i)
            {
                dl->AddLine(P(c.x - r, c.y + ys[i] * r), P(c.x + r, c.y + ys[i] * r), col, th);
                dl->AddCircleFilled(P(c.x + ks[i] * r, c.y + ys[i] * r), r * 0.22f, col);
            }
        }
    }

    void DrawLogo(ImDrawList* dl, ImVec2 p, float size)
    {
        dl->AddRectFilled(p, P(p.x + size, p.y + size), U32(Drawing::colAccent), size * 0.27f);
        const float sc = size / 20.f;
        const float fs = TxtH(sc);
        Txt(dl, P(p.x + (size - TxtW("Z", sc)) * 0.5f, p.y + (size - fs) * 0.5f),
            IM_COL32(255, 255, 255, 255), "Z", sc);
    }

    bool PrimaryButton(const char* label, ImVec2 size)
    {
        const bool lightAccent = Luma(Drawing::colAccent) > 0.65f;
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, Drawing::fFrameRounding);
        ImGui::PushStyleColor(ImGuiCol_Text, lightAccent ? ImVec4(0.05f, 0.05f, 0.08f, 1.f) : ImVec4(1, 1, 1, 1));
        bool c = ImGui::Button(label, size);
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
        return c;
    }

    bool NavItem(const char* id, const char* label, int icon, bool active, bool enabled)
    {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        const ImVec2 p = ImGui::GetCursorScreenPos();
        const ImVec2 sz = P(kSidebarW - 24.f, 38.f);

        ImGui::PushID(id);
        bool clicked = ImGui::InvisibleButton("##nav", sz) && enabled;
        const bool hov = ImGui::IsItemHovered() && enabled;
        ImGui::PopID();
        if (hov) ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

        const float a = enabled ? 1.f : 0.40f;
        const float t = Anim(ImGui::GetID(id), active ? 1.f : 0.f, 16.f);

        if (active)
            dl->AddRectFilled(p, P(p.x + sz.x, p.y + sz.y), U32(Alpha(Drawing::colAccent, 0.16f * t)), 8.f);
        else if (hov)
            dl->AddRectFilled(p, P(p.x + sz.x, p.y + sz.y), U32(Alpha(Drawing::colText, 0.05f)), 8.f);

        if (active)
            dl->AddRectFilled(P(p.x, p.y + 9.f), P(p.x + 3.f, p.y + sz.y - 9.f), U32(Drawing::colAccent), 2.f);

        const ImVec4 fg = active ? Drawing::colAccent : (hov ? Drawing::colText : Drawing::colTextDim);
        const ImU32 col = U32(Alpha(fg, a));
        DrawIcon(dl, icon, P(p.x + 24.f, p.y + sz.y * 0.5f), 8.f, col);
        Txt(dl, P(p.x + 46.f, p.y + (sz.y - TxtH()) * 0.5f), col, label);
        return clicked;
    }

    void LanguageSwitch(ImVec2 p, float w)
    {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        const float h = 30.f;
        const float seg = (w - 6.f) * 0.5f;
        const float t = Anim(ImGui::GetID("##lang"), (float)Drawing::nLanguage, 14.f);

        dl->AddRectFilled(p, P(p.x + w, p.y + h), U32(Drawing::colFrameBg), 8.f);
        const float ix = p.x + 3.f + seg * t;
        dl->AddRectFilled(P(ix, p.y + 3.f), P(ix + seg, p.y + h - 3.f), U32(Drawing::colAccent), 6.f);

        const bool lightAccent = Luma(Drawing::colAccent) > 0.65f;
        const ImU32 onCol = lightAccent ? IM_COL32(15, 15, 20, 255) : IM_COL32(255, 255, 255, 255);
        const ImU32 offCol = U32(Drawing::colTextDim);

        const char* names[2] = { "EN", "VI" };
        for (int i = 0; i < 2; ++i)
        {
            ImGui::SetCursorScreenPos(P(p.x + 3.f + seg * i, p.y));
            ImGui::PushID(i);
            if (ImGui::InvisibleButton("##l", P(seg, h))) Drawing::nLanguage = i;
            if (ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
            ImGui::PopID();
            const float sc = 0.92f;
            Txt(dl, P(p.x + 3.f + seg * i + (seg - TxtW(names[i], sc)) * 0.5f, p.y + (h - TxtH(sc)) * 0.5f),
                (Drawing::nLanguage == i) ? onCol : offCol, names[i], sc);
        }
    }

    void BeginCard(const char* title, const char* sub, const char* chip, bool chipOn)
    {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        g_cardPos = ImGui::GetCursorScreenPos();
        g_cardPos.x += kMargin;
        g_cardW = ImGui::GetContentRegionAvail().x - kMargin * 2.f;
        g_innerW = g_cardW - kCardPad * 2.f;

        g_split.Split(dl, 2);
        g_split.SetCurrentChannel(dl, 1);

        ImGui::SetCursorScreenPos(P(g_cardPos.x + kCardPad, g_cardPos.y + kCardPad));
        ImGui::BeginGroup();

        //ImGui::PushStyleColor(ImGuiCol_Text, Drawing::colText);
        //ImGui::SetWindowFontScale(Drawing::fUiScale * 1.1f);
        //ImGui::TextUnformatted(title);
        //ImGui::SetWindowFontScale(Drawing::fUiScale);
        //ImGui::PopStyleColor();

        if (sub)
        {
            //ImGui::PushStyleColor(ImGuiCol_Text, Drawing::colTextDim);
            //ImGui::SetWindowFontScale(Drawing::fUiScale * 0.88f);
            //ImGui::TextUnformatted(sub);
            //ImGui::SetWindowFontScale(Drawing::fUiScale);
            //ImGui::PopStyleColor();
        }

        if (chip)
        {
            //const float sc = 0.82f;
            //const float cw = TxtW(chip, sc) + 28.f, ch = 22.f;
            //const ImVec2 cp = P(g_cardPos.x + g_cardW - kCardPad - cw, g_cardPos.y + kCardPad);
            //const ImVec4 base = chipOn ? ImVec4(0.30f, 0.85f, 0.50f, 1.f) : Drawing::colTextDim;
            //dl->AddRectFilled(cp, P(cp.x + cw, cp.y + ch), U32(Alpha(base, 0.16f)), ch * 0.5f);
            //dl->AddCircleFilled(P(cp.x + 12.f, cp.y + ch * 0.5f), 3.5f, U32(base));
            //Txt(dl, P(cp.x + 22.f, cp.y + (ch - TxtH(sc)) * 0.5f), U32(base), chip, sc);
        }

        //ImGui::Dummy(P(0, 4));
        //const float y = ImGui::GetCursorScreenPos().y;
        //dl->AddLine(P(g_cardPos.x + kCardPad, y), P(g_cardPos.x + g_cardW - kCardPad, y), U32(Drawing::colBorder));
        //ImGui::Dummy(P(0, 4));
    }

    void EndCard()
    {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImGui::EndGroup();
        const float yEnd = ImGui::GetItemRectMax().y + kCardPad;

        g_split.SetCurrentChannel(dl, 0);
        const ImVec2 a = g_cardPos, b = P(g_cardPos.x + g_cardW, yEnd);
        dl->AddRectFilled(a, b, U32(Drawing::colCard), Drawing::fFrameRounding + 4.f);
        dl->AddRect(a, b, U32(Drawing::colBorder), Drawing::fFrameRounding + 4.f);
        g_split.Merge(dl);

        ImGui::SetCursorScreenPos(P(g_cardPos.x - kMargin, yEnd + 2.f));
        ImGui::Dummy(P(0, 0));
    }

    bool SettingRow(const char* label, const char* desc, bool* v, bool last)
    {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        const float h = 54.f;
        const ImVec2 p = ImGui::GetCursorScreenPos();
        const float w = g_innerW;

        ImGui::PushID(label);
        const bool clicked = ImGui::InvisibleButton("##row", P(w, h));
        const bool hov = ImGui::IsItemHovered();
        if (clicked) *v = !*v;
        const ImGuiID tid = ImGui::GetID("##tg");
        ImGui::PopID();
        if (hov) ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

        if (hov)
            dl->AddRectFilled(P(p.x - 6.f, p.y), P(p.x + w + 6.f, p.y + h), U32(Alpha(Drawing::colText, 0.035f)), 8.f);

        Txt(dl, P(p.x, p.y + 9.f), U32(Drawing::colText), label);
        Txt(dl, P(p.x, p.y + 9.f + TxtH() + 4.f), U32(Drawing::colTextDim), desc, 0.86f);

        const float tw = 42.f, th = 22.f;
        const ImVec2 tp = P(p.x + w - tw, p.y + (h - th) * 0.5f);
        const float t = Anim(tid, *v ? 1.f : 0.f, 16.f);
        const ImVec4 off = Mix(Drawing::colFrameBg, Drawing::colText, 0.14f);
        dl->AddRectFilled(tp, P(tp.x + tw, tp.y + th), U32(Mix(off, Drawing::colAccent, t)), th * 0.5f);
        const float kx = tp.x + th * 0.5f + (tw - th) * t;
        dl->AddCircleFilled(P(kx + 0.5f, tp.y + th * 0.5f + 1.f), th * 0.5f - 3.f, IM_COL32(0, 0, 0, 40));   
        dl->AddCircleFilled(P(kx, tp.y + th * 0.5f), th * 0.5f - 3.f, IM_COL32(255, 255, 255, 255));

        if (!last)
        {
            const float ly = p.y + h + ImGui::GetStyle().ItemSpacing.y * 0.5f;
            dl->AddLine(P(p.x, ly), P(p.x + w, ly), U32(Alpha(Drawing::colBorder, 0.7f)));
        }
        return clicked;
    }

    bool ColorRow(const char* label, ImVec4* c)
    {
        const float h = 38.f;
        ImDrawList* dl = ImGui::GetWindowDrawList();
        const ImVec2 p = ImGui::GetCursorScreenPos();
        Txt(dl, P(p.x, p.y + (h - TxtH()) * 0.5f), U32(Drawing::colText), label);

        ImGui::PushID(label);
        ImGui::SetCursorScreenPos(P(p.x + g_innerW - 34.f, p.y + (h - ImGui::GetFrameHeight()) * 0.5f));
        const bool ch = ImGui::ColorEdit3("##c", &c->x,
            ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
        ImGui::PopID();

        ImGui::SetCursorScreenPos(p);
        ImGui::Dummy(P(g_innerW, h));
        return ch;
    }

    bool SliderRow(const char* label, float* v, float lo, float hi, const char* fmt)
    {
        const float h = 38.f;
        ImDrawList* dl = ImGui::GetWindowDrawList();
        const ImVec2 p = ImGui::GetCursorScreenPos();
        Txt(dl, P(p.x, p.y + (h - TxtH()) * 0.5f), U32(Drawing::colText), label);

        ImGui::PushID(label);
        ImGui::SetCursorScreenPos(P(p.x + g_innerW - 200.f, p.y + (h - ImGui::GetFrameHeight()) * 0.5f));
        ImGui::SetNextItemWidth(200.f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, Drawing::fFrameRounding);
        const bool ch = ImGui::SliderFloat("##s", v, lo, hi, fmt);
        ImGui::PopStyleVar();
        ImGui::PopID();

        ImGui::SetCursorScreenPos(p);
        ImGui::Dummy(P(g_innerW, h));
        return ch;
    }
    void StatusLine(const std::string& text)
    {
        if (text.empty()) return;
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, Drawing::colTextDim);
        ImGui::TextWrapped("%s", text.c_str());
        ImGui::PopStyleColor();
    }

    // Compact switch used by the Misc list.
    bool ToggleSwitch(const char* id, bool* v)
    {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        const float tw = 42.f, th = 22.f;
        const ImVec2 p = ImGui::GetCursorScreenPos();

        ImGui::PushID(id);
        const bool clicked = ImGui::InvisibleButton("##tg", P(tw, th));
        const bool hov = ImGui::IsItemHovered();
        const ImGuiID tid = ImGui::GetID("##anim");
        ImGui::PopID();

        if (clicked) *v = !*v;
        if (hov) ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

        const float t = Anim(tid, *v ? 1.f : 0.f, 16.f);
        const ImVec4 off = Mix(Drawing::colFrameBg, Drawing::colText, 0.14f);
        dl->AddRectFilled(p, P(p.x + tw, p.y + th),
            U32(Mix(off, Drawing::colAccent, t)), th * 0.5f);

        const float kx = p.x + th * 0.5f + (tw - th) * t;
        dl->AddCircleFilled(P(kx + 0.5f, p.y + th * 0.5f + 1.f),
            th * 0.5f - 3.f, IM_COL32(0, 0, 0, 40));
        dl->AddCircleFilled(P(kx, p.y + th * 0.5f),
            th * 0.5f - 3.f, IM_COL32(255, 255, 255, 255));

        return clicked;
    }
}




