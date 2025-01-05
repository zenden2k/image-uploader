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

class CNetworkDebugDlg : public CDialogImpl<CNetworkDebugDlg>,
    public CDialogResize<CNetworkDebugDlg>,
    public CWinDataExchange<CNetworkDebugDlg>,
    public CMessageFilter
{
public:
    enum { IDD = IDD_NETWORKDEBUGDLG };
    /* enum {
        ID_COPYDIRECTURL = 13000, ID_COPYTHUMBURL, ID_COPYVIEWURL
    };*/
    CNetworkDebugDlg();

    BEGIN_MSG_MAP(CNetworkDebugDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
        COMMAND_ID_HANDLER(IDC_ERRORLOGBUTTON, OnErrorLogButtonClicked)
        NOTIFY_CODE_HANDLER(LVN_ITEMCHANGED, OnListViewItemChanged)
        MESSAGE_HANDLER(WM_CLOSE, OnClose)
        CHAIN_MSG_MAP(CDialogResize<CNetworkDebugDlg>)
        REFLECT_NOTIFICATIONS()   
    END_MSG_MAP()

    BEGIN_DLGRESIZE_MAP(CNetworkDebugDlg)
        BEGIN_DLGRESIZE_GROUP()
            DLGRESIZE_CONTROL(IDC_DEBUGLIST, DLSZ_SIZE_X | DLSZ_SIZE_Y)
            DLGRESIZE_CONTROL(IDC_DEBUGDETAILS, DLSZ_SIZE_X | DLSZ_SIZE_Y)
        END_DLGRESIZE_GROUP()
    END_DLGRESIZE_MAP()

    BEGIN_DDX_MAP(CNetworkDebugDlg)
        DDX_CONTROL(IDC_DEBUGLIST, listView_)
        DDX_CONTROL_HANDLE(IDC_DEBUGDETAILS, detailsEdit_)
    END_DDX_MAP()

    // Handler prototypes (uncomment arguments if needed):
    //    LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    //    LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    //    LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnErrorLogButtonClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

    LRESULT OnListViewItemChanged(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);

    BOOL PreTranslateMessage(MSG* pMsg) override;
    //LRESULT OnListViewNMCustomDraw(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

private:
    int contextMenuItemId;

    NetworkDebugModel model_;
    CNetworkDebugListView listView_;
    CEdit detailsEdit_;
    CIcon icon_, iconSmall_;
    
    CFont detailsEditFont_;
};

