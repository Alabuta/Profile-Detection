#include <algorithm>
#include <cctype>
#include <cwctype>

#include <WindowsX.h>

#include "Edit.h"


Edit::Edit(std::wstring _name, int _x, int _y, int _width, int _height, int _lineCount) : Control(_name, _x, _y, _width, _height)
{
    line_ = _name;
    lineCount_ = _lineCount;

    line_.resize(lineCount_);
}

HWND Edit::Instantiate(HWND _parent, std::uint64_t _id)
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

    handle_ = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", name_.data(), WS_CHILD | WS_VISIBLE | ES_LEFT | ES_NUMBER | WS_TABSTOP,
        rect_.left, rect_.top, rect_.right, rect_.bottom,
        _parent, reinterpret_cast<HMENU>(_id), reinterpret_cast<HINSTANCE>(GetWindowLongPtrW(_parent, GWLP_HINSTANCE)), nullptr);

    controlsTable.insert_or_assign(handle_, this);

    defaultCallbackFunc_ = reinterpret_cast<WNDPROC>(GetWindowLongPtrW(handle_, GWLP_WNDPROC));
    SetWindowLongPtrW(handle_, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(ProcessAll));

    SendMessageW(handle_, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), MAKELPARAM(TRUE, 0));

    Edit_LimitText(handle_, lineCount_);

    return handle_;
}

void Edit::AddOnChangeListener(std::function<void(std::wstring const &)> const &_listener)
{
    listeners_.emplace_front(_listener);
}

void Edit::NotifyAllListeners() const
{
    for (auto &&listener : listeners_)
        listener(line_);
}

std::wstring const &Edit::line(std::wstring &&_line)
{
    _line.push_back(L'\0');
    Edit_ReplaceSel(handle_, _line.c_str());

    return line_ = _line;
}

LRESULT Edit::HandleMessage(UINT _msg, WPARAM _wParam, LPARAM _lParam)
{
    UNREFERENCED_PARAMETER(_msg);
    UNREFERENCED_PARAMETER(_lParam);

    switch (HIWORD(_wParam)) {
        case EN_UPDATE:
            std::wstring line;
            line.swap(line_);

            auto number = Edit_GetLine(handle_, 0, line.c_str(), lineCount_);

            if (number > 0)
                line_.assign(line.c_str(), number);

            /*std::wstring::iterator it = line_.begin();

            while (it != line_.end()) {
                it = line_.erase(std::find_if(line_.begin(), line_.end(), [] (wchar_t ch) {
                    return std::iswdigit(ch) == 0 ? true : false;
                }));
            }*/

            if (number > 0)
                NotifyAllListeners();
    }

    return 0L;
}

LRESULT Edit::Process(HWND _hWnd, UINT _msg, WPARAM _wParam, LPARAM _lParam)
{
    return CallWindowProcW(defaultCallbackFunc_, _hWnd, _msg, _wParam, _lParam);
}