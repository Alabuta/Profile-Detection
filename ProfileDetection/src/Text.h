#pragma once

#include <string>
#include <functional>
#include <forward_list>

#include "Window.h"
#include "Control.h"


class Text final : public Control {
public:
    enum class eALIGN {
        nLEFT = 0, nCENTER, nRIGHT
    };

    explicit Text(std::wstring name, int x, int y, int width, int height, eALIGN align);
    virtual HWND Instantiate(HWND const &parent, unsigned __int64 id) override;

    virtual LRESULT HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) override;

private:
    eALIGN align_;

    static WNDPROC defaultCallbackFunc_;
    LRESULT Process(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override;
};