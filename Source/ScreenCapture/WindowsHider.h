#pragma once

#include <windows.h>

class WindowsHider {
    HWND hwnd_ {}, parent_ {};
    bool isVisible_ = false, isParentVisible_ = false;

public:
    WindowsHider(HWND hwnd);
	void show();
    ~WindowsHider();
};
