#pragma once

#include <string>
#include <functional>
#include <forward_list>

#include "Window.h"

class Control {
public:

    virtual ~Control() = default;
    virtual HWND Instantiate(HWND const &parent, unsigned __int64 id) = 0;

    virtual LRESULT HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) = 0;
    virtual LRESULT Process(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) = 0;

    void SetName(std::wstring &&name);
    void SetRect(RECT &&name);

    HWND const &handle() const { return handle_; }
    std::wstring const &name() const { return name_; }

    RECT const &rect() const { return rect_; }

protected:
    HWND handle_;
    std::wstring name_;

    RECT rect_;

    explicit Control(std::wstring name, int x, int y, int width, int height);

    Control() = delete;
    Control(Control &&) = delete;
    Control(Control const &) = delete;

    static std::unordered_map<HWND, Control *> controlsTable;

    static LRESULT CALLBACK ProcessAll(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};