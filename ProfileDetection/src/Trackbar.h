#pragma once

#include <string>
#include <functional>
#include <forward_list>

#include "Window.h"
#include "Control.h"


class Trackbar final : public Control {
public:

    explicit Trackbar(std::wstring name, int x, int y, int width, int height, int min, int max);
    virtual HWND Instantiate(HWND const &parent, unsigned __int64 id) override;

    void AddOnChangeListener(std::function<void(int)> const &listener);

    virtual LRESULT HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) override;

    int value() const;

private:
    int min_, max_;
    int value_;

    std::forward_list<std::function<void(int)>> listeners_;

    void NotifyAllListeners() const;

    static WNDPROC defaultCallbackFunc_;
    LRESULT Process(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override;
};

inline int Trackbar::value() const
{
    return value_;
}