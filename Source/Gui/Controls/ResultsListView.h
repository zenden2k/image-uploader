#ifndef GUI_CONTROLS_RESULTSLISTVIEW_H
#define GUI_CONTROLS_RESULTSLISTVIEW_H

#include "atlheaders.h"

/**
 ListView with no flickering
**/
class CResultsListView : public CWindowImpl<CResultsListView, CListViewCtrl> {
    public:

    BEGIN_MSG_MAP(CCustomEditControl<T>)
        //MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
        MESSAGE_HANDLER(WM_PAINT, OnPaint)
        MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
    END_MSG_MAP()

    DECLARE_WND_SUPERCLASS(_T("CResultsListView"), CListViewCtrl::GetWndClassName())
    CResultsListView();
    bool AttachToDlgItem(HWND parent, UINT dlgID);
    //LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnEraseBackground(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
}; // end class

#endif