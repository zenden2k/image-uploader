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

    int selectedProxyTypeIndex = 0;
    for (size_t i = 0; i < proxyTypes.size(); i++) {
        auto& item = proxyTypes[i];
        if (item.second == settings_->ConnectionSettings.ProxyType) {
            selectedProxyTypeIndex = i;
        }
        int index = serverTypeCombo_.AddString(item.first);
        serverTypeCombo_.SetItemData(index, item.second);
    }
	
    useProxyRadioButton_.SetCheck(settings_->ConnectionSettings.UseProxy == ConnectionSettingsStruct::kUserProxy ? BST_CHECKED : BST_UNCHECKED);
    useSystemProxyRadioButton_.SetCheck(settings_->ConnectionSettings.UseProxy == ConnectionSettingsStruct::kSystemProxy ? BST_CHECKED : BST_UNCHECKED);
    noProxyRadioButton_.SetCheck(settings_->ConnectionSettings.UseProxy == ConnectionSettingsStruct::kNoProxy ? BST_CHECKED : BST_UNCHECKED);

    SetDlgItemInt(IDC_PORTEDIT, settings_->ConnectionSettings.ProxyPort);
    SetDlgItemText(IDC_ADDRESSEDIT, U2W(settings_->ConnectionSettings.ServerAddress));
    serverTypeCombo_.SetCurSel(selectedProxyTypeIndex);
    proxyTypeChanged();
    return TRUE;
}

LRESULT CSettingsDlg::OnCancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
    EndDialog(IDCANCEL); 
    return 0;
}

LRESULT CSettingsDlg::OnOK(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
    settings_->ConnectionSettings.ServerAddress = W2U(GuiTools::GetWindowText(GetDlgItem(IDC_ADDRESSEDIT)));
    settings_->ConnectionSettings.ProxyPort = GetDlgItemInt(IDC_PORTEDIT);
    int proxyTypeIndex = serverTypeCombo_.GetCurSel();
    settings_->ConnectionSettings.ProxyType = serverTypeCombo_.GetItemData(proxyTypeIndex);
    if (SendDlgItemMessage(IDC_USEPROXYSERVER, BM_GETCHECK) != 0) {
        settings_->ConnectionSettings.UseProxy = ConnectionSettingsStruct::kUserProxy;
    }
    else if (SendDlgItemMessage(IDC_USESYSTEMPROXY, BM_GETCHECK) != 0) {
        settings_->ConnectionSettings.UseProxy = ConnectionSettingsStruct::kSystemProxy;
    }
    else {
        settings_->ConnectionSettings.UseProxy = ConnectionSettingsStruct::kNoProxy;
    }
    //settings_->copySettings(basicSettings_);
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

