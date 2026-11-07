# Zalo Enhanced — Source Structure (Standalone ImGui, D3D11)

The application uses a frameless, standalone ImGui window with no native Win32 title bar or borders.

D3D11 requires an internal HWND for swapchain creation, configured with `WS_POPUP`. Users only see the ImGui interface, with window dragging handled by the header and closing via the X button.

```text
ImGui Standalone/
  main.cpp                  Entry point: CoInitializeEx → UI::Render()
  UI/
    UI.h / UI.cpp           D3D11 host, frameless HWND, swapchain, ImGui loop, Vietnamese fonts
    Drawing.h / Drawing.cpp Shared UI state, i18n, themes, sidebar, header, body routing
    Widgets.h / Widgets.cpp Reusable components: cards, rows, toggles, navigation, logo, status
    Tabs/
      PrivacyTab.h/.cpp     Privacy: 7 blur options, Enable/Disable All
      ThemeTab.h/.cpp       Theme presets, Apply to Zalo
      StickerTab.h/.cpp     WebP conversion, sticker library, Enable/Remove
      MiscTab.h/.cpp        Fetch live config, search, display values/toggles, Apply
      DebugTab.h/.cpp       Force Stop, Open 9222, Disable Touch, Test CDP
    ImGui/                  ImGui library and Win32/DX11 backends
  Command/
    JavaScript.h            JS payloads: BLUR_API, THEME_API, CUSTOM_STICKER, MISC_FETCH/SET
    ThemeCss.h               Liquid Glass CSS
    Sticker.hpp              WIC-based image-to-WebP engine (PNG + WebP)
  Core/
    ExecuteJS.hpp            CDP: ExecuteJavaScript / EvaluateJavaScript
    Actions.hpp              Feature adapters: ApplyBlurState, ApplyTheme, EnableSticker, Fetch/SetMisc
  Utils/
    Zalo_Utils.hpp           FindZaloPath, IsZaloRunning, ForceStop, OpenDebugging (9222)
    Http.hpp                 HTTP, file I/O, Base64, JsonEscape
    FileDialog.hpp            Native Win32 file picker
    Utils.hpp                 Wide/UTF-8, Trim, ToLower
```

## Development Rules

- **Single-tab changes:** Only edit the corresponding files in `UI/Tabs/`.
- **Shared widgets:** Modify `UI/Widgets.*`; never duplicate widgets across tabs.
- **Zalo/CDP actions:** Modify `Core/Actions.hpp`.
- **JavaScript payloads:** Modify `Command/JavaScript.h`.
- **Tab state:** Keep state inside each tab's `.cpp` using an anonymous namespace. Do not move it into `Drawing.*`.
- **Header safety:** Never define non-`inline` functions in headers included by multiple translation units. Use `inline` or move implementations into `.cpp` files.

## Build

```bash
MSBuild "ImGui Standalone.sln" /p:Configuration=Release /p:Platform=x64
```
