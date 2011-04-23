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

#include "../../atlheaders.h"
#include "GeneralSettings.h"
#include <uxtheme.h>
#include "../../Func/common.h"

#include "../../Func/Settings.h"
#include "LogWindow.h"
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
	
	// Translating controls
	TRC(IDOK, "OK");
	TRC(IDCANCEL, "Отмена");
	TRC(IDC_INTEGRATIONGROUP, "Интеграция с проводником Windows");
	TRC(IDC_SHELLINTEGRATION, "Интеграция в контекстное меню оболочки");
	//TRC(IDC_SHELLIMGCONTEXTMENUITEM, "Пункт в контекстное меню файлов изображений");
	TRC(IDC_STARTUPLOADINGFROMSHELL, "Сразу начинать загрузку на сервер:");
	TRC(IDC_SHELLVIDEOCONTEXTMENUITEM, "Пункт в контекстном меню видеофайлов");
	TRC(IDC_CASCADEDCONTEXTMENU, "Вложенное контекстное меню");
	TRC(IDC_CHANGESWILLBE, "Внимание: чтобы изменения в языке вступили в силу, программу необходимо перезапустить.");
	TRC(IDC_LANGUAGELABEL, "Язык интерфейса:");
	TRC(IDC_VIEWLOG, "Показать лог");
	TRC(IDC_LOGGROUP, "Контроль ошибок");
	TRC(IDC_IMAGEEDITLABEL, "Графический редактор:");
	TRC(IDC_AUTOSHOWLOG, "Автоматически показывать окно лога в случае ошибок");
	TRC(IDC_SHELLSENDTOITEM, "Добавить Image Uploader в меню \"Отправить\"");
	TRC(IDC_CONFIRMONEXIT, "Спрашивать подтверждение при выходе");
	
	SetDlgItemText(IDC_IMAGEEDITORPATH, Settings.ImageEditorPath);
	
	for(int i=0; i<_EngineList->count(); i++)
	{	
		CString buf = _T(" ");
		//TCHAR buf[300] = _T(" ");
		buf+= Utf8ToWCstring(_EngineList->byIndex(i)->Name);
		SendDlgItemMessage(IDC_SERVERLIST, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)buf);
	}
	SendDlgItemMessage(IDC_SERVERLIST,CB_SETCURSEL, Settings.QuickServerID);

	TCHAR buf[MAX_PATH];
	CString buf2;

	SendDlgItemMessage(IDC_LANGLIST,CB_ADDSTRING,0,(WPARAM)_T("Русский"));

	while(GetNextLngFile(buf, sizeof(buf)/sizeof(TCHAR)))
	{
		if(lstrlen(buf) == 0 || lstrcmpi(GetFileExt(buf), _T("lng"))) continue;
		buf2 = GetOnlyFileName(buf );
		SendDlgItemMessage(IDC_LANGLIST,CB_ADDSTRING,0,(WPARAM)(LPCTSTR)buf2);
	}

	int Index = SendDlgItemMessage(IDC_LANGLIST,CB_FINDSTRING, 0, (WPARAM)(LPCTSTR)Settings.Language);
	if(Index==-1) Index=0;
	SendDlgItemMessage(IDC_LANGLIST,CB_SETCURSEL,Index);

	SendDlgItemMessage(IDC_SHELLIMGCONTEXTMENUITEM, BM_SETCHECK, Settings.ExplorerContextMenu);
	
	bool shellIntegrationAvailable = FileExists(GetAppFolder() + "ExplorerIntegration.dll")!=0;

	SendDlgItemMessage(IDC_SHELLVIDEOCONTEXTMENUITEM, BM_SETCHECK, Settings.ExplorerVideoContextMenu);
	SendDlgItemMessage(IDC_SHELLSENDTOITEM, BM_SETCHECK, Settings.SendToContextMenu);
	SendDlgItemMessage(IDC_CONFIRMONEXIT, BM_SETCHECK, Settings.ConfirmOnExit);
	SendDlgItemMessage(IDC_CASCADEDCONTEXTMENU, BM_SETCHECK, Settings.ExplorerCascadedMenu);
	
	SendDlgItemMessage(IDC_STARTUPLOADINGFROMSHELL, BM_SETCHECK, Settings.QuickUpload);
	SendDlgItemMessage(IDC_AUTOSHOWLOG, BM_SETCHECK, Settings.AutoShowLog);

	::EnableWindow(GetDlgItem(IDC_SHELLVIDEOCONTEXTMENUITEM), shellIntegrationAvailable);
	::EnableWindow(GetDlgItem(IDC_CASCADEDCONTEXTMENU), shellIntegrationAvailable);
	::EnableWindow(GetDlgItem(IDC_SHELLIMGCONTEXTMENUITEM), shellIntegrationAvailable);
	
	BOOL b;
	OnClickedQuickUpload(0, IDC_STARTUPLOADINGFROMSHELL,0, b);
	ShellIntegrationChanged();
	return 1;  // Let the system set the focus
}

LRESULT CGeneralSettings::OnBnClickedBrowse(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	TCHAR Buf[MAX_PATH*4];
	SelectDialogFilter(Buf, sizeof(Buf)/sizeof(TCHAR),2, 
		CString(TR("Исполняемые файлы")),
		_T("*.exe;*.com;*.bat;*.cmd;"),
		TR("Все файлы"),
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
	
	TCHAR szBuf[256];
	SendDlgItemMessage(IDC_LANGLIST, CB_GETLBTEXT, Index, (WPARAM)szBuf);
	Settings.Language = szBuf;

	GetDlgItemText(IDC_IMAGEEDITORPATH, szBuf, 256);
	Settings.ImageEditorPath = szBuf;
	Settings.ExplorerContextMenu_changed = Settings.ExplorerContextMenu; 
	Settings.ExplorerContextMenu = SendDlgItemMessage(IDC_SHELLIMGCONTEXTMENUITEM, BM_GETCHECK)==BST_CHECKED;
	Settings.ExplorerContextMenu_changed ^= (Settings.ExplorerContextMenu);
	
	bool Temp = Settings.ExplorerVideoContextMenu;
	Settings.ExplorerVideoContextMenu = SendDlgItemMessage(IDC_SHELLVIDEOCONTEXTMENUITEM, BM_GETCHECK)==BST_CHECKED;
	Temp ^= Settings.ExplorerVideoContextMenu;

	Temp = Settings.ExplorerCascadedMenu;
	Settings.ExplorerCascadedMenu = SendDlgItemMessage(IDC_CASCADEDCONTEXTMENU, BM_GETCHECK)==BST_CHECKED;
	Temp ^= Settings.ExplorerCascadedMenu;

	Settings.SendToContextMenu_changed = Settings.SendToContextMenu;
	Settings.SendToContextMenu = SendDlgItemMessage(IDC_SHELLSENDTOITEM, BM_GETCHECK)==BST_CHECKED;
	Settings.SendToContextMenu_changed ^= Settings.SendToContextMenu;

	Settings.QuickUpload = SendDlgItemMessage(IDC_STARTUPLOADINGFROMSHELL, BM_GETCHECK)==BST_CHECKED;
	Settings.QuickServerID = SendDlgItemMessage(IDC_SERVERLIST, CB_GETCURSEL, 0, 0);
	Settings.AutoShowLog = SendDlgItemMessage(IDC_AUTOSHOWLOG,  BM_GETCHECK )==BST_CHECKED;
	Settings.ConfirmOnExit = SendDlgItemMessage(IDC_CONFIRMONEXIT, BM_GETCHECK)==BST_CHECKED;
	
	return true;
}

LRESULT CGeneralSettings::OnClickedQuickUpload(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	bool Checked = SendDlgItemMessage(IDC_STARTUPLOADINGFROMSHELL, BM_GETCHECK)==BST_CHECKED;
	EnableNextN(GetDlgItem(wID), 1, Checked);
	return 0;
}

LRESULT CGeneralSettings::OnBnClickedViewLog(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	LogWindow.Show();
	return 0;
}

LRESULT CGeneralSettings::OnShellIntegrationCheckboxChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	ShellIntegrationChanged();
	return 0;
}
	
void CGeneralSettings::ShellIntegrationChanged()
{
	bool checked = SendDlgItemMessage(IDC_SHELLIMGCONTEXTMENUITEM, BM_GETCHECK)==BST_CHECKED;
	EnableNextN(GetDlgItem(IDC_SHELLINTEGRATION), 2, checked);
}