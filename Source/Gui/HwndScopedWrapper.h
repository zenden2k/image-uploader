#ifndef IU_GUI_HWNDSCOPEDWRAPPER_H
#define IU_GUI_HWNDSCOPEDWRAPPER_H

#pragma once 

#include <windows.h>

class HwndScopedWrapper {
public:
    HwndScopedWrapper() {
        handle_ = nullptr;
    }

    HwndScopedWrapper(HWND handle) {
        handle_ = handle;
    }

    HwndScopedWrapper& operator=(HWND handle) {
        if (handle_ && handle_ != handle) {
            ::DestroyWindow(handle_);
        }
        handle_ = handle;
        return *this;
    }
    HwndScopedWrapper& operator=(const HwndScopedWrapper& other) = delete;
    void detach() {
        handle_ = nullptr;
    }

    operator HWND() {
        return handle_;
    }

    ~HwndScopedWrapper() {
        if (handle_) {
            ::DestroyWindow(handle_);
        }
    }
protected:
    HWND handle_;

};

#endif 