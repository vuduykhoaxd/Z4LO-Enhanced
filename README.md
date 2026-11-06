<<<<<<< HEAD
# Z4LO-Enhanced

Z4LO-Enhanced is a Windows desktop application built with C++ and Dear ImGui.
It provides a custom ImGui interface for enhancing the Zalo desktop experience,
including privacy controls, themes, sticker tools, and developer utilities.

This is an independent, community-driven project for learning and research purposes.
See [DISCLAIMER.md](./DISCLAIMER.md) for the full terms of use.

## Overview

Z4LO-Enhanced is built on top of the ImGui Standalone template.
The host window is created in the same way as the original template:
a small overlapped Win32 window is created hidden, and the visible window
is rendered through ImGui as a viewport. On top of that foundation, this
project adds a full custom UI layer with tab-based navigation.

## Features

- **Privacy tab:** Toggles for blurring messages, images, stickers, links,
  embeds, conversation lists, and notifications.
- **Theme tab:** Preset themes, custom colors, window rounding, spacing,
  and UI scaling.
- **Sticker tab:** Custom sticker tools and WIC-based image conversion
  (COM is initialized in `main.cpp` for this reason).
- **Misc tab:** Miscellaneous application settings.
- **Debug tab:** Diagnostics and developer options.
- **Custom UI layer:** Sidebar layout, custom header with close button,
  dark theme system, and Vietnamese text support via Segoe UI with
  Extended Latin glyph ranges.
- **Dual language UI:** English and Vietnamese strings.

## Project Structure

```text
.
├── ImGui Standalone.sln           Visual Studio solution
├── ImGui Standalone/
│   ├── main.cpp                   Entry point (wWinMain, COM init)
│   ├── UI/
│   │   ├── UI.cpp / UI.h          Window creation and D3D11 render loop
│   │   ├── Drawing.cpp / Drawing.h Main window drawing and theming
│   │   ├── Widgets.cpp / Widgets.h  Custom widgets
│   │   ├── Tabs/                  Tab implementations (Privacy, Theme,
│   │   │                            Sticker, Misc, Debug)
│   │   └── ImGui/                 Dear ImGui core and backends
│   ├── Command/                   Feature commands (sticker, theme CSS, JS)
│   ├── Core/                      Action execution helpers
│   └── Utils/                     File dialog, HTTP, and Zalo utilities
├── zRE/                           Local analysis notes and extracted data
├── DISCLAIMER.md                  Disclaimer and terms of use
├── LICENSE                        License (free for download and modification)
└── README.md                      This file
```

## Requirements

- Windows 10 or later (Windows SDK 10.0)
- Visual Studio 2022 with the Desktop development with C++ workload
  (MSBuild toolset `v143`)
- DirectX 11 SDK / system DirectX 11 runtime (`d3d11.lib`)
- Segoe UI font (included with Windows; used for Vietnamese glyph coverage)

## Build Instructions

1. Clone the repository.
2. Open `ImGui Standalone.sln` in Visual Studio 2022.
3. Select configuration `Release` and platform `x64` (recommended).
4. Build the solution (`Build > Build Solution` or `Ctrl+Shift+B`).

The output is a Windows GUI application. Run the resulting executable
to launch the ImGui interface.

## Usage

- `END` key: Close the application.
- The application window can be moved and resized through the viewport window.
- Use the tabs in the sidebar to switch between Privacy, Theme, Sticker,
  Misc, and Debug sections.

## License

This project is released under the MIT License. See [LICENSE](./LICENSE)
for details. You are free to download, copy, modify, and distribute the
source code, subject to the terms and conditions of that license.

Third-party components such as Dear ImGui retain their own licenses.
Refer to the files under `ImGui Standalone/UI/ImGui/` for details.

## Trademark Notice

Zalo is a trademark of its respective owner. This project is not affiliated
with, endorsed by, or sponsored by Zalo, VNG, or any related company.

## Contributing

Contributions are welcome. Please open an issue or submit a pull request
with a clear description of the change. Keep code style consistent with
surrounding files.

## Contact

Maintainer: vuduykhoaxd
Repository: https://github.com/vuduykhoaxd/Z4LO-Enhanced
=======
# Z4LO Enhanced
>>>>>>> f782e7d07441ec0ed8ca72eabe1791a8c8c7eb1a
