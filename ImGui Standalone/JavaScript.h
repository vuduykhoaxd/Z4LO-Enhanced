#pragma once

std::string SIMULATETOUCH_JS = R"({
  "id": 1001,
  "method": "Emulation.setTouchEmulationEnabled",
  "params": {
    "enabled": false
  }
})";
std::string command =
R"({"id":2,"method":"Emulation.setTouchEmulationEnabled","params":{"enabled":false}})";
const string BLUR_JS = R"JS(
(function () {
    const STYLE_ID = "zalo-blur-style";

    const css = `
        .message-frame.card .text-message__container,
        .message-frame.card .message-quote-fragment__container,
        .message-frame.card .sticker-group-message-wrapper,
        .message-frame.card .no-border-photo-sticker-message-content,
        .message-frame.card .card--sticker--container {
            filter: blur(6px) !important;
            transition: filter 0.2s ease !important;
        }

        .message-frame.card:hover .text-message__container,
        .message-frame.card:hover .message-quote-fragment__container,
        .message-frame.card:hover .sticker-group-message-wrapper,
        .message-frame.card:hover .no-border-photo-sticker-message-content,
        .message-frame.card:hover .card--sticker--container {
            filter: blur(0) !important;
        }

        .message-frame.card .zavatar,
        .message-frame.card .message-sender-name-wrapper {
            filter: none !important;
        }
    `;

    let style = document.getElementById(STYLE_ID);

    if (!style) {
        style = document.createElement("style");
        style.id = STYLE_ID;
        document.head.appendChild(style);
    }

    style.textContent = css;

    return "BLUR_ENABLED";
})();
)JS";

const string DISABLE_BLUR_JS = R"JS(
(function () {
    const style = document.getElementById("zalo-blur-style");

    if (style) {
        style.remove();
    }

    return "BLUR_DISABLED";
})();
)JS";
