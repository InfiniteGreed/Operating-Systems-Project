#include "gui.h"
#include <windows.h>
#include <string>
#include <sstream>
#include <vector>
#include <utility>

static int s_dialogResult = -1;

static LRESULT CALLBACK SimpleDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_COMMAND: {
        int id = LOWORD(wParam);
        s_dialogResult = id;
        DestroyWindow(hwnd);
        return 0;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

static int runModalWithButtons(const wchar_t *title, const wchar_t *label, const std::vector<std::pair<int,std::wstring>>& buttons)
{
    HINSTANCE hinst = GetModuleHandle(NULL);
    const wchar_t *cls = L"SimpleModalClass";

    WNDCLASSW wc = {0};
    wc.lpfnWndProc = SimpleDlgProc;
    wc.hInstance = hinst;
    wc.lpszClassName = cls;
    RegisterClassW(&wc);

    int width = 360, height = 160;
    HWND hwnd = CreateWindowExW(0, cls, title, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        CW_USEDEFAULT, CW_USEDEFAULT, width, height, NULL, NULL, hinst, NULL);
    if (!hwnd) return -1;

    HWND hLabel = CreateWindowExW(0, L"STATIC", label, WS_CHILD | WS_VISIBLE | SS_CENTER,
        10, 12, width-20, 24, hwnd, NULL, hinst, NULL);

    int btnW = 100, btnH = 28;
    int gap = 12;
    int totalBtnW = (int)buttons.size() * btnW + (int)buttons.size() * gap;
    int startX = (width - totalBtnW) / 2;
    int y = 60;
    for (size_t i = 0; i < buttons.size(); ++i) {
        CreateWindowExW(0, L"BUTTON", buttons[i].second.c_str(), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            startX + int(i)*(btnW+gap), y, btnW, btnH, hwnd, (HMENU)buttons[i].first, hinst, NULL);
    }

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    s_dialogResult = -1;
    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
        if (s_dialogResult != -1) break;
    }

    UnregisterClassW(cls, hinst);
    return s_dialogResult;
}

int askRunScheduler()
{
    std::vector<std::pair<int,std::wstring>> btns = { {101, L"Yes"}, {102, L"No"} };
    int res = runModalWithButtons(L"Run Scheduler?", L"Do you want to run CPU Scheduling?", btns);
    if (res == 101) return 1;
    return 0;
}

int pickAlgorithm()
{
    std::vector<std::pair<int,std::wstring>> btns = { {201, L"FCFS"}, {202, L"SJF"}, {203, L"Round Robin"} };
    int res = runModalWithButtons(L"Pick Algorithm", L"Choose scheduling algorithm:", btns);
    if (res == 201) return 1;
    if (res == 202) return 2;
    if (res == 203) return 3;
    return -1;
}

static LRESULT CALLBACK InputDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_COMMAND: {
        int id = LOWORD(wParam);
        if (id == 301) {
            wchar_t buf[64] = {0};
            HWND he = GetDlgItem(hwnd, 302);
            GetWindowTextW(he, buf, 64);
            int val = _wtoi(buf);
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, val);
            DestroyWindow(hwnd);
        } else if (id == 303) {
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, -1);
            DestroyWindow(hwnd);
        }
        return 0;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

int askTimeQuantum(int defaultQ)
{
    HINSTANCE hinst = GetModuleHandle(NULL);
    const wchar_t* cls = L"InputModalClass";
    WNDCLASSW wc = {0};
    wc.lpfnWndProc = InputDlgProc;
    wc.hInstance = hinst;
    wc.lpszClassName = cls;
    RegisterClassW(&wc);

    int width = 340, height = 140;
    HWND hwnd = CreateWindowExW(0, cls, L"Time Quantum", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        CW_USEDEFAULT, CW_USEDEFAULT, width, height, NULL, NULL, hinst, NULL);
    if (!hwnd) return -1;

    CreateWindowExW(0, L"STATIC", L"Enter time quantum (integer):", WS_CHILD | WS_VISIBLE | SS_LEFT,
        12, 12, width-24, 20, hwnd, NULL, hinst, NULL);

    HWND he = CreateWindowExW(0, L"EDIT", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
        12, 36, 120, 24, hwnd, (HMENU)302, hinst, NULL);
    wchar_t defbuf[32]; swprintf_s(defbuf, L"%d", defaultQ);
    SetWindowTextW(he, defbuf);

    CreateWindowExW(0, L"BUTTON", L"OK", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        150, 36, 64, 28, hwnd, (HMENU)301, hinst, NULL);
    CreateWindowExW(0, L"BUTTON", L"Cancel", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        220, 36, 64, 28, hwnd, (HMENU)303, hinst, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
        if (!IsWindow(hwnd)) break;
    }

    int result = (int)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
    UnregisterClassW(cls, hinst);
    return result;
}
