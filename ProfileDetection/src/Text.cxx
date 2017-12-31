#include <random>

#include "Text.h"



Text::Text(std::wstring _name, int _x, int _y, int _width, int _height, eALIGN _align) : Control(_name, _x, _y, _width, _height)
{
    align_ = _align;
}

HWND Text::Instantiate(HWND _parent, std::uint64_t _id)
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

    auto const alignStyle = align_ == eALIGN::nLEFT ? SS_LEFT : (align_ == eALIGN::nCENTER ? SS_CENTER : SS_RIGHT);

    handle_ = CreateWindowExW(0, L"STATIC", name_.data(), WS_CHILD | WS_VISIBLE | alignStyle,
        rect_.left, rect_.top, rect_.right, rect_.bottom,
        _parent, reinterpret_cast<HMENU>(_id), (HINSTANCE)GetWindowLongPtrW(_parent, GWLP_HINSTANCE), nullptr);

    controlsTable.insert_or_assign(handle_, this);

    defaultCallbackFunc_ = reinterpret_cast<WNDPROC>(GetWindowLongPtrW(handle_, GWLP_WNDPROC));
    SetWindowLongPtrW(handle_, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(ProcessAll));

    SendMessageW(handle_, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), MAKELPARAM(TRUE, 0));

    return handle_;
}

LRESULT Text::HandleMessage(UINT _msg, WPARAM _wParam, LPARAM _lParam)
{
    switch (_msg) {
        case WM_CTLCOLORSTATIC:
            RECT rect;
            GetClientRect(handle_, &rect);
            InvalidateRect(GetParent(handle_), &rect, FALSE);

            /*MapWindowPoints(handle_, GetParent(handle_), (POINT *)&rect, 2);
            RedrawWindow(GetParent(handle_), &rect, NULL, RDW_ERASE | RDW_INVALIDATE);*/

            SetBkMode(reinterpret_cast<HDC>(_wParam), TRANSPARENT);

            //DrawTextW(reinterpret_cast<HDC>(_wParam), name_.data(), -1, &rect_, DT_LEFT);

            //return reinterpret_cast<LRESULT>(CreateSolidBrush(RGB(225, 243, 224)));
            return reinterpret_cast<LRESULT>(GetStockObject(NULL_BRUSH));
    }

    return 0L;
}

LRESULT Text::Process(HWND _hWnd, UINT _msg, WPARAM _wParam, LPARAM _lParam)
{
    return CallWindowProcW(defaultCallbackFunc_, _hWnd, _msg, _wParam, _lParam);
}