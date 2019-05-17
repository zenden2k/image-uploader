#ifndef GUI_CONTROLS_RESULTSLISTVIEW_H
#define GUI_CONTROLS_RESULTSLISTVIEW_H

#include "atlheaders.h"

/**
 ListView with no flickering
**/
class CResultsListView : public CWindowImpl<CResultsListView, CListViewCtrl> {
    public:

    BEGIN_MSG_MAP(CResultsListView)
        //MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
        MESSAGE_HANDLER(WM_PAINT, OnPaint)
        MESSAGE_HANDLER(WM_SIZE, OnSize)
        MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
    END_MSG_MAP()

    DECLARE_WND_SUPERCLASS(_T("CResultsListView"), CListViewCtrl::GetWndClassName())
    CResultsListView();
    void Init();
    bool AttachToDlgItem(HWND parent, UINT dlgID);
    //LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnEraseBackground(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
protected:
    void CreateDoubleBuffer();
    CDC dcMem_;
    CBitmap bmMem_;
    HBITMAP bmpOld_;
}; // end class

#endif