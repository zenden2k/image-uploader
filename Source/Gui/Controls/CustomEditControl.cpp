#include "CustomEditControl.h"

bool CCustomEditControl::AttachToDlgItem(HWND parent, int dlgID) {
    HWND hWnd = ::GetDlgItem(parent,dlgID);
    return SubclassWindow(hWnd)!= FALSE;
}

void CCustomEditControl::setOnPasteCallback(PasteCallback callback) {
    onPasteCallback_ = std::move(callback);
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
    if ( onPasteCallback_ ) {
        if (onPasteCallback_(this) ) {
            bHandled = true;
        }
    }

    return 0;
}
