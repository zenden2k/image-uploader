#ifndef GUI_CONTROLS_CUSTOMEDITCONTROL_H
#define GUI_CONTROLS_CUSTOMEDITCONTROL_H

#include "atlheaders.h"
#include "Core/3rdpart/FastDelegate.h"

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
    fastdelegate::FastDelegate1<CCustomEditControl*,bool> onPaste;
}; // end class

#endif