#include "app.h"
#include "memory.h"
#include "process.h"
#include "scheduler.h"
#include <windows.h>
#include <commctrl.h>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>

#define ID_USERNAME 2001
#define ID_PASSWORD 2002
#define ID_LOGIN    2003
#define ID_PROGRESS 2004
#define ID_ALGO_COMBO 2006
#define ID_QUANTUM 2007
#define ID_RUN 2008
#define ID_PROC_TABLE 2009
#define ID_LOG 3000

#define ID_CPU_BAR 4001
#define ID_CPU_LABEL 4002
#define ID_GPU_BAR 4003
#define ID_GPU_LABEL 4004
#define ID_RAM_BAR 4005
#define ID_RAM_LABEL 4006
#define ID_DISK_BAR 4007
#define ID_DISK_LABEL 4008

static HWND ghLog = NULL;
static MemoryManager *gMM = nullptr;
static std::vector<Process> gProcesses;
static HBRUSH ghBlackBrush = NULL;
static std::string gInitialCapturedLog;
static bool gLoggedIn = false;

static void AppendLog(HWND hwndLog, const std::string &text)
{
    if (!hwndLog) return;
    int len = GetWindowTextLengthA(hwndLog);
    SendMessageA(hwndLog, EM_SETSEL, (WPARAM)len, (LPARAM)len);
    SendMessageA(hwndLog, EM_REPLACESEL, FALSE, (LPARAM)text.c_str());
    SendMessageA(hwndLog, EM_REPLACESEL, FALSE, (LPARAM)"\r\n");
}

static std::string trim(const std::string &s) {
    size_t a = 0, b = s.size();
    while (a < b && std::isspace((unsigned char)s[a])) a++;
    while (b > a && std::isspace((unsigned char)s[b-1])) b--;
    return s.substr(a, b-a);
}

static void ProcessAndAppendLog(HWND hwndLog, const std::string &captured)
{
    std::istringstream iss(captured);
    std::string line;
    while (std::getline(iss, line)) {
        line = trim(line);
        if (line.empty()) continue;
        std::string out;
        if (line.rfind("Page fault:", 0) == 0) {
            out = "[PAGEFAULT] " + line.substr(strlen("Page fault:"));
        } else if (line.rfind("Evicted", 0) == 0) {
            out = "[EVICT] " + line;
        } else if (line.rfind("Allocated process", 0) == 0) {
            out = "[ALLOC] " + line.substr(strlen("Allocated process"));
        } else {
            out = line;
        }
        AppendLog(hwndLog, out);
    }
}

#define ID_LU_USERNAME 5001
#define ID_LU_PASSWORD 5002
#define ID_LU_LOGIN    5003
#define ID_LU_PROGRESS 5004

LRESULT CALLBACK LoginWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_CREATE: {
        HINSTANCE hinst = GetModuleHandle(NULL);
        CreateWindowExA(0, "STATIC", "Username:", WS_CHILD | WS_VISIBLE, 12, 12, 80, 20, hwnd, NULL, hinst, NULL);
        CreateWindowExA(0, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER, 100, 12, 200, 22, hwnd, (HMENU)ID_LU_USERNAME, hinst, NULL);
        CreateWindowExA(0, "STATIC", "Password:", WS_CHILD | WS_VISIBLE, 12, 44, 80, 20, hwnd, NULL, hinst, NULL);
        CreateWindowExA(0, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_PASSWORD, 100, 44, 200, 22, hwnd, (HMENU)ID_LU_PASSWORD, hinst, NULL);
        CreateWindowExA(0, "BUTTON", "Login", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 310, 12, 60, 54, hwnd, (HMENU)ID_LU_LOGIN, hinst, NULL);
        CreateWindowExW(0, PROGRESS_CLASSW, NULL, WS_CHILD | WS_VISIBLE | PBS_SMOOTH, 12, 80, 360, 14, hwnd, (HMENU)ID_LU_PROGRESS, hinst, NULL);
        return 0;
    }
    case WM_COMMAND: {
        int id = LOWORD(wParam);
        if (id == ID_LU_LOGIN) {
            char user[128], pass[128];
            GetWindowTextA(GetDlgItem(hwnd, ID_LU_USERNAME), user, sizeof(user));
            GetWindowTextA(GetDlgItem(hwnd, ID_LU_PASSWORD), pass, sizeof(pass));
            std::string su(user), sp(pass);
            if (su == "admin" && sp == "adminpass") {
                HWND hProg = GetDlgItem(hwnd, ID_LU_PROGRESS);
                SendMessageW(hProg, PBM_SETRANGE, 0, MAKELPARAM(0,100));
                for (int i=0;i<=100;i+=5) {
                    SendMessageW(hProg, PBM_SETPOS, i, 0);
                    MSG msg; while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) { TranslateMessage(&msg); DispatchMessage(&msg); }
                    Sleep(25);
                }

                gMM = new MemoryManager(4, 256);
                gProcesses.clear();
                gProcesses.push_back(Process(4,2,5));
                gProcesses.push_back(Process(6,1,9));
                gProcesses.push_back(Process(8,0,3));
                gProcesses.push_back(Process(3,2,4));

                std::ostringstream cap;
                auto *oldbuf = std::cout.rdbuf(cap.rdbuf());
                for (auto &p : gProcesses) {
                    gMM->allocateProcess(p.pid, 8);
                    p.accessMemory(0, *gMM);
                    p.accessMemory(256 + 10, *gMM);
                }
                std::cout.rdbuf(oldbuf);
                gInitialCapturedLog = cap.str();
                gLoggedIn = true;
                DestroyWindow(hwnd);
            } else {
                MessageBoxA(hwnd, "Incorrect username or password.", "Login", MB_OK | MB_ICONERROR);
            }
        }
        return 0;
    }
    case WM_DESTROY:
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

bool showLoginWindow(HINSTANCE hinst)
{
    const wchar_t *lcls = L"OSLoginWindowClass";
    WNDCLASSW lwc = {0};
    lwc.lpfnWndProc = LoginWndProc;
    lwc.hInstance = hinst;
    lwc.lpszClassName = lcls;
    RegisterClassW(&lwc);

    int w = 400, h = 140;
    int x = (GetSystemMetrics(SM_CXSCREEN) - w) / 2;
    int y = (GetSystemMetrics(SM_CYSCREEN) - h) / 2;
    HWND lwh = CreateWindowExW(0, lcls, L"Login - Simple OS Simulator", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        x, y, w, h, NULL, NULL, hinst, NULL);
    if (!lwh) { UnregisterClassW(lcls, hinst); return false; }

    ShowWindow(lwh, SW_SHOW);
    UpdateWindow(lwh);

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
        if (gLoggedIn) break;
    }

    DestroyWindow(lwh);
    UnregisterClassW(lcls, hinst);
    return gLoggedIn;
}

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_CREATE: {
        HINSTANCE hinst = GetModuleHandle(NULL);

        INITCOMMONCONTROLSEX icex; icex.dwSize = sizeof(icex); icex.dwICC = ICC_PROGRESS_CLASS | ICC_LISTVIEW_CLASSES; InitCommonControlsEx(&icex);

        CreateWindowExA(0, "STATIC", "CPU:", WS_CHILD | WS_VISIBLE, 12, 64, 48, 16, hwnd, NULL, hinst, NULL);
        CreateWindowExW(0, PROGRESS_CLASSW, NULL, WS_CHILD | WS_VISIBLE | PBS_SMOOTH, 68, 66, 560, 14, hwnd, (HMENU)ID_CPU_BAR, hinst, NULL);
        CreateWindowExA(0, "STATIC", "0%", WS_CHILD | WS_VISIBLE, 636, 62, 48, 20, hwnd, (HMENU)ID_CPU_LABEL, hinst, NULL);

        CreateWindowExA(0, "STATIC", "GPU:", WS_CHILD | WS_VISIBLE, 12, 96, 48, 16, hwnd, NULL, hinst, NULL);
        CreateWindowExW(0, PROGRESS_CLASSW, NULL, WS_CHILD | WS_VISIBLE | PBS_SMOOTH, 68, 98, 560, 14, hwnd, (HMENU)ID_GPU_BAR, hinst, NULL);
        CreateWindowExA(0, "STATIC", "0%", WS_CHILD | WS_VISIBLE, 636, 92, 48, 20, hwnd, (HMENU)ID_GPU_LABEL, hinst, NULL);

        CreateWindowExA(0, "STATIC", "Disk:", WS_CHILD | WS_VISIBLE, 12, 128, 48, 16, hwnd, NULL, hinst, NULL);
        CreateWindowExW(0, PROGRESS_CLASSW, NULL, WS_CHILD | WS_VISIBLE | PBS_SMOOTH, 68, 130, 560, 14, hwnd, (HMENU)ID_DISK_BAR, hinst, NULL);
        CreateWindowExA(0, "STATIC", "0%", WS_CHILD | WS_VISIBLE, 636, 124, 48, 20, hwnd, (HMENU)ID_DISK_LABEL, hinst, NULL);

        CreateWindowExA(0, "STATIC", "RAM:", WS_CHILD | WS_VISIBLE, 12, 160, 48, 16, hwnd, NULL, hinst, NULL);
        CreateWindowExW(0, PROGRESS_CLASSW, NULL, WS_CHILD | WS_VISIBLE | PBS_SMOOTH, 68, 162, 560, 14, hwnd, (HMENU)ID_RAM_BAR, hinst, NULL);
        CreateWindowExA(0, "STATIC", "0%", WS_CHILD | WS_VISIBLE, 636, 158, 48, 20, hwnd, (HMENU)ID_RAM_LABEL, hinst, NULL);

        CreateWindowExW(0, PROGRESS_CLASSW, NULL, WS_CHILD | WS_VISIBLE | PBS_SMOOTH, 12, 196, 672, 18, hwnd, (HMENU)ID_PROGRESS, hinst, NULL);

        HWND hList = CreateWindowExA(WS_EX_CLIENTEDGE, "SysListView32", NULL,
            WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS,
            12, 220, 672, 260, hwnd, (HMENU)ID_PROC_TABLE, hinst, NULL);
        ListView_SetExtendedListViewStyle(hList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
        LVCOLUMNA col = {0};
        col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
        col.cx = 60; col.pszText = (LPSTR)"PID"; ListView_InsertColumn(hList, 0, &col);
        col.cx = 60; col.pszText = (LPSTR)"AT"; ListView_InsertColumn(hList, 1, &col);
        col.cx = 60; col.pszText = (LPSTR)"BT"; ListView_InsertColumn(hList, 2, &col);
        col.cx = 80; col.pszText = (LPSTR)"CT"; ListView_InsertColumn(hList, 3, &col);
        col.cx = 80; col.pszText = (LPSTR)"TAT"; ListView_InsertColumn(hList, 4, &col);
        col.cx = 80; col.pszText = (LPSTR)"WT"; ListView_InsertColumn(hList, 5, &col);

        CreateWindowExA(0, "STATIC", "CPU Scheduling:", WS_CHILD | WS_VISIBLE, 12, 500, 120, 20, hwnd, NULL, hinst, NULL);
        HWND hCombo = CreateWindowExA(0, "COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, 140, 496, 300, 100, hwnd, (HMENU)ID_ALGO_COMBO, hinst, NULL);
        SendMessageA(hCombo, CB_ADDSTRING, 0, (LPARAM)"FCFS");
        SendMessageA(hCombo, CB_ADDSTRING, 0, (LPARAM)"SJF");
        SendMessageA(hCombo, CB_ADDSTRING, 0, (LPARAM)"Round Robin");
        CreateWindowExA(0, "STATIC", "Quantum:", WS_CHILD | WS_VISIBLE, 456, 500, 60, 20, hwnd, NULL, hinst, NULL);
        CreateWindowExA(0, "EDIT", "2", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER, 524, 500, 60, 22, hwnd, (HMENU)ID_QUANTUM, hinst, NULL);
        CreateWindowExA(0, "BUTTON", "Run", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_DISABLED, 604, 496, 80, 28, hwnd, (HMENU)ID_RUN, hinst, NULL);

        ghLog = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY, 12, 540, 672, 200, hwnd, (HMENU)ID_LOG, hinst, NULL);

        ghBlackBrush = CreateSolidBrush(RGB(0,0,0));

        ListView_SetBkColor(hList, RGB(0,0,0));
        ListView_SetTextBkColor(hList, RGB(0,0,0));
        ListView_SetTextColor(hList, RGB(255,255,255));

        if (!gInitialCapturedLog.empty() && ghLog) {
            ProcessAndAppendLog(ghLog, gInitialCapturedLog);
            for (auto &p : gProcesses) {
                std::ostringstream oss;
                oss << "Process ID: " << p.pid << " AT=" << p.arrival_time << " BT=" << p.burst_time;
                AppendLog(ghLog, oss.str());
            }
        }

        EnableWindow(GetDlgItem(hwnd, ID_ALGO_COMBO), TRUE);
        EnableWindow(GetDlgItem(hwnd, ID_QUANTUM), TRUE);
        EnableWindow(GetDlgItem(hwnd, ID_RUN), TRUE);

        SetTimer(hwnd, 1001, 500, NULL);

        return 0;
    }

    case WM_COMMAND: {
        int id = LOWORD(wParam);
        if (id == ID_RUN) {
            HWND hCombo = GetDlgItem(hwnd, ID_ALGO_COMBO);
            int sel = (int)SendMessageA(hCombo, CB_GETCURSEL, 0, 0);
            if (sel == CB_ERR) { AppendLog(ghLog, "Please select an algorithm"); break; }
            int algo = sel + 1;

            int n = (int)gProcesses.size();
            std::vector<Process> sched(gProcesses.begin(), gProcesses.end());
            for (int i=0;i<n;i++) { sched[i].completion_time = -1; sched[i].turnaround_time = 0; sched[i].waiting_time = 0; }

            std::ostringstream capture;
            auto *oldbuf = std::cout.rdbuf(capture.rdbuf());

            if (algo == 1) fcfs(sched, n);
            else if (algo == 2) sjf(sched, n);
            else if (algo == 3) {
                char qbuf[64]; GetWindowTextA(GetDlgItem(hwnd, ID_QUANTUM), qbuf, sizeof(qbuf));
                int q = atoi(qbuf);
                if (q <= 0) { AppendLog(ghLog, "Invalid quantum"); }
                else roundRobin(sched, n, q);
            }

            std::cout.rdbuf(oldbuf);
            ProcessAndAppendLog(ghLog, capture.str());

            HWND hList = GetDlgItem(hwnd, ID_PROC_TABLE);
            ListView_DeleteAllItems(hList);
            for (int i = 0; i < n; ++i) {
                char buf[256];
                LVITEMA item = {0};
                item.mask = LVIF_TEXT;
                item.iItem = i;
                item.iSubItem = 0;
                snprintf(buf, sizeof(buf), "%d", sched[i].pid);
                item.pszText = buf;
                SendMessageA(hList, LVM_INSERTITEMA, 0, (LPARAM)&item);

                LVITEMA sub = {0};
                sub.mask = LVIF_TEXT;
                sub.iItem = i;

                snprintf(buf, sizeof(buf), "%d", sched[i].arrival_time);
                sub.iSubItem = 1; sub.pszText = buf; SendMessageA(hList, LVM_SETITEMTEXTA, (WPARAM)i, (LPARAM)&sub);

                snprintf(buf, sizeof(buf), "%d", sched[i].burst_time);
                sub.iSubItem = 2; sub.pszText = buf; SendMessageA(hList, LVM_SETITEMTEXTA, (WPARAM)i, (LPARAM)&sub);

                snprintf(buf, sizeof(buf), "%d", sched[i].completion_time);
                sub.iSubItem = 3; sub.pszText = buf; SendMessageA(hList, LVM_SETITEMTEXTA, (WPARAM)i, (LPARAM)&sub);

                snprintf(buf, sizeof(buf), "%d", sched[i].turnaround_time);
                sub.iSubItem = 4; sub.pszText = buf; SendMessageA(hList, LVM_SETITEMTEXTA, (WPARAM)i, (LPARAM)&sub);

                snprintf(buf, sizeof(buf), "%d", sched[i].waiting_time);
                sub.iSubItem = 5; sub.pszText = buf; SendMessageA(hList, LVM_SETITEMTEXTA, (WPARAM)i, (LPARAM)&sub);
            }
        }
        return 0;
    }

    case WM_TIMER: {
        if (wParam == 1001) {
            int total = (int)gProcesses.size();
            int active = 0;
            for (auto &p : gProcesses) if (p.completion_time == 0) ++active;
            int noise = rand() % 20;
            int cpuPercent = (std::min)(100, (int)((double)active / (std::max)(1,total) * 70.0) + noise);

            int ramPercent = 0;
            if (gMM) {
                int inUse = gMM->framesInUse();
                int totalF = gMM->getNumFrames();
                ramPercent = (int)((double)inUse / (std::max)(1,totalF) * 100.0);
            }

            static int tick = 0; tick++;
            int gpuBase = (int)(50 + 30 * sin(tick / 6.28318));
            int gpuPercent = (std::min)(100, (std::max)(0, gpuBase));

            int diskPercent = 10 + (rand() % 40);

            HWND hCpu = GetDlgItem(hwnd, ID_CPU_BAR);
            HWND hGpu = GetDlgItem(hwnd, ID_GPU_BAR);
            HWND hDisk = GetDlgItem(hwnd, ID_DISK_BAR);
            HWND hRam = GetDlgItem(hwnd, ID_RAM_BAR);
            HWND lCpu = GetDlgItem(hwnd, ID_CPU_LABEL);
            HWND lGpu = GetDlgItem(hwnd, ID_GPU_LABEL);
            HWND lDisk = GetDlgItem(hwnd, ID_DISK_LABEL);
            HWND lRam = GetDlgItem(hwnd, ID_RAM_LABEL);
            SendMessageW(hCpu, PBM_SETPOS, cpuPercent, 0);
            SendMessageW(hGpu, PBM_SETPOS, gpuPercent, 0);
            SendMessageW(hDisk, PBM_SETPOS, diskPercent, 0);
            SendMessageW(hRam, PBM_SETPOS, ramPercent, 0);
            char buf[32];
            sprintf_s(buf, "%d%%", cpuPercent); SetWindowTextA(lCpu, buf);
            sprintf_s(buf, "%d%%", gpuPercent); SetWindowTextA(lGpu, buf);
            sprintf_s(buf, "%d%%", diskPercent); SetWindowTextA(lDisk, buf);
            sprintf_s(buf, "%d%%", ramPercent); SetWindowTextA(lRam, buf);
        }
        break;
    }

    case WM_DESTROY:
        if (ghBlackBrush) { DeleteObject(ghBlackBrush); ghBlackBrush = NULL; }
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int runApp()
{
    HINSTANCE hinst = GetModuleHandle(NULL);
    const wchar_t *cls = L"OSMainWindowClass";
    WNDCLASSW wc = {0};
    wc.lpfnWndProc = MainWndProc;
    wc.hInstance = hinst;
    wc.lpszClassName = cls;
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    RegisterClassW(&wc);

    if (!showLoginWindow(hinst)) {
        UnregisterClassW(cls, hinst);
        return 0;
    }

    HWND hwnd = CreateWindowExW(0, cls, L"Simple OS Simulator", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        CW_USEDEFAULT, CW_USEDEFAULT, 700, 760, NULL, NULL, hinst, NULL);
    if (!hwnd) return -1;

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    if (gMM) { delete gMM; gMM = nullptr; }
    UnregisterClassW(cls, hinst);
    return (int)msg.wParam;
}
