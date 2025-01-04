#pragma once

#include <map>

#include "atlheaders.h"
#include "resource.h"
#include "Func/MyEngineList.h"
#include "Core/TaskDispatcher.h"
#include "Gui/Models/NetworkDebugModel.h"
#include "Gui/Controls/NetworkDebugListView.h"
#include "Core/Upload/UploadEngine.h"
//#include "Gui/Dialogs/WizardDlg.h"

class WtlGuiSettings;

class CNetworkDebugDlg : public CDialogImpl<CNetworkDebugDlg>, public CDialogResize<CNetworkDebugDlg>, public CWinDataExchange<CNetworkDebugDlg> {
public:
    enum { IDD = IDD_NETWORKDEBUGDLG };
    /* enum {
        ID_COPYDIRECTURL = 13000, ID_COPYTHUMBURL, ID_COPYVIEWURL
    };*/
    CNetworkDebugDlg();

    BEGIN_MSG_MAP(CNetworkDebugDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
        COMMAND_ID_HANDLER(IDOK, OnOK)
        COMMAND_ID_HANDLER(IDC_BUTTONSKIP, OnSkip)
        COMMAND_ID_HANDLER(IDC_BUTTONSKIPALL, OnSkipAll)
        COMMAND_ID_HANDLER(IDC_IGNORE, OnIgnore)
        COMMAND_ID_HANDLER(IDC_IGNOREALL, OnIgnoreAll)
        COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
        COMMAND_ID_HANDLER(IDC_ERRORLOGBUTTON, OnErrorLogButtonClicked)
        NOTIFY_CODE_HANDLER(LVN_ITEMCHANGED, OnListViewItemChanged)
        CHAIN_MSG_MAP(CDialogResize<CNetworkDebugDlg>)
        REFLECT_NOTIFICATIONS()   
    END_MSG_MAP()

    BEGIN_DLGRESIZE_MAP(CNetworkDebugDlg)
        DLGRESIZE_CONTROL(IDC_FILELIST, DLSZ_SIZE_X | DLSZ_SIZE_Y)
        DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_X | DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_BUTTONSKIP, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_BUTTONSKIPALL, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_IGNORE, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_IGNOREALL, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDCANCEL, DLSZ_MOVE_X | DLSZ_MOVE_Y)
    END_DLGRESIZE_MAP()

    BEGIN_DDX_MAP(CNetworkDebugDlg)
        DDX_CONTROL(IDC_FILELIST, listView_)
       // DDX_CONTROL(IDC_ANIMATIONSTATIC, loadingAnimation_)
    END_DDX_MAP()

    // Handler prototypes (uncomment arguments if needed):
    //    LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    //    LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    //    LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    //LRESULT OnTaskDispatcherMsg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnSkip(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    
    LRESULT OnErrorLogButtonClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnSkipAll(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnIgnore(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnIgnoreAll(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnListViewItemChanged(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
    //LRESULT OnListViewNMCustomDraw(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

private:
    int contextMenuItemId;

    NetworkDebugModel model_;
    CNetworkDebugListView listView_;

    CIcon icon_, iconSmall_;

    FileFormatCheckResult result_;
    CFont labelBoldFont_;

    /**
     * @throws ValidationException
     */
    void validateSettings();

    void enableButtons();
};

