#pragma once

#include <string>

// JavaScript payloads ported from Z4LOEnhanced (console) to keep the
// ImGui build feature-identical: blur API, theme API, custom sticker UI.
// Debug tab: raw CDP commands (passed with rawCdp=true).
inline const std::string SIMULATE_MOUSE_JS = R"JS({
    "id": 6,
    "method": "Emulation.setEmitTouchEventsForMouse",
    "params": { "enabled": false }
})JS";

inline const std::string SIMULATE_TOUCH_JS = R"JS({
    "id": 2,
    "method": "Emulation.setTouchEmulationEnabled",
    "params": { "enabled": false }
})JS";

inline const std::string BLUR_API_JS = R"JS(
(function () {
    const STYLE_ID = "zalo-blur-style";
    const STATE_KEY = "zalo-blur-state-v2";
    const BLUR_PX = 7;

    const MESSAGE_SELECTORS = [
        ".message-content-render .text-message__container",
        ".message-content-render .message-quote-fragment__container",
        ".message-content-render .message-quote-fragment__content-container"
    ];

    const IMAGE_SELECTORS = [
        ".message-content-render .chatImageMessage",
        ".message-content-render .chatImageMessage--audit",
        ".message-content-render .img-msg-v2",
        ".message-content-render .img-msg-v2__bub",
        ".message-content-render .img-msg-v2__th",
        ".message-content-render .img-msg-v2__hdb",
        ".message-content-render .ci-th__thb",
        ".message-content-render .ci-th__thumb",
        ".message-content-render .ci-th__tiny",
        ".message-content-render .ci-th__up",
        ".message-content-render .image-box",
        ".message-content-render .zimg-el",
        ".message-content-render .img-center-box",
        ".message-content-render .photo-message-v2",
        ".message-content-render .loading-image__embeded-image"
    ];

    const STICKER_SELECTORS = [
        ".message-content-render .sticker-group-message-wrapper",
        ".message-content-render .sticker-message-content-wrapper",
        ".message-content-render .normal-sticker-message-content",
        ".message-content-render .card--sticker--container",
        ".message-content-render .sticker-set",
        ".message-content-render .sticker",
        ".message-content-render .photo-sticker",
        ".message-content-render .photo-sticker__container",
        ".message-content-render .photo-sticker__img",
        ".message-content-render .photo-sticker-message",
        ".message-content-render .photo-sticker-message__content",
        ".message-content-render .photo-sticker-message__st",
        ".message-content-render .no-border-photo-sticker-message-content-wrapper",
        ".message-content-render .no-border-photo-sticker-message-content",
        ".message-content-render .no-border-photo-sticker-message-content__image"
    ];

    const AI_STICKER_SELECTORS = [
        ".message-content-render .ai-sticker-message",
        ".message-content-render .ai-sticker-message__content",
        ".message-content-render .ai-sticker-message-content-wrapper",
        ".message-content-render .ai-sticker-message-content",
        ".message-content-render .ai-sticker",
        ".message-content-render .ai-sticker__view",
        ".message-content-render .default-ai-sticker-view",
        ".message-content-render .ai-sticker-badge"
    ];

    const URL_SELECTORS = [
        ".message-content-render .link-message",
        ".message-content-render .link-message__text-wrapper",
        ".message-content-render .link-message__preview-link",
        ".message-content-render .link-message__link-content",
        ".message-content-render .link-message__link-title",
        ".message-content-render .link-message__link-source",
        ".message-content-render .link-message__link-description",
        ".message-content-render .link-msg",
        ".message-content-render .link-message-v2",
        ".message-content-render .card--link",
        ".message-content-render .card--link-container",
        ".message-content-render .card--link-img",
        ".message-content-render .text-is-link"
    ];

    const EMBED_SELECTORS = [
        ".message-content-render .image-background",
        ".message-content-render .image-background__embed-img",
        ".message-content-render .embed-message",
        ".message-content-render .message-embed",
        ".message-content-render .z-embed",
        ".message-content-render .video-message",
        ".message-content-render .video-message__wrapper",
        ".message-content-render .card--oa",
        ".message-content-render .oa-message",
        ".message-content-render iframe",
        ".message-content-render video"
    ];

    const CONVERSATION_ROOTS = [
        ".conv-list-semi-compact",
        ".conv-list-compact"
    ];

    const CONVERSATION_TARGETS = [
        ".conv-item-title__name",
        ".z-conv-message",
        ".z-conv-message__preview-message",
        ".z-conv-message__preview-sender-name",
        ".conversationList__avatar",
        ".conv-item__avatar",
        ".conv-status",
        ".conv__mute",
        ".conv-unread-react",
        ".conv-unread-react__sender-name"
    ];

    const GROUPS = {
        message: MESSAGE_SELECTORS,
        image: IMAGE_SELECTORS,
        sticker: STICKER_SELECTORS,
        aiSticker: AI_STICKER_SELECTORS,
        url: URL_SELECTORS,
        embed: EMBED_SELECTORS
    };

    const DEFAULT_STATE = {
        message: false,
        image: false,
        sticker: false,
        aiSticker: false,
        url: false,
        embed: false,
        conversationList: false
    };

    function loadState() {
        const state = Object.assign({}, DEFAULT_STATE);

        try {
            const raw = localStorage.getItem(STATE_KEY);

            if (raw) {
                const saved = JSON.parse(raw);

                Object.keys(DEFAULT_STATE).forEach(function (key) {
                    if (typeof saved[key] === "boolean") {
                        state[key] = saved[key];
                    }
                });
            }
        } catch (error) {
        }

        return state;
    }

    let state = loadState();

    function saveState() {
        try {
            localStorage.setItem(STATE_KEY, JSON.stringify(state));
        } catch (error) {
        }
    }

    function buildRules(selectors, hoverSelectors) {
        const rules = [];

        rules.push(
            selectors.join(",\n") +
            " {\n" +
            "    filter: blur(" + BLUR_PX + "px) !important;\n" +
            "    transition: filter 0.2s ease !important;\n" +
            "}"
        );

        if (hoverSelectors.length > 0) {
            rules.push(
                hoverSelectors.join(",\n") +
                " {\n" +
                "    filter: blur(0) !important;\n" +
                "}"
            );
        }

        return rules;
    }

    function buildConversationSelectors(hover) {
        const selectors = [];

        CONVERSATION_ROOTS.forEach(function (root) {
            CONVERSATION_TARGETS.forEach(function (target) {
                selectors.push(
                    hover
                        ? root + " .conv-item:hover " + target
                        : root + " " + target
                );
            });
        });

        return selectors;
    }

    function applyBlur() {
        let rules = [];

        Object.keys(GROUPS).forEach(function (key) {
            if (!state[key]) {
                return;
            }

            const selectors = GROUPS[key];

            const hoverSelectors = selectors.map(function (selector) {
                return selector.replace(
                    ".message-content-render ",
                    ".message-content-render:hover "
                );
            });

            rules = rules.concat(
                buildRules(selectors, hoverSelectors)
            );
        });

        if (state.conversationList) {
            rules = rules.concat(
                buildRules(
                    buildConversationSelectors(false),
                    buildConversationSelectors(true)
                )
            );
        }

        let style = document.getElementById(STYLE_ID);

        if (rules.length === 0) {
            if (style) {
                style.remove();
            }

            saveState();
            return state;
        }

        if (!style) {
            style = document.createElement("style");
            style.id = STYLE_ID;
            (document.head || document.documentElement).appendChild(style);
        }

        style.textContent = rules.join("\n\n");
        saveState();

        return state;
    }

    window.blur_msg = function (message, image, sticker, aiSticker, url, embed) {
        const keys = ["message", "image", "sticker", "aiSticker", "url", "embed"];
        const values = [message, image, sticker, aiSticker, url, embed];

        if (arguments.length === 0) {
            keys.forEach(function (key) {
                state[key] = true;
            });
        } else {
            keys.forEach(function (key, index) {
                if (values[index] !== undefined && values[index] !== null) {
                    state[key] = Boolean(values[index]);
                }
            });
        }

        return JSON.stringify(applyBlur());
    };

    window.blur_conversation_list = function (enabled) {
        if (enabled === undefined || enabled === null) {
            state.conversationList = !state.conversationList;
        } else {
            state.conversationList = Boolean(enabled);
        }

        return JSON.stringify(applyBlur());
    };

    window.toggle_blur = function (key) {
        if (key === "conversationList") {
            return window.blur_conversation_list();
        }

        if (Object.prototype.hasOwnProperty.call(state, key)) {
            state[key] = !state[key];
        }

        return JSON.stringify(applyBlur());
    };

    window.zalo_blur_state = function () {
        return JSON.stringify(state);
    };

    window.disable_blur = function () {
        Object.keys(DEFAULT_STATE).forEach(function (key) {
            state[key] = false;
        });

        const style = document.getElementById(STYLE_ID);

        if (style) {
            style.remove();
        }

        saveState();

        return JSON.stringify(state);
    };

    // Called by the ImGui app with the complete UI state. Applying the full
    // object avoids drift between the UI and Zalo's localStorage.
    window.__z4applyState = function (next) {
        Object.keys(DEFAULT_STATE).forEach(function (key) {
            if (next && typeof next[key] === "boolean") {
                state[key] = next[key];
            }
        });

        if (next && typeof next.conversationList === "boolean") {
            state.conversationList = next.conversationList;
        }

        return JSON.stringify(applyBlur());
    };

    // No auto-apply on injection: the Privacy tab starts with every toggle
    // off, and only touches Zalo when the user flips a switch.

    return "BLUR_API_READY";
})();
)JS";

inline const std::string THEME_API_JS = R"JS(
(function () {
    window.apply_theme = function (css, styleId) {
        const id = styleId || "custom-zalo-theme";

        if (!css) {
            return "THEME_EMPTY";
        }

        const old = document.getElementById(id);

        if (old) {
            old.remove();
        }

        const style = document.createElement("style");
        style.id = id;
        style.textContent = css;
        (document.head || document.documentElement).appendChild(style);

        return "THEME_APPLIED";
    };

    window.apply_theme_base64 = function (encoded, styleId) {
        if (!encoded) {
            return "THEME_EMPTY";
        }

        const binary = atob(encoded);
        const bytes = new Uint8Array(binary.length);

        for (let i = 0; i < binary.length; i++) {
            bytes[i] = binary.charCodeAt(i);
        }

        const css = new TextDecoder("utf-8").decode(bytes);

        return window.apply_theme(css, styleId);
    };

    window.remove_theme = function (styleId) {
        const id = styleId || "custom-zalo-theme";
        const style = document.getElementById(id);

        if (!style) {
            return "THEME_NOT_FOUND";
        }

        style.remove();

        return "THEME_REMOVED";
    };
})();
)JS";

inline const std::string CUSTOM_STICKER_JS = R"JS(
    (function () {
    var STYLE_ID = "z4-custom-style";
    var PANEL_ID = "z4-custom-panel";
    var TAB_ID = "z4-custom-tab";

    // Sticker list is injected by the C++ side:
    // [{ name, path, url }] - url is a file:// thumbnail URL.
    if (!window.__z4customStickers) window.__z4customStickers = [];

    // Refresh hook called by the C++ side after updating the list.
    window.__z4customPanelRefresh = function () {
        var panel = document.getElementById(PANEL_ID);
        var tab = document.getElementById(TAB_ID);
        if (panel) panel.remove();
        if (tab) tab.remove();
        // MutationObserver re-injects tab + panel with fresh content.
        return "REFRESHED";
    };

    window.__z4customInstalled = true;

    function fileUrlFromPath(p) {
        // p like "C:\Users\...\sticker_xxx.png"
        var parts = p.split("\\");
        var drive = parts.length > 0 ? parts[0] : "";
        var rest = parts.slice(1).map(encodeURIComponent).join("/");
        return "file:///" + drive + (rest ? "/" + rest : "");
    }

    function ensureStyle() {
        if (document.getElementById(STYLE_ID)) return;
        var s = document.createElement("style");
        s.id = STYLE_ID;
        s.textContent =
            "#z4-custom-tab{cursor:pointer;display:inline-flex;flex-direction:column;align-items:center;gap:2px;padding:4px 8px;border-radius:8px;color:var(--text-secondary,#9aa8c7);}" +
            "#z4-custom-tab:hover,#z4-custom-tab.z4-active{background:var(--background-inactiveness-subtle,rgba(255,255,255,.08));color:var(--text-primary,#fff);}" +
            "#z4-custom-tab .z4-tab-label{font-size:10px;line-height:1;white-space:nowrap;}" +
            "#" + PANEL_ID + "{position:absolute;left:0;right:0;top:0;bottom:0;z-index:20;background:var(--layer-background,#111);overflow:auto;padding:10px;box-sizing:border-box;}" +
            ".z4-grid{display:grid;grid-template-columns:repeat(4,1fr);gap:10px;}" +
            ".z4-item{width:65px;height:65px;border-radius:10px;display:flex;align-items:center;justify-content:center;cursor:pointer;background:rgba(255,255,255,.04);overflow:hidden;position:relative;}" +
            ".z4-item:hover{background:rgba(255,255,255,.12);}" +
            ".z4-item img{width:57px;height:57px;object-fit:contain;pointer-events:none;}" +
            ".z4-plus{font-size:28px;color:var(--text-secondary,#9aa8c7);line-height:1;}" +
            ".z4-hint{color:var(--text-secondary,#9aa8c7);font-size:12px;line-height:1.5;margin-top:10px;}" +
            ".z4-head{display:flex;align-items:center;gap:8px;margin-bottom:10px;}" +
            ".z4-head .z4-title{font-weight:600;color:var(--text-primary,#fff);}" +
            ".z4-head .z4-sub{color:var(--text-secondary,#9aa8c7);font-size:11px;}" +
            ".z4-badge{margin-left:auto;font-size:10px;padding:2px 6px;border-radius:99px;background:rgba(122,162,247,.18);color:#bcd0ff;}";
        (document.head || document.documentElement).appendChild(s);
    }

    function getController() {
        try {
            var input = document.querySelector(".rich-input") ||
                        document.querySelector('[contenteditable="true"]');
            if (!input) return null;
            var ikey = Object.keys(input).find(function (k) {
                return k.indexOf("__reactInternalInstance") === 0;
            });
            if (!ikey) return null;
            var cur = input[ikey], guard = 0;
            while (cur && guard < 60) {
                if (cur.stateNode && cur.stateNode.chatController) {
                    return cur.stateNode.chatController;
                }
                cur = cur.return;
                guard++;
            }
        } catch (e) {}
        return null;
    }

    function toast(msg) {
        try {
            var el = document.createElement("div");
            el.textContent = msg;
            el.style.cssText = "position:fixed;left:50%;bottom:96px;transform:translateX(-50%);" +
                "background:#1b233d;color:#e6ecff;padding:8px 14px;border-radius:10px;" +
                "border:1px solid rgba(140,170,255,.25);z-index:99999;font-size:13px;" +
                "box-shadow:0 8px 32px rgba(0,0,0,.4);max-width:80%;";
            document.body.appendChild(el);
            setTimeout(function () { el.remove(); }, 3200);
        } catch (e) {}
    }

    window.__z4customSend = async function (pathStr, name) {
        // Real custom sticker (syncs to phone):
        // 1) Upload WebP as FILE to get image/webp CDN URL
        // 2) _sendPhotoSticker with that URL (transparent bubble)
        var cc = getController();
        if (!cc) { toast("No chat controller. Open a chat first."); return "NO_CONTROLLER"; }
        try {
            toast("Uploading: " + (name || pathStr));
            if (cc.onCloseStickerPopup) { try { cc.onCloseStickerPopup(); } catch (e) {} }
            else if (cc.closePopup) { try { cc.closePopup(); } catch (e) {} }

            var fwdPath = pathStr.replace(/\\/g, "/");
            var fileName = fwdPath.split("/").pop() || "z4_custom.webp";
            var isWebp = /\.webp$/i.test(fileName);
            // Fallback: if PNG given but WebP exists, use WebP.
            if (!isWebp) {
                var webpPath = fwdPath.replace(/\.(png|jpe?g)$/i, ".webp");
                try {
                    var probe = await window["$zFileManager"].svcStatAsync(webpPath);
                    if (probe && probe.size > 0) {
                        fwdPath = webpPath;
                        fileName = fileName.replace(/\.(png|jpe?g)$/i, ".webp");
                        isWebp = true;
                    }
                } catch (e) {}
            }
            var st = await window["$zFileManager"].svcStatAsync(fwdPath);
            var size = st && st.size ? st.size : 0;

            var conv = cc._getConversationInfo();
            var beforeLen = cc._eventStore.getState().conversations.alls[cc.convId].messages.length;
            var fileObj = {path: fwdPath, name: fileName, size: size, needReadBlob: true};
            await cc._uploadDragFile(fileObj, {targetConv: conv, isPasted: true, _mouseOverChatInput: true});
            setTimeout(function () {
                try {
                    var btn = document.querySelector(".send-msg-btn");
                    if (btn) btn.click();
                } catch (e) {}
            }, 1200);

            // Poll for the uploaded file message to get its CDN href.
            var href = null, w = 0, h = 0;
            var t0 = Date.now();
            while (Date.now() - t0 < 20000) {
                var msgs = cc._eventStore.getState().conversations.alls[cc.convId].messages;
                if (msgs.length > beforeLen) {
                    var last = msgs[msgs.length - 1];
                    if (last && last.message && last.message.href) {
                        href = last.message.href;
                        break;
                    }
                }
                await new Promise(function (r) { setTimeout(r, 500); });
            }
            if (!href) { toast("Upload timeout, sticker not sent"); return "TIMEOUT"; }

            // Get dimensions from local file.
            var imgUrl = fileUrlFromPath(fwdPath);
            var dims = await new Promise(function (resolve) {
                var img = new Image();
                img.onload = function () { resolve({w: img.naturalWidth || 257, h: img.naturalHeight || 257}); };
                img.onerror = function () { resolve({w: 257, h: 257}); };
                img.src = imgUrl;
            });

            var customId = "z4_custom_" + Date.now();
            cc._sendPhotoSticker({
                id: customId, url: href, pngUrl: href, previewUrl: href,
                width: dims.w, height: dims.h, pngWidth: dims.w, pngHeight: dims.h
            });
            toast("Sticker sent (sync to phone)");
            return "SENT:" + customId;
        } catch (err) {
            toast("Send error: " + err.message);
            return "ERR " + err.message;
        }
    };

    function buildPanel() {
        var panel = document.getElementById(PANEL_ID);
        if (!panel) {
            panel = document.createElement("div");
            panel.id = PANEL_ID;
            panel.style.display = "none";
        }

        var list = window.__z4customStickers || [];
        var html = '<div class="z4-head"><span class="z4-title">Custom</span>' +
            '<span class="z4-sub">Z4LOEnhanced</span>' +
            '<span class="z4-badge">' + list.length + ' sticker' + (list.length === 1 ? '' : 's') + '</span></div>' +
            '<div class="z4-grid">';

        // "+" tile: creation happens in the ImGui app (Custom Sticker tab).
        html += '<div class="z4-item" data-z4-action="plus" title="Create a sticker in the app, then click Enable Custom Sticker again">' +
            '<span class="z4-plus">+</span></div>';

        for (var i = 0; i < list.length; i++) {
            var st = list[i];
            var url = st.url || fileUrlFromPath(st.path);
            html += '<div class="z4-item" data-z4-path="' + String(st.path).replace(/"/g, '&quot;') + '"' +
                ' title="' + String(st.name || st.path).replace(/"/g, '&quot;') + '">' +
                '<img draggable="false" src="' + url.replace(/"/g, '&quot;') + '"></div>';
        }

        html += '</div><div class="z4-hint">Click a sticker to send it in the open chat. ' +
            'New stickers: convert them in the app, then click Enable Custom Sticker again to refresh this list.</div>';
        panel.innerHTML = html;
        return panel;
    }

    function inject() {
        if (window.__z4customDisabled) return;

        ensureStyle();

        var selector = document.getElementById("stickerSelector");
        if (!selector) return;

        // 1) Tab button in the bottom menu strip.
        var bar = selector.querySelector(".sticker-selector__menu__scrollbar");
        if (bar && !document.getElementById(TAB_ID)) {
            var tab = document.createElement("span");
            tab.id = TAB_ID;
            tab.innerHTML = '<i class="fa fa-diamond" style="font-size:var(--f18);"></i>' +
                            '<span class="z4-tab-label">Custom</span>';
            tab.addEventListener("click", function (ev) {
                ev.stopPropagation();
                var panel = document.getElementById(PANEL_ID);
                if (!panel) {
                    panel = buildPanel();
                    selector.appendChild(panel);
                    wirePanel(panel);
                }
                var show = panel.style.display === "none";
                panel.style.display = show ? "block" : "none";
                tab.classList.toggle("z4-active", show);
            });
            bar.appendChild(tab);
        }

        // 2) Hidden panel inside the popup (positioned absolute).
        if (!document.getElementById(PANEL_ID)) {
            var panel = buildPanel();
            selector.appendChild(panel);
            wirePanel(panel);
        }
    }

    function wirePanel(panel) {
        if (panel.__z4wired) return;
        panel.__z4wired = true;
        panel.addEventListener("click", function (ev) {
            var item = ev.target.closest(".z4-item");
            if (!item) return;
            if (item.getAttribute("data-z4-action") === "plus") {
                toast("Create a sticker in the app, then click Enable again.");
                return;
            }
            var path = item.getAttribute("data-z4-path");
            var title = item.getAttribute("title") || path;
            if (path) window.__z4customSend(path, title);
        });
    }

    // (Re)inject whenever the sticker popup is opened or re-rendered.
    // React wipes our nodes, so re-add them on demand.
    if (!window.__z4customObserver || !window.__z4customObserver.observe) {
        var obs = new MutationObserver(function () {
            inject();
        });
        obs.observe(document.body, { childList: true, subtree: true });
        window.__z4customObserver = obs;
    }

    inject();

    return "CUSTOM_STICKER_READY:" + (window.__z4customStickers || []).length;
})();
)JS";

// ================================================================
//  Misc tab: live Zalo config (webpack module NDmK)
//  The require function is installed by pushing a tiny module into
//  webpackJsonp (same trick verified against Zalo PC 26.10.20).
// ================================================================
inline const std::string MISC_API_JS = R"JS(
(function () {
    window.__z4ensureCfg = function () {
        if (window.__z4cfg) return window.__z4cfg;

        if (!window.__zreq) {
            try {
                var chunkId = [Math.floor(Math.random() * 1e6) + 900000];
                var moduleId = "__z4_cfg_" + Math.random().toString(36).slice(2);
                var modules = {};
                modules[moduleId] = function (module, exports, __webpack_require__) {
                    window.__zreq = __webpack_require__;
                };
                window.webpackJsonp.push([chunkId, modules, [[moduleId]]]);
            } catch (e) {
                return null;
            }
        }

        if (!window.__zreq || !window.__zreq.m) return null;

        var cfgId = null;
        for (var id in window.__zreq.m) {
            var src = "";
            try { src = window.__zreq.m[id].toString(); } catch (e) { continue; }
            if (src.indexOf("enable_guggy:0") !== -1) { cfgId = id; break; }
        }

        if (cfgId === null) return null;

        try {
            var mod = window.__zreq(cfgId);
            window.__z4cfg = (mod && mod.default) ? mod.default : mod;
            return window.__z4cfg;
        } catch (e) {
            return null;
        }
    };

    return "MISC_API_READY";
})();
)JS";

inline const std::string MISC_FETCH_JS = MISC_API_JS + R"JS(
(function () {
    var cfg = window.__z4ensureCfg && window.__z4ensureCfg();
    if (!cfg) return "ERR\tCannot load Zalo config (webpackJsonp unavailable)";

    var lines = ["OK"];
    for (var key in cfg) {
        var v = cfg[key];
        var t = typeof v;
        if (t === "boolean" || t === "number") {
            lines.push(key + "\t" + t + "\t" + String(v));
        } else if (t === "string") {
            lines.push(key + "\tstring\t" + String(v).replace(/[\t\r\n]+/g, " ").slice(0, 300));
        } else if (Array.isArray(v)) {
            lines.push(key + "\tarray\t" + v.join(",").replace(/[\t\r\n]+/g, " ").slice(0, 300));
        }
    }
    return lines.join("\n");
})();
)JS";

// __Z4KEY__ and __Z4VALUE__ are replaced by the C++ side.
inline const std::string MISC_SET_JS = MISC_API_JS + R"JS(
(function () {
    var cfg = window.__z4ensureCfg && window.__z4ensureCfg();
    if (!cfg) return "ERR\tCannot load Zalo config";

    var key = "__Z4KEY__";
    if (!(key in cfg)) return "ERR\tKey not found";

    cfg[key] = __Z4VALUE__;
    return "OK\t" + String(cfg[key]);
})();
)JS";
