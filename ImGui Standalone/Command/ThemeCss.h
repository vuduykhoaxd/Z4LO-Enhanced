#pragma once

#include <string>

// Liquid Glass theme CSS ported from Z4LOEnhanced.
inline const std::string THEME_CSS = R"CSS(

    /* ============================================================
       LIQUID GLASS THEME CHO ZALO PC
       ============================================================ */

    /* ===== 1. BIN TON CC ===== */
    :root, body {
      --lg-bg: #0b1020;
      --lg-bg-2: #0f1630;
      --lg-glass: rgba(30, 41, 70, 0.45);
      --lg-glass-strong: rgba(30, 41, 70, 0.7);
      --lg-border: rgba(140, 170, 255, 0.15);
      --lg-border-strong: rgba(140, 170, 255, 0.3);
      --lg-text: #e6ecff;
      --lg-text-dim: #9aa8c7;
      --lg-accent: #7aa2f7;
      --lg-accent-2: #bb9af7;
      --lg-danger: #f7768e;
      --lg-shadow: 0 8px 32px rgba(0, 0, 0, 0.35);

      --background-primary: #0b1020 !important;
      --background-secondary: #0f1630 !important;
      --text-primary: #e6ecff !important;
      --text-secondary: #9aa8c7 !important;
      --layer-background: rgba(11, 16, 32, 0.8) !important;
      --background-inactiveness-subtle: rgba(140, 170, 255, 0.15) !important;
    }

    /* Nn tng th c gradient nh */
    body {
      background:
        radial-gradient(1200px 600px at 10% -10%, rgba(122, 162, 247, 0.12), transparent 60%),
        radial-gradient(1000px 500px at 110% 110%, rgba(187, 154, 247, 0.10), transparent 60%),
        #0b1020 !important;
      color: #e6ecff !important;
    }

    /* ===== 2. TITLEBAR ===== */
    #titleBar {
      background: rgba(15, 22, 48, 0.55) !important;
      backdrop-filter: blur(18px) saturate(160%) !important;
      -webkit-backdrop-filter: blur(18px) saturate(160%) !important;
      border-bottom: 1px solid rgba(140, 170, 255, 0.15) !important;
      box-shadow: 0 4px 24px rgba(0, 0, 0, 0.25) !important;
    }
    #titleBar .title-name { color: #e6ecff !important; }
    #titleBar .titlebar__menu__btn { color: #9aa8c7 !important; }
    #titleBar .titlebar__menu__btn:hover {
      background: rgba(140, 170, 255, 0.12) !important;
      color: #e6ecff !important;
      border-radius: 6px !important;
    }

    /* ===== 3. SIDEBAR ===== */
    #sidebarNav, #main-tab {
      background: rgba(15, 22, 48, 0.5) !important;
      backdrop-filter: blur(18px) saturate(160%) !important;
      -webkit-backdrop-filter: blur(18px) saturate(160%) !important;
      border-right: 1px solid rgba(140, 170, 255, 0.12) !important;
    }

    /* Avatar ngi dng */
    .nav__tabs__zalo .zavatar .a-child.default-alphabet {
      background-color: rgba(122, 162, 247, 0.25) !important;
      color: #e6ecff !important;
      border: 1px solid rgba(122, 162, 247, 0.4) !important;
    }

    /* Tab sidebar */
    .leftbar-tab {
      color: #9aa8c7 !important;
      border-radius: 10px !important;
      transition: background 0.18s ease, color 0.18s ease !important;
    }
    .leftbar-tab:hover {
      background: rgba(140, 170, 255, 0.1) !important;
      color: #e6ecff !important;
    }
    .leftbar-tab.selected {
      background: rgba(122, 162, 247, 0.18) !important;
      color: #e6ecff !important;
      border: 1px solid rgba(122, 162, 247, 0.35) !important;
      box-shadow: inset 0 0 0 1px rgba(255, 255, 255, 0.03) !important;
    }
    .leftbar-tab .lb-tab-title { color: #9aa8c7 !important; }
    .leftbar-tab.selected .lb-tab-title { color: #e6ecff !important; font-weight: 600 !important; }
    .leftbar-tab .internal-icon { color: inherit !important; }
    .leftbar-tab.selected .internal-icon { color: #7aa2f7 !important; }

    /* Badge cha c */
    .leftbar-unread-badge.z-noti-badge {
      background: #f7768e !important;
      color: #fff !important;
      border: 1px solid rgba(11, 16, 32, 0.8) !important;
      box-shadow: 0 0 12px rgba(247, 118, 142, 0.5) !important;
    }
    .leftbar-unread-badge .z-noti-badge__content { color: #fff !important; }

    /* ===== 4. DANH SCH HI THOI ===== */
    #conversationListId {
      background: rgba(15, 22, 48, 0.35) !important;
      backdrop-filter: blur(12px) !important;
      -webkit-backdrop-filter: blur(12px) !important;
    }
    .conv-item {
      background: transparent !important;
      border-bottom: 1px solid rgba(140, 170, 255, 0.08) !important;
      border-radius: 10px !important;
      margin: 2px 6px !important;
      transition: background 0.15s ease !important;
    }
    .conv-item:hover {
      background: rgba(140, 170, 255, 0.08) !important;
    }
    .conv-item.selected {
      background: rgba(122, 162, 247, 0.15) !important;
      border: 1px solid rgba(122, 162, 247, 0.3) !important;
    }
    .conv-item-title__name { color: #e6ecff !important; }
    .z-conv-message__preview-message { color: #9aa8c7 !important; }
    .preview-time { color: #6b7a99 !important; }

    /*  tm kim */
    #contact-search {
      background: rgba(15, 22, 48, 0.6) !important;
      border-bottom: 1px solid rgba(140, 170, 255, 0.1) !important;
    }
    .group-search {
      background: rgba(140, 170, 255, 0.08) !important;
      border: 1px solid rgba(140, 170, 255, 0.15) !important;
      border-radius: 10px !important;
    }
    .group-search__icon, .fake-textholder { color: #9aa8c7 !important; }
    #contact-search-input { color: #e6ecff !important; }

    /* Thanh tab Tt c / Cha c */
    .msg-filters-bar, .tab-main {
      background: rgba(15, 22, 48, 0.4) !important;
      border-bottom-color: rgba(140, 170, 255, 0.1) !important;
    }
    .tab-item { color: #9aa8c7 !important; }
    .tab-item.selected { color: #e6ecff !important; }

    /* ===== 5. KHUNG CHAT CHNH ===== */
    #chatViewContainer {
      background: transparent !important;
    }
    #chatView {
      background:
        radial-gradient(800px 400px at 20% 0%, rgba(122, 162, 247, 0.06), transparent 70%),
        rgba(11, 16, 32, 0.6) !important;
    }
    #header {
      background: rgba(15, 22, 48, 0.55) !important;
      backdrop-filter: blur(18px) saturate(160%) !important;
      -webkit-backdrop-filter: blur(18px) saturate(160%) !important;
      border-bottom: 1px solid rgba(140, 170, 255, 0.12) !important;
    }
    .header-title { color: #e6ecff !important; }
    .community__chat-box-indicator { color: #9aa8c7 !important; }

    /* ===== 6. BONG BNG TIN NHN ===== */
    .message-frame.card {
      background: rgba(30, 41, 70, 0.55) !important;
      backdrop-filter: blur(14px) saturate(150%) !important;
      -webkit-backdrop-filter: blur(14px) saturate(150%) !important;
      border: 1px solid rgba(140, 170, 255, 0.15) !important;
      border-radius: 14px !important;
      box-shadow: 0 4px 18px rgba(0, 0, 0, 0.25), inset 0 1px 0 rgba(255, 255, 255, 0.05) !important;
    }
    .message-frame.card .text { color: #e6ecff !important; }
    .message-sender-name-content { color: #7aa2f7 !important; font-weight: 600 !important; }
    .card-send-time__sendTime { color: #6b7a99 !important; }
    .message-frame--highlight-from-admin {
      border-color: rgba(187, 154, 247, 0.4) !important;
    }

    /* Ngy thng */
    .chat-date .content { color: #6b7a99 !important; }
    .chat-date .start, .chat-date .end {
      background: rgba(140, 170, 255, 0.2) !important;
    }

    /* ===== 7. KHI TRCH DN (QUOTE) ===== */
    .message-quote-fragment__container,
    .message-quote-fragment__container--white {
      background: rgba(122, 162, 247, 0.1) !important;
      border-left: 3px solid #7aa2f7 !important;
      border-radius: 8px !important;
      padding: 6px 10px !important;
      backdrop-filter: blur(8px) !important;
      -webkit-backdrop-filter: blur(8px) !important;
    }
    .message-quote-fragment__title,
    .quote-name {
      color: #7aa2f7 !important;
      font-weight: 600 !important;
    }
    .message-quote-fragment__description { color: #9aa8c7 !important; }
    .message-quote-fragment__icon,
    .quoteMessage {
      background: rgba(140, 170, 255, 0.12) !important;
      border-radius: 6px !important;
    }
    .message-quote-fragment__icon img { opacity: 0.9 !important; }

    /* ===== 8.  NHP TIN NHN ===== */
    #chatInput, .chat-box-input-container {
      background: rgba(15, 22, 48, 0.55) !important;
      backdrop-filter: blur(18px) saturate(160%) !important;
      -webkit-backdrop-filter: blur(18px) saturate(160%) !important;
      border-top: 1px solid rgba(140, 170, 255, 0.12) !important;
    }
    #richInput {
      color: #e6ecff !important;
      caret-color: #7aa2f7 !important;
    }
    #richInput:empty::before { color: #6b7a99 !important; }
    .chat-box-toolbar-button { color: #9aa8c7 !important; }
    .chat-box-toolbar-button:hover {
      background: rgba(140, 170, 255, 0.1) !important;
      color: #e6ecff !important;
      border-radius: 8px !important;
    }

    /* Nt trn (Thm bn, To nhm) */
    .z--btn--v2.btn-tertiary-neutral {
      background: rgba(140, 170, 255, 0.08) !important;
      color: #9aa8c7 !important;
      border: 1px solid rgba(140, 170, 255, 0.12) !important;
      border-radius: 50% !important;
    }
    .z--btn--v2.btn-tertiary-neutral:hover {
      background: rgba(140, 170, 255, 0.18) !important;
      color: #e6ecff !important;
    }

    /* ===== 9. MENU / TOOLTIP ===== */
    .tooltip,
    .floating-menu-wrapper,
    .MQA-container,
    .MQA-contents {
      background: rgba(20, 28, 52, 0.85) !important;
      backdrop-filter: blur(20px) saturate(160%) !important;
      -webkit-backdrop-filter: blur(20px) saturate(160%) !important;
      border: 1px solid rgba(140, 170, 255, 0.18) !important;
      border-radius: 12px !important;
      color: #e6ecff !important;
      box-shadow: 0 12px 40px rgba(0, 0, 0, 0.45) !important;
    }
    .MSABtn-btn:hover { background: rgba(140, 170, 255, 0.12) !important; }

    /* ===== 10. REACTION BAR ===== */
    .reaction-emoji-list,
    .emoji-list-wrapper {
      background: rgba(20, 28, 52, 0.85) !important;
      backdrop-filter: blur(16px) !important;
      -webkit-backdrop-filter: blur(16px) !important;
      border: 1px solid rgba(140, 170, 255, 0.18) !important;
      border-radius: 12px !important;
    }
    .react-icon-list, .total-reacts { color: #9aa8c7 !important; }

    /* ===== 11. THANH CUN ===== */
    ::-webkit-scrollbar { width: 8px !important; height: 8px !important; }
    ::-webkit-scrollbar-track { background: transparent !important; }
    ::-webkit-scrollbar-thumb {
      background: rgba(140, 170, 255, 0.2) !important;
      border-radius: 8px !important;
    }
    ::-webkit-scrollbar-thumb:hover {
      background: rgba(140, 170, 255, 0.35) !important;
    }
  
)CSS";
