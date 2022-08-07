
#include <map>
#include "atlheaders.h"
#include "resource.h"

#include "ServersCheckerSettings.h"

namespace ServersListTool {


class CSettingsDlg :
    public CDialogImpl<CSettingsDlg>, public CWinDataExchange<CSettingsDlg> {
public:
    enum { IDD = IDD_SETTINGSDLG};
    
    CSettingsDlg(ServersCheckerSettings* settings, BasicSettings* basicSettings);
    BEGIN_MSG_MAP(CSettingsDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
        COMMAND_ID_HANDLER(IDOK, OnOK)
        COMMAND_HANDLER(IDC_USEPROXYSERVER, BN_CLICKED, OnClickedProxyType)
        COMMAND_HANDLER(IDC_NOPROXY, BN_CLICKED, OnClickedProxyType)
        COMMAND_HANDLER(IDC_USESYSTEMPROXY, BN_CLICKED, OnClickedProxyType)
    END_MSG_MAP()

    BEGIN_DDX_MAP(CSettingsDlg)
        DDX_CONTROL_HANDLE(IDC_SERVERTYPECOMBO, serverTypeCombo_)
        DDX_CONTROL_HANDLE(IDC_USEPROXYSERVER, useProxyRadioButton_)
        DDX_CONTROL_HANDLE(IDC_USESYSTEMPROXY, useSystemProxyRadioButton_)
        DDX_CONTROL_HANDLE(IDC_NOPROXY, noProxyRadioButton_)
    END_DDX_MAP()

    // Handler prototypes (uncomment arguments if needed):
    //    LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    //    LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    //    LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnCancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnOK(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnClickedProxyType(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    void proxyTypeChanged();
    CComboBox serverTypeCombo_;
    ServersCheckerSettings* settings_;
    BasicSettings* basicSettings_;
    CButton useProxyRadioButton_, useSystemProxyRadioButton_, noProxyRadioButton_;
    std::vector<std::pair<CString, int>> proxyTypes = {
        { _T("HTTP"), 0 },
        { _T("HTTPS"), 5 },
        { _T("SOCKS4"), 1 },
        { _T("SOCKS4A"), 2 },
        { _T("SOCKS5"), 3 },
        { _T("SOCKS5(DNS)"), 4 },
    };
};

}
