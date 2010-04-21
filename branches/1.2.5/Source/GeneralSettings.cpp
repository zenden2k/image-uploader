/*
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2009 ZendeN <zenden2k@gmail.com>
	 
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
#include "GeneralSettings.h"
#include <uxtheme.h>
#include "common.h"
// CGeneralSettings
CGeneralSettings::CGeneralSettings()
{
	findfile = NULL;
}

CGeneralSettings::~CGeneralSettings()
{

}
	
int CGeneralSettings::GetNextLngFile(LPTSTR szBuffer, int nLength)
{
	*wfd.cFileName = 0;
	
	if(!findfile)
	{
		findfile = FindFirstFile(GetAppFolder() + "Lang\\*.lng", &wfd);
		if(!findfile) goto error;
	}
	else if(!FindNextFile(findfile,&wfd)) goto error;
	
	int nLen = lstrlen(wfd.cFileName);

	if(!nLen) goto error;

	lstrcpyn(szBuffer, wfd.cFileName, min(nLength, nLen+1));

	return TRUE;

	error:  // File not found
		if(findfile) FindClose(findfile);
		return FALSE;
}

LRESULT CGeneralSettings::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	TabBackgroundFix(m_hWnd);
	// Translating controls
	TRC(IDOK, "OK");
	TRC(IDCANCEL, "������");
	TRC(IDC_INTEGRATIONGROUP, "���������� � ����������� Windows");
	TRC(IDC_SHELLINTEGRATION, "���������� � ����������� ���� ��������");
	//TRC(IDC_SHELLIMGCONTEXTMENUITEM, "����� � ����������� ���� ������ �����������");
	TRC(IDC_STARTUPLOADINGFROMSHELL, "����� �������� �������� �� ������:");
	TRC(IDC_SHELLVIDEOCONTEXTMENUITEM, "����� � ����������� ���� �����������");
	TRC(IDC_CASCADEDCONTEXTMENU, "��������� ����������� ����");
	TRC(IDC_CHANGESWILLBE, "��������: ����� ��������� � ����� �������� � ����, ��������� ���������� �������������.");
	TRC(IDC_LANGUAGELABEL, "���� ����������:");
	TRC(IDC_VIEWLOG, "�������� ���");
	TRC(IDC_LOGGROUP, "�������� ������");
	TRC(IDC_IMAGEEDITLABEL, "����������� ��������:");
	TRC(IDC_AUTOSHOWLOG, "������������� ���������� ���� ���� � ������ ������");
	TRC(IDC_SHELLSENDTOITEM, "�������� Image Uploader � ���� \"���������\"");
	TRC(IDC_CONFIRMONEXIT, "���������� ������������� ��� ������");
	BOOL temp;

	SetDlgItemText(IDC_IMAGEEDITORPATH, Settings.ImageEditorPath);
	
	for(int i=0; i<EnginesList.GetCount(); i++)
	{	
		TCHAR buf[300] = _T(" ");
		lstrcat(buf, EnginesList[i].Name);
		SendDlgItemMessage(IDC_SERVERLIST, CB_ADDSTRING, 0, (LPARAM)buf);
	}
	SendDlgItemMessage(IDC_SERVERLIST,CB_SETCURSEL, Settings.QuickServerID);

	TCHAR buf[MAX_PATH],buf2[MAX_PATH];

	SendDlgItemMessage(IDC_LANGLIST,CB_ADDSTRING,0,(WPARAM)_T("�������"));

	while(GetNextLngFile(buf, sizeof(buf)/sizeof(TCHAR)))
	{
		if(lstrlen(buf) == 0 || lstrcmpi(GetFileExt(buf), _T("lng"))) continue;
		GetOnlyFileName(buf, buf2);
		SendDlgItemMessage(IDC_LANGLIST,CB_ADDSTRING,0,(WPARAM)buf2);
	}

	int Index = SendDlgItemMessage(IDC_LANGLIST,CB_FINDSTRING, 0, (WPARAM)(LPCTSTR)Settings.Language);
	if(Index==-1) Index=0;
	SendDlgItemMessage(IDC_LANGLIST,CB_SETCURSEL,Index);

	SendDlgItemMessage(IDC_SHELLIMGCONTEXTMENUITEM, BM_SETCHECK, Settings.ExplorerContextMenu);
	SendDlgItemMessage(IDC_SHELLVIDEOCONTEXTMENUITEM, BM_SETCHECK, Settings.ExplorerVideoContextMenu);
	SendDlgItemMessage(IDC_SHELLSENDTOITEM, BM_SETCHECK, Settings.SendToContextMenu);
	SendDlgItemMessage(IDC_CONFIRMONEXIT, BM_SETCHECK, Settings.ConfirmOnExit);
	SendDlgItemMessage(IDC_CASCADEDCONTEXTMENU, BM_SETCHECK, Settings.ExplorerCascadedMenu);
	
	SendDlgItemMessage(IDC_STARTUPLOADINGFROMSHELL, BM_SETCHECK, Settings.QuickUpload);
	SendDlgItemMessage(IDC_AUTOSHOWLOG, BM_SETCHECK, Settings.AutoShowLog);
	
	BOOL b;
	OnClickedQuickUpload(0, IDC_STARTUPLOADINGFROMSHELL,0, b);
	return 1;  // Let the system set the focus
}

LRESULT CGeneralSettings::OnBnClickedBrowse(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	TCHAR Buf[MAX_PATH*4];
	SelectDialogFilter(Buf, sizeof(Buf)/sizeof(TCHAR),2, 
		CString(TR("����������� �����")),
		_T("*.exe;*.com;*.bat;*.cmd;"),
		TR("��� �����"),
		_T("*.*"));

	CFileDialog fd(true, 0, 0, 4|2, Buf, m_hWnd);
	CString s;
	s = GetAppFolder();
	fd.m_ofn.lpstrInitialDir = s;
	if ( fd.DoModal() != IDOK || !fd.m_szFileName ) return 0;

	CString FileName = CString(_T("\""))+ fd.m_szFileName + CString(_T("\""));
	FileName += _T(" \"%1\"");

	SetDlgItemText(IDC_IMAGEEDITORPATH, FileName);
	return 0;
}

	
bool CGeneralSettings::Apply()
{
	int Index = SendDlgItemMessage(IDC_LANGLIST, CB_GETCURSEL);
	if(Index < 0) return 0;
	
	SendDlgItemMessage(IDC_LANGLIST, CB_GETLBTEXT, Index, (WPARAM)(LPCTSTR)Settings.Language);
	TCHAR szBuf[256];
	GetDlgItemText(IDC_IMAGEEDITORPATH, szBuf, 256);
	Settings.ImageEditorPath = szBuf;
	Settings.ExplorerContextMenu_changed = Settings.ExplorerContextMenu; 
	Settings.ExplorerContextMenu = SendDlgItemMessage(IDC_SHELLIMGCONTEXTMENUITEM, BM_GETCHECK);
	Settings.ExplorerContextMenu_changed ^= (Settings.ExplorerContextMenu);
	
	bool Temp = Settings.ExplorerVideoContextMenu;
	Settings.ExplorerVideoContextMenu = SendDlgItemMessage(IDC_SHELLVIDEOCONTEXTMENUITEM, BM_GETCHECK);
	Temp ^= Settings.ExplorerVideoContextMenu;
	Settings.ExplorerContextMenu_changed|=Temp;

	Temp = Settings.ExplorerCascadedMenu;
	Settings.ExplorerCascadedMenu = SendDlgItemMessage(IDC_CASCADEDCONTEXTMENU, BM_GETCHECK);
	Temp ^= Settings.ExplorerCascadedMenu;
	Settings.ExplorerContextMenu_changed|=Temp;

	Settings.SendToContextMenu_changed = Settings.SendToContextMenu;
	Settings.SendToContextMenu = SendDlgItemMessage(IDC_SHELLSENDTOITEM, BM_GETCHECK);
	Settings.SendToContextMenu_changed ^= Settings.SendToContextMenu;

	Settings.QuickUpload = SendDlgItemMessage(IDC_STARTUPLOADINGFROMSHELL, BM_GETCHECK);
	Settings.QuickServerID = SendDlgItemMessage(IDC_SERVERLIST, CB_GETCURSEL, 0, 0);
	Settings.AutoShowLog = SendDlgItemMessage(IDC_AUTOSHOWLOG,  BM_GETCHECK );
	Settings.ConfirmOnExit = SendDlgItemMessage(IDC_CONFIRMONEXIT, BM_GETCHECK);
	
	return true;
}

LRESULT CGeneralSettings::OnClickedQuickUpload(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	bool Checked = SendDlgItemMessage(IDC_STARTUPLOADINGFROMSHELL, BM_GETCHECK);
	EnableNextN(GetDlgItem(wID), 1, Checked);
	return 0;
}

LRESULT CGeneralSettings::OnBnClickedViewLog(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	LogWindow.Show();
	return 0;
}
