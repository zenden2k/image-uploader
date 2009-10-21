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
#include "resource.h"

#include "aboutdlg.h"
#include "MainDlg.h"

LRESULT CMainDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	PageWnd = m_hWnd;
	TRC(IDC_ADDIMAGES, "Добавить файлы");
	TRC(IDC_ADDVIDEO, "Импорт видео");
	TRC(IDC_SCREENSHOT, "Сделать снимок экрана");
	TRC(IDC_PROPERTIES, "Свойства");
	TRC(IDC_DELETE, "Удалить из списка");
	
	ThumbsView.SubclassWindow(GetDlgItem(IDC_FILELIST));
	ThumbsView.Init(true);

	WaitThreadStop.Create();
	WaitThreadStop.ResetEvent();
	return TRUE;
}

LRESULT CMainDlg::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	WaitThreadStop.Close();
	return 0;
}

void CMainDlg::CloseDialog(int nVal)
{
	DestroyWindow();
}

LRESULT CMainDlg::OnBnClickedAddvideo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	WizardDlg->executeFunc(_T("importvideo"));
	return 0;
}

bool CMainDlg::CheckEditInteger(int Control)
{
	TCHAR Buffer[MAX_PATH];
	GetDlgItemText(Control, Buffer, sizeof(Buffer)/sizeof(TCHAR));
	if(lstrlen(Buffer) == 0) return false;
	int n = GetDlgItemInt(Control, false);
	if(n) SetDlgItemInt(Control, (n<0)?(-n):n);
	else SetDlgItemText(Control, _T(""));

	return false;
}

LRESULT CMainDlg::OnBnClickedAddimages(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	WizardDlg->executeFunc(_T("addimages"));
	return 0;
}

LRESULT CMainDlg::OnContextMenu(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	MENUITEMINFO mi;
	HWND 	hwnd = (HWND) wParam;  
	POINT ClientPoint, ScreenPoint;
	if(hwnd != GetDlgItem(IDC_FILELIST)) return 0;

	if(lParam == -1) 
	{
		ClientPoint.x = 0;
		ClientPoint.y = 0;
		ScreenPoint = ClientPoint;
		::ClientToScreen(hwnd, &ScreenPoint);
	}
	else
	{
		ScreenPoint.x = LOWORD(lParam); 
		ScreenPoint.y = HIWORD(lParam); 
		ClientPoint = ScreenPoint;
		::ScreenToClient(hwnd, &ClientPoint);
	}

	CMenu menu;
   LV_HITTESTINFO hti;
	hti.pt = ClientPoint;
	ThumbsView.HitTest(&hti);

	if(hti.iItem<0) // 
	{
		CMenu FolderMenu;
		FolderMenu.LoadMenu(IDR_CONTEXTMENU2);
		CMenu sub = FolderMenu.GetSubMenu(0);
	
		bool IsClipboard=false;

		if(OpenClipboard())
		{
			IsClipboard = IsClipboardFormatAvailable(CF_BITMAP);
			CloseClipboard();
		}
		
		sub.EnableMenuItem(IDC_PASTE, (IsClipboard)?MF_ENABLED	:MF_GRAYED	);
		mi.cbSize = sizeof(mi);
		mi.fMask = MIIM_TYPE;
		mi.fType = MFT_STRING;
		mi.dwTypeData = TR("Добавить изображения");
		sub.SetMenuItemInfo(IDC_ADDIMAGES, false, &mi);
		mi.dwTypeData = TR("Добавить файлы");
		sub.SetMenuItemInfo(IDM_ADDFILES, false, &mi);
		mi.dwTypeData = TR("Добавить каталог");
		sub.SetMenuItemInfo(IDM_ADDFOLDER, false, &mi);
		TCHAR buf[MAX_PATH];
		lstrcpy(buf, TR("Вставить"));
		lstrcat(buf, _T("\t"));
		lstrcat(buf,Settings.Hotkeys.getByFunc("paste").localKey.toString());
		mi.dwTypeData = buf;
		sub.SetMenuItemInfo(IDC_PASTE, false, &mi);
		mi.dwTypeData = TR("Удалить всё");
		sub.SetMenuItemInfo(IDC_DELETEALL, false, &mi);
		sub.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON, ScreenPoint.x, ScreenPoint.y, m_hWnd);
	}
	else
	{
		RECT r;
		GetClientRect(&r);
		menu.LoadMenu(IDR_CONTEXTMENU);

		mi.cbSize = sizeof(mi);
		mi.fMask = MIIM_TYPE;
		mi.fType = MFT_STRING;

		CMenu sub = menu.GetSubMenu(0);
		sub.SetMenuDefaultItem(0, true);

		bool bIsImageFile = IsImage( FileList[hti.iItem].FileName);
		if(!bIsImageFile){
		sub.DeleteMenu(IDM_VIEW,MF_BYCOMMAND	);
		sub.DeleteMenu(IDM_EDIT,MF_BYCOMMAND	);}

 		mi.dwTypeData  = TR("Просмотр");
		sub.SetMenuItemInfo(IDM_VIEW, false, &mi);

		mi.dwTypeData  = TR("Открыть папку с файлом");
		sub.SetMenuItemInfo(IDM_OPENINFOLDER, false, &mi);
		mi.dwTypeData  = TR("Удалить");
		sub.SetMenuItemInfo(IDM_DELETE, false, &mi);
		mi.dwTypeData  = TR("Свойства");
		sub.SetMenuItemInfo(IDC_PROPERTIES, false, &mi);
		mi.dwTypeData  = TR("Редактировать");

		sub.SetMenuItemInfo(IDM_EDIT, false, &mi);
		
		sub.EnableMenuItem(IDM_EDIT, bIsImageFile?MF_ENABLED	:MF_GRAYED	);
		sub.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON, ScreenPoint.x, ScreenPoint.y, m_hWnd);
	}
	return 0;
}

bool CMainDlg::AddToFileList(LPCTSTR FileName,  Image *Img)
{
	CFileListItem fl;
	LPTSTR szFilename, szFilepath;
	
	if(!FileName) return FALSE;

	if(!FileExists(FileName)) return FALSE;
	ZeroMemory(&fl, sizeof(fl));
	
	int len = lstrlen(FileName);
	szFilename = new TCHAR[len + 1];
	szFilepath = new TCHAR[len + 1];
	
	if(!szFilename || !szFilepath) return FALSE;

	lstrcpy(szFilename, FileName);

	fl.FileName = szFilename;

	
	TCHAR szBuffer[256] = _T("\0");
	int FileSize = MyGetFileSize(FileName);
	if(FileSize<-1) FileSize = 0;

	FileList.Add(fl);

	WCHAR Buf[256];
	if(IsImage(FileName))
	GetOnlyFileName(FileName, Buf);
	else lstrcpy(Buf, myExtractFileName(FileName));
	if(FileName) 
		ThumbsView.AddImage(FileName, Buf, Img);
		
	EnableNext(FileList.GetCount()>0);
	return TRUE;
}

// Выбран пункт меню или нажата кнопка удаления файла из списка
LRESULT CMainDlg::OnDelete(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	ThumbsView.DeleteSelected();
	return 0;
}

LRESULT CMainDlg::OnBnClickedScreenshot(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	WizardDlg->executeFunc(_T("screenshotdlg"));
	return 0;
}

bool CMainDlg::OnShow()
{
	ShowPrev();
	ShowNext();
	EnablePrev();
	EnableExit();
	EnableNext(FileList.GetCount()>0);
	ThumbsView.SetFocus();
	ThumbsView.LoadThumbnails();
	return true;
}

LRESULT CMainDlg::OnLvnItemDelete(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
{
	NM_LISTVIEW * pnmv = reinterpret_cast <NM_LISTVIEW *>  (pNMHDR);  
	if(!pnmv) return 0;

	FileList.RemoveAt(pnmv->iItem);

	EnableNext(FileList.GetCount()>0);
	return 0;
}

LRESULT CMainDlg::OnBnClickedDelete(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add your control notification handler code here
	ThumbsView.DeleteSelected();
	return 0;
}

bool CMainDlg::OnHide()
{
	ThumbsView.StopAndWait();
	if(IsRunning())
	{
		WaitThreadStop.SetEvent(); // Sending stop message to child thread
		WaitForThread(3500);
	}
	return false;
}

// Системный диалог свойств файла
BOOL CMainDlg::FileProp()
{
	LV_ITEM lvItem;
	int nCurItem;
	SHELLEXECUTEINFO ShInfo;

	if ((nCurItem = ThumbsView.GetNextItem(-1, LVNI_ALL|LVNI_SELECTED))<0)
		return FALSE;

	ZeroMemory(&lvItem, sizeof(LV_ITEM));

	LPCTSTR FileName = ThumbsView.GetFileName(nCurItem);

	ZeroMemory(&ShInfo, sizeof(SHELLEXECUTEINFO));
	ShInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	ShInfo.nShow = SW_SHOW;
	ShInfo.fMask = SEE_MASK_INVOKEIDLIST | SEE_MASK_IDLIST;
	ShInfo.hwnd = m_hWnd;
	ShInfo.lpVerb = TEXT("properties");
	ShInfo.lpFile = FileName;
	
	ShellExecuteEx(&ShInfo);

	return TRUE;
}

LRESULT CMainDlg::OnBnClickedProperties(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	FileProp();
	return 0;
}

LRESULT CMainDlg::OnEdit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int nCurItem;

	if ((nCurItem=ThumbsView.GetNextItem(-1, LVNI_ALL|LVNI_SELECTED))<0)
            return FALSE;
    
	LPCTSTR FileName = ThumbsView.GetFileName(nCurItem);
	if(!FileName) return FALSE;
	// Edit this bullshit
	CString EditorCmd = Settings.ImageEditorPath;
	CString EditorCmdLine ;
	EditorCmd.Replace(_T("%1"), _T("%s"));
	EditorCmdLine.Format(EditorCmd, (LPCTSTR)FileName);
	
	TCHAR FilePathBuffer[256];
	ExtractFilePath(FileName, FilePathBuffer);

	CCmdLine EditorLine(EditorCmdLine);

	SHELLEXECUTEINFO Sei;
	ZeroMemory(&Sei, sizeof(Sei));
	Sei.cbSize = sizeof(Sei);
	Sei.fMask  = SEE_MASK_NOCLOSEPROCESS	;
	Sei.hwnd = m_hWnd;
	Sei.lpVerb = _T ("open");

	Sei.lpFile = EditorLine.ModuleName();
	Sei.lpParameters = EditorLine.OnlyParams();
	Sei.nShow = SW_SHOW;
	
	ShellExecuteEx(&Sei);

	if(Sei.hProcess)
	{
		if(IsRunning())
		{
			WaitThreadStop.SetEvent();
			WaitForThread(9999);
		}
		ThumbsView.OutDateThumb(nCurItem);
		m_EditorProcess = Sei.hProcess;
		Start();
	}
	return 0;
}

LRESULT CMainDlg::OnImageView(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	ThumbsView.ViewSelectedImage();
	return 0;
}

LRESULT CMainDlg::OnMenuItemPaste(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	WizardDlg->executeFunc(_T("paste"));
	return 0;
}

LRESULT CMainDlg::OnAddFolder(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	WizardDlg->executeFunc(_T("addfolder"));
	return 0;
}
	
LRESULT CMainDlg::OnBnClickedDeleteAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	ThumbsView.MyDeleteAllItems();
	return 0;
}

DWORD CMainDlg::Run()
{
	HANDLE Events[2];
	Events[0] = m_EditorProcess;
	Events[1] = WaitThreadStop.m_hEvent;
	
	DWORD res = WaitForMultipleObjects(2, Events, FALSE, INFINITE);
	ThumbsView.UpdateOutdated();
	WaitThreadStop.ResetEvent();
	return 0;
}

LRESULT CMainDlg::OnOpenInFolder(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int nCurItem;

	if ((nCurItem = ThumbsView.GetNextItem(-1, LVNI_ALL|LVNI_SELECTED))<0)
            return FALSE;
    
	LPCTSTR FileName = ThumbsView.GetFileName(nCurItem);
	if(!FileName) return FALSE;
	
	// Executing explorer with highlighting the file
	ShellExecuteW(NULL, NULL, L"explorer.exe", CString(_T("/select, ")) + FileName, NULL, SW_SHOWNORMAL);

	return 0;
}
LRESULT CMainDlg::OnAddFiles(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
	WizardDlg->executeFunc(_T("addfiles"));
	return 0;
}
