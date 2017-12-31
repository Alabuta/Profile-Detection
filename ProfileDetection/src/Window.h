#pragma once

#ifndef _UNICODE
#define _UNICODE
#endif

#ifndef  UNICODE
#define  UNICODE
#endif

#include <memory>
#include <string>
#include <unordered_map>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

class Control;

class Window final {
public:

    explicit Window(std::wstring &&name, HINSTANCE hInstance, int w, int h);
    void Destroy();

    template<class T>
    std::weak_ptr<T> AddControl(std::unique_ptr<Control> &&control);

    HWND hWnd() const { return hWnd_; }

    int32_t width() const;
    int32_t height() const;

    static int Update();

private:
    static std::unordered_map<HWND, Window *> windowsTable;

    HWND hWnd_{nullptr};

    std::wstring wndName_{L""};
    std::wstring wndClass_{L""};

    RECT clientRect_{0, 0, 0, 0};

    WORD controlIDs_{0};

    std::unordered_map<WORD, std::shared_ptr<Control>> controls_;

    Window() = delete;
    Window(Window const &) = delete;
    Window(Window &&) = delete;

    LRESULT Process(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    static LRESULT CALLBACK ProcessAllWindows(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};

template<class T>
std::weak_ptr<T> Window::AddControl(std::unique_ptr<Control> &&_control)
{
    auto pair = controls_.insert_or_assign(++controlIDs_, std::move(_control));
    std::get<0>(pair)->second->Instantiate(hWnd_, controlIDs_);

    return std::dynamic_pointer_cast<T>(std::get<0>(pair)->second);
}

inline int32_t Window::width() const
{
    return static_cast<int32_t>(clientRect_.right);
}

inline int32_t Window::height() const
{
    return static_cast<int32_t>(clientRect_.bottom);
}