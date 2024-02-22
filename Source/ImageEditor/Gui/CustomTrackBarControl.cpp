#include "CustomTrackBarControl.h"

namespace ImageEditor {

LRESULT CCustomTrackBarControl::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    bHandled = FALSE;

    // For user convenience, we make these keys [ ] to behave like arrow keys
    // when trackbar control is focused (to change brush size)
    if (wParam == VK_OEM_4) { // '['
        bHandled = TRUE;
        DefWindowProc(uMsg, VK_LEFT, lParam);
    } else if (wParam == VK_OEM_6) { // ']'
        bHandled = TRUE;
        DefWindowProc(uMsg, VK_RIGHT, lParam);
    }
    return 0;
}

}
