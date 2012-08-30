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

#include "MainDlg.h"
#include "atlheaders.h"
#include "resource.h"
#include "aboutdlg.h"
#include "Func/Settings.h"
#include "Common/CmdLine.h"
#include <Func/WinUtils.h>
#include <Func/Myutils.h>
#include <Func/Common.h>
#include <Gui/GuiTools.h>

LRESULT CMainDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	PageWnd = m_hWnd;
	TRC(IDC_ADDIMAGES, "�������� �����");
	TRC(IDC_ADDVIDEO, "������ �����");
	TRC(IDC_SCREENSHOT, "C����� ������");
	TRC(IDC_PROPERTIES, "��������");
	TRC(IDC_DELETE, "������� �� ������");
	
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
		bool IsClipboard=	WizardDlg->IsClipboardDataAvailable();
		sub.EnableMenuItem(IDC_PASTE, (IsClipboard)?MF_ENABLED	:MF_GRAYED	);
		mi.cbSize = sizeof(mi);
		mi.fMask = MIIM_TYPE;
		mi.fType = MFT_STRING;
		mi.dwTypeData = TR("�������� �����������");
		sub.SetMenuItemInfo(IDC_ADDIMAGES, false, &mi);
		mi.dwTypeData = TR("�������� �����");
		sub.SetMenuItemInfo(IDM_ADDFILES, false, &mi);
		mi.dwTypeData = TR("�������� �������");
		sub.SetMenuItemInfo(IDM_ADDFOLDER, false, &mi);
		TCHAR buf[MAX_PATH];
		lstrcpy(buf, TR("��������"));
		lstrcat(buf, _T("\t"));
		lstrcat(buf,Settings.Hotkeys.getByFunc("paste").localKey.toString());
		mi.dwTypeData = buf;
		sub.SetMenuItemInfo(IDC_PASTE, false, &mi);
		mi.dwTypeData = TR("������� ��");
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

 		mi.dwTypeData  = TR("��������");
		sub.SetMenuItemInfo(IDM_VIEW, false, &mi);

		mi.dwTypeData  = TR("������� ����� � ������");
		sub.SetMenuItemInfo(IDM_OPENINFOLDER, false, &mi);

		mi.dwTypeData  = TR("��������� ���...");
		sub.SetMenuItemInfo(IDM_SAVEAS, false, &mi);

		mi.dwTypeData  = TR("�������");
		sub.SetMenuItemInfo(IDM_DELETE, false, &mi);
		mi.dwTypeData  = TR("��������");
		sub.SetMenuItemInfo(IDC_PROPERTIES, false, &mi);
		mi.dwTypeData  = TR("�������������");

		sub.SetMenuItemInfo(IDM_EDIT, false, &mi);
		
		sub.EnableMenuItem(IDM_EDIT, bIsImageFile?MF_ENABLED	:MF_GRAYED	);
		sub.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON, ScreenPoint.x, ScreenPoint.y, m_hWnd);
	}
	return 0;
}

bool CMainDlg::AddToFileList(LPCTSTR FileName, const CString& virtualFileName, Gdiplus::Image *Img)
{
	CFileListItem fl; //internal list item
	
	if(!FileName) return FALSE;

	if(!WinUtils::FileExists(FileName)) return FALSE;
	fl.selected = false;

	int len = lstrlen(FileName);

	fl.FileName = FileName;

	if(virtualFileName.IsEmpty())
	fl.VirtualFileName = WinUtils::myExtractFileName(FileName);
	else
	fl.VirtualFileName = virtualFileName;
	

	TCHAR szBuffer[256] = _T("\0");
	int FileSize = IuCoreUtils::getFileSize(WCstringToUtf8( FileName ));
	if(FileSize<-1) FileSize = 0;

	FileList.Add(fl);

	CString Buf;
	if(IsImage(FileName))
	Buf = WinUtils::GetOnlyFileName(FileName );
	else Buf = WinUtils::myExtractFileName(FileName);
	if(FileName) 
		ThumbsView.AddImage(fl.FileName, fl.VirtualFileName, Img);
		
	EnableNext(FileList.GetCount()>0);
	return TRUE;
}

// ������ ����� ���� ��� ������ ������ �������� ����� �� ������
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

LRESULT CMainDlg::OnLvnItemDelete(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& bHandled)
{
	NM_LISTVIEW * pnmv = reinterpret_cast <NM_LISTVIEW *>  (pNMHDR);  
	if(!pnmv) return 0;

	FileList.RemoveAt(pnmv->iItem);

	EnableNext(FileList.GetCount()>0);
	bHandled = false;
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
		WaitThreadStop.SetEvent(); // Sending stop message destinated for child thread
		WaitForThread(3500);
	}
	return false;
}

// ��������� ������ ������� �����
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
	WinUtils::ExtractFilePath(FileName, FilePathBuffer);

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
	
	// Executing Windows Explorer; file will be highlighted
	ShellExecuteW(NULL, NULL, L"explorer.exe", CString(_T("/select, ")) + FileName, NULL, SW_SHOWNORMAL);
	return 0;
}
LRESULT CMainDlg::OnAddFiles(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
	WizardDlg->executeFunc(_T("addfiles"));
	return 0;
}

LRESULT  CMainDlg::OnSaveAs(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int nCurItem = -1;

	std::deque<CString> selectedFiles;
	while ((nCurItem = ThumbsView.GetNextItem(nCurItem, LVNI_ALL|LVNI_SELECTED)) >= 0 ) {
		LPCTSTR FileName = ThumbsView.GetFileName(nCurItem);
		if ( ! FileName ) {
			continue;
		}
		selectedFiles.push_back( FileName );
		
	}
	if ( selectedFiles.empty() ) {
		return FALSE;
	}
	
	if ( selectedFiles.size() == 1 ) {

		TCHAR Buf[MAX_PATH*4];
		CString FileName = selectedFiles[0];
		CString fileExt = WinUtils::GetFileExt(FileName);
		GuiTools::SelectDialogFilter(Buf, sizeof(Buf)/sizeof(TCHAR),2,
			TR("�����")+CString(" *.")+fileExt, CString(_T("*."))+fileExt,
			TR("��� �����"),_T("*.*"));

		CFileDialog fd(false, fileExt, FileName,4|2,Buf,m_hWnd);

		TCHAR Buffer[1000];
		//fd.m_ofn.lpstrInitialDir = Settings.VideoFolder;
		if(fd.DoModal()!=IDOK || !fd.m_szFileName) return 0;

		CopyFile( FileName, fd.m_szFileName, false );
	} else {
		CString newPath = GuiTools::SelectFolderDialog(m_hWnd, CString());
		if ( !newPath.IsEmpty() ) {
			int fileCount = selectedFiles.size();
			for ( int i = 0; i < fileCount; i++ ) {
				CopyFile( selectedFiles[i], newPath + _T("\\") + WinUtils::myExtractFileName(selectedFiles[i] ) , false );
			}
		}
		/*CFolderDialog fd(m_hWnd,TR("����� �����"), BIF_RETURNONLYFSDIRS|BIF_NEWDIALOGSTYLE );
		if(fd.DoModal(m_hWnd) == IDOK)
		{
			CString newPath = fd.GetFolderPath();
			int fileCount = selectedFiles.size();
			for ( int i = 0; i < fileCount; i++ ) {
				CopyFile( selectedFiles[i], newPath + _T("\\") + WinUtils::myExtractFileName(selectedFiles[i] ) , false );
			}
			return true;
		}*/
	}

	return 0;
}