#pragma once

// ================================================================
//  Image -> Sticker engine (WIC, pure Win32)
//  Ported from Z4LOEnhanced so the ImGui build can convert existing
//  images into transparent WebP stickers and send them to Zalo.
// ================================================================

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>
#include <objbase.h>
#include <shellapi.h>
#include <wincodec.h>
#include <wrl/client.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>
#include <sstream>

#include "../Utils/Utils.hpp"

#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "shell32.lib")

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

namespace fs = std::filesystem;
using namespace std;

inline fs::path GetAppDirectory() {
    wchar_t buffer[MAX_PATH]{};
    DWORD length = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
    if (length == 0 || length >= MAX_PATH) {
        return fs::current_path();
    }
    return fs::path(buffer).parent_path();
}
// ================================================================
//  Image -> Sticker
//  Turn an existing picture into a sticker (cutout + outline +
//  shadow) and store it outside Zalo.
// ================================================================

struct RgbaImage {
    UINT width = 0;
    UINT height = 0;
    std::vector<unsigned char> pixels; // straight RGBA, 4 bytes per pixel
};

inline fs::path g_lastStickerPath;

inline fs::path GetStickerDirectory() {
    PWSTR raw = nullptr;

    if (SUCCEEDED(SHGetKnownFolderPath(
        FOLDERID_RoamingAppData, 0, nullptr, &raw))) {
        fs::path dir(raw);
        CoTaskMemFree(raw);

        dir /= L"Z4LOEnhanced";
        dir /= L"stickers";

        std::error_code ec;
        fs::create_directories(dir, ec);

        return dir;
    }

    fs::path fallback = GetAppDirectory() / L"stickers";
    std::error_code ec;
    fs::create_directories(fallback, ec);

    return fallback;
}

inline bool StickerLoadImage(const fs::path& path, RgbaImage& image) {
    using Microsoft::WRL::ComPtr;

    ComPtr<IWICImagingFactory> factory;
    HRESULT hr = CoCreateInstance(
        CLSID_WICImagingFactory,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(factory.GetAddressOf())
    );

    if (FAILED(hr)) {
        cout << "WIC factory failed: 0x" << hex << hr << dec << "\n";
        return false;
    }

    ComPtr<IWICBitmapDecoder> decoder;

    hr = factory->CreateDecoderFromFilename(
        path.c_str(),
        nullptr,
        GENERIC_READ,
        WICDecodeMetadataCacheOnDemand,
        decoder.GetAddressOf()
    );

    if (FAILED(hr)) {
        cout << "Cannot decode image (unsupported format?).\n";
        return false;
    }

    ComPtr<IWICBitmapFrameDecode> frame;

    if (FAILED(decoder->GetFrame(0, frame.GetAddressOf()))) {
        cout << "Cannot read the first frame.\n";
        return false;
    }

    ComPtr<IWICFormatConverter> converter;

    if (FAILED(factory->CreateFormatConverter(converter.GetAddressOf()))) {
        return false;
    }

    hr = converter->Initialize(
        frame.Get(),
        GUID_WICPixelFormat32bppRGBA,
        WICBitmapDitherTypeNone,
        nullptr,
        0.0,
        WICBitmapPaletteTypeCustom
    );

    if (FAILED(hr)) {
        cout << "Pixel format conversion failed.\n";
        return false;
    }

    frame->GetSize(&image.width, &image.height);

    if (image.width == 0 || image.height == 0) {
        return false;
    }

    const UINT stride = image.width * 4;

    image.pixels.resize(
        static_cast<size_t>(stride) * image.height
    );

    hr = converter->CopyPixels(
        nullptr,
        stride,
        static_cast<UINT>(image.pixels.size()),
        image.pixels.data()
    );

    return SUCCEEDED(hr);
}

inline bool StickerSavePng(const fs::path& path, const RgbaImage& image) {
    using Microsoft::WRL::ComPtr;

    ComPtr<IWICImagingFactory> factory;

    if (FAILED(CoCreateInstance(
        CLSID_WICImagingFactory,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(factory.GetAddressOf())
    ))) {
        return false;
    }

    ComPtr<IWICStream> stream;

    if (FAILED(factory->CreateStream(stream.GetAddressOf())) ||
        FAILED(stream->InitializeFromFilename(path.c_str(), GENERIC_WRITE))) {
        return false;
    }

    ComPtr<IWICBitmapEncoder> encoder;

    if (FAILED(factory->CreateEncoder(
        GUID_ContainerFormatPng,
        nullptr,
        encoder.GetAddressOf()
    ))) {
        return false;
    }

    if (FAILED(encoder->Initialize(stream.Get(), WICBitmapEncoderNoCache))) {
        return false;
    }

    ComPtr<IWICBitmapFrameEncode> frame;
    ComPtr<IPropertyBag2> properties;

    if (FAILED(encoder->CreateNewFrame(
        frame.GetAddressOf(),
        properties.GetAddressOf()
    ))) {
        return false;
    }

    if (FAILED(frame->Initialize(properties.Get()))) {
        return false;
    }

    frame->SetSize(image.width, image.height);

    WICPixelFormatGUID format = GUID_WICPixelFormat32bppBGRA;

    if (FAILED(frame->SetPixelFormat(&format))) {
        return false;
    }

    // Convert RGBA -> BGRA for the PNG encoder.
    std::vector<unsigned char> bgra(image.pixels.size());

    for (size_t i = 0; i < image.pixels.size(); i += 4) {
        bgra[i + 0] = image.pixels[i + 2];
        bgra[i + 1] = image.pixels[i + 1];
        bgra[i + 2] = image.pixels[i + 0];
        bgra[i + 3] = image.pixels[i + 3];
    }

    const UINT stride = image.width * 4;

    if (FAILED(frame->WritePixels(
        image.height,
        stride,
        static_cast<UINT>(bgra.size()),
        reinterpret_cast<BYTE*>(bgra.data())
    ))) {
        return false;
    }

    return SUCCEEDED(frame->Commit()) && SUCCEEDED(encoder->Commit());
}

inline bool StickerSaveWebp(const fs::path& path, const RgbaImage& image) {
    using Microsoft::WRL::ComPtr;

    ComPtr<IWICImagingFactory> factory;

    if (FAILED(CoCreateInstance(
        CLSID_WICImagingFactory,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(factory.GetAddressOf())
    ))) {
        return false;
    }

    ComPtr<IWICStream> stream;

    if (FAILED(factory->CreateStream(stream.GetAddressOf())) ||
        FAILED(stream->InitializeFromFilename(path.c_str(), GENERIC_WRITE))) {
        return false;
    }

    ComPtr<IWICBitmapEncoder> encoder;

    // WebP container GUID: {62040D03-ECB0-4F95-811C-5A9C7FE83336}
    static const GUID GUID_ContainerFormatWebp = {
        0x62040D03, 0xECB0, 0x4F95, {0x81, 0x1C, 0x5A, 0x9C, 0x7F, 0xE8, 0x33, 0x36}
    };

    if (FAILED(factory->CreateEncoder(
        GUID_ContainerFormatWebp,
        nullptr,
        encoder.GetAddressOf()
    ))) {
        return false;
    }

    if (FAILED(encoder->Initialize(stream.Get(), WICBitmapEncoderNoCache))) {
        return false;
    }

    ComPtr<IWICBitmapFrameEncode> frame;
    ComPtr<IPropertyBag2> properties;

    if (FAILED(encoder->CreateNewFrame(
        frame.GetAddressOf(),
        properties.GetAddressOf()
    ))) {
        return false;
    }

    if (FAILED(frame->Initialize(properties.Get()))) {
        return false;
    }

    frame->SetSize(image.width, image.height);

    WICPixelFormatGUID format = GUID_WICPixelFormat32bppBGRA;

    if (FAILED(frame->SetPixelFormat(&format))) {
        return false;
    }

    std::vector<unsigned char> bgra(image.pixels.size());

    for (size_t i = 0; i < image.pixels.size(); i += 4) {
        bgra[i + 0] = image.pixels[i + 2];
        bgra[i + 1] = image.pixels[i + 1];
        bgra[i + 2] = image.pixels[i + 0];
        bgra[i + 3] = image.pixels[i + 3];
    }

    const UINT stride = image.width * 4;

    if (FAILED(frame->WritePixels(
        image.height,
        stride,
        static_cast<UINT>(bgra.size()),
        reinterpret_cast<BYTE*>(bgra.data())
    ))) {
        return false;
    }

    return SUCCEEDED(frame->Commit()) && SUCCEEDED(encoder->Commit());
}

inline bool StickerHasAlpha(const RgbaImage& image) {
    for (size_t i = 3; i < image.pixels.size(); i += 4) {
        if (image.pixels[i] < 250) {
            return true;
        }
    }

    return false;
}

// Remove a flat background (white / solid colour) by flood-filling
// from the border. Pure C++, no external model required.
inline void StickerRemoveFlatBackground(RgbaImage& image, int tolerance = 28) {
    const int w = static_cast<int>(image.width);
    const int h = static_cast<int>(image.height);

    if (w <= 0 || h <= 0) {
        return;
    }

    auto pixel = [&](int x, int y) -> unsigned char* {
        return &image.pixels[(static_cast<size_t>(y) * w + x) * 4];
    };

    // Reference colour = average of the four corners.
    int r = 0, g = 0, b = 0;
    const int corners[4][2] = {
        { 0, 0 }, { w - 1, 0 }, { 0, h - 1 }, { w - 1, h - 1 }
    };

    for (const auto& c : corners) {
        unsigned char* p = pixel(c[0], c[1]);
        r += p[0]; g += p[1]; b += p[2];
    }

    r /= 4; g /= 4; b /= 4;

    std::vector<unsigned char> background(
        static_cast<size_t>(w) * h, 0
    );

    std::vector<int> stack;
    stack.reserve(static_cast<size_t>(w) * h);

    auto tryPush = [&](int x, int y) {
        if (x < 0 || y < 0 || x >= w || y >= h) {
            return;
        }

        const size_t index = static_cast<size_t>(y) * w + x;

        if (background[index]) {
            return;
        }

        unsigned char* p = pixel(x, y);

        if (p[3] > 16 &&
            std::abs(static_cast<int>(p[0]) - r) <= tolerance &&
            std::abs(static_cast<int>(p[1]) - g) <= tolerance &&
            std::abs(static_cast<int>(p[2]) - b) <= tolerance) {
            background[index] = 1;
            stack.push_back(static_cast<int>(index));
        }
    };

    for (int x = 0; x < w; ++x) {
        tryPush(x, 0);
        tryPush(x, h - 1);
    }

    for (int y = 0; y < h; ++y) {
        tryPush(0, y);
        tryPush(w - 1, y);
    }

    while (!stack.empty()) {
        const int index = stack.back();
        stack.pop_back();

        const int x = index % w;
        const int y = index / w;

        tryPush(x + 1, y);
        tryPush(x - 1, y);
        tryPush(x, y + 1);
        tryPush(x, y - 1);
    }

    for (size_t i = 0; i < background.size(); ++i) {
        if (background[i]) {
            image.pixels[i * 4 + 3] = 0;
        }
    }
}

inline void StickerErodeAlpha(RgbaImage& image, int radius) {
    if (radius <= 0) {
        return;
    }

    const int w = static_cast<int>(image.width);
    const int h = static_cast<int>(image.height);

    std::vector<unsigned char> source(
        static_cast<size_t>(w) * h
    );

    for (int i = 0; i < w * h; ++i) {
        source[i] = image.pixels[static_cast<size_t>(i) * 4 + 3];
    }

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            unsigned char smallest = 255;

            for (int dy = -radius; dy <= radius; ++dy) {
                for (int dx = -radius; dx <= radius; ++dx) {
                    const int nx = x + dx;
                    const int ny = y + dy;

                    if (nx < 0 || ny < 0 || nx >= w || ny >= h) {
                        smallest = 0;
                        continue;
                    }

                    smallest = std::min(
                        smallest,
                        source[static_cast<size_t>(ny) * w + nx]
                    );
                }
            }

            image.pixels[(static_cast<size_t>(y) * w + x) * 4 + 3] =
                smallest;
        }
    }
}

inline void StickerFeatherAlpha(RgbaImage& image) {
    const int w = static_cast<int>(image.width);
    const int h = static_cast<int>(image.height);

    std::vector<unsigned char> source(
        static_cast<size_t>(w) * h
    );

    for (int i = 0; i < w * h; ++i) {
        source[i] = image.pixels[static_cast<size_t>(i) * 4 + 3];
    }

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int sum = 0;
            int count = 0;

            for (int dy = -1; dy <= 1; ++dy) {
                for (int dx = -1; dx <= 1; ++dx) {
                    const int nx = x + dx;
                    const int ny = y + dy;

                    if (nx < 0 || ny < 0 || nx >= w || ny >= h) {
                        continue;
                    }

                    sum += source[static_cast<size_t>(ny) * w + nx];
                    ++count;
                }
            }

            image.pixels[(static_cast<size_t>(y) * w + x) * 4 + 3] =
                static_cast<unsigned char>(sum / count);
        }
    }
}

inline bool StickerRunRembg(const fs::path& input, const fs::path& output) {
    std::wstring command =
        L"rembg i \"" + input.wstring() + L"\" \"" +
        output.wstring() + L"\"";

    STARTUPINFOW si{};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    PROCESS_INFORMATION pi{};

    std::vector<wchar_t> buffer(command.begin(), command.end());
    buffer.push_back(L'\0');

    if (!CreateProcessW(
        nullptr,
        buffer.data(),
        nullptr,
        nullptr,
        FALSE,
        CREATE_NO_WINDOW,
        nullptr,
        nullptr,
        &si,
        &pi
    )) {
        return false;
    }

    const DWORD wait = WaitForSingleObject(pi.hProcess, 120000);

    DWORD exitCode = 1;

    if (wait == WAIT_OBJECT_0) {
        GetExitCodeProcess(pi.hProcess, &exitCode);
    }

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

    return wait == WAIT_OBJECT_0 && exitCode == 0 &&
        fs::exists(output);
}

// Separable max filter (dilation) over a binary mask.
inline std::vector<unsigned char> StickerDilateMask(
    const std::vector<unsigned char>& mask,
    int w,
    int h,
    int radius
) {
    std::vector<unsigned char> horizontal(
        static_cast<size_t>(w) * h, 0
    );
    std::vector<unsigned char> result(
        static_cast<size_t>(w) * h, 0
    );

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            unsigned char value = 0;

            for (int dx = -radius; dx <= radius; ++dx) {
                const int nx = x + dx;

                if (nx < 0 || nx >= w) {
                    continue;
                }

                value = std::max(
                    value,
                    mask[static_cast<size_t>(y) * w + nx]
                );
            }

            horizontal[static_cast<size_t>(y) * w + x] = value;
        }
    }

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            unsigned char value = 0;

            for (int dy = -radius; dy <= radius; ++dy) {
                const int ny = y + dy;

                if (ny < 0 || ny >= h) {
                    continue;
                }

                value = std::max(
                    value,
                    horizontal[static_cast<size_t>(ny) * w + x]
                );
            }

            result[static_cast<size_t>(y) * w + x] = value;
        }
    }

    return result;
}

inline std::vector<unsigned char> StickerBlurMask(
    const std::vector<unsigned char>& mask,
    int w,
    int h,
    int radius
) {
    std::vector<unsigned char> horizontal(
        static_cast<size_t>(w) * h, 0
    );
    std::vector<unsigned char> result(
        static_cast<size_t>(w) * h, 0
    );

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int sum = 0;
            int count = 0;

            for (int dx = -radius; dx <= radius; ++dx) {
                const int nx = x + dx;

                if (nx < 0 || nx >= w) {
                    continue;
                }

                sum += mask[static_cast<size_t>(y) * w + nx];
                ++count;
            }

            horizontal[static_cast<size_t>(y) * w + x] =
                static_cast<unsigned char>(sum / count);
        }
    }

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int sum = 0;
            int count = 0;

            for (int dy = -radius; dy <= radius; ++dy) {
                const int ny = y + dy;

                if (ny < 0 || ny >= h) {
                    continue;
                }

                sum += horizontal[static_cast<size_t>(ny) * w + x];
                ++count;
            }

            result[static_cast<size_t>(y) * w + x] =
                static_cast<unsigned char>(sum / count);
        }
    }

    return result;
}

inline RgbaImage StickerStylize(
    const RgbaImage& source,
    int stroke,
    int shadowOffset,
    int padding
) {
    const int w = static_cast<int>(source.width);
    const int h = static_cast<int>(source.height);

    stroke = std::max(0, stroke);
    shadowOffset = std::max(0, shadowOffset);
    padding = std::max(0, padding);

    std::vector<unsigned char> mask(
        static_cast<size_t>(w) * h, 0
    );

    for (int i = 0; i < w * h; ++i) {
        mask[i] = source.pixels[static_cast<size_t>(i) * 4 + 3] >= 96
            ? 1 : 0;
    }

    // Outward dilation gives the white sticker border.
    const std::vector<unsigned char> outline =
        StickerDilateMask(mask, w, h, std::max(stroke, 1));

    const int blurRadius = std::max(1, stroke / 2 + 1);

    std::vector<unsigned char> shadow =
        StickerBlurMask(outline, w, h, blurRadius);

    const int margin = stroke + padding;
    const int width = w + margin * 2;
    const int height = h + margin * 2 + shadowOffset;

    RgbaImage result;
    result.width = static_cast<UINT>(width);
    result.height = static_cast<UINT>(height);
    result.pixels.assign(
        static_cast<size_t>(width) * height * 4, 0
    );

    auto draw = [&](int x, int y, unsigned char r,
        unsigned char g, unsigned char b, unsigned char a) {
        if (x < 0 || y < 0 || x >= width || y >= height) {
            return;
        }

        const size_t offset =
            (static_cast<size_t>(y) * width + x) * 4;

        const float sa = a / 255.0f;
        const float da = result.pixels[offset + 3] / 255.0f;
        const float outA = sa + da * (1.0f - sa);

        if (outA <= 0.0f) {
            return;
        }

        result.pixels[offset + 0] = static_cast<unsigned char>(
            (r * sa + result.pixels[offset + 0] * da * (1.0f - sa)) / outA
        );
        result.pixels[offset + 1] = static_cast<unsigned char>(
            (g * sa + result.pixels[offset + 1] * da * (1.0f - sa)) / outA
        );
        result.pixels[offset + 2] = static_cast<unsigned char>(
            (b * sa + result.pixels[offset + 2] * da * (1.0f - sa)) / outA
        );
        result.pixels[offset + 3] = static_cast<unsigned char>(
            outA * 255.0f
        );
    };

    // 1) soft shadow
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            const unsigned char value =
                shadow[static_cast<size_t>(y) * w + x];

            if (value == 0) {
                continue;
            }

            const unsigned char alpha = static_cast<unsigned char>(
                value * 0.35f
            );

            draw(
                x + margin,
                y + margin + shadowOffset,
                0, 0, 0, alpha
            );
        }
    }

    // 2) white sticker border
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            if (!outline[static_cast<size_t>(y) * w + x]) {
                continue;
            }

            draw(x + margin, y + margin, 255, 255, 255, 255);
        }
    }

    // 3) original artwork on top
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            const size_t offset =
                (static_cast<size_t>(y) * w + x) * 4;

            const unsigned char alpha = source.pixels[offset + 3];

            if (alpha == 0) {
                continue;
            }

            draw(
                x + margin,
                y + margin,
                source.pixels[offset + 0],
                source.pixels[offset + 1],
                source.pixels[offset + 2],
                alpha
            );
        }
    }

    return result;
}

inline void StickerTrim(RgbaImage& image, int padding) {
    int left = static_cast<int>(image.width);
    int top = static_cast<int>(image.height);
    int right = -1;
    int bottom = -1;

    for (int y = 0; y < static_cast<int>(image.height); ++y) {
        for (int x = 0; x < static_cast<int>(image.width); ++x) {
            const size_t offset =
                (static_cast<size_t>(y) * image.width + x) * 4;

            if (image.pixels[offset + 3] == 0) {
                continue;
            }

            left = std::min(left, x);
            top = std::min(top, y);
            right = std::max(right, x);
            bottom = std::max(bottom, y);
        }
    }

    if (right < 0) {
        return;
    }

    left = std::max(0, left - padding);
    top = std::max(0, top - padding);
    right = std::min(
        static_cast<int>(image.width) - 1, right + padding
    );
    bottom = std::min(
        static_cast<int>(image.height) - 1, bottom + padding
    );

    const int width = right - left + 1;
    const int height = bottom - top + 1;

    RgbaImage cropped;
    cropped.width = static_cast<UINT>(width);
    cropped.height = static_cast<UINT>(height);
    cropped.pixels.resize(
        static_cast<size_t>(width) * height * 4
    );

    for (int y = 0; y < height; ++y) {
        memcpy(
            &cropped.pixels[static_cast<size_t>(y) * width * 4],
            &image.pixels[
                (static_cast<size_t>(top + y) * image.width +
                    left) * 4
            ],
            static_cast<size_t>(width) * 4
        );
    }

    image = std::move(cropped);
}

inline RgbaImage StickerResize(const RgbaImage& source, UINT width, UINT height) {
    RgbaImage result;
    result.width = width;
    result.height = height;
    result.pixels.resize(static_cast<size_t>(width) * height * 4, 0);

    const float scaleX =
        static_cast<float>(source.width) / static_cast<float>(width);
    const float scaleY =
        static_cast<float>(source.height) / static_cast<float>(height);

    for (UINT y = 0; y < height; ++y) {
        float sy = (y + 0.5f) * scaleY - 0.5f;
        sy = std::clamp(sy, 0.0f, static_cast<float>(source.height - 1));

        const int y0 = static_cast<int>(sy);
        const int y1 = std::min(y0 + 1, static_cast<int>(source.height) - 1);
        const float fy = sy - y0;

        for (UINT x = 0; x < width; ++x) {
            float sx = (x + 0.5f) * scaleX - 0.5f;
            sx = std::clamp(sx, 0.0f, static_cast<float>(source.width - 1));

            const int x0 = static_cast<int>(sx);
            const int x1 = std::min(x0 + 1, static_cast<int>(source.width) - 1);
            const float fx = sx - x0;

            float premul[4] = { 0, 0, 0, 0 };

            const int xs[2] = { x0, x1 };
            const int ys[2] = { y0, y1 };
            const float wx[2] = { 1.0f - fx, fx };
            const float wy[2] = { 1.0f - fy, fy };

            for (int syi = 0; syi < 2; ++syi) {
                for (int sxi = 0; sxi < 2; ++sxi) {
                    const size_t offset =
                        (static_cast<size_t>(ys[syi]) * source.width +
                            xs[sxi]) * 4;

                    const float alpha =
                        source.pixels[offset + 3] / 255.0f;
                    const float weight = wx[sxi] * wy[syi];

                    premul[0] += source.pixels[offset + 0] * alpha * weight;
                    premul[1] += source.pixels[offset + 1] * alpha * weight;
                    premul[2] += source.pixels[offset + 2] * alpha * weight;
                    premul[3] += source.pixels[offset + 3] * weight;
                }
            }

            const size_t offset =
                (static_cast<size_t>(y) * width + x) * 4;

            const float alpha = premul[3] / 255.0f;

            if (alpha > 0.0f) {
                result.pixels[offset + 0] = static_cast<unsigned char>(
                    std::clamp(premul[0] / alpha, 0.0f, 255.0f)
                );
                result.pixels[offset + 1] = static_cast<unsigned char>(
                    std::clamp(premul[1] / alpha, 0.0f, 255.0f)
                );
                result.pixels[offset + 2] = static_cast<unsigned char>(
                    std::clamp(premul[2] / alpha, 0.0f, 255.0f)
                );
            }

            result.pixels[offset + 3] = static_cast<unsigned char>(
                std::clamp(premul[3], 0.0f, 255.0f)
            );
        }
    }

    return result;
}

inline bool StickerCopyToClipboard(const fs::path& pngPath) {
    RgbaImage image;

    if (!StickerLoadImage(pngPath, image)) {
        return false;
    }

    std::ifstream file(pngPath, ios::binary);
    std::string png(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>()
    );

    if (!OpenClipboard(nullptr)) {
        return false;
    }

    EmptyClipboard();

    bool ok = false;

    if (!png.empty()) {
        HGLOBAL handle = GlobalAlloc(GMEM_MOVEABLE, png.size());

        if (handle) {
            void* memory = GlobalLock(handle);
            memcpy(memory, png.data(), png.size());
            GlobalUnlock(handle);

            const UINT format = RegisterClipboardFormatW(L"PNG");

            if (format && SetClipboardData(format, handle)) {
                ok = true;
            }
            else {
                GlobalFree(handle);
            }
        }
    }

    const size_t headerSize = sizeof(BITMAPV5HEADER);
    const size_t pixelSize =
        static_cast<size_t>(image.width) * image.height * 4;

    HGLOBAL handle = GlobalAlloc(GMEM_MOVEABLE, headerSize + pixelSize);

    if (handle) {
        BYTE* memory = static_cast<BYTE*>(GlobalLock(handle));

        BITMAPV5HEADER* header =
            reinterpret_cast<BITMAPV5HEADER*>(memory);

        ZeroMemory(header, sizeof(BITMAPV5HEADER));

        header->bV5Size = sizeof(BITMAPV5HEADER);
        header->bV5Width = static_cast<LONG>(image.width);
        header->bV5Height = -static_cast<LONG>(image.height);
        header->bV5Planes = 1;
        header->bV5BitCount = 32;
        header->bV5Compression = BI_BITFIELDS;
        header->bV5RedMask = 0x00FF0000;
        header->bV5GreenMask = 0x0000FF00;
        header->bV5BlueMask = 0x000000FF;
        header->bV5AlphaMask = 0xFF000000;
        header->bV5CSType = LCS_WINDOWS_COLOR_SPACE;

        BYTE* destination = memory + headerSize;

        for (size_t i = 0;
            i < static_cast<size_t>(image.width) * image.height;
            ++i) {
            destination[i * 4 + 0] = image.pixels[i * 4 + 2];
            destination[i * 4 + 1] = image.pixels[i * 4 + 1];
            destination[i * 4 + 2] = image.pixels[i * 4 + 0];
            destination[i * 4 + 3] = image.pixels[i * 4 + 3];
        }

        GlobalUnlock(handle);

        if (SetClipboardData(CF_DIBV5, handle)) {
            ok = true;
        }
        else {
            GlobalFree(handle);
        }
    }

    CloseClipboard();

    return ok;
}

inline std::string StickerTimestamp() {
    SYSTEMTIME time{};
    GetLocalTime(&time);

    char buffer[64]{};

    sprintf_s(
        buffer,
        "%04d%02d%02d_%02d%02d%02d",
        time.wYear, time.wMonth, time.wDay,
        time.wHour, time.wMinute, time.wSecond
    );

    return buffer;
}

inline void StickerAppendLibrary(
    const fs::path& folder,
    const fs::path& file,
    const std::string& source,
    const std::string& mode,
    int stroke
) {
    const fs::path index = folder / L"library.tsv";

    const bool exists = fs::exists(index);

    ofstream out(index, ios::app);

    if (!out) {
        return;
    }

    if (!exists) {
        out << "file\tsource\tmode\tstroke\tcreated\n";
    }

    out << WideToUtf8(file.filename().wstring()) << '\t'
        << source << '\t'
        << mode << '\t'
        << stroke << '\t'
        << StickerTimestamp() << '\n';
}

// ================================================================
//  High-level helpers used by the ImGui Custom Sticker tab.
// ================================================================

// mode: 0 = auto cutout, 1 = rembg (AI), 2 = keep image as-is.
inline bool ConvertImageToStickerFile(
    const fs::path& input,
    int mode,
    int stroke,
    bool shadow,
    int maxSize,
    std::string& message,
    fs::path& outPath
) {
    if (!fs::exists(input)) {
        message = "File not found: " + WideToUtf8(input.wstring());
        return false;
    }

    RgbaImage image;
    if (!StickerLoadImage(input, image)) {
        message = "Cannot decode image (unsupported format?).";
        return false;
    }

    const int safeStroke = std::clamp(stroke, 0, 32);
    const int safeSize = std::clamp(maxSize, 64, 2048);

    if (mode == 1) {
        const fs::path cutout = fs::temp_directory_path() / L"z4_rembg_out.png";
        if (StickerRunRembg(input, cutout) && StickerLoadImage(cutout, image)) {
            // rembg cutout applied
        }
        else {
            if (!StickerHasAlpha(image)) {
                StickerRemoveFlatBackground(image);
                StickerErodeAlpha(image, 1);
                StickerFeatherAlpha(image);
            }
        }
        std::error_code ec;
        fs::remove(cutout, ec);
    }
    else if (mode == 0) {
        if (!StickerHasAlpha(image)) {
            StickerRemoveFlatBackground(image);
            StickerErodeAlpha(image, 1);
            StickerFeatherAlpha(image);
        }
    }

    RgbaImage sticker = StickerStylize(
        image,
        safeStroke,
        shadow ? std::max(3, safeStroke) : 0,
        8
    );

    if (sticker.width == 0 || sticker.height == 0) {
        message = "Failed: empty image.";
        return false;
    }

    const int longest = static_cast<int>(std::max(sticker.width, sticker.height));
    if (longest > safeSize && safeSize > 0) {
        const float ratio = static_cast<float>(safeSize) / static_cast<float>(longest);
        sticker = StickerResize(
            sticker,
            std::max<UINT>(1, static_cast<UINT>(sticker.width * ratio)),
            std::max<UINT>(1, static_cast<UINT>(sticker.height * ratio))
        );
    }

    StickerTrim(sticker, 4);

    const fs::path folder = GetStickerDirectory();
    const std::string ts = StickerTimestamp();
    const fs::path pngPath = folder / (L"sticker_" + Utf8ToWide(ts) + L".png");
    const fs::path webpPath = folder / (L"sticker_" + Utf8ToWide(ts) + L".webp");

    if (!StickerSavePng(pngPath, sticker)) {
        message = "Failed to write: " + WideToUtf8(pngPath.wstring());
        return false;
    }

    const auto hasValidWebp = [&webpPath]() {
        std::error_code ec;
        if (!fs::exists(webpPath, ec) || ec) return false;
        const auto size = fs::file_size(webpPath, ec);
        return !ec && size > 0;
    };

    bool webpOk = StickerSaveWebp(webpPath, sticker) && hasValidWebp();

    if (!webpOk) {
        // The WIC WebP encoder ships as a Store extension and is missing on
        // many machines. Fall back to Python Pillow (same as the console
        // build). A temp .py file avoids cmd.exe quote-escaping issues.
        std::error_code ec;
        fs::remove(webpPath, ec);

        const fs::path script = fs::temp_directory_path() / L"z4_webp_encode.py";
        std::ofstream py(script, std::ios::binary);

        if (py) {
            py << "from PIL import Image\n"
               << "im = Image.open(r'" << WideToUtf8(pngPath.wstring()) << "')\n"
               << "im.save(r'" << WideToUtf8(webpPath.wstring())
               << "', 'WEBP', quality=90)\n";
            py.close();

            std::string cmd = "python \"";
            cmd += WideToUtf8(script.wstring());
            cmd += "\" >nul 2>&1";

            system(cmd.c_str());
            fs::remove(script, ec);
        }

        webpOk = hasValidWebp();
    }

    if (!webpOk) {
        // Never leave an empty .webp behind: the picker and sender prefer it
        // over the PNG twin and would upload a broken file.
        std::error_code ec;
        fs::remove(webpPath, ec);
    }

    g_lastStickerPath = webpOk ? webpPath : pngPath;
    outPath = g_lastStickerPath;

    StickerAppendLibrary(
        folder,
        pngPath,
        WideToUtf8(input.filename().wstring()),
        mode == 1 ? "rembg" : (mode == 2 ? "none" : "auto"),
        safeStroke
    );

    std::ostringstream info;
    info << "Saved " << sticker.width << "x" << sticker.height
         << " sticker (" << (webpOk ? "WebP + PNG" : "PNG only") << ").";
    if (!webpOk) {
        info << " WebP failed - phone sync may not render.";
    }
    message = info.str();
    return true;
}

inline void OpenStickerFolder() {
    const fs::path folder = GetStickerDirectory();
    ShellExecuteW(nullptr, L"open", folder.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
}

inline std::vector<fs::path> CollectStickerFiles() {
    std::vector<fs::path> files;
    const fs::path folder = GetStickerDirectory();
    std::error_code ec;

    if (!fs::exists(folder, ec)) {
        return files;
    }

    for (const auto& entry : fs::directory_iterator(folder, ec)) {
        if (ec) break;
        if (!entry.is_regular_file()) continue;

        const fs::path p = entry.path();
        std::string lower = WideToUtf8(p.extension().wstring());
        std::transform(lower.begin(), lower.end(), lower.begin(),
            [](unsigned char c) { return (char)tolower(c); });

        if (lower == ".png" || lower == ".jpg" || lower == ".jpeg" ||
            lower == ".webp" || lower == ".gif") {
            std::error_code sizeEc;
            const auto size = fs::file_size(p, sizeEc);
            if (sizeEc || size == 0) continue;   // skip empty/broken files

            // Prefer WebP: hide the PNG/JPG twin so each sticker appears once,
            // but only when the WebP itself is valid.
            if (lower == ".png" || lower == ".jpg" || lower == ".jpeg") {
                fs::path twin = p;
                twin.replace_extension(L"webp");
                std::error_code ec2;
                const auto twinSize =
                    fs::exists(twin, ec2) ? fs::file_size(twin, ec2) : 0;
                if (!ec2 && twinSize > 0) continue;
            }
            files.push_back(p);
        }
    }

    std::sort(files.begin(), files.end());
    return files;
}

inline fs::path GetLatestStickerFile() {
    std::vector<fs::path> files = CollectStickerFiles();
    if (files.empty()) return {};

    fs::path newest;
    fs::file_time_type newestTime{};

    for (const auto& p : files) {
        std::error_code ec;
        const auto t = fs::last_write_time(p, ec);
        if (ec) continue;
        if (newest.empty() || t > newestTime) {
            newest = p;
            newestTime = t;
        }
    }

    return newest;
}

inline bool CopyLastStickerToClipboard(std::string& message) {
    fs::path target = g_lastStickerPath;
    if (target.empty() || !fs::exists(target)) {
        target = GetLatestStickerFile();
    }
    if (target.empty()) {
        message = "No sticker in the library yet.";
        return false;
    }

    if (!StickerCopyToClipboard(target)) {
        message = "Clipboard copy failed.";
        return false;
    }

    message = "Copied: " + WideToUtf8(target.filename().wstring());
    return true;
}



