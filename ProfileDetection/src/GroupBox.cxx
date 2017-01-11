#include "GroupBox.h"


WNDPROC GroupBox::defaultCallbackFunc_{nullptr};

GroupBox::GroupBox(std::wstring _name, int _x, int _y, int _width, int _height) : Control(_name, _x, _y, _width, _height)
{}

HWND GroupBox::Instantiate(HWND const &_parent, unsigned __int64 _id)
{
    static HFONT const hFont = CreateFontW(
        -14, 0, 0, 0,
        FW_NORMAL,
        FALSE, FALSE, FALSE,
        DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY,
        DEFAULT_PITCH,
        L"Segou UI"
    );

    handle_ = CreateWindowExW(0, L"BUTTON", name_.data(), WS_CHILD | WS_VISIBLE | WS_GROUP | BS_GROUPBOX,
        rect_.left, rect_.top, rect_.right, rect_.bottom,
        _parent, reinterpret_cast<HMENU>(_id), (HINSTANCE)GetWindowLongPtrW(_parent, GWLP_HINSTANCE), nullptr);

    controlsTable.insert_or_assign(handle_, this);

    defaultCallbackFunc_ = reinterpret_cast<WNDPROC>(GetWindowLongPtrW(handle_, GWLP_WNDPROC));
    SetWindowLongPtrW(handle_, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(ProcessAll));

    SendMessageW(handle_, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), MAKELPARAM(TRUE, 0));

    GetClientRect(handle_, &clientRect_);

    return handle_;
}

LRESULT GroupBox::HandleMessage(UINT _msg, WPARAM _wParam, LPARAM _lParam)
{
    UNREFERENCED_PARAMETER(_msg);
    UNREFERENCED_PARAMETER(_wParam);
    UNREFERENCED_PARAMETER(_lParam);

    return 0L;
}

LRESULT GroupBox::Process(HWND _hWnd, UINT _msg, WPARAM _wParam, LPARAM _lParam)
{
    switch (_msg) {
        case WM_COMMAND:
            if (HIWORD(_wParam) != 1)
                controls_[LOWORD(_wParam)]->HandleMessage(_msg, _wParam, _lParam);
            return 0L;

        case WM_HSCROLL:
        case WM_VSCROLL:
            controls_[GetDlgCtrlID((HWND)_lParam)]->HandleMessage(_msg, _wParam, _lParam);
            return 0L;

        case WM_SIZE:
            clientRect_.right = LOWORD(_lParam);
            clientRect_.bottom = HIWORD(_lParam);
            return 0L;

        case WM_DESTROY:
            controlsTable.erase(controlsTable.find(handle_));
            break;
    }

    return CallWindowProcW(defaultCallbackFunc_, _hWnd, _msg, _wParam, _lParam);
}