#include "UI/UI.h"
#include <objbase.h>

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
    // Needed by WIC (sticker conversion) and the shell clipboard helpers.
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

    UI::Render();

    CoUninitialize();
    return 0;
}
