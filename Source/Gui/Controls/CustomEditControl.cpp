#include "CustomEditControl.h"

CCustomEditControl::CCustomEditControl() {
}

bool CCustomEditControl::AttachToDlgItem(HWND parent, UINT dlgID) {

    HWND hWnd = ::GetDlgItem(parent,dlgID);
    return SubclassWindow(hWnd);
}

LRESULT CCustomEditControl::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    WPARAM vk = wParam;
    if ( vk == _T('A') && GetKeyState(VK_CONTROL) & 0x80 ) { // Ctrl + A 
        SendMessage(EM_SETSEL, 0, -1);
    }
    bHandled = false;
    return 0;
}

LRESULT CCustomEditControl::OnPaste(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    bHandled = false;
    if ( !onPaste.empty() ) {
        if ( onPaste(this) ) {
            bHandled = true;
        }
    }

    return 0;
}