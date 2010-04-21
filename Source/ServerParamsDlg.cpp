/*
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2010 ZendeN <zenden2k@gmail.com>
	 
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

#include "stdafx.h"
#include "ServerParamsDlg.h"

// CServerParamsDlg
CServerParamsDlg::CServerParamsDlg(UploadEngine &ue): m_ue(ue)
{
}

CServerParamsDlg::~CServerParamsDlg()
{
}

LRESULT CServerParamsDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CenterWindow(GetParent());
	TRC(IDCANCEL, "Отмена");
	TRC(IDOK, "OK");
	DlgResize_Init();
	CString WindowTitle;
	WindowTitle.Format(TR("Параметры сервера %s"),m_ue.Name);
	SetWindowText(WindowTitle);

	m_wndParamList.SubclassWindow(GetDlgItem(IDC_PARAMLIST));
	m_wndParamList.SetExtendedListStyle(PLS_EX_SHOWSELALWAYS | PLS_EX_SINGLECLICKEDIT);

	CUploadScript *m_pluginLoader = iuPluginManager.getPlugin(m_ue.PluginName, Settings.ServersSettings[m_ue.Name]);
	if(!m_pluginLoader)
	{
		return 0;
	}
	m_pluginLoader->getServerParamList(m_paramNameList);

	std::map<std::wstring,std::wstring>::iterator it;
	for(it = m_paramNameList.begin(); it!= m_paramNameList.end(); it++)
	{

		CString name = it->first.c_str();
		CString humanName = it->second.c_str();

		 m_wndParamList.AddItem( PropCreateSimple(humanName, Settings.ServersSettings[m_ue.Name].params[name]) );
     
	}
	return 1;  // Let the system set the focus
}

LRESULT CServerParamsDlg::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	std::map<std::wstring,std::wstring>::iterator it;
	for(it = m_paramNameList.begin(); it!= m_paramNameList.end(); it++)
	{

		CString name = it->first.c_str();
		CString humanName = it->second.c_str();
	
		HPROPERTY pr = m_wndParamList.FindProperty(humanName);
		CComVariant vValue;
      pr->GetValue(&vValue);

		Settings.ServersSettings[m_ue.Name].params[name]= vValue.bstrVal;	      
	}

	EndDialog(wID);
	return 0;
}

LRESULT CServerParamsDlg::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(wID);
	return 0;
}
