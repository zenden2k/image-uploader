#ifndef IU_CORE_PROGRAMWINDOW_H
#define IU_CORE_PROGRAMWINDOW_H

#pragma once

#ifdef IU_WTL
    typedef HWND WindowHandle;
#elif defined(IU_QT)
    class QWidget;
    typedef QWidget* WindowHandle;
#else
    typedef int WindowHandle;
#endif

#ifdef _WIN32
    typedef HWND WindowNativeHandle;
#elif defined(IU_QT)
    class QWidget;
    typedef QWidget* WindowHandle;
#else
    typedef int WindowHandle;
#endif

class IProgramWindow {
public:
    virtual WindowHandle getHandle() = 0;
    virtual WindowNativeHandle getNativeHandle() = 0;
    virtual void setServersChanged(bool changed) = 0;
};

#endif
