#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "Window.h"
#include "Control.h"


class GroupBox final : public Control {
public:

    explicit GroupBox(std::wstring name, int x, int y, int width, int height);
    HWND Instantiate(HWND parent, std::uint64_t id) override;

    template<class T>
    std::weak_ptr<T> AddControl(std::unique_ptr<Control> &&control);

    int32_t width() const;
    int32_t height() const;

    LRESULT HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) override;

private:
	inline static WNDPROC defaultCallbackFunc_{ nullptr };

    RECT clientRect_{0, 0, 0, 0};

    WORD controlIDs_{0};
    std::unordered_map<WORD, std::shared_ptr<Control>> controls_;

    LRESULT Process(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override;
};

template<class T>
std::weak_ptr<T> GroupBox::AddControl(std::unique_ptr<Control> &&_control)
{
    auto pair = controls_.insert_or_assign(++controlIDs_, std::move(_control));
    std::get<0>(pair)->second->Instantiate(handle_, controlIDs_);

    return std::dynamic_pointer_cast<T>(std::get<0>(pair)->second);
}

inline int32_t GroupBox::width() const
{
    return static_cast<int32_t>(clientRect_.right);
}

inline int32_t GroupBox::height() const
{
    return static_cast<int32_t>(clientRect_.bottom);
}