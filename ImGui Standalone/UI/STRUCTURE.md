# Cấu trúc source — Zalo Enhanced (ImGui standalone, D3D11)

Cửa sổ hiển thị là cửa sổ ImGui không khung (borderless, không tiêu đề Win32).
D3D11 vẫn cần một HWND nội bộ để tạo swapchain, nhưng HWND đó không hiện khung
`WS_POPUP` — người dùng chỉ thấy cửa sổ ImGui, kéo bằng vùng header, đóng bằng nút X.

```
ImGui Standalone/
  main.cpp                  Entry: CoInitializeEx → UI::Render()
  UI/
    UI.h / UI.cpp           D3D11 host: tạo HWND ẩn khung, swapchain, vòng loop ImGui+D3D11, font Segoe UI (tiếng Việt)
    Drawing.h / Drawing.cpp Shell: state dùng chung, i18n T(), preset/theme style, sidebar, header, body routing
    Widgets.h / Widgets.cpp Widgets dùng chung: card, row, toggle, nav, logo, status, preset
    Tabs/                   Mỗi tab 1 file, sửa chức năng chỉ mở đúng file của nó
      PrivacyTab.h/.cpp     Privacy: 7 blur + Enable/Disable all
      ThemeTab.h/.cpp       Theme: preset UI + card Apply to Zalo
      StickerTab.h/.cpp     Sticker: convert WebP + thư viện + Enable/Remove
      MiscTab.h/.cpp        Misc: Fetch live config → search → [Tên] ([Giá trị]/[Toggle]) [Apply]
      DebugTab.h/.cpp       Debug: Force stop / Open 9222 / Disable touch / Test CDP
    ImGui/                  Thư viện ImGui (imgui + impl_dx11 + impl_win32)
  Command/
    JavaScript.h            JS payload: BLUR_API, THEME_API, CUSTOM_STICKER, MISC_FETCH/SET
    ThemeCss.h              Liquid Glass CSS
    Sticker.hpp             Engine WIC: ảnh → sticker WebP (PNG+WebP)
  Core/
    ExecuteJS.hpp           CDP: ExecuteJavaScript / EvaluateJavaScript
    Actions.hpp             Adapter từng tab gọi CDP/thư viện: ApplyBlurState, ApplyTheme, EnableSticker, Fetch/SetMisc, ...
  Utils/
    Zalo_Utils.hpp          FindZaloPath / IsZaloRunning / ForceStop / OpenDebugging (9222)
    Http.hpp                HttpGetUrl / Read-Write file / Base64 / JsonEscape
    FileDialog.hpp          Hộp thoại chọn file Win32
    Utils.hpp               Wide/UTF-8, Trim, ToLower
```

## Quy tắc sửa

- Sửa riêng 1 tab → chỉ mở file trong `UI/Tabs/` của tab đó.
- Widgets dùng chung sửa ở `UI/Widgets.*`, không copy vào từng tab.
- Hành động gọi vào Zalo/CDP sửa ở `Core/Actions.hpp`, payload JS sửa ở `Command/JavaScript.h`.
- State của tab đặt trong file `.cpp` của tab đó (anonymous namespace), không gom vào `Drawing.*`.
- Không thêm hàm không-`inline` vào header được nhiều file include (sẽ lỗi LNK2005 khi link) — header dùng `inline` hoặc để vào `.cpp`.

Build: `MSBuild ImGui Standalone.sln /p:Configuration=Release /p:Platform=x64`
