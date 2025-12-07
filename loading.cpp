#include "loading.h"
#include <windows.h>
#include <commctrl.h>
#include <string>



void showLoadingWindow(int milliseconds)
{
    using std::to_string;

    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(icex);
    icex.dwICC = ICC_PROGRESS_CLASS;
    InitCommonControlsEx(&icex);

    HINSTANCE hinst = GetModuleHandle(NULL);
    const wchar_t *wcname = L"LoadingWinClass";

    WNDCLASSW wc = {0};
    wc.lpfnWndProc = DefWindowProcW;
    wc.hInstance = hinst;
    wc.lpszClassName = wcname;
    RegisterClassW(&wc);

    HWND hwnd = CreateWindowExW(0, wcname, L"Loading...",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        CW_USEDEFAULT, CW_USEDEFAULT, 360, 120,
        NULL, NULL, hinst, NULL);

    if (!hwnd) return;

    HWND hProg = CreateWindowExW(0, PROGRESS_CLASSW, NULL,
        WS_CHILD | WS_VISIBLE | PBS_SMOOTH,
        20, 40, 300, 20,
        hwnd, NULL, hinst, NULL);

    std::wstring label = L"Booting OS... Please wait";
    HWND hLabel = CreateWindowExW(0, L"STATIC", label.c_str(),
        WS_CHILD | WS_VISIBLE | SS_CENTER,
        20, 10, 300, 20, hwnd, NULL, hinst, NULL);

    ShowWindow(hwnd, SW_SHOW);

    int stepMs = 50;
    if (milliseconds < stepMs) stepMs = milliseconds;
    int steps = milliseconds / stepMs;
    if (steps <= 0) steps = 1;

    SendMessageW(hProg, PBM_SETRANGE, 0, MAKELPARAM(0, 100));

    for (int i = 0; i <= steps; ++i) {
        int pos = (i * 100) / steps;
        SendMessageW(hProg, PBM_SETPOS, pos, 0);

        MSG msg;
        while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }

        Sleep(stepMs);
    }

    DestroyWindow(hProg);
    DestroyWindow(hLabel);
    DestroyWindow(hwnd);
    UnregisterClassW(wcname, hinst);
}
