/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2018 Sergey Svistunov (zenden2k@gmail.com)

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

*/

#include "UploadSettingsPage.h"

#include "Core/CommonDefs.h"
#include "WizardDlg.h"
#include "Gui/GuiTools.h"
#include "Func/WinUtils.h"
#include "Gui/Components/MyFileDialog.h"
#include "Core/Settings/WtlGuiSettings.h"

// CUploadSettingsPage
CUploadSettingsPage::CUploadSettingsPage()
{
        
}

CUploadSettingsPage::~CUploadSettingsPage()
{
}

void CUploadSettingsPage::TranslateUI()
{
    TRC(IDC_CONNECTIONSETTINGS, "Connection settings");
    TRC(IDC_USEPROXYSERVER, "Use provided proxy");
    TRC(IDC_USESYSTEMPROXY, "Use system proxy settings");
    TRC(IDC_NOPROXY, "No proxy (direct connection)");
    TRC(IDC_ADDRESSLABEL, "Address:");
    TRC(IDC_PORTLABEL, "Port:");
    TRC(IDC_SERVERTYPE, "Proxy type:");
    TRC(IDC_NEEDSAUTH, "Authorization on proxy");
    TRC(IDC_LOGINLABEL, "Login:");
    TRC(IDC_PASSWORDLABEL, "Password:");
    TRC(IDC_AUTOCOPYTOCLIPBOARD, "Copy automatically results to clipboard");
    TRC(IDC_UPLOADERRORLABEL, "Uploading errors");
    TRC(IDC_RETRIES1LABEL, " Num of retries per file:");
    TRC(IDC_RETRIES2LABEL, "Num of retries per action:");
    TRC(IDC_UPLOADBUFFERLABEL, "Upload Buffer Size:");
    TRC(IDC_MAXTHREADSLABEL, "Threads number:");
    TRC(IDC_EXECUTESCRIPTCHECKBOX, "Execute Squirrel script for each task (file)");
    TRC(IDC_UPLOADSPEEDLIMITLABEL, "Upload speed limit per thread:");
}
    
LRESULT CUploadSettingsPage::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    WtlGuiSettings& Settings = *ServiceLocator::instance()->settings<WtlGuiSettings>();
    TabBackgroundFix(m_hWnd);
    TranslateUI();

    BOOL temp;
    DoDataExchange(FALSE);
    int selectedProxyTypeIndex = 0;
    for (size_t i = 0; i < proxyTypes.size(); i++) {
        auto& item = proxyTypes[i];
		if (item.second == Settings.ConnectionSettings.ProxyType) {
            selectedProxyTypeIndex = i;
		}
        int index = serverTypeCombo_.AddString(item.first);
        serverTypeCombo_.SetItemData(index, item.second);

	}
    /*serverTypeCombo_.AddString(_T("HTTP"));
    serverTypeCombo_.AddString(_T("SOCKS4"));
    serverTypeCombo_.AddString(_T("SOCKS4A"));
    serverTypeCombo_.AddString(_T("SOCKS5"));
    serverTypeCombo_.AddString(_T("SOCKS5(DNS)"));
    serverTypeCombo_.AddString(_T("HTTPS"));*/
	
    
    // ---- connection settings -----
    SetDlgItemText(IDC_ADDRESSEDIT, U2W(Settings.ConnectionSettings.ServerAddress));
    SendDlgItemMessage(IDC_NEEDSAUTH, BM_SETCHECK, (WPARAM) Settings.ConnectionSettings.NeedsAuth);
    SendDlgItemMessage(IDC_AUTOCOPYTOCLIPBOARD, BM_SETCHECK, (WPARAM) Settings.AutoCopyToClipboard);
    

    SendDlgItemMessage(IDC_USEPROXYSERVER, BM_SETCHECK, (WPARAM)(Settings.ConnectionSettings.UseProxy == ConnectionSettingsStruct::kUserProxy)?TRUE:FALSE);
    SendDlgItemMessage(IDC_USESYSTEMPROXY, BM_SETCHECK, (WPARAM)(Settings.ConnectionSettings.UseProxy == ConnectionSettingsStruct::kSystemProxy)?TRUE:FALSE);
    SendDlgItemMessage(IDC_NOPROXY, BM_SETCHECK, (WPARAM)(Settings.ConnectionSettings.UseProxy == ConnectionSettingsStruct::kNoProxy)?TRUE:FALSE);

    SetDlgItemText(IDC_PROXYLOGINEDIT, U2W(Settings.ConnectionSettings.ProxyUser));
    SetDlgItemText(IDC_PROXYPASSWORDEDIT, (CString)Settings.ConnectionSettings.ProxyPassword);
    SetDlgItemInt(IDC_UPLOADBUFFERSIZEEDIT,Settings.UploadBufferSize/1024);
    if(Settings.ConnectionSettings.ProxyPort) 
        SetDlgItemInt(IDC_PORTEDIT, Settings.ConnectionSettings.ProxyPort);

    serverTypeCombo_.SetCurSel(selectedProxyTypeIndex);

    SendDlgItemMessage(IDC_NEEDSAUTH, BM_SETCHECK, (WPARAM) Settings.ConnectionSettings.NeedsAuth);
    SetDlgItemInt(IDC_MAXTHREADSEDIT, Settings.MaxThreads);

    SetDlgItemInt(IDC_FILERETRYLIMIT, Settings.FileRetryLimit);
    SetDlgItemInt(IDC_ACTIONRETRYLIMIT, Settings.ActionRetryLimit);

    OnClickedUseProxy(BN_CLICKED, IDC_USEPROXYSERVER, 0, temp);
    GuiTools::SetCheck(m_hWnd, IDC_EXECUTESCRIPTCHECKBOX, Settings.ExecuteScript);
    executeScriptCheckboxChanged();
    SetDlgItemText(IDC_SCRIPTFILENAMEEDIT, IuCoreUtils::Utf8ToWstring(Settings.ScriptFileName).c_str());
    SetDlgItemInt(IDC_UPLOADSPEEDLIMITEDIT, Settings.MaxUploadSpeed);
	
    return 1;  // Let the system set the focus
}

LRESULT CUploadSettingsPage::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    EndDialog(wID);
    return 0;
}

LRESULT CUploadSettingsPage::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    EndDialog(wID);
    return 0;
}

LRESULT CUploadSettingsPage::OnClickedUseProxy(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled)
{
    proxyRadioChanged();
    return 0;
}
    
LRESULT CUploadSettingsPage::OnClickedUseProxyAuth(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    bool Checked = SendDlgItemMessage(wID, BM_GETCHECK)!=0;
    GuiTools::EnableNextN(GetDlgItem(wID), 4, Checked);
    return 0;
}

LRESULT CUploadSettingsPage::OnClickedNoProxy(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
    proxyRadioChanged();
    return 0;
}

LRESULT CUploadSettingsPage::OnClickedUseSystemProxy(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
    proxyRadioChanged();
    return 0;
}

LRESULT CUploadSettingsPage::OnExecuteScriptCheckboxClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
    executeScriptCheckboxChanged();
    return 0;
}

bool CUploadSettingsPage::Apply()
{
    WtlGuiSettings& Settings = *ServiceLocator::instance()->settings<WtlGuiSettings>();
    CheckBounds(IDC_FILERETRYLIMIT, 1, 10, IDC_RETRIES1LABEL);
    CheckBounds(IDC_ACTIONRETRYLIMIT, 1, 10, IDC_RETRIES2LABEL);
    
    DoDataExchange(TRUE);
    Settings.FileRetryLimit = GetDlgItemInt(IDC_FILERETRYLIMIT);
    Settings.ActionRetryLimit = GetDlgItemInt(IDC_ACTIONRETRYLIMIT);
    if (SendDlgItemMessage(IDC_USEPROXYSERVER, BM_GETCHECK) != 0) {
        Settings.ConnectionSettings.UseProxy = ConnectionSettingsStruct::kUserProxy;
    } else if (SendDlgItemMessage(IDC_USESYSTEMPROXY, BM_GETCHECK) != 0) {
        Settings.ConnectionSettings.UseProxy = ConnectionSettingsStruct::kSystemProxy;
    } else {
        Settings.ConnectionSettings.UseProxy = ConnectionSettingsStruct::kNoProxy;
    }
    
    Settings.ConnectionSettings.NeedsAuth = SendDlgItemMessage(IDC_NEEDSAUTH, BM_GETCHECK)!=0;
    Settings.AutoCopyToClipboard = SendDlgItemMessage(IDC_AUTOCOPYTOCLIPBOARD, BM_GETCHECK)!=0;
    TCHAR Buffer[128];

    GetDlgItemText(IDC_ADDRESSEDIT,Buffer, 128);
    Settings.ConnectionSettings.ServerAddress = W2U(Buffer);
    Settings.ConnectionSettings.ProxyPort = GetDlgItemInt(IDC_PORTEDIT);
    
    GetDlgItemText(IDC_PROXYLOGINEDIT, Buffer, 128);
    Settings.ConnectionSettings.ProxyUser = W2U(Buffer);
    GetDlgItemText(IDC_PROXYPASSWORDEDIT, Buffer, 128);
    Settings.ConnectionSettings.ProxyPassword = Buffer;
    Settings.ConnectionSettings.ProxyType = serverTypeCombo_.GetItemData(serverTypeCombo_.GetCurSel());
    Settings.UploadBufferSize = static_cast<int>(GetDlgItemInt(IDC_UPLOADBUFFERSIZEEDIT) * 1024);
    if (!Settings.UploadBufferSize) {
        Settings.UploadBufferSize = 65536;
    }
    CheckBounds(IDC_MAXTHREADSEDIT, 1, 20, IDC_MAXTHREADSLABEL);
    Settings.MaxThreads = GetDlgItemInt(IDC_MAXTHREADSEDIT);
    if (Settings.MaxThreads <= 0 || Settings.MaxThreads > 50 )
    {
        Settings.MaxThreads = 3;
    }

	int maxUploadSpeed = GetDlgItemInt(IDC_UPLOADSPEEDLIMITEDIT);
	if (maxUploadSpeed < 0) {
        maxUploadSpeed = 0;
	}
    Settings.MaxUploadSpeed = maxUploadSpeed;

    GuiTools::GetCheck(m_hWnd, IDC_EXECUTESCRIPTCHECKBOX, Settings.ExecuteScript);
    CString scriptFile = GuiTools::GetDlgItemText(m_hWnd, IDC_SCRIPTFILENAMEEDIT);

    if (Settings.ExecuteScript && !WinUtils::FileExists(scriptFile)){
        CString message1, message2;
        CString fieldTitle = GuiTools::GetDlgItemText(m_hWnd, IDC_EXECUTESCRIPTCHECKBOX);
        message1.Format(TR("Error in the field '%s':\n"), static_cast<LPCTSTR>(fieldTitle));
        message2.Format(TR("File %s doesn't exist"), static_cast<LPCTSTR>(scriptFile));
        throw ValidationException(message1+message2, GetDlgItem(IDC_SCRIPTFILENAMEEDIT));
    }
    Settings.ScriptFileName = W2U(scriptFile);

    return true;
}

LRESULT CUploadSettingsPage::OnBnClickedBrowseScriptButton(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    IMyFileDialog::FileFilterArray filters = {
        { CString(_T("Squirrel 3 script (.nut)")), _T("*.nut;"), },
        { TR("All files"), _T("*.*") }
    };

    auto dlg = MyFileDialogFactory::createFileDialog(m_hWnd, WinUtils::GetAppFolder(), CString(), filters, false);

    if (dlg->DoModal(m_hWnd) != IDOK) {
        return 0;
    }

    CString fileName = dlg->getFile();

    if (fileName.IsEmpty()) {
        return 0;
    }

    SetDlgItemText(IDC_SCRIPTFILENAMEEDIT, fileName);

    return 0;
}

void CUploadSettingsPage::proxyRadioChanged() {
    bool Checked = SendDlgItemMessage(IDC_USEPROXYSERVER, BM_GETCHECK) != 0;
    GuiTools::EnableNextN(GetDlgItem(IDC_USEPROXYSERVER), Checked ? 8 : 11, Checked);

    BOOL bHandled = false;
    if (Checked) {
        OnClickedUseProxyAuth(BN_CLICKED, IDC_NEEDSAUTH, nullptr, bHandled);
    }
}

void CUploadSettingsPage::executeScriptCheckboxChanged() {
    bool Checked = SendDlgItemMessage(IDC_EXECUTESCRIPTCHECKBOX, BM_GETCHECK) != 0;
    GuiTools::EnableNextN(GetDlgItem(IDC_EXECUTESCRIPTCHECKBOX), 2, Checked);
}

void CUploadSettingsPage::CheckBounds(int controlId, int minValue, int maxValue, int labelId) const {
    int value = GetDlgItemInt(controlId);
    if (value < minValue || value > maxValue) {
        CString fieldName = labelId != -1 ? GuiTools::GetDlgItemText(m_hWnd, labelId) : _T("Unknown field");
        CString message;
        message.Format(TR("Error in the field '%s': value should be between %d and %d."), static_cast<LPCTSTR>(fieldName), minValue, maxValue);
        throw ValidationException(message, GetDlgItem(controlId));
    }
}