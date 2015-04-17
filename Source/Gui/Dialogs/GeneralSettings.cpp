/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2015 Sergey Svistunov (zenden2k@gmail.com)

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
	TRC(IDCANCEL, "Отмена");
	TRC(IDC_CHANGESWILLBE, "Внимание: чтобы изменения в языке вступили в силу, программу необходимо перезапустить.");
	TRC(IDC_LANGUAGELABEL, "Язык интерфейса:");
	TRC(IDC_VIEWLOG, "Показать лог");
	TRC(IDC_LOGGROUP, "Контроль ошибок");
	TRC(IDC_IMAGEEDITLABEL, "Графический редактор:");
	TRC(IDC_AUTOSHOWLOG, "Автоматически показывать окно лога в случае ошибок");
	TRC(IDC_CONFIRMONEXIT, "Спрашивать подтверждение при выходе");
	TRC(IDC_DROPVIDEOFILESTOTHELIST, "Добавлять видеофайлы в список после перетаскивания");
	
	SetDlgItemText(IDC_IMAGEEDITORPATH, Settings.ImageEditorPath);
	

	TCHAR buf[MAX_PATH];
	CString buf2;

	SendDlgItemMessage(IDC_LANGLIST,CB_ADDSTRING,0,(WPARAM)_T("Русский"));

	while(GetNextLngFile(buf, sizeof(buf)/sizeof(TCHAR)))
	{
		if(lstrlen(buf) == 0 || lstrcmpi(WinUtils::GetFileExt(buf), _T("lng"))) continue;
		buf2 = WinUtils::GetOnlyFileName(buf );
		SendDlgItemMessage(IDC_LANGLIST,CB_ADDSTRING,0,(WPARAM)(LPCTSTR)buf2);
	}

	int Index = SendDlgItemMessage(IDC_LANGLIST,CB_FINDSTRING, 0, (WPARAM)(LPCTSTR)Settings.Language);
	if(Index==-1) Index=0;
	SendDlgItemMessage(IDC_LANGLIST,CB_SETCURSEL,Index);

	
	SendDlgItemMessage(IDC_CONFIRMONEXIT, BM_SETCHECK, Settings.ConfirmOnExit);
	SendDlgItemMessage(IDC_AUTOSHOWLOG, BM_SETCHECK, Settings.AutoShowLog);
	GuiTools::SetCheck(m_hWnd, IDC_DROPVIDEOFILESTOTHELIST, Settings.DropVideoFilesToTheList);
	
	return 1;  // Let the system set the focus
}

LRESULT CGeneralSettings::OnBnClickedBrowse(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	TCHAR Buf[MAX_PATH*4];
	GuiTools::SelectDialogFilter(Buf, sizeof(Buf)/sizeof(TCHAR),2, 
		CString(TR("Исполняемые файлы")),
		_T("*.exe;*.com;*.bat;*.cmd;"),
		TR("Все файлы"),
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
	Settings.DropVideoFilesToTheList = GuiTools::GetCheck(m_hWnd, IDC_DROPVIDEOFILESTOTHELIST);
	
	return true;
}


LRESULT CGeneralSettings::OnBnClickedViewLog(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	LogWindow.Show();
	return 0;
}