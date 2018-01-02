#include "Window.h"
#include "Control.h"


Control::Control(std::wstring _name, int _x, int _y, int _width, int _height) :
	name_(_name), rect_(std::move(RECT{ _x, _y, _width, _height }))
{}

void Control::SetName(std::wstring &&_name)
{
    name_ = std::move(_name);
    SetWindowTextW(handle_, name_.data());
}

void Control::SetRect(RECT &&_name)
{
    rect_ = std::move(_name);
}

LRESULT CALLBACK Control::ProcessAll(HWND _hWnd, UINT _msg, WPARAM _wParam, LPARAM _lParam)
{
    auto control = controlsTable[_hWnd];

    if (control != nullptr)
        return control->Process(_hWnd, _msg, _wParam, _lParam);

    else
        return DefWindowProcW(_hWnd, _msg, _wParam, _lParam);
}


Control1::Control1(std::wstring _name, int _x, int _y, int _width, int _height) :
	name_(_name), rect_(RECT{ _x, _y, _width, _height })
{}

LRESULT CALLBACK Control1::ProcessAll(HWND _hWnd, UINT _msg, WPARAM _wParam, LPARAM _lParam)
{
	auto control = controlsTable[_hWnd];

	if (control != nullptr)
		return control->Process(_hWnd, _msg, _wParam, _lParam);

	else
		return DefWindowProcW(_hWnd, _msg, _wParam, _lParam);

}