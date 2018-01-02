#include <iostream>

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

#include "Window.h"
#include "Control.h"

#include <WindowsX.h>
#include <CommCtrl.h>
#include <DwmAPI.h>

#pragma comment(lib, "Comctl32.lib")
#pragma comment(lib, "UxTheme.lib")

/*static*/ std::unordered_map<HWND, Window *> Window::windowsTable;

Window::Window(std::wstring &&name, HINSTANCE hInstance, int w, int h) :
    wndName_(std::move(name)), wndClass_(wndName_ + L"Class")
{
    // This method for only start of system and used before splash screen creation.
    HWND const hExistWnd = FindWindowW(wndName_.data(), nullptr);

    if (hExistWnd != nullptr) {
        SetForegroundWindow(hExistWnd);
        ShowWindow(hExistWnd, SW_SHOWNORMAL);

        return;
    }

    _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_ALLOC_MEM_DF | _CRTDBG_DELAY_FREE_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    //_CrtSetBreakAlloc(84);

    WNDCLASSEXW const wcx = {
        sizeof(WNDCLASSEXW),
        0,
        (WNDPROC)Window::ProcessAllWindows,
        0, 0,
        hInstance,
        LoadIconW(hInstance, MAKEINTRESOURCEW(101)),
        LoadCursorW(nullptr, IDC_ARROW),
        /*CreateSolidBrush(RGB(225, 243, 224))*/GetSysColorBrush(COLOR_3DFACE), nullptr,
        wndClass_.data(),
        wcx.hIcon
    };

    if (RegisterClassExW(&wcx) == 0ui16) {
        std::cerr << "Can't register window class.\n";
        return;
    }

    hWnd_ = CreateWindowExW(0, wndClass_.data(), wndName_.data(),
                            WS_POPUP, 0, 0, w, h,
                            nullptr, nullptr, wcx.hInstance, nullptr);

    if (hWnd_ == nullptr) {
        std::cerr << "Can't create window handle.\n";

        UnregisterClassW(wndClass_.data(), GetModuleHandleW(nullptr));
        return;
    }

    windowsTable.insert_or_assign(hWnd_, this);

    ShowWindow(hWnd_, SW_HIDE);

    RECT rc;
    GetWindowRect(hWnd_, &rc);

    {
        // Primary desktop work area rectangle.
        RECT rcArea;
        SystemParametersInfo(SPI_GETWORKAREA, 0, &rcArea, 0);

        // Adjust rectangle of the window to new desktop rectangle.
        rc.right = (rc.right -= rc.left) > (rcArea.right -= rcArea.left) ? rcArea.right : rc.right;
        rc.bottom = (rc.bottom -= rc.top) > (rcArea.bottom -= rcArea.top) ? rcArea.bottom : rc.bottom;

        // The left-top frame corner of the window.
        if (rc.left == 0) rc.left = ((rcArea.right - rc.right) >> 1) + rcArea.left;
        if (rc.top == 0) rc.top = ((rcArea.bottom - rc.bottom) >> 1) + rcArea.top;
    }

    SetWindowLongPtrW(hWnd_, GWL_EXSTYLE, WS_EX_APPWINDOW | WS_EX_WINDOWEDGE | WS_EX_COMPOSITED);
    SetWindowLongPtrW(hWnd_, GWL_STYLE, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX /*| WS_CLIPCHILDREN*/);

    WTA_OPTIONS wta = {
        WTNCA_NODRAWICON | WTNCA_NOSYSMENU,
        WTNCA_NODRAWICON | WTNCA_NOSYSMENU
    };

    SetWindowThemeAttribute(hWnd_, WTA_NONCLIENT, &wta, sizeof(WTA_OPTIONS));

    SetWindowPos(hWnd_, nullptr, rc.left, rc.top, rc.right, rc.bottom, SWP_DRAWFRAME | SWP_NOZORDER);

    ShowWindow(hWnd_, SW_SHOWNORMAL);
    SetForegroundWindow(hWnd_);
}

void Window::Destroy()
{
    if (hWnd_ != nullptr) {
        DestroyWindow(hWnd_);

        EnumChildWindows(hWnd_, [] (HWND hWnd, LPARAM lParam) mutable
        {
            Window::windowsTable.erase(windowsTable.find(hWnd));

            return TRUE;
        }, 0);

        windowsTable.erase(windowsTable.find(hWnd_));

        hWnd_ = nullptr;

        if (windowsTable.empty())
            PostQuitMessage(0);
    }

    UnregisterClassW(wndClass_.data(), GetModuleHandleW(nullptr));
}

/*static*/ int Window::Update()
{
    INITCOMMONCONTROLSEX const icce = {sizeof(INITCOMMONCONTROLSEX), 0xFFFFFFFF};
    InitCommonControlsEx(&icce);

    MSG msg;

#pragma warning(push, 3)
#pragma warning(disable: 4127)
    while (true) {
#pragma warning(pop)
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE | PM_NOYIELD) != 0) {
            if (msg.message == WM_QUIT)
                return static_cast<int>(msg.wParam);

            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }

    return 0;
}

LRESULT Window::Process(HWND _hWnd, UINT _msg, WPARAM _wParam, LPARAM _lParam)
{
    __assume(_msg < WM_APP + 1);

    switch (_msg) {
        case WM_SIZE:
            clientRect_.right = LOWORD(_lParam);
            clientRect_.bottom = HIWORD(_lParam);
            return 0L;

        case WM_HSCROLL:
        case WM_VSCROLL:
            controls1_[GetDlgCtrlID((HWND)_lParam)]->HandleMessage(_msg, _wParam, _lParam);
            return 0L;

        case WM_COMMAND:
            if (HIWORD(_wParam) != 1)
                controls1_[LOWORD(_wParam)]->HandleMessage(_msg, _wParam, _lParam);
            return 0L;

        case WM_PARENTNOTIFY:
            switch (LOWORD(_wParam)) {
                case WM_DESTROY:
                    return 0L;

                case WM_POINTERDOWN:
                    return 0L;
            }
            return 0L;

        case WM_CLOSE:
            Destroy();
            return 0L;

            //case WM_DESTROY:
                //PostQuitMessage(0);
                //return 0L;
    }

    return DefWindowProcW(_hWnd, _msg, _wParam, _lParam);
}

/*static*/ LRESULT CALLBACK Window::ProcessAllWindows(HWND _hWnd, UINT _msg, WPARAM _wParam, LPARAM _lParam)
{
    auto window = windowsTable[_hWnd];

    if (window != nullptr)
        return window->Process(_hWnd, _msg, _wParam, _lParam);

    else
        return DefWindowProcW(_hWnd, _msg, _wParam, _lParam);
}