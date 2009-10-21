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
#include "WizardDlg.h"
#include "langclass.h"
#include "langselect.h"
#include "common/markupmsxml.h"
#include "common/regexp.h"
#include <io.h>
#include "floatingwindow.h"

// CWizardDlg
CWizardDlg::CWizardDlg(): m_lRef(0), FolderAdd(this)
{
	*LastVideoFile = 0;
	CurPage = -1;
	PrevPage = -1;
	ZeroMemory(Pages, sizeof(Pages));
	DragndropEnabled = true;
	hLocalHotkeys = 0;
	QuickUploadMarker=false;
	m_bShowAfter = true;
	m_bHandleCmdLineFunc = false;
}

CWizardDlg::~CWizardDlg()
{
	for(int i=0; i<5; i++) 
	{
		CWizardPage *p = Pages[i];
		if(Pages[i]) delete p;
	}
}

TCHAR MediaInfoDllPath[MAX_PATH] = _T("");
LRESULT CWizardDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_bShowWindow = true;
	LPDWORD DlgCreationResult = (LPDWORD) lParam; 
	ATLASSERT(DlgCreationResult != NULL);
	// center the dialog on the screen
	CenterWindow();
	HICON hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
	SetIcon(hIcon, TRUE);
	HICON hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
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

	CString MediaDll = GetAppFolder()+_T("\\Modules\\MediaInfo.dll");
	if(FileExists( MediaDll)) lstrcpy(MediaInfoDllPath, MediaDll);
	else
	{
		CString MediaDll2 =CString(ClassName)+_T("\\Tools\\MediaInfo.dll");
		if(FileExists( MediaDll2)) lstrcpy(MediaInfoDllPath, MediaDll2);
	}
	SetWindowText(APPNAME);
	TCHAR Language[128];
	
	TCHAR szFileName[256], szPath[256];
	GetModuleFileName(0, szFileName, 1023);
   ExtractFilePath(szFileName, szPath);

   Lang.SetDirectory(CString(szPath) + "Lang\\");
   Lang.LoadList();
	

	if(!lstrlen(Settings.Language))
	{
		CLangSelect LS;
		if(LS.DoModal(m_hWnd) == IDCANCEL)
		{
			*DlgCreationResult = 1;
			return 0;
		}
		Settings.Language = LS.Language;
		if(!Lang.LoadLanguage(Settings.Language));
		if(MessageBox(TR("Добавить Image Uploader в контекстное меню проводника Windows?"),APPNAME, MB_YESNO|MB_ICONQUESTION)==IDYES)
		{
			Settings.ExplorerContextMenu = true;
			Settings.ExplorerContextMenu_changed = true;
			Settings.ExplorerVideoContextMenu = true;
			/*Settings.SendToContextMenu = true;
			Settings.SendToContextMenu_changed = true;*/
		}	
		Settings.SaveSettings();
	}
	else 
	{
		Lang.LoadLanguage(Settings.Language);
	}

	CString ErrorStr;
	if(!LoadUploadEngines(ErrorStr))  // Завершаем работу программы, если файл servers.lst отсутствует
	{
		CString ErrBuf ;
		ErrBuf.Format(TR("Невозможно открыть файл со спиком серверов \"servers.xml\"!\n\nПричина:  %s\n\nПродолжить работу программы?"),ErrorStr);
		if(MessageBox(ErrBuf, APPNAME, MB_ICONERROR|MB_YESNO)==IDNO)
		{
			*DlgCreationResult = 2;
			return 0;
		}
	}
	
	Settings.ServerID = GetUploadEngineIndex(Settings.ServerName);
	Settings.FileServerID = GetUploadEngineIndex(Settings.FileServerName);

	Settings.QuickServerID = GetUploadEngineIndex(Settings.QuickServerName);
	LogWindow.Create(0);

	if(!*MediaInfoDllPath)
		WriteLog(logWarning, APPNAME, TR("Библиотека MediaInfo.dll не найдена. \nПолучение технических данных о файлах мультимедиа будет недоступно.")); 
	TRC(IDC_ABOUT,"О программе...");
	if(!CmdLine.IsOption(_T("tray")))
   TRC(IDCANCEL,"Выход");
	else 
		TRC(IDCANCEL,"Скрыть");
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
	return 0;  // Let the system set the focus
}

bool IsDirectory(LPCTSTR szFileName)
{
	 return GetFileAttributes(szFileName)&FILE_ATTRIBUTE_DIRECTORY;	
}

bool CWizardDlg::ParseCmdLine()
{
	LPCTSTR szBuffer; 
	int type = 0;
	int count=0;

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

	for(int i=0; i<CmdLine.GetCount(); i++)
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
	return count;

}

LRESULT CWizardDlg::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	//if(CmdLine.IsOption(_T("tray")))
	if(floatWnd.m_hWnd)
	{ 
		ShowWindow(/*SW_HIDE*/SW_HIDE);
		if(Pages[2])
		((CMainDlg*)Pages[2])->ThumbsView.MyDeleteAllItems();
//		EnableExit();
		/*if(CurPage!=2)*/ ShowPage(0); 
		return 0;
	}
	CloseWizard();
	return 0;
}

BOOL CWizardDlg::PreTranslateMessage(MSG* pMsg)
{
	/*if(TranslateAccelerator(m_hWnd, hAccel, pMsg)) 
	{
		return TRUE;
	}*/

	
	if(hLocalHotkeys &&TranslateAccelerator(m_hWnd, hLocalHotkeys, pMsg)) 
	{
		return TRUE;
	}

	if( pMsg->message == WM_KEYDOWN)
	{
		TCHAR Buffer[MAX_PATH];
		GetClassName(pMsg->hwnd, Buffer, sizeof(Buffer)/sizeof(TCHAR));
		if( pMsg->wParam == /*VkKeyScan('a')*/'A' && !lstrcmpi(Buffer,_T("Edit") ) && GetKeyState(VK_CONTROL)<0)
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
			if(Pages[CurPage]  && VK_BACK == pMsg->wParam  && GetForegroundWindow()==m_hWnd && lstrcmpi(Buffer,_T("Edit") ))
			{
				if(pMsg->message==WM_KEYDOWN && ::IsWindowEnabled(GetDlgItem(IDC_PREV)))
				{ 
					OnPrevBnClicked(0,0,0); 
					return TRUE;
				}
				else if (pMsg->message==WM_KEYUP) 
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
	
	ShowWindow(SW_HIDE);
	if(CurPage >= 0)
	{
		Pages[CurPage]->OnHide();
	}
	Exit();
	DestroyWindow();
	::PostQuitMessage(nVal);
}

bool CWizardDlg::ShowPage(int idPage,int prev,int next)
{
	if(idPage == CurPage) return true;

	if(GetCurrentThreadId()!=GetWindowThreadProcessId(m_hWnd, NULL))
	{return SendMessage(WM_MY_SHOWPAGE, (WPARAM)idPage);}
   
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
			tmp3=new CUploadSettings();
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
	float width=(float)rc.right-(float)rc.left;
	RectF bounds(0,0,width,50);
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
			CreatePage(2);
			MainDlg = (CMainDlg*) Pages[2];
			MainDlg->AddToFileList(szBuffer);
			ShowPage(2);
		
		}
 
	}
	if(MainDlg) MainDlg->ThumbsView.LoadThumbnails();
	DragFinish(hDrop);
	return 0;
   
}

bool CWizardDlg::LoadUploadEngines(CString &Error)
{
	int i = 0;

	CMarkupMSXML XML;
	CString XmlFileName = GetAppFolder() + _T("Data\\servers.xml");

	if(!FileExists(XmlFileName))
	{
		Error = TR("Файл не найден.");
		return false;
	}

	if(!XML.Load(XmlFileName))
	{
		Error = XML.GetError();
		return false;
	}

	if(!XML.FindElem(_T("Servers")))
	{
		Error = _T("Unable to find Servers node");
		return false;	
	}

	XML.IntoElem();

	while(XML.FindElem(_T("Server")))
	{
		UploadEngine UE;
		UE.NumOfTries = 0;
		UE.NeedAuthorization=(bool) _ttoi(XML.GetAttrib(_T("Authorize")));
		*UE.Name =0;
		CString RetryLimit = XML.GetAttrib(_T("RetryLimit"));
		if(RetryLimit.IsEmpty())
		{
			UE.RetryLimit = Settings.FileRetryLimit;
		}
		else UE.RetryLimit = _ttoi(RetryLimit);

		CString ServerName = XML.GetAttrib(_T("Name"));
		UE.Debug =  (bool) _ttoi(XML.GetAttrib(_T("Debug")));
		UE.ImageHost =  !(bool) _ttoi(XML.GetAttrib(_T("FileHost")));
		UE.MaxFileSize =   _ttoi(XML.GetAttrib(_T("MaxFileSize")));
		XML.IntoElem();
		lstrcpyn(UE.Name, ServerName, 63);

		if(XML.FindElem(_T("Actions")))
		{
			XML.IntoElem();
			int ActionIndex = 0;
			while(XML.FindElem())
			{
				UploadAction UA;
				UA.NumOfTries = 0;
				UA.Index = ActionIndex;

				CString RetryLimit = XML.GetAttrib(_T("RetryLimit"));
				if(RetryLimit.IsEmpty())
				{
					UA.RetryLimit =Settings.ActionRetryLimit;
				}
				else UA.RetryLimit = _ttoi(RetryLimit);
				UA.IgnoreErrors = _ttoi(XML.GetAttrib(_T("IgnoreErrors")));
				UA.Description= XML.GetAttrib(_T("Description"));
				ActionIndex++;
				UA.Type = XML.GetAttrib(_T("Type"));
				UA.Url = XML.GetAttrib(_T("Url"));
				UA.PostParams = XML.GetAttrib(_T("PostParams"));
				UA.RegExp = XML.GetAttrib(_T("RegExp"));
				UA.OnlyOnce = _ttoi(XML.GetAttrib(_T("OnlyOnce")));

				CString AssignVars = XML.GetAttrib(_T("AssignVars"));
				CComBSTR BstrAssignVars = AssignVars;	
				RegExp exp;
				exp.SetPattern(_T("([A-z0-9_]*?):([0-9]{1,3})"));
				exp.Execute(BstrAssignVars);

				int n = exp.MatchCount();

				for(int i=0; i<n; i++) // count of variables
				{
					int nSub =	exp.SubMatchCount(i);
					CString VariableName, VariableIndex;
					ActionVariable AV;
					AV.Name = exp.GetSubMatch(i,0);
					AV.nIndex = _ttoi(exp.GetSubMatch(i,1));
					UA.Variables.push_back(AV); //Adding variable name and it's index to the map
				}
				UE.Actions.push_back(UA);
			}

			XML.OutOfElem();
		}

		if(XML.FindElem(_T("Result")))
		{
			UE.DownloadUrlTemplate = XML.GetAttrib(_T("DownloadUrlTemplate"));
			UE.ImageUrlTemplate = XML.GetAttrib(_T("ImageUrlTemplate"));
			UE.ThumbUrlTemplate = XML.GetAttrib(_T("ThumbUrlTemplate"));
		}
		XML.OutOfElem();

		EnginesList.Add(UE);
	}
	return true;

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
		TCHAR OnlyName[MAX_PATH];
		GetOnlyFileName(FileName, OnlyName);
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
			BOOL b;
			FILEGROUPDESCRIPTOR *fgd = (FILEGROUPDESCRIPTOR*) hdrop;

			for(int i=0; i<fgd->cItems; i++)
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

						TCHAR buf[256];
						PVOID hdrop2;
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
							else if(IsImage(OutFileName) && (CurPage==0||CurPage==2))
							{
								CreatePage(2);
								CMainDlg* MainDlg = (CMainDlg*) Pages[2];
								MainDlg->AddToFileList(OutFileName);
								MainDlg->ThumbsView.LoadThumbnails();
								ShowPage(2);
							}
						}
					}
				}

				GlobalUnlock ( hdrop );
				return true;
			}
		}
	}
	return false;
}

bool CWizardDlg::HandleDropHDROP(IDataObject *pDataObj)
{
	FORMATETC tc = { CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	FORMATETC ftc;
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
	FORMATETC FtcBitmap, ftc;
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

	// Прием файлов через файлдескриптор
	if(HandleDropFiledescriptors(pDataObj))
		return S_OK;

	// An image was dropped
	if(HandleDropBitmap(pDataObj))
		return S_OK;

	return S_OK;
}

LRESULT CWizardDlg::OnPaste(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(!OpenClipboard()) return 0;

	if(IsClipboardFormatAvailable(CF_BITMAP)) 
	{
		HBITMAP bmp = (HBITMAP) GetClipboardData(CF_BITMAP);

		if(!bmp) return CloseClipboard();

		PasteBitmap(bmp);
	}
	CloseClipboard();
	return 0;
}

void CWizardDlg::PasteBitmap(HBITMAP Bmp)
{
	if(CurPage!=0 && CurPage!=2) return;

	TCHAR buf2[256];
	SIZE dim;
	GetBitmapDimensionEx(Bmp, &dim);
	wsprintf(buf2, _T("w=%d, h=%d"),dim);
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
	RECT rc={0,0,0,0},Rec;
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
				 if(SendMessage(m_pWizardDlg->m_hWnd, WM_MY_ADDIMAGE,(WPARAM) (LPCTSTR) (CString(currentDir) + CString(_T("\\"))+ CString( s_Dir.name)),  FALSE))
					 count++;
		  }
	 } while( _tfindnext( hDir, &s_Dir ) == 0 );

    _findclose( hDir );

    return 0;


}

DWORD CFolderAdd::Run()
{
	TCHAR Buffer[MAX_PATH];
	TCHAR FullPath[MAX_PATH*3];
	EnableWindow(m_pWizardDlg->m_hWnd, false);
	for(int i=0; i<m_Paths.GetCount(); i++)
	{
		CString CurPath = m_Paths[i];
		if(IsDirectory(CurPath))
			ProcessDir(CurPath, m_bSubDirs);
		else 
			if(!m_bImagesOnly || IsImage(CurPath))
			if(SendMessage(m_pWizardDlg->m_hWnd, WM_MY_ADDIMAGE,(WPARAM) (LPCTSTR) (CurPath),  FALSE))
				count++;
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
	TCHAR szNameBuffer[MAX_PATH], szBuffer2[MAX_PATH], TempPath[256];
	
	GetTempPath(256, TempPath);
	wsprintf(szBuffer2,_T("%s*.*"), m_szPath);
	
	
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

bool CWizardDlg::AddImage(LPCTSTR FileName, bool Show)
{
	CreatePage(2);
	CMainDlg* MainDlg = (CMainDlg*) Pages[2];
	if(!MainDlg) return false;
	MainDlg->AddToFileList(FileName);
	if(Show){
		MainDlg->ThumbsView.LoadThumbnails();
		ShowPage(2);}
	return true;
}

LRESULT CWizardDlg::OnAddImages(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{	
	LPTSTR szFileName= (LPTSTR) wParam;
	if(!szFileName) ATLTRACE("Filename = NULL!");
	return  AddImage(szFileName,  lParam);

}

CMyFolderDialog::CMyFolderDialog(HWND hWnd)
{
	OleInitialize(NULL);
	CFolderDialogImpl::CFolderDialogImpl(hWnd, _T("Выбор папки"), BIF_RETURNONLYFSDIRS|BIF_NEWDIALOGSTYLE );
}
void CMyFolderDialog::OnInitialized()
{
	HWND wnd = CreateWindowEx(0, _T("button"), TR("Включая поддиректории"), WS_VISIBLE|BS_CHECKBOX|WS_CHILD|BS_AUTOCHECKBOX, 15,5, 200,30, m_hWnd, 0,0, 0);
	SendMessage(wnd, WM_SETFONT, (WPARAM)SendMessage(m_hWnd, WM_GETFONT, 0,0),  MAKELPARAM(false, 0));
	SendMessage(wnd, BM_SETCHECK, (WPARAM)(m_bSubdirs?BST_CHECKED	:BST_UNCHECKED),0);
	OldProc =(DLGPROC) SetWindowLong(m_hWnd, DWL_DLGPROC, (DWORD)DialogProc);	
	SetWindowLong(m_hWnd, GWL_USERDATA	,(DWORD) this);	
	SubdirsCheckbox = wnd;
	m_bSubdirs = true;
}

 BOOL CALLBACK CMyFolderDialog::DialogProc(

    HWND hwndDlg,	// handle to dialog box
    UINT uMsg,	// message
    WPARAM wParam,	// first message parameter
    LPARAM lParam 	// second message parameter
   )
{
		//MessageBox(hwndDlg, _T("COMEONE SLICK"), 0, 0);
	CMyFolderDialog *th =(CMyFolderDialog *) GetWindowLong(hwndDlg, GWL_USERDATA);
	if(!th) return false;

	if(uMsg == WM_COMMAND && HIWORD(wParam)== BN_CLICKED && ((HWND) lParam)== th->SubdirsCheckbox)
		th->m_bSubdirs = SendMessage(th->SubdirsCheckbox, BM_GETCHECK,0,0) == BST_CHECKED	;
		//MessageBox(hwndDlg, _T("COMEONE SLICK"), 0, 0);
	return th->OldProc(hwndDlg, uMsg, wParam, lParam);
	return true;
}

 LRESULT CWizardDlg::OnEraseBkg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	bHandled = true; // Не даем системе очистить окно стандартным цветом (предотвращаем мерцание)
	return 1;
}
	
LRESULT 	CWizardDlg::OnWmShowPage(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	int PageIndex = wParam;
	ShowPage(PageIndex);
	return 0;
}
	
bool CWizardDlg::funcAddImages()
{
	TCHAR Buf[MAX_PATH*4];
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
}

bool CWizardDlg::executeFunc(CString funcName)
{
	bool LaunchCopy= false;

	if(CurPage == 4) LaunchCopy= true;
	if(CurPage == 1) LaunchCopy= true;


	if(CurPage == 3) ShowPage(2);

	if(!IsWindowEnabled())LaunchCopy= true; 

	if(LaunchCopy)
	{
		if(Settings.TrayIconSettings.DontLaunchCopy)
		{	
				//SetForegroundWindow(m_hWnd);
			 if(IsWindowVisible() && IsWindowEnabled())
				SetForegroundWindow(m_hWnd);
			else if(!IsWindowEnabled()) SetActiveWindow();
			FlashWindow(true);
		}
		else
			IULaunchCopy(_T("/func=")+funcName,CAtlArray<CString>());
		return false;
	}
	if(funcName == _T("addimages"))
		return funcAddImages();
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
	ExtractFilePath(fd.m_szFileName, Buffer); // Запоминаем каталог видео
	Settings.VideoFolder = Buffer;
	CreatePage(1);
	lstrcpyn(LastVideoFile, fd.m_szFileName, MAX_PATH);
	((CVideoGrabber*)Pages[1])->SetFileName(fd.m_szFileName); // C-style conversion .. but i like it :)
	ShowPage(1,0,(Pages[2])?2:3);
	ShowWindow(SW_SHOW);
		m_bShowWindow = true;
	return true;
}

bool CWizardDlg::funcScreenshotDlg()
{
	CScreenshotDlg dlg;
	dlg.MainDlg = this;
	dlg.m_Action = 1;

	if(dlg.DoModal(m_hWnd) != IDOK) return false;
	
	CreatePage(2); //Ну типа страничка с картинками!
	((CMainDlg*)Pages[2])->AddToFileList(dlg.FileName);
	((CMainDlg*)Pages[2])->ThumbsView.EnsureVisible(((CMainDlg*)Pages[2])->ThumbsView.GetItemCount()-1,true);
	((CMainDlg*)Pages[2])->ThumbsView.LoadThumbnails();
	ShowPage(2,0,3);
	m_bShowWindow = true;
	return true;
}

bool CWizardDlg::funcRegionScreenshot(bool ShowAfter)
{
	m_bShowAfter = ShowAfter;
	ShowWindow(SW_HIDE);
	EnableWindow(false);
	RegionSelect.Parent = m_hWnd;
	RegionSelect.Execute(this);
	return true;
}

void CWizardDlg::OnScreenshotFinished(int Result)
{
	EnableWindow();
	if(Result || m_bShowAfter)
	{
		//if(m_bCurrentFunc
		ShowWindow(SW_SHOWNORMAL);
		SetForegroundWindow(m_hWnd);
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
	_screenShotdlg.MainDlg = this;
	_screenShotdlg.m_Action = 1;

	EnableWindow(false); //Disabling window (for disabling tray icon commands possible execution)
	_screenShotdlg.Execute(m_hWnd, this, true);
	return true;
}

bool CWizardDlg::funcWindowScreenshot(bool Delay)
{
	_screenShotdlg.MainDlg = this;
	_screenShotdlg.m_Action = 1;
	_screenShotdlg.m_bDelay = Delay;
	EnableWindow(false); //Disabling window (for disabling tray icon commands possible execution)
	_screenShotdlg.Execute(m_hWnd, this, false);
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
	  TRC(IDCANCEL,"Выход");
	else 
		TRC(IDCANCEL,"Скрыть");

	if(!(m_hotkeys==Settings.Hotkeys))
	{
		UnRegisterLocalHotkeys();
		RegisterLocalHotkeys();}
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
	lstrcpyn(LastVideoFile, fd.m_szFileName, MAX_PATH);
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
}
CWizardDlg * pWizardDlg;