#include "DialogIndirect.h"

#include "3rdpart/DarkMode.h"

LRESULT Dialog_OnColorDialog(HWND hWnd, HDC hdc, HWND hwndChild) {
    if (DarkModeHelper::instance()->g_darkModeEnabled) {
        COLORREF color = DarkModeHelper::instance()->GetBackgroundColor();
        HBRUSH br = DarkModeHelper::instance()->GetBackgroundBrush();
        SetClassLongPtr(hWnd, GCL_HBRBACKGROUND, /*(LONG_PTR)GetStockObject(BLACK_BRUSH)*/(LONG_PTR)br);
        SetTextColor(hdc, RGB(255, 255, 255));
        SetBkColor(hdc, color);
        return reinterpret_cast<LRESULT>(br);
    }
    SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));
    SetBkColor(hdc, GetSysColor(COLOR_BTNFACE));
    return reinterpret_cast<LRESULT>(GetSysColorBrush(COLOR_BTNFACE));
}
