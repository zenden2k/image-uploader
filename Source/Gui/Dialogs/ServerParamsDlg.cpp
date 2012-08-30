/*
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2011 ZendeN <zenden2k@gmail.com>
	 
    HomePage:    http://zenden.ws/imageuploader

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "ServerParamsDlg.h"

#include "Func/LangClass.h"
#include "Func/Settings.h"
#include <Func/Myutils.h>
#include "Func/common.h"
#include <Func/Settings.h>
#include <Gui/GuiTools.h>

// CServerParamsDlg
CServerParamsDlg::CServerParamsDlg(const ServerProfile& serverProfile){
	m_ue  = _EngineList->byName(( serverProfile.serverName() ) );
	serverProfile_ = serverProfile;
}

CServerParamsDlg::~CServerParamsDlg()
{
}

LRESULT CServerParamsDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CenterWindow(GetParent());
	TRC(IDCANCEL, "Отмена");
	TRC(IDOK, "OK");
	TRC(IDC_LOGINLABEL, "Логин:");
	TRC(IDC_PASSWORDLABEL, "Пароль:");
	TRC(IDC_DOAUTH, "Выполнять авторизацию");
	TRC(IDCANCEL, "Отмена");

	createToolbar();
	profileListCombobox_.Attach( GetDlgItem(IDC_PROFILELISTCOMBO) );
	profileListCombobox_.AddString(_T("Профиль по-умолчанию"));
	LoginInfo li = Settings.ServerByUtf8Name(m_ue->Name).authData;
	SetDlgItemText(IDC_LOGINEDIT, Utf8ToWCstring(li.Login));
	SetDlgItemText(IDC_PASSWORDEDIT, Utf8ToWCstring(li.Password));
	SetDlgItemText(IDC_LOGINFRAME, Utf8ToWCstring(m_ue->Name));
	SendDlgItemMessage(IDC_DOAUTH, BM_SETCHECK, (li.DoAuth?BST_CHECKED:BST_UNCHECKED));

	DlgResize_Init();
	CString WindowTitle;
	CString serverName = Utf8ToWCstring(m_ue->Name);
	WindowTitle.Format(TR("Параметры сервера %s"),(LPCTSTR)serverName);
	SetWindowText(WindowTitle);
	doAuthChanged();
	m_wndParamList.SubclassWindow(GetDlgItem(IDC_PARAMLIST));
	m_wndParamList.SetExtendedListStyle(PLS_EX_SHOWSELALWAYS | PLS_EX_SINGLECLICKEDIT);
	if ( serverProfile_.profileName().IsEmpty() ) {
		profileListCombobox_.SetCurSel(0);
	}

	::EnableWindow(GetDlgItem(IDC_BROWSESERVERFOLDERS), m_ue->SupportsFolders);

	CScriptUploadEngine *m_pluginLoader = iuPluginManager.getPlugin(m_ue->PluginName, Settings.ServerByUtf8Name(m_ue->Name));
	if(!m_pluginLoader)
	{
		return 0;
	}
	m_pluginLoader->getServerParamList(m_paramNameList);

	std::map<std::string,std::string>::iterator it;
	for( it = m_paramNameList.begin(); it!= m_paramNameList.end(); ++it)
	{
		CString name = it->first.c_str();
		CString humanName = it->second.c_str();
		m_wndParamList.AddItem( PropCreateSimple(humanName, Utf8ToWCstring(Settings.ServerByUtf8Name(m_ue->Name).params[WCstringToUtf8(name)])) );
	}
	
	return 1;  // Let the system set the focus
}

LRESULT CServerParamsDlg::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	std::map<std::string,std::string>::iterator it;
	for(it = m_paramNameList.begin(); it!= m_paramNameList.end(); ++it) {
		CString name      = it->first.c_str();
		CString humanName = it->second.c_str();
		HPROPERTY pr = m_wndParamList.FindProperty(humanName);
		CComVariant vValue;
		pr->GetValue(&vValue);
		Settings.ServerByUtf8Name(m_ue->Name).params[WCstringToUtf8(name)]= WCstringToUtf8(vValue.bstrVal);	      
	}

	EndDialog(wID);
	return 0;
}

LRESULT CServerParamsDlg::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(wID);
	return 0;
}

LRESULT CServerParamsDlg::OnClickedDoAuth(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	doAuthChanged();
	return 0;
}

void CServerParamsDlg::createToolbar() {
	RECT profileRect = GuiTools::GetDialogItemRect(m_hWnd, IDC_TOOLBARPLACEHOLDER);

	m_ProfileEditToolbar.Create(m_hWnd,profileRect,_T(""), WS_CHILD|WS_VISIBLE|WS_CHILD | TBSTYLE_LIST |TBSTYLE_FLAT| TBSTYLE_TOOLTIPS|CCS_NORESIZE|/*CCS_BOTTOM |CCS_ADJUSTABLE|*/CCS_NODIVIDER|TBSTYLE_AUTOSIZE  );
	m_ProfileEditToolbar.SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS);
	m_ProfileEditToolbar.SetButtonStructSize();
	m_ProfileEditToolbar.SetButtonSize(17,17);

	CIcon ico = (HICON)LoadIcon(GetModuleHandle(0),MAKEINTRESOURCE(IDI_ICONWHITEPAGE));
	CIcon saveIcon = LoadIcon(GetModuleHandle(0),MAKEINTRESOURCE(IDI_ICONSAVE));
	CIcon deleteIcon = LoadIcon(GetModuleHandle(0),MAKEINTRESOURCE(IDI_ICONDELETE));
	CImageList list;
	list.Create(16,16,ILC_COLOR32 | ILC_MASK,0,6);
	list.AddIcon(ico);
	list.AddIcon(saveIcon);
	list.AddIcon(deleteIcon);
	m_ProfileEditToolbar.SetImageList(list);
	m_ProfileEditToolbar.AddButton(BUTTON_NEWPROFILE, TBSTYLE_BUTTON |BTNS_AUTOSIZE, TBSTATE_ENABLED, 0, TR("Создать профиль"), 0);
	m_ProfileEditToolbar.AddButton(BUTTON_SAVEPROFILE, TBSTYLE_BUTTON |BTNS_AUTOSIZE, TBSTATE_ENABLED, 1, TR("Сохранить профиль"), 0);
	m_ProfileEditToolbar.AddButton(BUTTON_DELETEPROFILE, TBSTYLE_BUTTON |BTNS_AUTOSIZE,  TBSTATE_ENABLED, 2, TR("Удалить профиль"), 0);

}

void CServerParamsDlg::doAuthChanged() {
	::EnableWindow(GetDlgItem(IDC_LOGINEDIT), IS_CHECKED(IDC_DOAUTH));
	::EnableWindow(GetDlgItem(IDC_PASSWORDEDIT), IS_CHECKED(IDC_DOAUTH));
}

ServerProfile CServerParamsDlg::serverProfile() const {
	return serverProfile_;
}

void CServerParamsDlg::saveCurrentProfile() {

}