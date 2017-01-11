#include <memory>
#include <array>
#include <locale>
#include <codecvt>

#ifndef _UNICODE
#define _UNICODE
#endif

#ifndef  UNICODE
#define  UNICODE
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <CommCtrl.h>
#include <Commdlg.h>

#include "IORoutines.h"


int GetPath(HWND const &_hWnd, std::string &_path, std::initializer_list<Filter> _filters, DWORD _flags)
{
    std::wstring filters;

    for (auto const &filter : _filters) {
        filters += filter.description + L" (" + filter.extensions + L")";
        filters.push_back(L'\0');
        filters += filter.extensions;
        filters.push_back(L'\0');
    }

    std::array<wchar_t, 512> path{L"\0"};

    static DWORD nFilterIndex = 1;

    OPENFILENAMEW ofn = {
        sizeof(OPENFILENAMEW),
        _hWnd,
        GetModuleHandleW(nullptr),
        filters.data(),
        nullptr, 0,
        nFilterIndex,
        path.data(), static_cast<DWORD>(path.size()),
        nullptr, 0,
        nullptr, nullptr,
        _flags,
        0, 0,
        nullptr, 0, nullptr, nullptr,
        nullptr, 0, 0
    };

    if (GetOpenFileNameW(&ofn) == 0)
        return -1;

    nFilterIndex = ofn.nFilterIndex;

    std::array<char, 512> cpath{"\0"};

    WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, path.data(), static_cast<int>(path.size()), cpath.data(), static_cast<int>(cpath.size()), nullptr, FALSE);

    _path.resize(cpath.size());
    std::copy(cpath.begin(), cpath.end(), _path.begin());

    return ofn.nFilterIndex;
}

int GetOpenPath(HWND const &_hWnd, std::string &_path, std::initializer_list<Filter> _filters)
{
    return GetPath(_hWnd, _path, _filters, OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST);
}

int GetSavePath(HWND const &_hWnd, std::string &_path, std::initializer_list<Filter> _filters)
{
    return GetPath(_hWnd, _path, _filters, OFN_OVERWRITEPROMPT);
}