#pragma once

#include <sstream>
#include <fstream>
#include <streambuf>
#include <iomanip>

struct Filter {
    std::wstring description;
    std::wstring extensions;
};

int GetOpenPath(HWND hWnd, std::string &path, std::initializer_list<Filter> filters);
int GetSavePath(HWND hWnd, std::string &path, std::initializer_list<Filter> filters);