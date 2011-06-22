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

#include "NewFolderDlg.h"
#include "Func/Common.h"
#include "LogWindow.h"
#include "Func/Settings.h"
#include "Func/pluginloader.h"
#include "Gui/GuiTools.h"

// CNewFolderDlg
CNewFolderDlg::CNewFolderDlg(CFolderItem &folder, bool CreateNewFolder,std::vector<std::string>& accessTypeList):
					m_folder(folder), m_bCreateNewFolder(CreateNewFolder), m_accessTypeList(accessTypeList)
{
}

CNewFolderDlg::~CNewFolderDlg()
{
}

LRESULT CNewFolderDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	TRC(IDC_FOLDERNAMELABEL, "Название папки/альбома:");
	TRC(IDC_FOLDERDESCRLABEL, "Описание:");
	TRC(IDC_ACCESSTYPELABEL, "Доступ:");
	TRC(IDCANCEL, "Отмена");
	TRC(IDOK, "OK");
	
	
	DlgResize_Init();
	CenterWindow(GetParent());
	if(m_bCreateNewFolder)
		SetWindowText(TR("Новая папка (альбом)"));
	else
		SetWindowText(TR("Редактирование папки"));
	SetDlgItemText(IDC_FOLDERTITLEEDIT, Utf8ToWCstring(m_folder.title));
	CString text = Utf8ToWCstring(m_folder.summary);
	text.Replace(_T("\r"), _T(""));
	text.Replace(_T("\n"), _T("\r\n"));
	SetDlgItemText(IDC_FOLDERDESCREDIT, text);
	for(size_t i=0; i< m_accessTypeList.size(); i++)
	{
		SendDlgItemMessage(IDC_ACCESSTYPECOMBO, CB_ADDSTRING, 0, (LPARAM) (LPCTSTR)Utf8ToWCstring(m_accessTypeList[i].c_str()));
	}
	SendDlgItemMessage(IDC_ACCESSTYPECOMBO, CB_SETCURSEL, m_folder.accessType);
	
	::SetFocus(GetDlgItem(IDC_FOLDERTITLEEDIT));
	return 0;  // Let the system set the focus
}

LRESULT CNewFolderDlg::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	m_sTitle= ZGuiTools::IU_GetWindowText(GetDlgItem(IDC_FOLDERTITLEEDIT));
	m_folder.title = WCstringToUtf8(m_sTitle);
	m_sDescription = ZGuiTools::IU_GetWindowText(GetDlgItem(IDC_FOLDERDESCREDIT));
	m_folder.summary = WCstringToUtf8(m_sDescription);
	int nAccessType = SendDlgItemMessage(IDC_ACCESSTYPECOMBO, CB_GETCURSEL);
	if(nAccessType >= 0)
	m_folder.accessType = nAccessType;
	EndDialog(wID);
	return 0;
}

LRESULT CNewFolderDlg::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(wID);
	return 0;
}
