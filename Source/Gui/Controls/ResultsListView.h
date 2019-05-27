#ifndef GUI_CONTROLS_RESULTSLISTVIEW_H
#define GUI_CONTROLS_RESULTSLISTVIEW_H

#include "atlheaders.h"

class UploadListModel; 
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
        REFLECTED_NOTIFY_CODE_HANDLER(LVN_GETDISPINFO, OnGetDispInfo)
        //REFLECTED_NOTIFY_CODE_HANDLER(NM_CUSTOMDRAW, OnListViewNMCustomDraw)
    END_MSG_MAP()

    DECLARE_WND_SUPERCLASS(_T("CResultsListView"), CListViewCtrl::GetWndClassName())
    CResultsListView();
    void Init();
    void SetModel(UploadListModel* model);
    bool AttachToDlgItem(HWND parent, UINT dlgID);
    //LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnEraseBackground(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnGetDispInfo(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
    LRESULT OnListViewNMCustomDraw(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
protected:
    void CreateDoubleBuffer();
    void onRowChanged(size_t index);
    CDC dcMem_;
    CBitmap bmMem_;
    HBITMAP bmpOld_;
    UploadListModel* model_;
}; // end class

#endif