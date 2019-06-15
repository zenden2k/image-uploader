// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "ServersCheckerSettingsDlg.h"

#include "Func/WinUtils.h"
#include "Gui/GuiTools.h"
#include "Core/Utils/CoreUtils.h"
#include "Core/ServiceLocator.h"

namespace ServersListTool {

CSettingsDlg::CSettingsDlg(ServersCheckerSettings* settings, BasicSettings* basicSettings): 
        settings_(settings), 
        basicSettings_(basicSettings) {

}

LRESULT CSettingsDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    DoDataExchange(FALSE);
    serverTypeCombo_.AddString(_T("HTTP"));
    serverTypeCombo_.AddString(_T("SOCKS4"));
    serverTypeCombo_.AddString(_T("SOCKS4A"));
    serverTypeCombo_.AddString(_T("SOCKS5"));
    serverTypeCombo_.AddString(_T("SOCKS5(DNS)"));

    useProxyRadioButton_.SetCheck(settings_->useProxy == ServersCheckerSettings::kUserProxy ? BST_CHECKED : BST_UNCHECKED);
    useSystemProxyRadioButton_.SetCheck(settings_->useProxy == ServersCheckerSettings::kSystemProxy ? BST_CHECKED : BST_UNCHECKED);
    noProxyRadioButton_.SetCheck(settings_->useProxy == ServersCheckerSettings::kNoProxy ? BST_CHECKED : BST_UNCHECKED);

    SetDlgItemInt(IDC_PORTEDIT, settings_->proxyPort);
    SetDlgItemText(IDC_ADDRESSEDIT, U2W(settings_->proxyAddress));
    serverTypeCombo_.SetCurSel(settings_->proxyType);
    proxyTypeChanged();
    return TRUE;
}

LRESULT CSettingsDlg::OnCancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
    EndDialog(IDCANCEL); 
    return 0;
}

LRESULT CSettingsDlg::OnOK(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
    settings_->proxyAddress = W2U(GuiTools::GetWindowText(GetDlgItem(IDC_ADDRESSEDIT)));
    settings_->proxyPort = GetDlgItemInt(IDC_PORTEDIT);
    if (SendDlgItemMessage(IDC_USEPROXYSERVER, BM_GETCHECK) != 0) {
        settings_->useProxy = ServersCheckerSettings::kUserProxy;
    }
    else if (SendDlgItemMessage(IDC_USESYSTEMPROXY, BM_GETCHECK) != 0) {
        settings_->useProxy = ServersCheckerSettings::kSystemProxy;
    }
    else {
        settings_->useProxy = ServersCheckerSettings::kNoProxy;
    }
    settings_->copySettings(basicSettings_);
    EndDialog(IDOK);
    return 0;
}

LRESULT CSettingsDlg::OnClickedProxyType(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
    proxyTypeChanged();
    return 0;
}

void CSettingsDlg::proxyTypeChanged() {
    bool checked = SendDlgItemMessage(IDC_USEPROXYSERVER, BM_GETCHECK) != 0;
    GuiTools::EnableNextN(GetDlgItem(IDC_USEPROXYSERVER), 6, checked);
}


}

