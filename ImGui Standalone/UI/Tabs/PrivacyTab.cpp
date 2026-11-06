#include "PrivacyTab.h"
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
    std::string g_privacyStatus;

    bool ApplyPrivacy()
    {
        return ApplyBlurState(
            Drawing::blur_msg,
            Drawing::blur_image,
            Drawing::blur_sticker,
            Drawing::blur_ai_sticker,
            Drawing::blur_url,
            Drawing::blur_embed,
            Drawing::blur_conversation_list,
            &g_privacyStatus
        );
    }
}

namespace Tabs {
    void RenderPrivacyTab()
    {
        auto apply = []() { ApplyPrivacy(); };

        BeginCard(Drawing::T("privacy_title"), Drawing::T("privacy_desc"));

        if (ImGui::Button(Drawing::T("enable_all"), P(150.f, 30.f)))
        {
            Drawing::blur_msg = Drawing::blur_image = Drawing::blur_sticker = true;
            Drawing::blur_ai_sticker = Drawing::blur_url = Drawing::blur_embed = true;
            Drawing::blur_conversation_list = true;
            apply();
        }
        ImGui::SameLine();
        if (ImGui::Button(Drawing::T("disable_all"), P(150.f, 30.f)))
        {
            Drawing::blur_msg = Drawing::blur_image = Drawing::blur_sticker = false;
            Drawing::blur_ai_sticker = Drawing::blur_url = Drawing::blur_embed = false;
            Drawing::blur_conversation_list = false;
            apply();
        }

        ImGui::Spacing();

        bool changed = false;
        changed |= SettingRow(Drawing::T("blur_label"), Drawing::T("blur_label_desc"), &Drawing::blur_msg, false);
        changed |= SettingRow(Drawing::T("blur_image_label"), Drawing::T("blur_image_desc"), &Drawing::blur_image, false);
        changed |= SettingRow(Drawing::T("blur_sticker_label"), Drawing::T("blur_sticker_desc"), &Drawing::blur_sticker, false);
        changed |= SettingRow(Drawing::T("blur_ai_label"), Drawing::T("blur_ai_desc"), &Drawing::blur_ai_sticker, false);
        changed |= SettingRow(Drawing::T("blur_url_label"), Drawing::T("blur_url_desc"), &Drawing::blur_url, false);
        changed |= SettingRow(Drawing::T("blur_embed_label"), Drawing::T("blur_embed_desc"), &Drawing::blur_embed, false);
        changed |= SettingRow(Drawing::T("blur_conversation_list_label"), Drawing::T("blur_conversation_list_desc"), &Drawing::blur_conversation_list, true);

        if (changed) apply();

        StatusLine(g_privacyStatus);
        EndCard();
    }
} // namespace Tabs

