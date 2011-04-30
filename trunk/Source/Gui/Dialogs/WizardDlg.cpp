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
#include "WizardDlg.h"
#include "langselect.h"
#include <io.h>
#include "floatingwindow.h"
#include "updatedlg.h"
#include "TextViewDlg.h"
#include "ImageDownloaderDlg.h"
#include "../../Core/ImageConverter.h"
#include "LogWindow.h"
#include "../../Func/Base.h"
#include "../../Func/HistoryManager.h"
 
// CWizardDlg
CWizardDlg::CWizardDlg(): m_lRef(0), FolderAdd(this)
{ 
	screenshotIndex = 1;
	CurPage = -1;
	PrevPage = -1;
	ZeroMemory(Pages, sizeof(Pages));
	DragndropEnabled = true;
	hLocalHotkeys = 0;
	QuickUploadMarker = false;
	m_bShowAfter = true;
	m_bHandleCmdLineFunc = false;
	updateDlg = 0;
	_EngineList = &m_EngineList;
	m_bScreenshotFromTray = false;
}

CWizardDlg::~CWizardDlg()
{
	//Detach();
	if(updateDlg) delete updateDlg;
	for(int i=0; i<5; i++) 
	{
		CWizardPage *p = Pages[i];
		if(Pages[i]) delete p;
	}
}

TCHAR MediaInfoDllPath[MAX_PATH] = _T("");
LRESULT CWizardDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	srand(unsigned int(time(0)));
	m_bShowWindow = true;
	   
	LPDWORD DlgCreationResult = (LPDWORD) lParam; 
	
	ATLASSERT(DlgCreationResult != NULL);
	// center the dialog on the screen
	CenterWindow();
	hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
	SetIcon(hIcon, TRUE);
	hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
	SetIcon(hIconSmall, FALSE);

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);
	OleInitialize(NULL);
	
	HRESULT res = ::RegisterDragDrop(m_hWnd,this);
	*MediaInfoDllPath=0;
	TCHAR Buffer[MAX_PATH];
	HKEY ExtKey;
	Buffer[0]=0;
	RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\KLCodecPack"), 0,/* REG_OPTION_NON_VOLATILE, */KEY_QUERY_VALUE,  &ExtKey/* NULL*/);
	TCHAR ClassName[MAX_PATH]=_T("\0");
	DWORD BufSize = sizeof(ClassName)/sizeof(TCHAR);
	DWORD Type = REG_SZ;
	RegQueryValueEx(ExtKey,	 _T("installdir"), 0, &Type, (LPBYTE)&ClassName, &BufSize);
	RegCloseKey(ExtKey);

	m_UpdateLink.ConvertStaticToHyperlink(GetDlgItem(IDC_UPDATESLABEL), _T("http://zenden.ws"));
	m_UpdateLink.setCommandID(IDC_UPDATESLABEL);
	
	CString MediaDll = GetAppFolder()+_T("\\Modules\\MediaInfo.dll");
	if(FileExists( MediaDll)) lstrcpy(MediaInfoDllPath, MediaDll);
	else
	{
		CString MediaDll2 =CString(ClassName)+_T("\\Tools\\MediaInfo.dll");
		if(FileExists( MediaDll2)) lstrcpy(MediaInfoDllPath, MediaDll2);
	}
	SetWindowText(APPNAME);

   Lang.SetDirectory(GetAppFolder() + "Lang\\");

	if(Settings.Language.IsEmpty())
	{
		CLangSelect LS;
		if(LS.DoModal(m_hWnd) == IDCANCEL)
		{
			*DlgCreationResult = 1;
			return 0;
		}
		Settings.Language = LS.Language;
		Lang.LoadLanguage(Settings.Language);
		
		/*if(MessageBox(TR("Добавить Image Uploader в контекстное меню проводника Windows?"),APPNAME, MB_YESNO|MB_ICONQUESTION)==IDYES)
		{
			Settings.ExplorerContextMenu = true;
			Settings.ExplorerContextMenu_changed = true;
			Settings.ExplorerVideoContextMenu = true;
		}	*/
		Settings.SaveSettings();
	}
	else 
	{
		Lang.LoadLanguage(Settings.Language);
	}

	
	CString ErrorStr;
	if(!LoadUploadEngines(IU_GetDataFolder()+_T("servers.xml"),ErrorStr))  // Завершаем работу программы, если файл servers.lst отсутствует
	{
		CString ErrBuf ;
		
		ErrBuf.Format(TR("Невозможно открыть файл со спиком серверов \"servers.xml\"!\n\nПричина:  %s\n\nПродолжить работу программы?"),(LPCTSTR)ErrorStr);
	
		if(MessageBox(ErrBuf, APPNAME, MB_ICONERROR|MB_YESNO)==IDNO)
		{
			*DlgCreationResult = 2;
			return 0;
		}
	}
	iuPluginManager.setScriptsDirectory(WCstringToUtf8(IU_GetDataFolder()+_T("\\Scripts\\")));
	std::vector<CString> list;
	CString serversFolder = IU_GetDataFolder()+_T("\\Servers\\");
	GetFolderFileList(list, serversFolder,_T("*.xml"));

	for(size_t i=0; i<list.size(); i++)
	{
		LoadUploadEngines(serversFolder+list[i], ErrorStr);
	}

	LoadUploadEngines(_T("userservers.xml"), ErrorStr);	
	Settings.ServerID		  = m_EngineList.GetUploadEngineIndex(Settings.ServerName);
	Settings.FileServerID  = m_EngineList.GetUploadEngineIndex(Settings.FileServerName);
	Settings.QuickServerID = m_EngineList.GetUploadEngineIndex(Settings.QuickServerName);
	if(!*MediaInfoDllPath)
		WriteLog(logWarning, APPNAME, TR("Библиотека MediaInfo.dll не найдена. \nПолучение технических данных о файлах мультимедиа будет недоступно.")); 
	TRC(IDC_ABOUT,"О программе...");
	if(!CmdLine.IsOption(_T("tray")))
		TRC(IDCANCEL,"Выход");
	else 
		TRC(IDCANCEL,"Скрыть");
	TRC(IDC_UPDATESLABEL, "Проверить обновления");
	TRC(IDC_PREV,"< Назад");

	ACCEL accel;
	accel.cmd = ID_PASTE;
	accel.fVirt = FCONTROL|FVIRTKEY;
	accel.key =  VkKeyScan('v') ;
	hAccel = CreateAcceleratorTable(&accel, 1);

	RegisterLocalHotkeys();
	if(ParseCmdLine()) return 0;
 
	CreatePage(0); 
	ShowPage(0);
	::SetFocus(Pages[0]->PageWnd);

	if(CmdLine.IsOption(_T("update")))
	{
		CreateUpdateDlg();
		updateDlg->ShowModal(m_hWnd);
	}
	else
	{
		if(time(0) - Settings.LastUpdateTime > 3600*24*7 /* 1 week */)
		{
			
			CreateUpdateDlg();
			updateDlg->Create(m_hWnd);
		
		}
	}
	return 0;  // Let the system set the focus
}

bool CWizardDlg::ParseCmdLine()
{ 
	int type = 0;
	int count = 0;

	int nIndex = 0;

	if(CmdLine.IsOption(_T("mediainfo")))
	{
		int nIndex = 0;
		CString VideoFileName;
		if(CmdLine.GetNextFile(VideoFileName, nIndex))
		{
			CMediaInfoDlg dlg;
			dlg.ShowInfo(VideoFileName);
			PostQuitMessage(0);
			return 0;
		}
	}

	for(size_t i=0; i<CmdLine.GetCount(); i++)
	{
		CString CurrentParam = CmdLine[i];
		if(CurrentParam .Left(6)==_T("/func="))
		{
			m_bShowWindow=false;
			CString cmd = CurrentParam.Right(CurrentParam.GetLength()-6);
			m_bHandleCmdLineFunc = true;
			if(!executeFunc(cmd))
				PostQuitMessage(0);
			return true;
		}
	}

	CString FileName;
	
	if(CmdLine.GetNextFile(FileName, nIndex))
	{
		if(IsVideoFile(FileName) && !CmdLine.IsOption(_T("upload")))
		{
			ShowPage(1, CurPage, (Pages[2])?2:3);
			CVideoGrabber* dlg = (CVideoGrabber*) Pages[1];
			dlg->SetFileName(FileName);			
			return true;
		}	
	}
	nIndex = 0;
	CStringList Paths;
	while(CmdLine.GetNextFile(FileName, nIndex))
	{
		if(FileExists(FileName) || IsDirectory(FileName))
		 Paths.Add(FileName);		
	}
	if(!Paths.IsEmpty())
	{
		QuickUploadMarker = (Settings.QuickUpload && !CmdLine.IsOption(_T("noquick"))) || (CmdLine.IsOption(_T("quick")));	
		FolderAdd.Do(Paths, CmdLine.IsOption(_T("imagesonly")), true);
	}
	return count!=0;
}

LRESULT CWizardDlg::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(floatWnd.m_hWnd)
	{ 
		ShowWindow(SW_HIDE);
		if(Pages[2] && CurPage == 4)
			((CMainDlg*)Pages[2])->ThumbsView.MyDeleteAllItems();
			ShowPage(0); 
	}
	else
		CloseWizard();
	return 0;
}

BOOL CWizardDlg::PreTranslateMessage(MSG* pMsg)
{	
	if(hLocalHotkeys &&TranslateAccelerator(m_hWnd, hLocalHotkeys, pMsg)) 
	{
		return TRUE;
	}

	if( pMsg->message == WM_KEYDOWN)
	{
		TCHAR Buffer[MAX_PATH];
		GetClassName(pMsg->hwnd, Buffer, sizeof(Buffer)/sizeof(TCHAR));
		if( pMsg->wParam == 'A' && !lstrcmpi(Buffer,_T("Edit") ) && GetKeyState(VK_CONTROL)<0)
		{
			::SendMessage(pMsg->hwnd, EM_SETSEL, 0, -1);	
			return TRUE;
		}

		if(VK_RETURN == pMsg->wParam  && GetForegroundWindow()==m_hWnd  )
		{
			if( !lstrcmpi(Buffer,_T("Button"))){
				::SendMessage(pMsg->hwnd, BM_CLICK, 0 ,0); return TRUE;}
			else if (Pages[0] && pMsg->hwnd==::GetDlgItem(Pages[0]->PageWnd,IDC_LISTBOX))
				return FALSE;
		}
		
		if(VK_BACK == pMsg->wParam)
		{
			if(Pages[CurPage] && VK_BACK == pMsg->wParam  && GetForegroundWindow() == m_hWnd && lstrcmpi(Buffer,_T("Edit") ))
			{
				if(pMsg->message==WM_KEYDOWN && ::IsWindowEnabled(GetDlgItem(IDC_PREV)))
				{ 
					OnPrevBnClicked(0,0,0); 
					return TRUE;
				}
				else if (pMsg->message == WM_KEYUP) 
					return TRUE;
			}
		}
	}
	
	return CWindow::IsDialogMessage(pMsg);
}

BOOL CWizardDlg::OnIdle()
{
	return FALSE;
}

LRESULT CWizardDlg::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// unregister message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->RemoveMessageFilter(this);
	pLoop->RemoveIdleHandler(this);
	return 0;
}

void CWizardDlg::CloseDialog(int nVal)
{
	if(updateDlg)
		updateDlg->Abort();
	ShowWindow(SW_HIDE);
	if(CurPage >= 0)
		Pages[CurPage]->OnHide();
	
	Exit();
	DestroyWindow();
	::PostQuitMessage(nVal);
}

bool CWizardDlg::ShowPage(int idPage,int prev,int next)
{
	if(idPage == CurPage) return true;

	if(GetCurrentThreadId()!=GetWindowThreadProcessId(m_hWnd, NULL))
	{
		return SendMessage(WM_MY_SHOWPAGE, (WPARAM)idPage)!=FALSE;
	}
   
 	if(!CreatePage(idPage)) return false;

	SetDlgItemText(IDC_NEXT,TR("Далее >"));

	HBITMAP bmp = Pages[idPage]->HeadBitmap;
	if(!bmp) ::ShowWindow(GetDlgItem(IDC_HEADBITMAP),SW_HIDE);
	else
	{
		::ShowWindow(GetDlgItem(IDC_HEADBITMAP),SW_SHOW);
		SendDlgItemMessage(IDC_HEADBITMAP,STM_SETIMAGE,IMAGE_BITMAP,(LPARAM)bmp);
	}

	::ShowWindow(Pages[idPage]->PageWnd, SW_SHOW);
	::SetFocus(Pages[idPage]->PageWnd);
	Pages[idPage]->OnShow();
	
		::ShowWindow(GetDlgItem(IDC_UPDATESLABEL), idPage == 0);
	::ShowWindow(GetDlgItem(IDC_ABOUT), idPage == 0);
	if(CurPage >= 0)
	{
		
		::ShowWindow(Pages[CurPage]->PageWnd, SW_HIDE);
		Pages[CurPage]->OnHide();
	}
	
	PrevPage = prev;
	NextPage = next;
	CurPage = idPage;
	return false;
}

LRESULT CWizardDlg::OnPrevBnClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
	if(PrevPage<0)
	{
		PrevPage = CurPage-1;
		if(PrevPage<0 || PrevPage==1)  PrevPage = 0;
	}	

	ShowPage(PrevPage);
	PrevPage=-1;
	return 0;
}

LRESULT CWizardDlg::OnNextBnClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
	if(!::IsWindowVisible(hWndCtl)) return 0;
	if(!Pages[CurPage]->OnNext()) return 0;
	if(NextPage < 0)
	{
		NextPage = CurPage+1;
		if(NextPage>4 ) NextPage=0;
		if(NextPage==1) NextPage=2;
	}	
	ShowPage(NextPage);
	NextPage = -1;
	return 0;
}

bool CWizardDlg::CreatePage(int PageID)
{
	RECT rc = {3,3,636,500};
	RECT rc2 = {3,100,636,500};
	RECT rcc;
	GetClientRect(&rcc);
	int width = rcc.right - rcc.left;
	int height = 430;
	if(((PVOID)Pages[PageID])!=0) return true;;
	switch(PageID)
	{
		case 0:
			CWelcomeDlg *tmp;
			tmp = new CWelcomeDlg();
			Pages[PageID] = tmp;
			Pages[PageID]->WizardDlg=this;
			tmp->Create(m_hWnd,rc);
			break;

		case 1:
			CVideoGrabber *tmp1;
			tmp1=new CVideoGrabber();
			Pages[PageID]=tmp1;
			Pages[PageID]->WizardDlg=this;
			tmp1->Create(m_hWnd,rc);
			break;

		case 2:
			CMainDlg *tmp2;
			tmp2=new CMainDlg();
			Pages[PageID]=tmp2;
			Pages[PageID]->WizardDlg=this;
			tmp2->Create(m_hWnd,rc);
			break;

		case 3:
			CUploadSettings *tmp3;
			tmp3=new CUploadSettings(&m_EngineList);
			Pages[PageID]=tmp3;
			Pages[PageID]->WizardDlg=this;
			tmp3->Create(m_hWnd,rc2);
			tmp3->SetWindowPos(0,0,50,0,0,SWP_NOSIZE);
			break;
		case 4:
			CUploadDlg *tmp4;
			tmp4=new CUploadDlg(this);
			Pages[PageID]=tmp4;
			Pages[PageID]->WizardDlg=this;
			tmp4->Create(m_hWnd, rc);
			tmp4->SetWindowPos(0, 0, 50, 0, 0,SWP_NOSIZE);
			break;
		default:
			return false;
	}
	Pages[PageID]->HeadBitmap = GenHeadBitmap(PageID);
	return true;
}

// Функция генерации заголовка страницы (если он нужен)
HBITMAP CWizardDlg::GenHeadBitmap(int PageID)
{
	if(PageID!=3 && PageID!=4) return 0;
	RECT rc;
	GetClientRect(&rc);
	int width=rc.right-rc.left;
	RectF bounds(0.0,0.0, float(width), float(50));
	Bitmap *BackBuffer;
	Graphics g(m_hWnd,true);
	
	BackBuffer = new Bitmap(width, 50, &g);
	if(!BackBuffer) return 0;
	Graphics gr(BackBuffer);
	COLORREF color=GetSysColor(COLOR_BTNFACE);
	
	LinearGradientBrush 
		brush(bounds, Color(255, 255, 255, 255), Color(255, 235,235,235), 
            LinearGradientModeVertical);
	gr.FillRectangle(&brush,bounds);

	LinearGradientBrush 
		br2(bounds, Color(130, 190, 190, 190), Color(255, 70, 70, 70), 
            LinearGradientModeBackwardDiagonal); 


		 StringFormat format;
    format.SetAlignment(StringAlignmentCenter);
    format.SetLineAlignment(StringAlignmentCenter);
    Font font(L"Arial", 12, FontStyleBold);

    
	LPTSTR Buffer = NULL;
	if(PageID == 3)
		gr.DrawString(TR("Параметры изображений и выбор сервера"), -1, &font, bounds, &format, &br2);
	else if(PageID==4)
		gr.DrawString(TR("Загрузка файлов на сервер"), -1, &font, bounds, &format, &br2);

	HBITMAP bmp=0;
	BackBuffer->GetHBITMAP(Color(255,255,255), &bmp);
	delete BackBuffer;
	return bmp;
}

LRESULT CWizardDlg::OnBnClickedAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add your control notification handler code here
	CAboutDlg dlg;
	dlg.DoModal();
	return 0;
}

void CWizardDlg::Exit()
{
	// Converting server id to server name
	if(Settings.ServerID >= 0 && m_EngineList.count())
		Settings.ServerName = Utf8ToWstring(m_EngineList.byIndex(Settings.ServerID)->Name).c_str();

	if(Settings.QuickServerID>=0 && m_EngineList.count())
		Settings.QuickServerName = Utf8ToWstring(m_EngineList.byIndex(Settings.QuickServerID)->Name).c_str();
	if(Settings.FileServerID>=0 && m_EngineList.count())
		Settings.FileServerName = Utf8ToWstring(m_EngineList.byIndex(Settings.FileServerID)->Name).c_str();

	Settings.SaveSettings();
}

LRESULT CWizardDlg::OnDropFiles(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
	bHandled = true;
	HDROP hDrop = (HDROP) wParam;
	TCHAR szBuffer[256] = _T("\0");
	if(CurPage > 2) return 0;

	int n = DragQueryFile(hDrop,	0xFFFFFFFF, 0, 0);

	CMainDlg* MainDlg = NULL;
	CStringList Paths;
	for (int i=0; i<n; i++)
	{

		DragQueryFile(hDrop,	i, szBuffer, sizeof(szBuffer)/sizeof(TCHAR));
		if(IsVideoFile(szBuffer) && n==1)
		{
			if(CurPage == 2)
			{
				if(MessageBox(TR("Вы хотите извлечь кадры из этого видеофайла? \r\n(иначе файл будет просто добавлен в список)"),APPNAME,MB_YESNO)==IDNO)
					goto filehost;
			}
			ShowPage(1, CurPage, (Pages[2])?2:3);
			CVideoGrabber* dlg = (CVideoGrabber*) Pages[1];
			dlg->SetFileName(szBuffer);
			
			break;
		}
		else if(CurPage==0||CurPage==2)
		{
			filehost:
			if(FileExists(szBuffer) || IsDirectory(szBuffer))
									 Paths.Add(szBuffer);			
		}
 
	}
	if(!Paths.IsEmpty())
	{
		CreatePage(2);
		FolderAdd.Do(Paths, false, true);
		ShowPage(2);
		if(MainDlg) MainDlg->ThumbsView.LoadThumbnails();
	}
	
	DragFinish(hDrop);
	return 0;
   
}

bool CWizardDlg::LoadUploadEngines(const CString &filename, CString &Error)
{
	m_EngineList.setNumOfRetries(Settings.FileRetryLimit, Settings.ActionRetryLimit);
	bool Result = m_EngineList.LoadFromFile(filename);
	Error = m_EngineList.ErrorStr();
	return Result;
}

STDMETHODIMP_(ULONG) CWizardDlg::AddRef()
{
	return InterlockedIncrement( &m_lRef );
}

STDMETHODIMP_(ULONG) CWizardDlg::Release()
{
	if ( InterlockedDecrement( &m_lRef ) == 0 )
   {
		//    delete this;
		return 0;
   }
	return m_lRef;
}

STDMETHODIMP CWizardDlg::QueryInterface( REFIID riid, void** ppv )
{
	*ppv = NULL;

	if ( riid == IID_IUnknown || riid == IID_IDropTarget )
		*ppv = this;

	if ( *ppv )
	{
		AddRef();
		return( S_OK );
	}
	return (E_NOINTERFACE);
}

//    IDropTarget methods
STDMETHODIMP CWizardDlg::DragEnter(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
	return S_OK;
}
    
STDMETHODIMP CWizardDlg::DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
	bool AcceptFile = true;
	if(!IsWindowEnabled() || !DragndropEnabled ) 
	{
	AcceptFile = false;
		
	}

	if(CurPage!=0&&CurPage!=2&&CurPage!=1) AcceptFile = false;

	if(!AcceptFile) 
	{
		*pdwEffect = DROPEFFECT_NONE;
		return S_FALSE;
	}
	*pdwEffect = DROPEFFECT_COPY;
	
	return S_OK;
}
    
STDMETHODIMP CWizardDlg::DragLeave( void)
{
	return S_OK;
}

CString MakeTempFileName(const CString FileName)
{
	CString FileNameBuf;
	FileNameBuf = IUTempFolder + FileName;

   if(FileExists(FileNameBuf))
	{
		CString OnlyName;
		OnlyName = GetOnlyFileName(FileName);
		CString Ext = GetFileExt(FileName);
		FileNameBuf = IUTempFolder + OnlyName + _T("_")+IntToStr(GetTickCount()^33333) + (Ext? _T("."):_T("")) + Ext;
	}
	return FileNameBuf;
}

bool SaveFromHGlobal(HGLOBAL Data, const CString FileName, CString &OutName)
{
	if(!Data) return false;
	CString FileNameBuf = MakeTempFileName(FileName);  

	DWORD filesize = GlobalSize(Data);
	if(!filesize) 
		return false;
	PVOID LockedData =(PVOID) GlobalLock (Data);

	HANDLE hFile = CreateFile(FileNameBuf, GENERIC_WRITE, 
                          0, NULL, CREATE_ALWAYS, 
                          FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		GlobalUnlock(Data);
		return false;
	}
				
	ULONG cbRead;

	WriteFile(hFile, LockedData, filesize, &cbRead, NULL);
				
	CloseHandle(hFile);
	GlobalUnlock(Data);
	OutName = FileNameBuf; 
	return true;
}

bool SaveFromIStream(IStream *pStream, const CString FileName, CString &OutName)
{
	if(!pStream) return false;
	CString FileNameBuf = MakeTempFileName(FileName);  

	HANDLE hFile = CreateFile(FileNameBuf, GENERIC_WRITE, 
                          0, NULL, CREATE_ALWAYS, 
                          FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return false;
				
	UCHAR bBuffer[4096];
	ULONG cbRead;
			
	while (SUCCEEDED(pStream->Read(bBuffer, sizeof(bBuffer), &cbRead)) && cbRead > 0)
	{
		WriteFile(hFile, bBuffer, cbRead, &cbRead, NULL);
	}

	CloseHandle(hFile);
	OutName = FileNameBuf;
	return true;
}

bool CWizardDlg::HandleDropFiledescriptors(IDataObject *pDataObj)
{
	FORMATETC tc2 = { RegisterClipboardFormat(CFSTR_FILEDESCRIPTOR), 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };

	if(pDataObj->QueryGetData(&tc2)==S_OK )
	{
		STGMEDIUM ddd;

		if(pDataObj->GetData(&tc2, &ddd)==S_OK ){

			PVOID hdrop = (PVOID) GlobalLock ( ddd.hGlobal );
			FILEGROUPDESCRIPTOR *fgd = (FILEGROUPDESCRIPTOR*) hdrop;
			CStringList Paths;
			for(size_t i=0; i<fgd->cItems; i++)
			{
				FORMATETC tc3 = { RegisterClipboardFormat(CFSTR_FILECONTENTS), 0, DVASPECT_CONTENT, i, TYMED_HGLOBAL };
				if(pDataObj->QueryGetData(&tc3)==S_OK )
				{
					STGMEDIUM ddd2;
					ddd2.tymed= TYMED_HGLOBAL;
					if(pDataObj->GetData(&tc3, &ddd2)==S_OK )
					{
						CString OutFileName;
						bool FileWasSaved = false;

						if(ddd2.tymed == TYMED_HGLOBAL)
						{
							FileWasSaved = SaveFromHGlobal(ddd2.hGlobal, fgd->fgd[i].cFileName, OutFileName);
						}

						if(ddd2.tymed== TYMED_ISTREAM)
						{	
							FileWasSaved = SaveFromIStream(ddd2.pstm, fgd->fgd[i].cFileName, OutFileName); 
						}

						if(FileWasSaved) // Additing received file to program
						{
							if(IsVideoFile(OutFileName))
							{
								ShowPage(1, CurPage, (Pages[2])?2:3);
								CVideoGrabber* dlg = (CVideoGrabber*) Pages[1];
								dlg->SetFileName(OutFileName);
								break;
							}
							else if((CurPage==0||CurPage==2))
							{
								
								if(FileExists(OutFileName) || IsDirectory(OutFileName))
									 Paths.Add(OutFileName);		
							}
						}
					}
				}

				GlobalUnlock ( hdrop );
				
				if(!Paths.IsEmpty())
				{
					CreatePage(2);
					//QuickUploadMarker = (Settings.QuickUpload && !CmdLine.IsOption(_T("noquick"))) || (CmdLine.IsOption(_T("quick")));
					FolderAdd.Do(Paths, /*CmdLine.IsOption(_T("imagesonly"))*/false, true);
					ShowPage(2);				
				}
				return true;
			}
		}
	}
	return false;
}

bool CWizardDlg::HandleDropHDROP(IDataObject *pDataObj)
{
	FORMATETC tc = { CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	if(pDataObj->QueryGetData(&tc)==S_OK ) 
	{
		STGMEDIUM ddd;
		if(pDataObj->GetData(&tc, &ddd)==S_OK)
		{
			PVOID hdrop = (PVOID) GlobalLock ( ddd.hGlobal );
			BOOL b;
			OnDropFiles(0,(WPARAM) hdrop, 0 ,b);
			GlobalUnlock ( hdrop );
			return true;
		}
	}
	return false;
}

bool CWizardDlg::HandleDropBitmap(IDataObject *pDataObj)
{
	FORMATETC FtcBitmap;
	FtcBitmap.cfFormat = CF_BITMAP;
	FtcBitmap.ptd = 0;
	FtcBitmap.dwAspect = 1;
	FtcBitmap.lindex = DVASPECT_CONTENT;
	FtcBitmap.tymed =  TYMED_HGLOBAL;

	if(pDataObj->QueryGetData(&FtcBitmap) == S_OK ) 
	{
		STGMEDIUM ddd;
		if(pDataObj->GetData(&FtcBitmap, &ddd)==S_OK)
		{
			PasteBitmap(ddd.hBitmap);

			return true;
		}
	}
	return false;
}

STDMETHODIMP CWizardDlg::Drop(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
	if(!IsWindowEnabled() || !DragndropEnabled) 
	{
		*pdwEffect = DROPEFFECT_NONE; 
		return S_FALSE;
	}

	if(HandleDropHDROP(pDataObj))
		return S_OK;


	if(HandleDropFiledescriptors(pDataObj))
		return S_OK;

	if(HandleDropBitmap(pDataObj))
		return S_OK;

	return S_OK;
}

LRESULT CWizardDlg::OnPaste(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(IsClipboardFormatAvailable(CF_BITMAP)) 
	{
		if(!OpenClipboard()) return 0;
		HBITMAP bmp = (HBITMAP) GetClipboardData(CF_BITMAP);

		if(!bmp) return CloseClipboard();

		PasteBitmap(bmp);
		CloseClipboard();
	}

	if(IsClipboardFormatAvailable(CF_UNICODETEXT)) 
	{
		CString text;
		IU_GetClipboardText(text);
		if(CImageDownloaderDlg::LinksAvailableInText(text))
		{
			CImageDownloaderDlg dlg(this,CString(text));
			dlg.DoModal(m_hWnd);
		}
	}	
	return 0;
}

void CWizardDlg::PasteBitmap(HBITMAP Bmp)
{
	if(CurPage!=0 && CurPage!=2) return;

	CString buf2;
	SIZE dim;
	GetBitmapDimensionEx(Bmp, &dim);
	Bitmap bm(Bmp,0);
	if(bm.GetLastStatus()==Ok)
	{
		MySaveImage(&bm,_T("clipboard"),buf2,1,100);

		CreatePage(2);
		CMainDlg* MainDlg = (CMainDlg*) Pages[2];
		MainDlg->AddToFileList( buf2);
		MainDlg->ThumbsView.LoadThumbnails();
		ShowPage(2);
	}
}


void CWizardDlg::AddFolder(LPCTSTR szFolder, bool SubDirs )
{
   CString Folder = szFolder;
	if(Folder[Folder.GetLength()-1]==_T('\\'))
		Folder.Delete(Folder.GetLength()-1);

	CStringList Paths;
	Paths.Add(Folder );
	FolderAdd.Do(Paths, true, SubDirs);
}

CFolderAdd::CFolderAdd(CWizardDlg *WizardDlg): m_pWizardDlg(WizardDlg)
{
	m_bSubDirs = true; 
}

void CFolderAdd::Do(CStringList &Paths, bool ImagesOnly, bool SubDirs)
{
	count = 0;
	m_bImagesOnly = ImagesOnly;
	RECT rc={0,0,0,0};
	m_bSubDirs = SubDirs;
   if(!dlg.m_hWnd)
	dlg.Create(m_pWizardDlg->m_hWnd, rc);
	dlg.SetWindowTitle(CString(ImagesOnly?TR("Поиск графических файлов..."):TR("Сбор файлов...")));
	m_Paths.RemoveAll();
	m_Paths.Copy(Paths);
	findfile = 0;
	ZeroMemory(&wfd, sizeof(wfd));

	Start(THREAD_PRIORITY_BELOW_NORMAL);
}

int CFolderAdd::ProcessDir( CString currentDir, bool bRecursive /* = true  */ )
{
    CString strWildcard;
	
	 dlg.SetInfo(CString(TR("Обрабатывается каталог:")), currentDir);
    strWildcard = currentDir + "\\*";

    _tfinddata_t s_Dir;
    intptr_t hDir;
  
    if( (hDir = _tfindfirst( strWildcard, &s_Dir )) == -1L )
        
		 return 1;

    do
    {
		 if(dlg.NeedStop()) { _findclose( hDir ); return 0;}

        if( s_Dir.name[ 0 ] != '.'  && ( s_Dir.attrib & _A_SUBDIR ) && bRecursive == true )
        {
            ProcessDir( currentDir + '\\' + s_Dir.name, bRecursive );
        }
        else if ( s_Dir.name[ 0 ] != '.' )
		  {
				if(!m_bImagesOnly || IsImage(s_Dir.name))
				{
					AddImageStruct ais;
					ais.show = !m_pWizardDlg->QuickUploadMarker;
					CString name = CString(currentDir) + CString(_T("\\"))+ CString( s_Dir.name);
					ais.RealFileName = name;
					if(SendMessage(m_pWizardDlg->m_hWnd, WM_MY_ADDIMAGE,(WPARAM) &ais,0 ))
						count++;
				}
		  }
	 } while( _tfindnext( hDir, &s_Dir ) == 0 );

    _findclose( hDir );

    return 0;
}

DWORD CFolderAdd::Run()
{
	EnableWindow(m_pWizardDlg->m_hWnd, false);
	for(size_t i=0; i<m_Paths.GetCount(); i++)
	{
		CString CurPath = m_Paths[i];
		if(IsDirectory(CurPath))
			ProcessDir(CurPath, m_bSubDirs);
		else 
			if(!m_bImagesOnly || IsImage(CurPath))
			{
				AddImageStruct ais;
				ais.show = !m_pWizardDlg->QuickUploadMarker;
				CString name = CurPath;
				ais.RealFileName = CurPath;
				if(SendMessage(m_pWizardDlg->m_hWnd, WM_MY_ADDIMAGE,(WPARAM) &ais,0 ))
		
				count++;
			}
		if(dlg.NeedStop()) break;
	}

	dlg.Hide();
	EnableWindow(m_pWizardDlg->m_hWnd, true);

	if(!count) 
		MessageBox(m_pWizardDlg->m_hWnd, m_bImagesOnly?TR("Не найдено ни одного файла изображений."):TR("Не найдено ни одного файла."), APPNAME, MB_ICONINFORMATION);
	else 	
	{
		if( m_pWizardDlg->QuickUploadMarker)
		{

			m_pWizardDlg->ShowPage(4);
		}
		else
		{

			m_pWizardDlg->ShowPage(2);

			((CMainDlg*) m_pWizardDlg->Pages[2])->ThumbsView.LoadThumbnails();
		}
	}
	dlg.DestroyWindow();
	dlg.m_hWnd = NULL;
	return 0;
}

int CFolderAdd::GetNextImgFile(LPTSTR szBuffer, int nLength)
{
	TCHAR szBuffer2[MAX_PATH], TempPath[256];
	
	GetTempPath(256, TempPath);
	wsprintf(szBuffer2,_T("%s*.*"), (LPCTSTR)m_szPath);
	
	if(!findfile)
	{
		findfile=FindFirstFile(szBuffer2, &wfd);
		if(!findfile) goto error;
	}
	else 
	{
		if(!FindNextFile(findfile, &wfd))
			goto error;

	}
	if(lstrlen(wfd.cFileName) < 1) goto error;
	lstrcpyn(szBuffer, wfd.cFileName, nLength);

	return TRUE;

	error:
	if(findfile) FindClose(findfile);
	return FALSE;
}

bool CWizardDlg::AddImage(const CString &FileName, const CString &VirtualFileName, bool Show)
{
	CreatePage(2);
	CMainDlg* MainDlg = (CMainDlg*) Pages[2];
	if(!MainDlg) return false;
	MainDlg->AddToFileList(FileName, VirtualFileName);
	if(Show){
		MainDlg->ThumbsView.LoadThumbnails();
		ShowPage(2);
	}
	return true;
}

LRESULT CWizardDlg::OnAddImages(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{	
	AddImageStruct* ais = reinterpret_cast<AddImageStruct*>(wParam);
	if(!ais) return 0;
	return  AddImage(ais->RealFileName, ais->VirtualFileName, ais->show);
}

CMyFolderDialog::CMyFolderDialog(HWND hWnd):
				CFolderDialogImpl(hWnd, TR("Выбор папки"), BIF_RETURNONLYFSDIRS|BIF_NEWDIALOGSTYLE|BIF_NONEWFOLDERBUTTON|BIF_VALIDATE )
{
	OleInitialize(NULL);
}
void CMyFolderDialog::OnInitialized()
{
	HWND wnd = CreateWindowEx(0, _T("button"), TR("Включая поддиректории"), WS_VISIBLE|BS_CHECKBOX|WS_CHILD|BS_AUTOCHECKBOX, 15,30, 200,24, m_hWnd, 0,0, 0);
	SendMessage(wnd, WM_SETFONT, (WPARAM)SendMessage(m_hWnd, WM_GETFONT, 0,0),  MAKELPARAM(false, 0));
	SendMessage(wnd, BM_SETCHECK, (WPARAM)(m_bSubdirs?BST_CHECKED	:BST_UNCHECKED),0);
	SetProp(m_hWnd, PROP_OBJECT_PTR, (HANDLE) this);
	OldProc  = (DLGPROC) SetWindowLongPtr(m_hWnd, DWLP_DLGPROC, (LONG_PTR)DialogProc);	
	SubdirsCheckbox = wnd;
	m_bSubdirs = true;
}

//  Overloaded WinProc function for BrowseForFolders dialog
BOOL CALLBACK CMyFolderDialog::DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CMyFolderDialog *th = (CMyFolderDialog *) GetProp(hwndDlg, PROP_OBJECT_PTR);
	if(!th) return FALSE;

	if(uMsg == WM_COMMAND && HIWORD(wParam)== BN_CLICKED && ((HWND) lParam)== th->SubdirsCheckbox)
		th->m_bSubdirs = SendMessage(th->SubdirsCheckbox, BM_GETCHECK,0,0) == BST_CHECKED;
	
	return th->OldProc(hwndDlg, uMsg, wParam, lParam);
}

 LRESULT CWizardDlg::OnEraseBkg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	bHandled = true; 
	return 1;
}
	
LRESULT 	CWizardDlg::OnWmShowPage(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	int PageIndex = wParam;
	ShowPage(PageIndex);
	return 0;
}
	
bool CWizardDlg::funcAddImages(bool AnyFiles)
{
	TCHAR Buf[MAX_PATH*4];
	if(AnyFiles)
		SelectDialogFilter(Buf, sizeof(Buf)/sizeof(TCHAR),1,TR("Любые файлы"),
		_T("*.*"));
	else
	SelectDialogFilter(Buf, sizeof(Buf)/sizeof(TCHAR),2, 
		CString(TR("Изображения"))+ _T(" (jpeg, bmp, png, gif ...)"),
		_T("*.jpg;*.gif;*.png;*.bmp;*.tiff"),
		TR("Любые файлы"),
		_T("*.*"));

	int nCount=0;
	CMultiFileDialog fd(0, 0, OFN_HIDEREADONLY, Buf, m_hWnd);
	
	TCHAR Buffer[1000];
	fd.m_ofn.lpstrInitialDir = Settings.ImagesFolder;

	if(fd.DoModal(m_hWnd) != IDOK) return 0;
	LPCTSTR FileName = 0;
	fd.GetDirectory(Buffer, sizeof(Buffer)/sizeof(TCHAR));

	CreatePage(2);
	do
	{
		
		FileName = (FileName) ? fd.GetNextFileName() : fd.GetFirstFileName();
		if(!FileName) break;
		fd.GetDirectory(Buffer, sizeof(Buffer)/sizeof(TCHAR));

		if(Buffer[lstrlen(Buffer)-1] != '\\')
		lstrcat(Buffer, _T("\\"));
		
		if(FileName)
		{
			lstrcat(Buffer, FileName);
			if(((CMainDlg*)Pages[2])->AddToFileList(Buffer))
				nCount++;
		
		}
	} while (FileName);
	 
	
	fd.GetDirectory(Buffer, sizeof(Buffer)/sizeof(TCHAR));
	Settings.ImagesFolder = Buffer;
	if(nCount)
		ShowPage(2, 0, 3);

	if(CurPage == 2)
		((CMainDlg*)Pages[2])->ThumbsView.LoadThumbnails();
	ShowWindow(SW_SHOW);
	m_bShowWindow = true;
	return true;
}

bool CWizardDlg::executeFunc(CString funcBody)
{
	bool LaunchCopy = false;

	if(CurPage == 4) LaunchCopy = true;
	if(CurPage == 1) LaunchCopy = true;
	if(CurPage == 3) ShowPage(2);

	if(!IsWindowEnabled())LaunchCopy= true; 

	CString funcName = StringSection(funcBody,  _T(','), 0 );
	CString funcParam1 = StringSection(funcBody, _T(','), 1);

	if(!funcParam1.IsEmpty())
	{
			m_bScreenshotFromTray = _ttoi(funcParam1);
	}
	if(LaunchCopy)
	{
		if(Settings.TrayIconSettings.DontLaunchCopy)
		{	
			 if(IsWindowVisible() && IsWindowEnabled())
				SetForegroundWindow(m_hWnd);
			else if(!IsWindowEnabled()) SetActiveWindow();
			FlashWindow(true);
		}
		else
			IULaunchCopy(_T("/func=")+funcBody,CAtlArray<CString>());
		return false;
	}
	if(funcName == _T("addimages"))
		return funcAddImages();
	else if(funcName == _T("addfiles"))
		return funcAddImages(true);
	if(funcName == _T("addfiles"))
		return funcAddFiles();
	else if(funcName == _T("importvideo"))
		return funcImportVideo();
	else if(funcName == _T("screenshotdlg"))
		return funcScreenshotDlg();
	else if(funcName == _T("regionscreenshot"))
		return funcRegionScreenshot();
	else if(funcName == _T("regionscreenshot_dontshow"))
		return funcRegionScreenshot(false);
	else if(funcName == _T("fullscreenshot"))
		return funcFullScreenshot();
	else if(funcName == _T("windowhandlescreenshot"))
		return funcWindowHandleScreenshot();
	else if(funcName == _T("freeformscreenshot"))
			return funcFreeformScreenshot();
	else if(funcName == _T("downloadimages"))
		return funcDownloadImages();
	else if(funcName == _T("windowscreenshot"))
		return funcWindowScreenshot();
	else if(funcName == _T("windowscreenshot_delayed"))
		return funcWindowScreenshot(true);
	else if(funcName == _T("addfolder"))
		return funcAddFolder();
	else if(funcName == _T("paste"))
		return funcPaste();
	else if(funcName == _T("settings"))
		return funcSettings();
	else if(funcName == _T("mediainfo"))
		return funcMediaInfo();
	return false;
}

bool CWizardDlg::funcImportVideo()
{
	TCHAR Buf[MAX_PATH*4];
	SelectDialogFilter(Buf, sizeof(Buf)/sizeof(TCHAR),2, 
			CString(TR("Видео файлы"))+ _T(" (avi, mpg, vob, wmv ...)"),
		_T("*.avi;*.mpeg;*.mpg;*.mp2;*.divx;*.vob;*.flv;*.wmv;*.asf;*.mkv;*.mp4;*.ts;*.mov;*.mpeg2ts;*.3gp;*.rm;"),
		TR("Все файлы"),
		_T("*.*"));

	CFileDialog fd(true,0,0,4|2,/*VIDEO_DIALOG_FORMATS*/Buf,m_hWnd);
	
	TCHAR Buffer[1000];
	fd.m_ofn.lpstrInitialDir = Settings.VideoFolder;
	if(fd.DoModal()!=IDOK || !fd.m_szFileName) return 0;
	ExtractFilePath(fd.m_szFileName, Buffer); 
	Settings.VideoFolder = Buffer;
	CreatePage(1);
	LastVideoFile = fd.m_szFileName;
	((CVideoGrabber*)Pages[1])->SetFileName(fd.m_szFileName); // C-style conversion .. 
	ShowPage(1,0,(Pages[2])?2:3);
	ShowWindow(SW_SHOW);
		m_bShowWindow = true;
	return true;
}

bool CWizardDlg::funcScreenshotDlg()
{
	CScreenshotDlg dlg;
	if(dlg.DoModal(m_hWnd) != IDOK) return false;
	
	CommonScreenshot(dlg.captureMode()); 
	m_bShowWindow = true;
	return true;
}

bool CWizardDlg::funcRegionScreenshot(bool ShowAfter)
{
	m_bShowAfter = ShowAfter;
	CommonScreenshot(cmRectangles);
	return true;
}

void CWizardDlg::OnScreenshotFinished(int Result)
{
	EnableWindow();

	if(m_bShowAfter || (Result && !floatWnd.m_hWnd))
	{
		m_bShowWindow = true;
		ShowWindow(SW_SHOWNORMAL);
		SetForegroundWindow(m_hWnd);
	}

	if(Result )
	{
		if((CMainDlg*)Pages[2])
		{
			((CMainDlg*)Pages[2])->ThumbsView.SetFocus();
			((CMainDlg*)Pages[2])->ThumbsView.SelectLastItem();
		}
	}
	else if (!Result && m_bHandleCmdLineFunc)
	{
		
		PostQuitMessage(0);
	}
	m_bHandleCmdLineFunc = false;

}

void CWizardDlg::OnScreenshotSaving(LPTSTR FileName, Bitmap* Bm)
{
	if(FileName && lstrlen(FileName))
	{
		CreatePage(2);
		((CMainDlg*)Pages[2])->AddToFileList(FileName);
		if(CurPage == 2)
		((CMainDlg*)Pages[2])->ThumbsView.LoadThumbnails();
		ShowPage(2,0,3);
	}
}

bool CWizardDlg::funcFullScreenshot()
{
	CommonScreenshot(cmFullScreen);	
	return true;
}

bool CWizardDlg::funcWindowScreenshot(bool Delay)
{
	CommonScreenshot(cmActiveWindow);
	return true;
}

bool CWizardDlg::funcAddFolder()
{
	CMyFolderDialog fd(m_hWnd);
	fd.m_bSubdirs = Settings.ParseSubDirs;
	if(fd.DoModal(m_hWnd) == IDOK)
	{
		Settings.ParseSubDirs = fd.m_bSubdirs;
		ShowWindow(SW_SHOW);
		m_bShowWindow = true;
		AddFolder(fd.GetFolderPath(),fd.m_bSubdirs);
		
		return true;
	}
	else return false;
}
LRESULT CWizardDlg::OnEnable(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	if(!floatWnd.m_hWnd)
	  TRC(IDCANCEL, "Выход");
	else 
		TRC(IDCANCEL, "Скрыть");

	if(!(m_hotkeys == Settings.Hotkeys))
	{
		UnRegisterLocalHotkeys();
		RegisterLocalHotkeys();
	}
	return 0;
}

void CWizardDlg::CloseWizard()
{
	if(CurPage!=0 && CurPage!=4 && Settings.ConfirmOnExit)
		if(MessageBox(TR("Вы уверены что хотите выйти из программы?"),APPNAME, MB_YESNO|MB_ICONQUESTION) != IDYES) return ;
	
	CloseDialog(0);
}


bool CWizardDlg::RegisterLocalHotkeys()
{
	ACCEL *Accels;
	m_hotkeys = Settings.Hotkeys;
	int n=m_hotkeys.GetCount();
	Accels = new ACCEL [n];
	int j =0;
	for(int i =0; i<n; i++)
	{
		if(!m_hotkeys[i].localKey.keyCode) continue;
		Accels[j]= m_hotkeys[i].localKey.toAccel();
		Accels[j].cmd = 10000+i;
		j++;
			
	}

	hLocalHotkeys = CreateAcceleratorTable(Accels,j);
	delete[] Accels;
	return true;
}

LRESULT CWizardDlg::OnLocalHotkey(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(CurPage==3) ShowPage(2);
	if(!IsWindowEnabled() || (CurPage!=0 && CurPage!=2))
		return 0;
	int hotkeyId = wID-ID_HOTKEY_BASE;
	executeFunc(m_hotkeys[hotkeyId].func);
	return 0;
}

bool CWizardDlg::UnRegisterLocalHotkeys()
{
	DestroyAcceleratorTable(hLocalHotkeys);
	m_hotkeys.RemoveAll();
	hLocalHotkeys = 0;
	return true;
}

bool CWizardDlg::funcPaste()
{
	BOOL b;
	OnPaste(0,0,0,b);
	return true;
}

bool CWizardDlg::funcSettings()
{
	CSettingsDlg dlg(0);
	//dlg.DoModal(m_hWnd);
	if(!IsWindowVisible())
		dlg.DoModal(0);
	else
		dlg.DoModal(m_hWnd);
	return true;
}

bool CWizardDlg::funcDownloadImages()
{
	CImageDownloaderDlg dlg(this,CString());
	dlg.DoModal(m_hWnd);
	return true;
}

bool CWizardDlg::funcMediaInfo()
{
	TCHAR Buf[MAX_PATH*4]; //String buffer which will contain filter for CFileDialog
	SelectDialogFilter(Buf, sizeof(Buf)/sizeof(TCHAR),3, 
			CString(TR("Видео файлы"))+ _T(" (avi, mpg, vob, wmv ...)"),
		_T("*.avi;*.mpeg;*.mpg;*.mp2;*.divx;*.vob;*.flv;*.wmv;*.asf;*.mkv;*.mp4;*.ts;*.mov;*.mpeg2ts;*.3gp;*.rm;"),
		CString(TR("Аудио файлы"))+ _T(" (mp3, wma, wav ...)"),
		_T("*.mp3;*.wav;*.wma;*.mid;*.asx"),
		
		TR("Все файлы"),
		_T("*.*"));

	CFileDialog fd(true,0,0,4|2,Buf,m_hWnd);
	fd.m_ofn.lpstrInitialDir = Settings.VideoFolder;

	if(fd.DoModal()!=IDOK || !fd.m_szFileName) return 0;
	TCHAR Buffer[512];
	ExtractFilePath(fd.m_szFileName, Buffer);
	Settings.VideoFolder = Buffer;
	CMediaInfoDlg dlg;
	LastVideoFile = fd.m_szFileName;
	dlg.ShowInfo(fd.m_szFileName);
	return true;
}

bool CWizardDlg::funcAddFiles()
{
	TCHAR Buf[MAX_PATH*4];
	SelectDialogFilter(Buf, sizeof(Buf)/sizeof(TCHAR),1, TR("Любые файлы"), _T("*.*"));

	int nCount=0;
	CMultiFileDialog fd(0, 0, OFN_HIDEREADONLY, Buf, m_hWnd);
	
	TCHAR Buffer[1000];
	fd.m_ofn.lpstrInitialDir = Settings.ImagesFolder;

	if(fd.DoModal(m_hWnd) != IDOK) return 0;
	LPCTSTR FileName = 0;
	fd.GetDirectory(Buffer, sizeof(Buffer)/sizeof(TCHAR));

	CreatePage(2);
	do
	{
		
		FileName = (FileName) ? fd.GetNextFileName() : fd.GetFirstFileName();
		if(!FileName) break;
		fd.GetDirectory(Buffer, sizeof(Buffer)/sizeof(TCHAR));

		if(Buffer[lstrlen(Buffer)-1] != '\\')
		lstrcat(Buffer, _T("\\"));
		
		if(FileName)
		{
			lstrcat(Buffer, FileName);
			if(((CMainDlg*)Pages[2])->AddToFileList(Buffer))
				nCount++;
		
		}
	} while (FileName);
	 
	
	fd.GetDirectory(Buffer, sizeof(Buffer)/sizeof(TCHAR));
	Settings.ImagesFolder = Buffer;
	if(nCount)
		ShowPage(2, 0, 3);

	if(CurPage == 2)
		((CMainDlg*)Pages[2])->ThumbsView.LoadThumbnails();
	ShowWindow(SW_SHOW);
	m_bShowWindow = true;
	return true;
}

LRESULT CWizardDlg::OnWmMyExit(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if(wParam  == 5)
	{
		CloseDialog(0);
	}
	return 0;
}

bool CWizardDlg::CanShowWindow()
{
	return (CurPage == 2 || CurPage == 0) && IsWindowVisible() && IsWindowEnabled();
}

void CWizardDlg::UpdateAvailabilityChanged(bool Available)
{
	if(Available)
	{
		TRC(IDC_UPDATESLABEL, "Доступны обновления");
	}
}
    
LRESULT CWizardDlg::OnUpdateClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{	
	CreateUpdateDlg();
	updateDlg->ShowModal(m_hWnd);
	return 0;
}

void CWizardDlg::CreateUpdateDlg()
{
	if(!updateDlg)
	{
		updateDlg = new CUpdateDlg();
		updateDlg->setUpdateCallback(this);
	}
}
  
struct BGRA_COLOR
{
	BYTE b;
	BYTE g;
	BYTE r;
	BYTE a;
};
// hack for stupid GDIplus
void Gdip_RemoveAlpha(Bitmap& source, Color color )
{
	Rect r( 0, 0, source.GetWidth(),source.GetHeight() );
	BitmapData  bdSrc;
	source.LockBits( &r,  ImageLockModeRead , PixelFormat32bppARGB,&bdSrc);

	BYTE* bpSrc = (BYTE*)bdSrc.Scan0;
	
	//bpSrc += (int)sourceChannel;
	

	for ( int i = r.Height * r.Width; i > 0; i-- )
	{
		BGRA_COLOR * c = (BGRA_COLOR *)bpSrc;
	
		if(c->a!=255)
		{
			//c = 255;
		
				DWORD * d= (DWORD*)bpSrc;
			*d= color.ToCOLORREF();
			c ->a= 255;
		}
		bpSrc += 4;
		
	}
	source.UnlockBits( &bdSrc );
}


bool CWizardDlg::CommonScreenshot(CaptureMode mode)
{
	bool needToShow = IsWindowVisible()!=FALSE;
	if(m_bScreenshotFromTray && Settings.TrayIconSettings.TrayScreenshotAction == TRAY_SCREENSHOT_UPLOAD   && !floatWnd.m_hWnd)
	{
		m_bScreenshotFromTray = false;
		//return false;
	}
	bool CanceledByUser = false;
	bool Result = false;
	if(needToShow)
		ShowWindow(SW_HIDE);
	EnableWindow(false);
	CScreenCaptureEngine engine;
	
	CString buf; // file name buffer

	Bitmap *result=0;
	WindowCapturingFlags wcfFlags;
	wcfFlags.AddShadow = Settings.ScreenshotSettings.AddShadow;
	wcfFlags.RemoveBackground = 	Settings.ScreenshotSettings.RemoveBackground;
	wcfFlags.RemoveCorners = Settings.ScreenshotSettings.RemoveCorners;
	int WindowHidingDelay = (needToShow||m_bScreenshotFromTray==2)? Settings.ScreenshotSettings.WindowHidingDelay: 0;
	
	engine.setDelay(WindowHidingDelay);
	if(mode == cmFullScreen)
	{
		engine.setDelay(WindowHidingDelay + Settings.ScreenshotSettings.Delay*1000);
		engine.captureScreen();
		result = engine.capturedBitmap();
	}
	else if (mode == cmActiveWindow)
	{
		int Delay = Settings.ScreenshotSettings.Delay;
		if(Delay <1) Delay = 1;
		engine.setDelay(WindowHidingDelay + Delay*1000);
		CActiveWindowRegion winRegion;
		winRegion.setWindowCapturingFlags(wcfFlags);
		winRegion.SetWindowHidingDelay(Settings.ScreenshotSettings.WindowHidingDelay);
		engine.captureRegion(&winRegion);
		result = engine.capturedBitmap();
	}
	else if(engine.captureScreen())
	{
		RegionSelect.Parent = m_hWnd;
		SelectionMode selMode;
		if(mode == cmFreeform)
			selMode = smFreeform;
		if(mode == cmRectangles)
			selMode = smRectangles;
		if(mode == cmWindowHandles)
			selMode = smWindowHandles;

		RegionSelect.m_SelectionMode = selMode;
		Bitmap *res = engine.capturedBitmap();
		if(res)
		{
			HBITMAP gdiBitmap=0;
			res->GetHBITMAP(Color(255,255,255), &gdiBitmap);
			if(RegionSelect.Execute(gdiBitmap, res->GetWidth(), res->GetHeight()))
			{
				if(RegionSelect.wasImageEdited() || (mode!=cmWindowHandles /*|| !Settings.ScreenshotSettings.ShowForeground*/) )
				engine.setSource(gdiBitmap);
				
				else{
					engine.setSource(0);
				}
				
				engine.setDelay(0);
				CScreenshotRegion* rgn = RegionSelect.region();
				if(rgn)
				{
					CWindowHandlesRegion *whr =  dynamic_cast<CWindowHandlesRegion*>(rgn);
					if(whr)
					{
						whr->SetWindowHidingDelay(int(Settings.ScreenshotSettings.WindowHidingDelay*1.2));
						whr->setWindowCapturingFlags(wcfFlags);
					}
					engine.captureRegion(rgn);	
					result = engine.capturedBitmap();
					DeleteObject(gdiBitmap);
				}
			}
			else CanceledByUser = true;
		}
	}

	if(!CanceledByUser)
	{
		if(result)
		{
			Result = true;
			bool CopyToClipboard = false;
			if((m_bScreenshotFromTray && Settings.TrayIconSettings.TrayScreenshotAction == TRAY_SCREENSHOT_CLIPBOARD) || Settings.ScreenshotSettings.CopyToClipboard)
			{

				CopyToClipboard = true;
			}
			int savingFormat = Settings.ScreenshotSettings.Format;
			if(savingFormat == 0)
				Gdip_RemoveAlpha(*result,Color(255,255,255,255));

			CString saveFolder = GenerateFileName(Settings.ScreenshotSettings.Folder, screenshotIndex,CPoint(result->GetWidth(),result->GetHeight()));
			MySaveImage(result,GenerateFileName(Settings.ScreenshotSettings.FilenameTemplate, screenshotIndex,CPoint(result->GetWidth(),result->GetHeight())),buf,savingFormat, Settings.ScreenshotSettings.Quality,(Settings.ScreenshotSettings.Folder.IsEmpty())?0:(LPCTSTR)saveFolder);
			screenshotIndex++;
			if(CopyToClipboard)
			{

				if ( OpenClipboard() )
				{
					EmptyClipboard();
					if(savingFormat != 0)
					Gdip_RemoveAlpha(*result,Color(255,255,255,255));
					HBITMAP out=0;
					result->GetHBITMAP(Color(255,255,255,255),&out);
					HDC dc = GetDC();
					CDC origDC,  destDC;
					origDC.CreateCompatibleDC(dc);
					CBitmap destBmp;
					destBmp.CreateCompatibleBitmap(dc, result->GetWidth(), result->GetHeight());
					HBITMAP oldOrigBmp = origDC.SelectBitmap(out);
					destDC.CreateCompatibleDC(dc);
					HBITMAP oldDestBmp = destDC.SelectBitmap(destBmp);
					destDC.BitBlt(0,0,result->GetWidth(),result->GetHeight(),origDC,0,0,SRCCOPY);
					destDC.SelectBitmap(oldDestBmp);
					origDC.SelectBitmap(oldOrigBmp);
					SetClipboardData(CF_BITMAP, destBmp);
					CloseClipboard(); //закрываем буфер обмена
					 DeleteObject(out);
					 ReleaseDC(dc);
					if(m_bScreenshotFromTray && Settings.TrayIconSettings.TrayScreenshotAction == TRAY_SCREENSHOT_CLIPBOARD)
					{
						floatWnd.ShowBaloonTip(TR("Снимок сохранен в буфере обмена"),_T("Image Uploader"));
						Result = false;
					}
				}
			}
			if(!m_bScreenshotFromTray || (Settings.TrayIconSettings.TrayScreenshotAction == TRAY_SCREENSHOT_ADDTOWIZARD || Settings.TrayIconSettings.TrayScreenshotAction== TRAY_SCREENSHOT_SHOWWIZARD))
			{
				CreatePage(2); 
				((CMainDlg*)Pages[2])->AddToFileList(buf);
				((CMainDlg*)Pages[2])->ThumbsView.EnsureVisible(((CMainDlg*)Pages[2])->ThumbsView.GetItemCount()-1,true);
				((CMainDlg*)Pages[2])->ThumbsView.LoadThumbnails();
				((CMainDlg*)Pages[2])->ThumbsView.SetFocus();
				ShowPage(2,0,3);
			}
			else if(m_bScreenshotFromTray && Settings.TrayIconSettings.TrayScreenshotAction == TRAY_SCREENSHOT_UPLOAD)
			{
				Result = false;
				floatWnd.UploadScreenshot(buf,buf);
			}

		}
		else
		{
			MessageBox(_T("Невозможно сделать снимок экрана!"));
		}
	}

	m_bShowAfter  = false;
	if(Result || needToShow )
	{
		if(needToShow || (!m_bScreenshotFromTray ||Settings.TrayIconSettings.TrayScreenshotAction!= TRAY_SCREENSHOT_ADDTOWIZARD))
		{
			m_bShowAfter = true;
		}
	} 
	else m_bShowAfter = false;
	m_bScreenshotFromTray = false;
	OnScreenshotFinished(Result);

	return Result;
}

bool CWizardDlg::funcWindowHandleScreenshot()
{
	return CommonScreenshot(cmWindowHandles);
}

bool CWizardDlg::funcFreeformScreenshot()
{
	return CommonScreenshot(cmFreeform);
}

bool CWizardDlg::IsClipboardDataAvailable()
{
	bool IsClipboard = IsClipboardFormatAvailable(CF_BITMAP)!=FALSE;

	if(!IsClipboard)
	{
		if(IsClipboardFormatAvailable(CF_UNICODETEXT)) 
		{
			CString text;
			IU_GetClipboardText(text);
			if(CImageDownloaderDlg::LinksAvailableInText(text))
			{
				IsClipboard = true;
			}
		}
	}
	return IsClipboard;
}

CWizardDlg * pWizardDlg;