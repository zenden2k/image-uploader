
#pragma once
#include "atlheaders.h"

class FileFormatCheckErrorModel;

class CFileFormatErrorView : public CWindowImpl<CFileFormatErrorView, CListViewCtrl> {
public:
    CFileFormatErrorView(FileFormatCheckErrorModel* model);
    ~CFileFormatErrorView() = default;
    DECLARE_WND_SUPERCLASS(_T("CFileFormatErrorView"), CListViewCtrl::GetWndClassName())

    BEGIN_MSG_MAP(CFileFormatErrorView)
        REFLECTED_NOTIFY_CODE_HANDLER(LVN_GETDISPINFO, OnGetDispInfo)
        REFLECTED_NOTIFY_CODE_HANDLER(NM_CUSTOMDRAW, OnListViewNMCustomDraw)
        //REFLECTED_NOTIFY_CODE_HANDLER(LVN_DELETEITEM, OnDeleteItem)
        //REFLECTED_NOTIFY_CODE_HANDLER(LVN_DELETEALLITEMS, OnDeleteItem)
        //REFLECTED_NOTIFY_CODE_HANDLER(LVN_ITEMCHANGED, OnItemChanged)
    END_MSG_MAP()

    void Init();
    // Handler prototypes:
    //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
    LRESULT OnGetDispInfo(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
    LRESULT OnListViewNMCustomDraw(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
protected:
    FileFormatCheckErrorModel* model_;
    void onRowChanged(size_t index);
};



