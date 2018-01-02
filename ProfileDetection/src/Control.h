#pragma once

#include <string>
#include <functional>
#include <forward_list>

#include "Window.h"

class Control {
public:

    virtual ~Control() = default;
    virtual HWND Instantiate(HWND parent, std::uint64_t id) = 0;

    virtual LRESULT HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) = 0;
    virtual LRESULT Process(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) = 0;

    void SetName(std::wstring &&name);
    void SetRect(RECT &&name);

    HWND handle() const { return handle_; }
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

    inline static std::unordered_map<HWND, Control *> controlsTable;

    static LRESULT CALLBACK ProcessAll(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};

class Control1 {
public:

	explicit Control1(std::wstring name, int x, int y, int width, int height);

	virtual LRESULT HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) = 0;
	virtual LRESULT Process(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) = 0;

	template<class T, typename std::enable_if_t<std::is_same_v<std::decay_t<T>, std::basic_string<wchar_t>>>...>
	void SetName(T &&name);

	template<class T, typename std::enable_if_t<std::is_same_v<std::decay_t<T>, RECT>>...>
	void SetRectangle(T &&rect);

	HWND handle() const { return handle_; }
	std::wstring const &name() const { return name_; }

	RECT const &rect() const { return rect_; }

protected:
	HWND handle_;
	std::wstring name_;

	RECT rect_{8, 8, 16, 16};

	inline static std::unordered_map<HWND, Control1 *> controlsTable;

	static LRESULT CALLBACK ProcessAll(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:

	Control1() = delete;
	Control1(Control1 &&) = delete;
	Control1(Control1 const &) = delete;
};

template<class T, typename std::enable_if_t<std::is_same_v<std::decay_t<T>, std::basic_string<wchar_t>>>...>
void Control1::SetName(T &&_name)
{
	name_ = std::forward(_name);
	SetWindowTextW(handle_, name_.data());
}

template<class T, typename std::enable_if_t<std::is_same_v<std::decay_t<T>, RECT>>...>
void Control1::SetRectangle(T &&rect)
{
	rect_ = std::forward(rect);
}