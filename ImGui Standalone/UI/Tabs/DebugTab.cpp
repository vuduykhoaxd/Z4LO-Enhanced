#include "DebugTab.h"
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
    std::string g_debugStatus;
}

namespace Tabs {
    void RenderDebugTab()
    {
        BeginCard(Drawing::T("debug_title"), Drawing::T("debug_desc"));

        ImGui::Text(Drawing::T("zalo_status"),
            IsZaloRunning() ? Drawing::T("running") : Drawing::T("not_running"));

        ImGui::Spacing();
        if (ImGui::Button(Drawing::T("force_stop"), P(170.f, 32.f)))
        {
            ForceStopZalo();
            g_debugStatus = "Zalo stopped.";
        }
        ImGui::SameLine();
        if (ImGui::Button(Drawing::T("open_debug"), P(210.f, 32.f)))
        {
            OpenZaloDebugging();
            g_debugStatus = "Launching Zalo with --remote-debugging-port=9222...";
        }

        ImGui::Spacing();
        if (ImGui::Button(Drawing::T("disable_touch"), P(210.f, 32.f)))
        {
            ExecuteJavaScript(SIMULATE_MOUSE_JS, true);
            ExecuteJavaScript(SIMULATE_TOUCH_JS, true);
            g_debugStatus = "Touch emulation disabled.";
        }
        ImGui::SameLine();
        if (ImGui::Button(Drawing::T("test_cdp"), P(190.f, 32.f)))
        {
            const std::string ws = GetWebSocketUrl();
            g_debugStatus = ws.empty()
                ? "No debuggable Zalo page on port 9222."
                : "CDP connected: " + ws;
        }

        StatusLine(g_debugStatus);
        EndCard();
    }
} // namespace Tabs

