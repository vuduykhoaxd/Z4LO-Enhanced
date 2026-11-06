#include "MiscTab.h"
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
    std::vector<MiscFlag> g_miscFlags;
    bool                  g_miscFetched = false;
    std::string           g_miscStatus;
    char                  g_miscSearch[96] = "";
    int                   g_miscVisible = 80;
}

namespace Tabs {
    void RenderMiscTab()
    {
        BeginCard(Drawing::T("misc_title"), Drawing::T("misc_desc"));

        if (ImGui::Button(Drawing::T("fetch_all"), P(230.f, 34.f)))
        {
            g_miscStatus = "Fetching live config from Zalo...";
            g_miscFlags.clear();
            g_miscFetched = FetchMiscConfig(g_miscFlags, g_miscStatus);
            g_miscVisible = 80;
            g_miscSearch[0] = '\0';
        }
        ImGui::SameLine();

        if (g_miscFetched) {
            ImGui::Text(Drawing::T("misc_count"),
                static_cast<int>(g_miscFlags.size()));
        }
        else {
            ImGui::TextUnformatted(Drawing::T("misc_fetch_hint"));
        }

        StatusLine(g_miscStatus);
        EndCard();

        if (!g_miscFetched) return;

        BeginCard(Drawing::T("misc_list_title"), Drawing::T("misc_list_desc"));

        ImGui::SetNextItemWidth(g_innerW);
        if (ImGui::InputTextWithHint("##misc_search",
            Drawing::T("search_hint"), g_miscSearch, sizeof(g_miscSearch)))
        {
            g_miscVisible = 80;
        }

        std::string needle = g_miscSearch;
        std::transform(needle.begin(), needle.end(), needle.begin(),
            [](unsigned char c) { return static_cast<char>(tolower(c)); });

        std::vector<int> matches;
        matches.reserve(g_miscFlags.size());

        for (int i = 0; i < static_cast<int>(g_miscFlags.size()); ++i)
        {
            if (needle.empty()) { matches.push_back(i); continue; }

            std::string key = g_miscFlags[i].key;
            std::transform(key.begin(), key.end(), key.begin(),
                [](unsigned char c) { return static_cast<char>(tolower(c)); });

            if (key.find(needle) != std::string::npos) matches.push_back(i);
        }

        const int shown = std::min<int>(g_miscVisible, static_cast<int>(matches.size()));
        ImGui::Text(Drawing::T("misc_showing"), shown,
            static_cast<int>(matches.size()));

        ImGui::BeginChild("##misc_list", P(g_innerW, 270.f), true);

        for (int n = 0; n < shown; ++n)
        {
            MiscFlag& flag = g_miscFlags[matches[n]];

            ImGui::PushID(matches[n]);

            // Layout per row: [Name] ([Value] / [Toggle]) [Apply]
            // Toggle / edit only changes the pending value; nothing is
            // sent to Zalo until Apply is pressed.
            ImGui::TextUnformatted(flag.key.c_str());

            if (flag.type == "bool")
            {
                ImGui::SameLine();
                ToggleSwitch("##value", &flag.b);
                flag.dirty = (flag.b != flag.appliedB);
                ImGui::SameLine();
                ImGui::TextDisabled("(%s)", flag.b ? "true" : "false");
                ImGui::SameLine();
                if (ImGui::Button(Drawing::T("apply"), P(64.f, 22.f)))
                {
                    std::string status;
                    if (SetMiscFlag(flag, flag.b ? "true" : "false", status)) {
                        flag.appliedB = flag.b;
                        flag.appliedText = flag.b ? "true" : "false";
                        flag.text = flag.appliedText;
                        flag.dirty = false;
                    }
                    g_miscStatus = status;
                }
            }
            else if (flag.type == "number")
            {
                ImGui::SameLine();
                ImGui::SetNextItemWidth(80.f);
                ImGui::InputDouble("##value", &flag.num, 0.0, 0.0, "%.6g");
                flag.dirty = (flag.num != flag.appliedNum);
                ImGui::SameLine();
                if (ImGui::Button(Drawing::T("apply"), P(64.f, 22.f)))
                {
                    char literal[64];
                    snprintf(literal, sizeof(literal), "%.17g", flag.num);
                    std::string status;
                    if (SetMiscFlag(flag, literal, status)) {
                        flag.appliedNum = flag.num;
                        flag.appliedText = std::string(literal);
                        flag.text = std::string(literal);
                        flag.dirty = false;
                    }
                    g_miscStatus = status;
                }
            }
            else if (flag.type == "string")
            {
                ImGui::SameLine();
                ImGui::SetNextItemWidth(140.f);
                ImGui::InputText("##value", flag.strBuf, sizeof(flag.strBuf));
                flag.dirty = (std::string(flag.strBuf) != flag.appliedText);
                ImGui::SameLine();
                if (ImGui::Button(Drawing::T("apply"), P(64.f, 22.f)))
                {
                    const std::string literal =
                        "\"" + JsonEscapeValue(flag.strBuf) + "\"";
                    std::string status;
                    if (SetMiscFlag(flag, literal, status)) {
                        flag.appliedText = flag.strBuf;
                        flag.text = flag.strBuf;
                        flag.dirty = false;
                    }
                    g_miscStatus = status;
                }
            }
            else
            {
                ImGui::SameLine();
                ImGui::TextDisabled("[%s] = %s", flag.type.c_str(),
                    flag.text.c_str());
                ImGui::SameLine();
                ImGui::BeginDisabled();
                ImGui::Button(Drawing::T("apply"), P(64.f, 22.f));
                ImGui::EndDisabled();
            }

            ImGui::PopID();

            if (n + 1 < shown) ImGui::Separator();
        }

        ImGui::EndChild();

        if (shown < static_cast<int>(matches.size()))
        {
            if (ImGui::Button(Drawing::T("load_more"), P(160.f, 30.f))) {
                g_miscVisible += 80;
            }
        }

        EndCard();
    }
} // namespace Tabs

