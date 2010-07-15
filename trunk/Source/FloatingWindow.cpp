// This file was generated by WTL subclass control wizard 
// FloatingWindow.cpp : Implementation of FloatingWindow

#include "stdafx.h"
#include "FloatingWindow.h"
#include "ResultsPanel.h"
#include "ScreenshotDlg.h"

// FloatingWindow
CFloatingWindow::CFloatingWindow()
{
	m_bFromHotkey = false;
	EnableClicks = true;
	m_FileQueueUploader = 0;
	hMutex = NULL;
	m_PrevActiveWindow = 0;
	m_bStopCapturingWindows = false;
	WM_TASKBARCREATED = RegisterWindowMessage(_T("TaskbarCreated"));
	m_bIsUploading = 0;
}

CFloatingWindow::~CFloatingWindow()
{
	CloseHandle(hMutex);
	DeleteObject(m_hIconSmall);
	m_hWnd = 0;
}

LRESULT CFloatingWindow::OnClose(void)
{
	return 0;
}

bool MyInsertMenu(HMENU hMenu, int pos, UINT id, const LPCTSTR szTitle,  HBITMAP bm=NULL)
{
	MENUITEMINFO MenuItem;
	 
	MenuItem.cbSize = sizeof(MenuItem);
	if(szTitle)
	MenuItem.fType = MFT_STRING;
	else MenuItem.fType = MFT_SEPARATOR;
	MenuItem.fMask = MIIM_TYPE	| MIIM_ID | MIIM_DATA;
	if(bm)
		MenuItem.fMask |= MIIM_CHECKMARKS;
	MenuItem.wID = id;
	MenuItem.hbmpChecked = bm;
	MenuItem.hbmpUnchecked = bm;
	MenuItem.dwTypeData = (LPWSTR)szTitle;
	return InsertMenuItem(hMenu, pos, TRUE, &MenuItem);
}

LRESULT CFloatingWindow::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	m_hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
	SetIcon(m_hIconSmall, FALSE);

	RegisterHotkeys();
	InstallIcon(APPNAME,m_hIconSmall,/*TrayMenu*/0);
	NOTIFYICONDATA nid;
	ZeroMemory(&nid, sizeof(nid));
	nid.cbSize =NOTIFYICONDATA_V2_SIZE;
	nid.hWnd = m_hWnd;
	nid.uVersion = NOTIFYICON_VERSION;
	Shell_NotifyIcon(NIM_SETVERSION, &nid);
	return 0;
}

LRESULT CFloatingWindow::OnExit(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
	pWizardDlg->CloseWizard();
	return 0;
}

LRESULT CFloatingWindow::OnTrayIcon(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	if(!EnableClicks ) return 0;
	

	if (LOWORD(lParam) == WM_LBUTTONDOWN)
	{
		m_bStopCapturingWindows = true;
	}
	if (LOWORD(lParam) == WM_MOUSEMOVE)
	{
		if(!m_bStopCapturingWindows)
		{
			HWND wnd =  GetForegroundWindow();
			if(wnd != m_hWnd)
			m_PrevActiveWindow = GetForegroundWindow();
		}
	}
	if (LOWORD(lParam) == WM_RBUTTONUP)
	{
		if(m_bIsUploading && Settings.Hotkeys[Settings.TrayIconSettings.RightClickCommand].commandId!=IDM_CONTEXTMENU) return 0;
		SendMessage(WM_COMMAND, MAKEWPARAM(Settings.Hotkeys[Settings.TrayIconSettings.RightClickCommand].commandId,0));
	}
	else if (LOWORD(lParam) == WM_LBUTTONDBLCLK)
	{
		EnableClicks = false;
		KillTimer(1);
		SetTimer(2, GetDoubleClickTime());
		if(m_bIsUploading && Settings.Hotkeys[Settings.TrayIconSettings.LeftDoubleClickCommand].commandId!=IDM_CONTEXTMENU) return 0;
		SendMessage(WM_COMMAND, MAKEWPARAM(Settings.Hotkeys[Settings.TrayIconSettings.LeftDoubleClickCommand].commandId,0));
	}
	else if (LOWORD(lParam) == WM_LBUTTONUP)
	{
		m_bStopCapturingWindows = false;
		if(m_bIsUploading && Settings.Hotkeys[Settings.TrayIconSettings.LeftDoubleClickCommand].commandId!=IDM_CONTEXTMENU) return 0;

		if(!Settings.Hotkeys[Settings.TrayIconSettings.LeftDoubleClickCommand].commandId)
			SendMessage(WM_COMMAND, MAKEWPARAM(Settings.Hotkeys[Settings.TrayIconSettings.LeftClickCommand].commandId,0));
		else
			SetTimer(1, (UINT) (1.2*GetDoubleClickTime()));
	}
	else if (LOWORD(lParam) == WM_MBUTTONUP)
	{
		if(m_bIsUploading && Settings.Hotkeys[Settings.TrayIconSettings.MiddleClickCommand].commandId!=IDM_CONTEXTMENU) return 0;

		SendMessage(WM_COMMAND, MAKEWPARAM(Settings.Hotkeys[Settings.TrayIconSettings.MiddleClickCommand].commandId,0));
	}
	else if(LOWORD(lParam) == NIN_BALLOONUSERCLICK)
	{
		CAtlArray<CUrlListItem> items;
		CUrlListItem it;
		it.ImageUrl = Utf8ToWstring(m_LastUploadedItem.imageUrl).c_str();
		it.ThumbUrl =  Utf8ToWstring(m_LastUploadedItem.thumbUrl).c_str();
		it.DownloadUrl = Utf8ToWstring(m_LastUploadedItem.downloadUrl).c_str();
		items.Add(it);
		if(it.ImageUrl.IsEmpty() && it.DownloadUrl.IsEmpty())
			return 0;
		CResultsWindow rp( pWizardDlg, items,false);
		rp.DoModal(m_hWnd);
	}
	return 0;
}

LRESULT CFloatingWindow::OnMenuSettings(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
	if(!pWizardDlg->IsWindowEnabled()) return 0;
	CSettingsDlg dlg(4);
	dlg.DoModal(pWizardDlg->m_hWnd);
	return 0;
}

LRESULT CFloatingWindow::OnCloseTray(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ShowWindow(SW_HIDE);
	pWizardDlg->ShowWindow(SW_SHOW);
	pWizardDlg->SetDlgItemText(IDCANCEL, TR("�����"));
	CloseHandle(hMutex);
	RemoveIcon();
	UnRegisterHotkeys();
	DestroyWindow();
	hMutex = NULL;
	m_hWnd = 0;
	return 0;
}

LRESULT CFloatingWindow::OnReloadSettings(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(!lParam)
	UnRegisterHotkeys();

	if(!wParam)
		Settings.LoadSettings();

	if(!lParam)
	RegisterHotkeys();
	return 0;
}

LRESULT CFloatingWindow::OnImportvideo(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
	if(pWizardDlg->executeFunc(_T("importvideo,1")))
		pWizardDlg->ShowWindow(SW_SHOW);
	return 0;
}

LRESULT CFloatingWindow::OnUploadFiles(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
	if(pWizardDlg->executeFunc(_T("addfiles,1")))
		pWizardDlg->ShowWindow(SW_SHOW);
	return 0;
}

LRESULT CFloatingWindow::OnUploadImages(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
	if(pWizardDlg->executeFunc(_T("addimages,1")))
		pWizardDlg->ShowWindow(SW_SHOW);
	return 0;
}

LRESULT CFloatingWindow::OnPasteFromWeb(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
	if(pWizardDlg->executeFunc(_T("downloadimages,1")))
		pWizardDlg->ShowWindow(SW_SHOW);
	return 0;
}

LRESULT CFloatingWindow::OnScreenshotDlg(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
	if(pWizardDlg->executeFunc(_T("screenshotdlg,2")));
		//pWizardDlg->ShowWindow(SW_SHOW);
	return 0;
}

LRESULT CFloatingWindow::OnRegionScreenshot(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
	pWizardDlg->executeFunc(_T("regionscreenshot_dontshow,")+(m_bFromHotkey?CString(_T("1")):CString(_T("2"))));
	return 0;
}

LRESULT CFloatingWindow::OnFullScreenshot(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
	pWizardDlg->executeFunc(_T("fullscreenshot,")+(m_bFromHotkey?CString(_T("1")):CString(_T("2"))));
	return 0;
}
LRESULT CFloatingWindow::OnWindowHandleScreenshot(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
	pWizardDlg->executeFunc(_T("windowhandlescreenshot,")+(m_bFromHotkey?CString(_T("1")):CString(_T("2"))));
	return 0;
}
LRESULT CFloatingWindow::OnFreeformScreenshot(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
	pWizardDlg->executeFunc(_T("freeformscreenshot,")+(m_bFromHotkey?CString(_T("1")):CString(_T("2"))));
	return 0;
}

LRESULT CFloatingWindow::OnWindowScreenshot(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
	if(m_PrevActiveWindow) SetForegroundWindow(m_PrevActiveWindow);
	if(pWizardDlg->executeFunc(_T("windowscreenshot_delayed,")+(m_bFromHotkey?CString(_T("1")):CString(_T("2")))));

	return 0;
}

LRESULT CFloatingWindow::OnAddFolder(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
	if(pWizardDlg->executeFunc(_T("addfolder")))
		pWizardDlg->ShowWindow(SW_SHOW);
	return 0;
}

LRESULT CFloatingWindow::OnShowAppWindow(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
	if(pWizardDlg->IsWindowEnabled())
		pWizardDlg->ShowWindow(SW_SHOWNORMAL);
	else if(pWizardDlg->IsWindowVisible())
		pWizardDlg->SetActiveWindow();
	return 0;
}

LRESULT CFloatingWindow::OnContextMenu(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
	if(!IsWindowEnabled()) return 0;

	CMenu TrayMenu ;
	TrayMenu.CreatePopupMenu();

	if(!m_bIsUploading)
	{
		// Inserting menu items
		int i = 0;
		MyInsertMenu(TrayMenu,i++,IDM_UPLOADFILES,TR("��������� �����")+CString(_T("...")));
		MyInsertMenu(TrayMenu,i++,IDM_ADDFOLDER,TR("��������� �����")+CString(_T("...")));
		MyInsertMenu(TrayMenu,i++,0,0);
		bool IsClipboard=false;

		if(OpenClipboard())
		{
			IsClipboard = IsClipboardFormatAvailable(CF_BITMAP);
			CloseClipboard();
		}
		if(IsClipboard)
		{
			MyInsertMenu(TrayMenu,i++,IDM_PASTEFROMCLIPBOARD,TR("�������� �� ������"));
			MyInsertMenu(TrayMenu,i++,0,0);
		}
		MyInsertMenu(TrayMenu,i++,IDM_IMPORTVIDEO ,TR("������ �����"));
		MyInsertMenu(TrayMenu,i++,0,0);
		MyInsertMenu(TrayMenu,i++,IDM_SCREENSHOTDLG,TR("��������")+CString(_T("...")));
		MyInsertMenu(TrayMenu,i++,IDM_REGIONSCREENSHOT,TR("������ ���������� �������"));
		MyInsertMenu(TrayMenu,i++,IDM_FULLSCREENSHOT,TR("������ ����� ������"));
		MyInsertMenu(TrayMenu,i++,IDM_WINDOWSCREENSHOT,TR("������ ��������� ����"));
		MyInsertMenu(TrayMenu,i++,IDM_WINDOWHANDLESCREENSHOT,TR("������ ���������� ��������"));
		MyInsertMenu(TrayMenu,i++,IDM_FREEFORMSCREENSHOT,TR("������ ������������ �����"));
		CMenu SubMenu ;
		SubMenu.CreatePopupMenu();
		SubMenu.InsertMenu(0,MFT_STRING|MFT_RADIOCHECK| (Settings.TrayIconSettings.TrayScreenshotAction==TRAY_SCREENSHOT_UPLOAD?MFS_CHECKED:0),IDM_SCREENTSHOTACTION_UPLOAD,TR("��������� �� ������"));
		SubMenu.InsertMenu(1,MFT_STRING|MFT_RADIOCHECK|(Settings.TrayIconSettings.TrayScreenshotAction==TRAY_SCREENSHOT_CLIPBOARD?MFS_CHECKED:0),IDM_SCREENTSHOTACTION_TOCLIPBOARD,TR("���������� � ����� ������"));
		SubMenu.InsertMenu(2,MFT_STRING|MFT_RADIOCHECK|(Settings.TrayIconSettings.TrayScreenshotAction==TRAY_SCREENSHOT_WIZARD?MFS_CHECKED:0),IDM_SCREENTSHOTACTION_TOWIZARD,TR("������� � �������"));

		MENUITEMINFO mi;
		mi.cbSize = sizeof(mi);
		mi.fMask = MIIM_TYPE|MIIM_ID|MIIM_SUBMENU;
		mi.fType = MFT_STRING;
		mi.hSubMenu = SubMenu;
		mi.wID = 10000;
		mi.dwTypeData  = TR("�������� �� �������");
		TrayMenu.InsertMenuItem(i++, true, &mi);

		SubMenu.Detach();
		MyInsertMenu(TrayMenu,i++,0,0);
		MyInsertMenu(TrayMenu,i++,IDM_SHOWAPPWINDOW,TR("�������� ���� ���������"));
		MyInsertMenu(TrayMenu,i++,0,0);
		MyInsertMenu(TrayMenu,i++,IDM_SETTINGS,TR("���������")+CString(_T("...")));
		MyInsertMenu(TrayMenu,i++,0,0);
		MyInsertMenu(TrayMenu,i++,IDM_EXIT,TR("�����"));
		if(Settings.Hotkeys[Settings.TrayIconSettings.LeftDoubleClickCommand].commandId)
		{
			SetMenuDefaultItem(TrayMenu, Settings.Hotkeys[Settings.TrayIconSettings.LeftDoubleClickCommand].commandId, false);
		}
	}
	else 
		MyInsertMenu(TrayMenu,0,IDM_STOPUPLOAD,TR("�������� ��������"));
	m_hTrayIconMenu = TrayMenu;
	CMenuHandle oPopup(m_hTrayIconMenu);
	PrepareMenu(oPopup);
	CPoint pos;
	GetCursorPos(&pos);
	SetForegroundWindow(m_hWnd);
	oPopup.TrackPopupMenu(TPM_LEFTALIGN, pos.x, pos.y, m_hWnd);
	// BUGFIX: See "PRB: Menus for Notification Icons Don't Work Correctly"
	PostMessage(WM_NULL);
	return 0;
}

LRESULT CFloatingWindow::OnTimer(UINT id)
{
	if(id == 1)
	{
		KillTimer(1);
		SendMessage(WM_COMMAND, MAKEWPARAM(Settings.Hotkeys[Settings.TrayIconSettings.LeftClickCommand].commandId,0));
	}
	if(id == 2)
		EnableClicks = true;

	KillTimer(id);
	return 0;
}
inline BOOL SetOneInstance(LPCTSTR szName)
{
	HANDLE hMutex=NULL;
	BOOL bFound=FALSE;
	hMutex = ::CreateMutex(NULL, TRUE, szName);
	if(GetLastError() == ERROR_ALREADY_EXISTS)
		bFound = TRUE;
	if(hMutex)
		::ReleaseMutex(hMutex);
	return bFound;
}

CFloatingWindow floatWnd;

void CFloatingWindow::CreateTrayIcon()
{
    BOOL bFound=FALSE;
    hMutex = ::CreateMutex(NULL, TRUE, _T("ImageUploader_TrayWnd_Mutex"));
    if(GetLastError() == ERROR_ALREADY_EXISTS)
        bFound = TRUE;
    if(hMutex)
        ::ReleaseMutex(hMutex);
   
	 if(!bFound)
	 {
		CRect r(100,100,400,400);
		floatWnd.Create(0,r,_T("ImageUploader_TrayWnd"),WS_OVERLAPPED|WS_POPUP|WS_CAPTION );
		floatWnd.ShowWindow(SW_HIDE);
	 }
}

BOOL IsRunningFloatingWnd()
{
    HANDLE hMutex=NULL;
    BOOL bFound=FALSE;
    hMutex = ::CreateMutex(NULL, TRUE, _T("ImageUploader_TrayWnd_Mutex"));
    if(GetLastError() == ERROR_ALREADY_EXISTS)
        bFound = TRUE;
	 if(hMutex){
        ::ReleaseMutex(hMutex);
		CloseHandle(hMutex);
	 }
    return bFound;
}

void  CFloatingWindow::RegisterHotkeys()
{
	m_hotkeys=Settings.Hotkeys;

	for(int i =0; i< m_hotkeys.GetCount(); i++)
	{
		if(m_hotkeys[i].globalKey.keyCode)
		{
			if(!RegisterHotKey(m_hWnd,i,m_hotkeys[i].globalKey.keyModifier,m_hotkeys[i].globalKey.keyCode))
			{
				CString msg;
				msg.Format(TR("���������� ���������������� ���������� ��������� ������\n%s.\n ��������, ��� ������ ������ ����������."),(LPCTSTR)m_hotkeys[i].globalKey.toString());
				WriteLog(logWarning, _T("Hotkeys"), msg);
			}
		}
	}
}

LRESULT CFloatingWindow::OnHotKey(int HotKeyID, UINT flags, UINT vk)
{
	if(HotKeyID <0 || HotKeyID > m_hotkeys.GetCount()-1) return 0;
	if(m_bIsUploading) return 0;

	if(m_hotkeys[HotKeyID].func == _T("windowscreenshot"))
	{
		pWizardDlg->executeFunc(_T("windowscreenshot,1"));
	}
	else
	{
		m_bFromHotkey = true;
		SetActiveWindow();
		SetForegroundWindow(m_hWnd);
		SendMessage(WM_COMMAND, MAKEWPARAM(m_hotkeys[HotKeyID].commandId,0));
		m_bFromHotkey = false;
	}
	return 0;
}

void  CFloatingWindow::UnRegisterHotkeys()
{
	for(int i =0; i< m_hotkeys.GetCount(); i++)
	{
		if(m_hotkeys[i].globalKey.keyCode)
		UnregisterHotKey(m_hWnd, i);
	}
	m_hotkeys.RemoveAll();
}
LRESULT CFloatingWindow::OnPaste(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
	if(pWizardDlg->executeFunc(_T("paste")))
		pWizardDlg->ShowWindow(SW_SHOW);
	return 0;
}

LRESULT CFloatingWindow::OnMediaInfo(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
	if(pWizardDlg->executeFunc(_T("mediainfo")))
		pWizardDlg->ShowWindow(SW_SHOW);
	return 0;
}

LRESULT CFloatingWindow::OnTaskbarCreated(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	InstallIcon(APPNAME, m_hIconSmall, 0);
	return 0;
}

LRESULT CFloatingWindow::OnScreenshotActionChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
	Settings.TrayIconSettings.TrayScreenshotAction = wID - IDM_SCREENTSHOTACTION_UPLOAD;
	Settings.SaveSettings();
	return 0;
}
#define ARRAYSIZE(a) (sizeof(a)/sizeof(a[0]))
void  CFloatingWindow::ShowBaloonTip(const CString& text, const CString& title)
{
	//MessageBox(text);
	NOTIFYICONDATA nid;
	ZeroMemory(&nid, sizeof(nid));
	nid.cbSize = NOTIFYICONDATA_V2_SIZE;
	nid.hWnd = m_hWnd;
	nid.uTimeout = 5500;
	nid.uFlags = NIF_INFO;
	nid.dwInfoFlags = NIIF_INFO;
	lstrcpyn(nid.szInfo, text, ARRAYSIZE(nid.szInfo)-1);
	lstrcpyn(nid.szInfoTitle, title, ARRAYSIZE(nid.szInfoTitle)-1);
	Shell_NotifyIcon(NIM_MODIFY, &nid);
}

void CFloatingWindow::UploadScreenshot(const CString& realName, const CString &displayName)
{
	delete m_FileQueueUploader;
	m_LastUploadedItem = FileListItem();
	m_FileQueueUploader = new CFileQueueUploader();
	m_FileQueueUploader->setCallback(this);
	CUploadEngine * engine = _EngineList->byIndex(Settings.ServerID);
	if(!engine) engine = _EngineList->byIndex(_EngineList->getRandomImageServer());
	if(!engine) return;

	CImageConverter imageConverter;
	imageConverter.setImageConvertingParams(Settings.ImageSettings);
	imageConverter.setThumbCreatingParams(Settings.ThumbSettings);
	bool GenThumbs = Settings.ThumbSettings.CreateThumbs && ((!Settings.ThumbSettings.UseServerThumbs)||(!engine->SupportThumbnails));
	imageConverter.setGenerateThumb(GenThumbs);
	imageConverter.Convert(realName);
	m_FileQueueUploader->setUploadSettings(engine,Settings.ServersSettings[engine->Name]);

	m_FileQueueUploader->AddFile(WCstringToUtf8(imageConverter.getImageFileName()),WCstringToUtf8(displayName),0);
	
	CString thumbFileName = imageConverter.getThumbFileName();
	if(!thumbFileName.IsEmpty())
	m_FileQueueUploader->AddFile(WCstringToUtf8(thumbFileName),WCstringToUtf8(thumbFileName),1);
	
	m_bIsUploading = true;
	m_FileQueueUploader->start();
	CString msg;
	msg.Format(TR("���� �������� \"%s\" �� ������ %s"), GetOnlyFileName(displayName),engine->Name);
	ShowBaloonTip(msg, TR("�������� ������"));
}

bool  CFloatingWindow::OnQueueFinished()
{
	m_bIsUploading = false;
	CString url;
	if((Settings.UseDirectLinks || m_LastUploadedItem.downloadUrl.empty()) && !m_LastUploadedItem.imageUrl.empty() )
		url = Utf8ToWstring(m_LastUploadedItem.imageUrl).c_str();
	else if((!Settings.UseDirectLinks || m_LastUploadedItem.imageUrl.empty()) && !m_LastUploadedItem.downloadUrl.empty() )
		url = Utf8ToWstring(m_LastUploadedItem.downloadUrl).c_str();

	if(url.IsEmpty())
	{
		ShowBaloonTip(TR("�� ������� ��������� ������ :("), _T("Image Uploader"));
		return true;
	}
	IU_CopyTextToClipboard(url);
	ShowBaloonTip(TrimString(url, 70) + CString("\r\n")+TR("������� �� ��� ��������� ��� �������� ���� � �����..."), TR("������ ������� ��������"));

	return true;
}

bool  CFloatingWindow::OnFileFinished(bool ok, FileListItem& result)
{
	if(ok)
	{
		if(result.id == 0)
		{
			m_LastUploadedItem = result;
		}
		else if(result.id == 1)
		{
			m_LastUploadedItem.thumbUrl = result.imageUrl;
		}
	}
		return true;
 }

bool CFloatingWindow::OnConfigureNetworkManager(NetworkManager* nm)
{

	IU_ConfigureProxy(*nm);
	return true;
}

LRESULT CFloatingWindow::OnStopUpload(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
	if(m_FileQueueUploader)
		m_FileQueueUploader->stop();
	return 0;
}