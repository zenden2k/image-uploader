#include "DarkMode.h"

#include <winrt/Windows.UI.ViewManagement.h>

using namespace winrt;
using namespace Windows::UI::ViewManagement;

inline bool IsColorLight(Windows::UI::Color& clr)
{
    return (((5 * clr.G) + (2 * clr.R) + clr.B) > (8 * 128));
}

COLORREF DarkModeHelper::GetBackgroundColor() {
    UISettings const ui_settings{};
    auto const accent_color{ ui_settings.GetColorValue(UIColorType::Background) };
    return RGB(accent_color.R, accent_color.G, accent_color.B);
}

HBRUSH DarkModeHelper::GetBackgroundBrush() {
    if (!backgroundBrush_.m_hBrush) {
        backgroundBrush_.CreateSolidBrush(GetBackgroundColor());
    }
    return backgroundBrush_;
}
