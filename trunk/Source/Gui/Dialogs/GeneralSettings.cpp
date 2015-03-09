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

#include "GeneralSettings.h"

#include <uxtheme.h>
#include "Func/common.h"
#include "Func/Settings.h"
#include "LogWindow.h"
#include "Gui/GuiTools.h"
#include <Func/WinUtils.h>
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
		findfile = FindFirstFile(WinUtils::GetAppFolder() + "Lang\\*.lng", &wfd);
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
	
	// Translating controls
	TRC(IDOK, "OK");
	TRC(IDCANCEL, "������");
	TRC(IDC_CHANGESWILLBE, "��������: ����� ��������� � ����� �������� � ����, ��������� ���������� �������������.");
	TRC(IDC_LANGUAGELABEL, "���� ����������:");
	TRC(IDC_VIEWLOG, "�������� ���");
	TRC(IDC_LOGGROUP, "�������� ������");
	TRC(IDC_IMAGEEDITLABEL, "����������� ��������:");
	TRC(IDC_AUTOSHOWLOG, "������������� ���������� ���� ���� � ������ ������");
	TRC(IDC_CONFIRMONEXIT, "���������� ������������� ��� ������");
	
	SetDlgItemText(IDC_IMAGEEDITORPATH, Settings.ImageEditorPath);
	

	TCHAR buf[MAX_PATH];
	CString buf2;

	SendDlgItemMessage(IDC_LANGLIST,CB_ADDSTRING,0,(WPARAM)_T("�������"));

	while(GetNextLngFile(buf, sizeof(buf)/sizeof(TCHAR)))
	{
		if(lstrlen(buf) == 0 || lstrcmpi(GetFileExt(buf), _T("lng"))) continue;
		buf2 = GetOnlyFileName(buf );
		SendDlgItemMessage(IDC_LANGLIST,CB_ADDSTRING,0,(WPARAM)(LPCTSTR)buf2);
	}

	int Index = SendDlgItemMessage(IDC_LANGLIST,CB_FINDSTRING, 0, (WPARAM)(LPCTSTR)Settings.Language);
	if(Index==-1) Index=0;
	SendDlgItemMessage(IDC_LANGLIST,CB_SETCURSEL,Index);

	
	SendDlgItemMessage(IDC_CONFIRMONEXIT, BM_SETCHECK, Settings.ConfirmOnExit);
	SendDlgItemMessage(IDC_AUTOSHOWLOG, BM_SETCHECK, Settings.AutoShowLog);
	
	return 1;  // Let the system set the focus
}

LRESULT CGeneralSettings::OnBnClickedBrowse(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	TCHAR Buf[MAX_PATH*4];
	GuiTools::SelectDialogFilter(Buf, sizeof(Buf)/sizeof(TCHAR),2, 
		CString(TR("����������� �����")),
		_T("*.exe;*.com;*.bat;*.cmd;"),
		TR("��� �����"),
		_T("*.*"));

	CFileDialog fd(true, 0, 0, 4|2, Buf, m_hWnd);
	CString s;
	s = WinUtils::GetAppFolder();
	fd.m_ofn.lpstrInitialDir = s;
	if ( fd.DoModal() != IDOK || !fd.m_szFileName[0] ) return 0;

	CString FileName = CString(_T("\""))+ fd.m_szFileName + CString(_T("\""));
	FileName += _T(" \"%1\"");

	SetDlgItemText(IDC_IMAGEEDITORPATH, FileName);
	return 0;
}

	
bool CGeneralSettings::Apply()
{
	int Index = SendDlgItemMessage(IDC_LANGLIST, CB_GETCURSEL);
	if(Index < 0) return 0;
	
	TCHAR szBuf[256];
	SendDlgItemMessage(IDC_LANGLIST, CB_GETLBTEXT, Index, (WPARAM)szBuf);
	Settings.Language = szBuf;

	GetDlgItemText(IDC_IMAGEEDITORPATH, szBuf, 256);
	Settings.ImageEditorPath = szBuf;
	
	Settings.AutoShowLog = SendDlgItemMessage(IDC_AUTOSHOWLOG,  BM_GETCHECK )==BST_CHECKED;
	Settings.ConfirmOnExit = SendDlgItemMessage(IDC_CONFIRMONEXIT, BM_GETCHECK)==BST_CHECKED;
	
	return true;
}


LRESULT CGeneralSettings::OnBnClickedViewLog(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	LogWindow.Show();
	return 0;
}