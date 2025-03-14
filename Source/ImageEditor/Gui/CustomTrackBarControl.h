#pragma once

#include "atlheaders.h"

namespace ImageEditor {

class CCustomTrackBarControl :
        public CWindowImpl<CCustomTrackBarControl, CTrackBarCtrl, CControlWinTraits>
{
    public:
    typedef CWindowImpl<CCustomTrackBarControl, CTrackBarCtrl, CControlWinTraits> TBase;

    DECLARE_WND_SUPERCLASS(_T("CCustomTrackBarControl"), CTrackBarCtrl::GetWndClassName())

    BEGIN_MSG_MAP(CCustomTrackBarControl)
        MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
    END_MSG_MAP()

    // Handler prototypes:
    //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

    LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};

}
