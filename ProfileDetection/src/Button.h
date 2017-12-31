#pragma once

#include <string>
#include <functional>
#include <forward_list>

#include "Window.h"
#include "Control.h"


class Button final : public Control {
public:

    explicit Button(std::wstring name, int x, int y, int width, int height);
    HWND Instantiate(HWND parent, std::uint64_t id) override;

    LRESULT HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) override;

    void AddOnClickListener(std::function<void()> const &listener);

private:
	inline static WNDPROC defaultCallbackFunc_{ nullptr };
    LRESULT Process(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override;

    std::forward_list<std::function<void()>> listeners_;

    void NotifyAllListeners() const;
};