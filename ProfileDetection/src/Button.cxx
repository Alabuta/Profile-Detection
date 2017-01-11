#include <random>

#include "Button.h"


WNDPROC Button::defaultCallbackFunc_{nullptr};

Button::Button(std::wstring _name, int _x, int _y, int _width, int _height) : Control(_name, _x, _y, _width, _height)
{}

HWND Button::Instantiate(HWND const &_parent, unsigned __int64 _id)
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

    handle_ = CreateWindowExW(0, L"BUTTON", name_.data(), WS_CHILD | WS_VISIBLE,
        rect_.left, rect_.top, rect_.right, rect_.bottom,
        _parent, reinterpret_cast<HMENU>(_id), (HINSTANCE)GetWindowLongPtrW(_parent, GWLP_HINSTANCE), nullptr);

    controlsTable.insert_or_assign(handle_, this);

    defaultCallbackFunc_ = reinterpret_cast<WNDPROC>(GetWindowLongPtrW(handle_, GWLP_WNDPROC));
    SetWindowLongPtrW(handle_, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(ProcessAll));

    SendMessageW(handle_, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), MAKELPARAM(TRUE, 0));

    return handle_;
}

LRESULT Button::HandleMessage(UINT _msg, WPARAM _wParam, LPARAM _lParam)
{
    UNREFERENCED_PARAMETER(_msg);
    UNREFERENCED_PARAMETER(_wParam);
    UNREFERENCED_PARAMETER(_lParam);

    NotifyAllListeners();

    return 0L;
}

void Button::AddOnClickListener(std::function<void()> const &_listener)
{
    listeners_.emplace_front(_listener);
}

//void Button::RemoveOnClickListener(std::function<void(int)> callback)
//{
//    listeners_.remove_if([&callback] (std::function<void()> &p) {
//        return callback.target<void()>() == p.target<void()>();
//    });
//}

void Button::NotifyAllListeners() const
{
    for (auto const &listener : listeners_)
        listener();
}

LRESULT Button::Process(HWND _hWnd, UINT _msg, WPARAM _wParam, LPARAM _lParam)
{
    return CallWindowProcW(defaultCallbackFunc_, _hWnd, _msg, _wParam, _lParam);
}