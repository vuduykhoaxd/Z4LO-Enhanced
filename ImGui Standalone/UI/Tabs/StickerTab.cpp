#include "StickerTab.h"
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
    std::wstring g_stickerInput;
    std::string  g_stickerStatus;
    int          g_stickerMode = 0;      // 0 auto, 1 rembg, 2 keep
    int          g_stickerStroke = 6;
    bool         g_stickerShadow = true;
    int          g_stickerMaxSize = 512;
}

namespace Tabs {
    void RenderStickerTab()
    {
        BeginCard(Drawing::T("sticker_create_title"), Drawing::T("sticker_create_desc"));

        if (ImGui::Button(Drawing::T("choose_image"), P(200.f, 32.f)))
        {
            const std::wstring file = OpenFileDialogW(
                nullptr,
                L"Choose an image to turn into a sticker",
                L"Images (*.png;*.jpg;*.jpeg;*.webp;*.bmp;*.gif)\0*.png;*.jpg;*.jpeg;*.webp;*.bmp;*.gif\0All files (*.*)\0*.*\0"
            );
            if (!file.empty()) {
                g_stickerInput = file;
                g_stickerStatus.clear();
            }
        }
        ImGui::SameLine();
        if (g_stickerInput.empty()) {
            ImGui::TextUnformatted(Drawing::T("no_file"));
        }
        else {
            ImGui::TextUnformatted(WideToUtf8(fs::path(g_stickerInput).filename().wstring()).c_str());
        }

        ImGui::Spacing();
        ImGui::TextUnformatted(Drawing::T("cutout_mode"));
        ImGui::RadioButton(Drawing::T("mode_auto"), &g_stickerMode, 0);
        ImGui::SameLine();
        ImGui::RadioButton(Drawing::T("mode_rembg"), &g_stickerMode, 1);
        ImGui::SameLine();
        ImGui::RadioButton(Drawing::T("mode_keep"), &g_stickerMode, 2);

        ImGui::SetNextItemWidth(220.f);
        ImGui::SliderInt(Drawing::T("border_px"), &g_stickerStroke, 0, 32);

        ImGui::Checkbox(Drawing::T("add_shadow"), &g_stickerShadow);

        ImGui::SetNextItemWidth(220.f);
        ImGui::SliderInt(Drawing::T("max_size"), &g_stickerMaxSize, 128, 1024);

        ImGui::Spacing();
        if (ImGui::Button(Drawing::T("convert_now"), P(210.f, 34.f)))
        {
            if (g_stickerInput.empty()) {
                g_stickerStatus = "Choose an image first.";
            }
            else {
                g_stickerStatus = "Converting... (the window may freeze for a moment)";
                fs::path out;
                ConvertImageToStickerFile(
                    g_stickerInput,
                    g_stickerMode,
                    g_stickerStroke,
                    g_stickerShadow,
                    g_stickerMaxSize,
                    g_stickerStatus,
                    out
                );
            }
        }

        StatusLine(g_stickerStatus);
        EndCard();

        BeginCard(Drawing::T("sticker_lib_title"), Drawing::T("sticker_lib_desc"));
        ImGui::Text(Drawing::T("sticker_count"),
            static_cast<int>(CollectStickerFiles().size()));

        if (ImGui::Button(Drawing::T("open_folder"), P(170.f, 30.f))) {
            OpenStickerFolder();
        }
        ImGui::SameLine();
        if (ImGui::Button(Drawing::T("copy_last"), P(170.f, 30.f))) {
            CopyLastStickerToClipboard(g_stickerStatus);
        }

        if (ImGui::Button(Drawing::T("enable_sticker"), P(210.f, 30.f))) {
            EnableCustomSticker(&g_stickerStatus);
        }
        ImGui::SameLine();
        if (ImGui::Button(Drawing::T("remove_sticker"), P(170.f, 30.f))) {
            RemoveCustomSticker(&g_stickerStatus);
        }

        StatusLine(g_stickerStatus);
        EndCard();
    }
} // namespace Tabs

