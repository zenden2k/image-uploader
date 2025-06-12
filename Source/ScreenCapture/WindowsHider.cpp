#include "WindowsHider.h"

#include "Gui/GuiTools.h"

WindowsHider::WindowsHider(HWND hwnd) {
    hwnd_ = hwnd;
    parent_ = ::GetParent(hwnd);
    isVisible_ = ::IsWindowVisible(hwnd);
    if (isVisible_) {
        GuiTools::DisableDwmAnimations(hwnd);
        ::ShowWindow(hwnd, SW_HIDE);
    }

    if (parent_) {
        isParentVisible_ = ::IsWindowVisible(parent_);
        if (isParentVisible_) {
            GuiTools::DisableDwmAnimations(parent_);
        }
        ::ShowWindow(parent_, SW_HIDE);
    }
}

void WindowsHider::show() {
    if (isParentVisible_) {
        ::ShowWindow(parent_, SW_SHOW);
        GuiTools::DisableDwmAnimations(parent_, FALSE);
        isParentVisible_ = false;
    }

    if (isVisible_) {
        ::ShowWindow(hwnd_, SW_SHOW);
        GuiTools::DisableDwmAnimations(hwnd_, FALSE);
        isVisible_ = false;
    }
}

WindowsHider::~WindowsHider() {
    show();
}
