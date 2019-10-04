#ifndef GUI_CONTROLS_CUSTOMEDITCONTROL_H
#define GUI_CONTROLS_CUSTOMEDITCONTROL_H

#include <functional>

#include "atlheaders.h"

//Custom edit control which handles Ctrl+A hotkey
class CCustomEditControl : public CWindowImpl<CCustomEditControl, CEdit> {
    public:

    BEGIN_MSG_MAP(CCustomEditControl<T>)
        MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
        MESSAGE_HANDLER(WM_PASTE, OnPaste)
    END_MSG_MAP()

    DECLARE_WND_SUPERCLASS(_T("CCustomEditControl"), CCustomEditControl::GetWndClassName())
    CCustomEditControl() = default;
    LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnPaste(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    bool AttachToDlgItem(HWND parent, UINT dlgID);
    using PasteCallback = std::function<bool(CCustomEditControl*)>;
    void setOnPasteCallback(PasteCallback callback);
protected:
    PasteCallback onPasteCallback_;
}; // end class

#endif