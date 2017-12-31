#pragma once

#include <string>
#include <forward_list>

#include "Window.h"
#include "Control.h"


class Edit final : public Control {
public:

    explicit Edit(std::wstring name, int x, int y, int width, int height, int lineCount = 3);
    HWND Instantiate(HWND parent, std::uint64_t id) override;

    void AddOnChangeListener(std::function<void(std::wstring const &)> const &listener);

    LRESULT HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) override;

    std::wstring const &line() const;
    std::wstring const &line(std::wstring &&line);

private:
    std::wstring line_;
    int lineCount_;

    std::forward_list<std::function<void(std::wstring const &)>> listeners_;

    void NotifyAllListeners() const;

	inline static WNDPROC defaultCallbackFunc_{ nullptr };
    LRESULT Process(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override;

};

inline std::wstring const &Edit::line() const
{
    return line_;
}