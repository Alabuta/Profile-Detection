#include <random>

#include "Trackbar.h"

#include <CommCtrl.h>
#include <WindowsX.h>



Trackbar::Trackbar(std::wstring _name, int _x, int _y, int _width, int _height, int _min, int _max) : Control(_name, _x, _y, _width, _height)
{
    min_ = _min;
    max_ = _max;
}

HWND Trackbar::Instantiate(HWND _parent, std::uint64_t _id)
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

    handle_ = CreateWindowExW(0, TRACKBAR_CLASS, name_.data(), WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS | TBS_HORZ | TBS_TOOLTIPS,
        rect_.left, rect_.top, rect_.right, rect_.bottom,
        _parent, reinterpret_cast<HMENU>(_id), reinterpret_cast<HINSTANCE>(GetWindowLongPtrW(_parent, GWLP_HINSTANCE)), nullptr);

    controlsTable.insert_or_assign(handle_, this);

    defaultCallbackFunc_ = reinterpret_cast<WNDPROC>(GetWindowLongPtrW(handle_, GWLP_WNDPROC));
    SetWindowLongPtrW(handle_, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(ProcessAll));

    SendMessageW(handle_, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), MAKELPARAM(TRUE, 0));

    SendMessageW(handle_, TBM_SETRANGE, TRUE, MAKELPARAM(min_, max_));
    SendMessageW(handle_, TBM_SETPAGESIZE, 0, 1);

    return handle_;
}

void Trackbar::AddOnChangeListener(std::function<void(int)> _listener)
{
    listeners_.emplace_front(_listener);
}

void Trackbar::NotifyAllListeners() const
{
    for (auto &&listener : listeners_)
        listener(value_);
}

LRESULT Trackbar::HandleMessage(UINT _msg, WPARAM _wParam, LPARAM _lParam)
{
    switch (_msg) {
        case WM_HSCROLL:
        case WM_VSCROLL:
            switch (LOWORD(_wParam)) {
                case TB_BOTTOM:
                case TB_ENDTRACK:
                case TB_LINEDOWN:
                case TB_LINEUP:
                case TB_PAGEDOWN:
                case TB_PAGEUP:
                case TB_TOP:
                    value_ = static_cast<int>(SendMessage(handle_, TBM_GETPOS, 0, 0));
                    NotifyAllListeners();
                    break;

                case TB_THUMBPOSITION:
                    value_ = static_cast<int>(HIWORD(_wParam));
                    NotifyAllListeners();
                    break;
            }
    }

    return 0L;
}

LRESULT Trackbar::Process(HWND _hWnd, UINT _msg, WPARAM _wParam, LPARAM _lParam)
{
    return CallWindowProcW(defaultCallbackFunc_, _hWnd, _msg, _wParam, _lParam);
}