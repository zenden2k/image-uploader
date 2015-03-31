#include "WebViewWindow.h"
#include <Func/WinUtils.h>
#include <Gui/GuiTools.h>
#include <Core/Logging.h>

HWND CWebViewWindow::window = 0;
 CWebViewWindow* CWebViewWindow::instance = 0;
CWebViewWindow::CWebViewWindow() : subclassWindow_(this) {
	instance = this;
	isModal_ = false;
	captureActivate_ = false;
	timerInterval_ = 0;
	combobox_ = 0;
	editControl_= 0;
	fileDialogEvent_.Create();
	fileFieldSuccess_ = false;
	messageLoopIsRunning_ = false;
	activeWindowBeforeFill_ = 0;
	dialogHook_ = 0;
}
CWebViewWindow::~CWebViewWindow() {
	/*if ( dialogHook_) {
		dialogHook_->Hook(0);
	}*/
	if ( hook_ ) {
		UnhookWindowsHookEx(hook_);
	}
	instance = 0;
	delete dialogHook_;
}

LRESULT CWebViewWindow::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	window = m_hWnd;
	icon_ = GuiTools::LoadBigIcon(IDR_MAINFRAME);
	iconSmall_ = GuiTools::LoadSmallIcon(IDR_MAINFRAME);

	SetIcon(icon_, TRUE);
	SetIcon(iconSmall_, FALSE);

	WinUtils::UseLatestInternetExplorerVersion(false);

	typedef HRESULT  (STDAPICALLTYPE *CoInternetSetFeatureEnabledFuncType) (INTERNETFEATURELIST , DWORD , BOOL); 
	HMODULE module = LoadLibrary(_T("Urlmon.dll"));
	CoInternetSetFeatureEnabledFuncType CoInternetSetFeatureEnabledFunc = (CoInternetSetFeatureEnabledFuncType)GetProcAddress(module, "CoInternetSetFeatureEnabled");
	if ( CoInternetSetFeatureEnabledFunc ) { 
		CoInternetSetFeatureEnabledFunc(FEATURE_DISABLE_NAVIGATION_SOUNDS, SET_FEATURE_ON_PROCESS, true);
	}
	RECT rc;
	GetWindowRect(&rc);
	hWndClient_ = view_.Create(m_hWnd, rc, _T("about:blank"), WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_HSCROLL | WS_VSCROLL, WS_EX_CLIENTEDGE);
	view_.PutSilent(TRUE); // Supress javascript errors http://stackoverflow.com/questions/7646055/supressing-script-error-in-ie8-c
	//view_.SetFocus();
	return 0;
}

LRESULT CWebViewWindow::OnResize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	RECT clientRect;
	GetClientRect(&clientRect);

	view_.SetWindowPos(NULL, &clientRect, SWP_NOMOVE);
	return 0;
}

bool CWebViewWindow::NavigateTo(const CString& url) {
	view_.Navigate(url);
	return true;
}

int CWebViewWindow::DoModal(HWND parent, bool show )
{
	
	isModal_ = true;

	if ( timerInterval_ ) {
		SetTimer(kUserTimer, timerInterval_);
	}
	if ( show ) {
		ShowWindow(SW_SHOW);
		SetActiveWindow();
		SetForegroundWindow(m_hWnd);
	}

	if ( show && parent ) {
		::EnableWindow(parent, false);
	}

	CenterWindow(parent);

	CMessageLoop loop;
	int res = loop.Run();
	if ( timerInterval_ ) {
		KillTimer(kUserTimer);
	}
	if ( show ) {
		if ( parent ) {
			::EnableWindow(parent, true);
		}
		ShowWindow(SW_HIDE);
		if ( parent ) {
			::SetActiveWindow(parent);
		}
	}
	window = 0;
	return res;
}

int CWebViewWindow::exec()
{
	return DoModal(0, false);
}

void CWebViewWindow::close()
{
	PostQuitMessage(1);
}

void CWebViewWindow::setTimerInterval(int interval)
{
	timerInterval_ = interval;
}

 TCHAR m_szClassName[MAX_PATH];
// helper to get class name (lowercase)
LPCTSTR GetWndClass(WPARAM wParam, LPARAM lParam)
{
	CBT_CREATEWND* pcw = (CBT_CREATEWND*)lParam;
	ATLASSERT(pcw);
	ATLASSERT(wParam);

	m_szClassName[0] = 0;
	LPCTSTR clname = pcw->lpcs->lpszClass;
	if(IS_INTRESOURCE(clname))
		::GetClassName((HWND)wParam, m_szClassName, CLASSNAME_LEN-1);
	else
		lstrcpy(m_szClassName, clname);

	ATLASSERT(m_szClassName[0]);
	CharLower(m_szClassName);
	return m_szClassName;
}
//FIXME: only one instance of CBThook may exist.

LRESULT CALLBACK CWebViewWindow::CBTHook(int nCode, WPARAM wParam, LPARAM lParam)
{
	if(nCode == HCBT_CREATEWND)
	{
		HWND hwnd = (HWND)wParam;
		CBT_CREATEWND *cw = (CBT_CREATEWND*)lParam;
		/*if ( !lstrcmp(	cw->lpcs->lpszClass, _T("#32770"))) { 
			
		}*/
		/*HWND hwnd = (HWND)wParam;
		CBT_CREATEWND *cw = (CBT_CREATEWND*)lParam;*/
		if ( 
			
			!lstrcmp(	GetWndClass(wParam, lParam), _T("#32770"))
			/*&& !lstrcmp(cw->lpcs->lpszName, _T("Открытие"))*/
			) { 
			//	LOG(INFO) << "WS_VISIBLE="<< (cw->lpcs->style & WS_VISIBLE);
				instance->handleDialogCreation(hwnd, true);

		}

	}
	if (nCode < 0)
	{
	return CallNextHookEx(hook_, nCode, wParam, lParam);
	}
	return 0;

}
HHOOK CWebViewWindow::hook_ = 0;
void CWebViewWindow::fillInputFileField(const CString& uploadFileName, CComPtr<IHTMLInputFileElement> inputFileElement, CComPtr<IAccessible> accesible )
{
	if ( !WinUtils::FileExists(uploadFileName)) {
		return;
	}
	activeWindowBeforeFill_ = ::GetActiveWindow();
	uploadFileName_ = uploadFileName;
	inputFileElement_ = inputFileElement;
	accesible_ = accesible;
	if ( !dialogHook_ ) {
		dialogHook_ = new CDialogHook(this);
	}
	
	PostMessage(WM_FILLINPUTFIELD);
	//LOG(INFO) << "threadId="<<GetCurrentThreadId();
}
/*
bool CWebViewWindow::fillInputFileField()
{
	fileFieldSuccess_ = false;

	//WinUtils::MsgWaitForSingleObject(fileDialogEvent_, 10000);
	messageLoopIsRunning_ = true;
	SetTimer(kMessageLoopTimeoutTimer, 10000);
	WinUtils::TimerWait(5000);
	/*CMessageLoop loop;
	loop.Run();*
	messageLoopIsRunning_ = false;
	//uploadFileName_.Empty();
	return fileFieldSuccess_;
}*/

bool CWebViewWindow::compareFileNameWithFileInputField()
{
	CComBSTR res;
	if ( SUCCEEDED( inputFileElement_->get_value(&res) ) && res  ) {
		return res.Length() && res[0] == uploadFileName_[0];
	}
	return false;
}

void CWebViewWindow::handleDialogCreation(HWND wnd, bool fromHook)
{
	if ( fromHook ) {
		FileDialogSubclassWindow *sw = new FileDialogSubclassWindow(this);
		sw->SubclassWindow(wnd);
		subclassedWindows_.push_back(sw);
		return;
	}
	//return;
	//LOG(ERROR) << "CWebViewWindowOnActivate="<<GuiTools::GetWindowText(wnd);
	EnumChildWindows(wnd, EnumChildProc, (LPARAM)this);

	bool isWindowVisible = ::IsWindowVisible(fileDialog_);
	if (editControl_ ) {

		fileDialog_ = wnd;

		//LOG(INFO) << "isWindowVisible" << isWindowVisible;
		::SetWindowLong(fileDialog_, GWL_STYLE, ::GetWindowLong(fileDialog_, GWL_STYLE) & WS_CHILD & ~ (WS_CAPTION |  WS_SYSMENU | WS_SIZEBOX | WS_MAXIMIZEBOX | 
			WS_MINIMIZEBOX|WS_POPUP|WS_BORDER|WS_VISIBLE));
		::ShowWindow(fileDialog_, SW_HIDE);
		::SetWindowPos(HWND_BOTTOM, fileDialog_,0,0,1,1,SWP_HIDEWINDOW);

		if ( !subclassWindow_.m_hWnd ) {
			subclassWindow_.SubclassWindow(fileDialog_);
		}
	}
}

void CWebViewWindow::SetFillTimer()
{
	PostMessage(WM_SETFILLTIMER);
}

LRESULT CWebViewWindow::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	if ( isModal_ ) {
		PostQuitMessage(0);
	} else {
		ShowWindow(SW_HIDE);
	}
	bHandled = true;
	return 1;
}

LRESULT CWebViewWindow::OnEnable(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if ( !wParam && !uploadFileName_.IsEmpty()) {
		//HWND wnd  = ::GetActiveWindow();
		//LOG(ERROR) << "CWebViewWindow window disabled ";
		captureActivate_ = true;
	}
	return 0;
}
/*
LRESULT CALLBACK MyHookProc(int nCode, WPARAM wParam, LPARAM lParam) 
{ 
	if (nCode < 0) 
		return CallNextHookEx(hook, nCode, wParam, lParam); 

	if (nCode == HCBT_CREATEWND) 
	{ 
		char szBuf[30]; 
		GetClassName((HWND)wParam, szBuf, sizeof(szBuf)); 

		if (strcmp(szBuf, "Notepad") == 0) 
		{ 
			return 1; 
		} 
	} 
	return CallNextHookEx(hook, nCode, wParam, lParam); 
}*/




LRESULT CWebViewWindow::OnActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if ( captureActivate_  && wParam == WA_INACTIVE && !::IsWindowEnabled(m_hWnd) ) {
		captureActivate_ = false;
		HWND wnd  = (HWND)lParam;
		

		TCHAR Buffer[MAX_PATH];
		GetClassName(wnd, Buffer, sizeof(Buffer)/sizeof(TCHAR));
		if ( !lstrcmp(Buffer, _T("#32770"))) {
			::EnableWindow(m_hWnd, true);
			::SetActiveWindow(m_hWnd);

			
			
			
			
			handleDialogCreation(wnd);
			SetTimer(1,400);
			/*CComPtr<IAccessible> editControlAccesible;
			VARIANT v;
			v.vt = VT_I4 ;
			v.lVal  = CHILDID_SELF;

			HRESULT hr = ::AccessibleObjectFromWindow(editControl_, OBJID_CLIENT , IID_IAccessible, (void**)(&editControlAccesible)); // 1 - захардкоженный идентификатор ловушк
			*//*if ( editControlAccesible ) {
				editControlAccesible->put_accValue(v, CComBSTR(uploadFileName_));
			}*/
		
		}


		
		/*::EndDialog(wnd, IDOK);*/
	}
	return 0;
}

LRESULT CWebViewWindow::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if ( wParam == kUserTimer && onTimer ) {
		onTimer();
		return 0;
	} /*else if ( wParam == kMessageLoopTimeoutTimer ) {
		KillTimer(kMessageLoopTimeoutTimer);
		if ( messageLoopIsRunning_ ) {
			messageLoopIsRunning_ = false;
			PostQuitMessage(0);
		}
	}*/
	else if ( wParam == 1 ) {

		VARIANT v;
		v.vt = VT_I4 ;
		v.lVal  = CHILDID_SELF;
		CComPtr<IAccessible> editControlAccesible;
		if ( ::IsWindow(editControl_) ) {
			
			HRESULT hr = ::AccessibleObjectFromWindow(editControl_, OBJID_CLIENT , IID_IAccessible, (void**)(&editControlAccesible)); 
		}

		if ( editControl_ && (!::IsWindow(editControl_) || !editControlAccesible )) {
			if ( compareFileNameWithFileInputField() ) {
				fileFieldSuccess_ = true;
				::SetActiveWindow(activeWindowBeforeFill_);
				activeWindowBeforeFill_ = 0; 
				KillTimer(1);
				if (onFileFieldFilled ) {
					onFileFieldFilled(uploadFileName_);
				}
				UnhookWindowsHookEx(hook_);
				hook_ = 0;
				uploadFileName_.Empty();
				editControl_ = 0;
				return 0;
			}
		} 

		 {
			
			if ( editControlAccesible ) {
				/*for ( int i = 0; i < 5; i++ )*/ {
					if ( ::IsWindow(fileDialog_) && !::IsWindowEnabled(fileDialog_)) {
						LOG(ERROR) << "fileDialog_ is disabled. Enabling it again.";
						HWND activeWindow  = ::GetActiveWindow();
						if ( activeWindow !=fileDialog_  ) {
							::EndDialog(activeWindow, IDOK);
						}
					} 
					if ( compareFileNameWithFileInputField() ) {
						fileFieldSuccess_ = true;

						KillTimer(kMessageLoopTimeoutTimer);
						/*if ( messageLoopIsRunning_ ) {
							messageLoopIsRunning_ = false;
							PostQuitMessage(0);
						}*/
						
						::SetActiveWindow(activeWindowBeforeFill_);
						activeWindowBeforeFill_ = 0; 
						//fileDialogEvent_.PulseEvent();
						KillTimer(1);
						if (onFileFieldFilled ) {
							onFileFieldFilled(uploadFileName_);
						}
						UnhookWindowsHookEx(hook_);
						hook_ = 0;
						uploadFileName_.Empty();
							return 0;
					}

					for ( int j = 0; j < 5; j++ ) {
						editControlAccesible->put_accValue(v, CComBSTR(uploadFileName_));
						if ( GuiTools::GetWindowText(editControl_) == uploadFileName_) {
							break;
						}
					}
					
					if ( GuiTools::GetWindowText(editControl_) == uploadFileName_) {
						::SendMessage(fileDialog_, WM_COMMAND, MAKELPARAM(IDOK, BN_CLICKED), (LPARAM)::GetDlgItem(fileDialog_, IDOK));

						if ( ::IsWindow(fileDialog_) && !::IsWindowEnabled(fileDialog_)  ) {
							HWND activeWindow  = ::GetActiveWindow();
							if ( activeWindow !=fileDialog_  ) {
								::EndDialog(activeWindow, IDOK);
							}
						}
						
					}
				}
			}
		}
		
	}
	return 0;
}

LRESULT CWebViewWindow::OnSetFillTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	SetTimer(1, 400);
	return 0;
}

LRESULT CWebViewWindow::OnFillInputField(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	
//	hook_ = SetWindowsHookEx(WH_CBT, (HOOKPROC) MakeCallback(&CWebViewWindow::CBTHook), _Module.GetModuleInstance(), GetCurrentThreadId());
	hook_ = SetWindowsHookEx(WH_CBT, (HOOKPROC) &CBTHook, _Module.GetModuleInstance(), GetCurrentThreadId());
	accesible_->accDoDefaultAction(CComVariant(0));
	return 0;
}

BOOL CALLBACK CWebViewWindow::EnumChildProc(HWND wnd, LPARAM lParam)
{
	CWebViewWindow* this_ = reinterpret_cast<CWebViewWindow*>(lParam);

	TCHAR Buffer[MAX_PATH] = _T("");
	GetClassName(wnd, Buffer, sizeof(Buffer)/sizeof(TCHAR));
	if ( (!lstrcmpi(Buffer, _T("ComboBoxEx32")) &&  (::GetWindowLong(wnd, GWL_STYLE))& CBS_DROPDOWN) ) {
		HWND edit = FindWindowEx(wnd, 0, _T("Edit"),0);
		if ( !edit ) {
			HWND combobox = FindWindowEx(wnd, 0, _T("Combobox"),0);
			edit = FindWindowEx(combobox, 0, _T("Edit"),0);
		}
		this_->editControl_ = edit;
		this_->combobox_ = wnd;
		
	} else if ( !lstrcmpi(Buffer, _T("Edit"))  ) {
		this_->editControl_ = wnd;
		this_->combobox_ = 0; 
	}
	return TRUE;
}

void FileDialogSubclassWindow::SubclassWindow(HWND wnd)
{
	TBase::SubclassWindow(wnd);
	SetTimer(kTimerId, 50);
}

FileDialogSubclassWindow::FileDialogSubclassWindow(CWebViewWindow* webViewWindow)
{
	webViewWindow_ = webViewWindow;
	editControl_ = 0;
	combobox_ = 0;
}

LRESULT FileDialogSubclassWindow::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	bHandled = false;
	EnumChildWindows(m_hWnd, EnumChildProc, (LPARAM)this);

	bool isWindowVisible = this->IsWindowVisible();
	if (this->editControl_ ) {


		//LOG(INFO) << "isWindowVisible" << isWindowVisible;
		this->SetWindowLong( GWL_STYLE, this->GetWindowLong(GWL_STYLE) & WS_CHILD & ~ (WS_CAPTION |  WS_SYSMENU | WS_SIZEBOX | WS_MAXIMIZEBOX | 
			WS_MINIMIZEBOX|WS_POPUP|WS_BORDER|WS_VISIBLE));
		this->SetWindowLong( GWL_EXSTYLE, 0);
		this->ShowWindow(SW_HIDE);
		this->SetWindowPos(HWND_BOTTOM,0,0,1,1,SWP_HIDEWINDOW);
		//SetTimer(kFillTimerId,400);
		webViewWindow_->fileDialog_ = m_hWnd;
		webViewWindow_->editControl_ = editControl_;
		webViewWindow_->combobox_  = combobox_;
		webViewWindow_->SetFillTimer();
	}
	return 0;
}

LRESULT FileDialogSubclassWindow::OnShow(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
	if ( wParam == TRUE ) {
		//LOG(INFO) << "FileDialogSubclassWindow::OnShow";
		//::SetWindowLong(m_hWnd, GWL_STYLE, ::GetWindowLong(m_hWnd, GWL_STYLE) & ~ (WS_VISIBLE));
		::SetWindowLong(m_hWnd, GWL_STYLE, ::GetWindowLong(m_hWnd, GWL_STYLE) & ~ (WS_VISIBLE|WS_CAPTION |  WS_SYSMENU | WS_SIZEBOX | WS_MAXIMIZEBOX | 
			WS_MINIMIZEBOX));
		//SetWindowLong(GWL_STYLE, GetWindowLong(GWL_STYLE) & ~WS_VISIBLE);
		::SetWindowPos(HWND_BOTTOM, m_hWnd,0,0,1,1,SWP_HIDEWINDOW);
		ShowWindow(SW_HIDE);
		bHandled = true;
	}
	return 0;
}

LRESULT FileDialogSubclassWindow::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	Detach();
	bHandled = false;
	return 0;
}

LRESULT FileDialogSubclassWindow::OnGetMinMaxInfo(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
{
	//LOG(INFO) << "OnGetMinMaxInfo";
	MINMAXINFO* mmi = (MINMAXINFO*) lParam;
	ZeroMemory(mmi, sizeof(MINMAXINFO));
	/*mmi->ptMaxSize.x = 1;
	mmi->ptMaxSize.y = 1;
	mmi->ptMaxPosition.x=1;
	mmi->ptMaxPosition.y=1;
	mmi->ptMaxPosition.x=1;
	mmi->ptMaxPosition.y=1;
	mmi->ptMinTrackSize.x=1;
	mmi->ptMinTrackSize.x=1;
	mmi->ptMaxTrackSize.x=1;
	mmi->ptMaxTrackSize.y=1;*/
	bHandled = true;
	return 0;
}

LRESULT FileDialogSubclassWindow::OnActivate(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if ( LOWORD(wParam) == WA_ACTIVE ) {
		bHandled = true;
		HWND old = (HWND)lParam;
		if ( old != m_hWnd ) {
			::SetActiveWindow(old);
		}
	}
	return 0;
}

LRESULT FileDialogSubclassWindow::OnTimer(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
	bHandled = false;
	if ( wParam == kTimerId ) {
		ShowWindow(SW_HIDE);
		::SetWindowPos(HWND_BOTTOM, m_hWnd,0,0,1,1,SWP_HIDEWINDOW);
	} else if (  wParam == kFillTimerId  ) {
		VARIANT v;
		v.vt = VT_I4 ;
		v.lVal  = CHILDID_SELF;
		CComPtr<IAccessible> editControlAccesible;
		if ( ::IsWindow(editControl_) ) {
			
			HRESULT hr = ::AccessibleObjectFromWindow(editControl_, OBJID_CLIENT , IID_IAccessible, (void**)(&editControlAccesible)); 
		}

		if ( editControl_ && (!::IsWindow(editControl_) || !editControlAccesible )) {
			if ( webViewWindow_->compareFileNameWithFileInputField() ) {

				KillTimer(kFillTimerId);
				if (webViewWindow_->onFileFieldFilled ) {
					webViewWindow_->onFileFieldFilled(webViewWindow_->uploadFileName_);
				}
				webViewWindow_->uploadFileName_.Empty();
				editControl_ = 0;
				return 0;
			}
		} 

		 {
			
			if ( editControlAccesible ) {
				/*for ( int i = 0; i < 5; i++ )*/ {
					if ( IsWindow() && !IsWindowEnabled()) {
						LOG(ERROR) << "fileDialog_ is disabled. Enabling it again.";
						HWND activeWindow  = ::GetActiveWindow();
						if ( activeWindow != m_hWnd  ) {
							::EndDialog(activeWindow, IDOK);
						}
					} 
					if ( webViewWindow_->compareFileNameWithFileInputField() ) {
						
						::SetActiveWindow(webViewWindow_->activeWindowBeforeFill_);
						webViewWindow_->activeWindowBeforeFill_ = 0; 
						//fileDialogEvent_.PulseEvent();
						KillTimer(kFillTimerId);
						if (webViewWindow_->onFileFieldFilled ) {
							webViewWindow_->onFileFieldFilled(webViewWindow_->uploadFileName_);
						}
						webViewWindow_->uploadFileName_.Empty();
							return 0;
					}

					for ( int j = 0; j < 5; j++ ) {
						editControlAccesible->put_accValue(v, CComBSTR(webViewWindow_->uploadFileName_));
						if ( GuiTools::GetWindowText(editControl_) == webViewWindow_->uploadFileName_) {
							break;
						}
					}
					
					if ( GuiTools::GetWindowText(editControl_) == webViewWindow_->uploadFileName_) {
						SendMessage(WM_COMMAND, MAKELPARAM(IDOK, BN_CLICKED), (LPARAM)(HWND)GetDlgItem(IDOK));

						if ( IsWindow() && !IsWindowEnabled()  ) {
							HWND activeWindow  = ::GetActiveWindow();
							if ( activeWindow !=m_hWnd  ) {
								::EndDialog(activeWindow, IDOK);
							}
						}
						
					}
				}
			}
		}
	}
		
	return 0;
}


LRESULT FileDialogSubclassWindow::OnWindowPosChanging(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
{
	bHandled = true;
	WINDOWPOS *wp = (WINDOWPOS *)lParam;
	wp->cx = 0;
	wp->cy = 0;
	wp->x = 0;
	wp->y = 0;
	wp->hwndInsertAfter = HWND_BOTTOM;
	wp->flags &=  SWP_HIDEWINDOW;
	return 0;
}

BOOL CALLBACK FileDialogSubclassWindow::EnumChildProc(HWND wnd, LPARAM lParam)
{
	FileDialogSubclassWindow* this_ = reinterpret_cast<FileDialogSubclassWindow*>(lParam);

	TCHAR Buffer[MAX_PATH] = _T("");
	GetClassName(wnd, Buffer, sizeof(Buffer)/sizeof(TCHAR));
	if ( (!lstrcmpi(Buffer, _T("ComboBoxEx32")) &&  (::GetWindowLong(wnd, GWL_STYLE))& CBS_DROPDOWN) ) {
		HWND edit = FindWindowEx(wnd, 0, _T("Edit"),0);
		if ( !edit ) {
			HWND combobox = FindWindowEx(wnd, 0, _T("Combobox"),0);
			edit = FindWindowEx(combobox, 0, _T("Edit"),0);
		}
		this_->editControl_ = edit;
		this_->combobox_ = wnd;

	} else if ( !lstrcmpi(Buffer, _T("Edit"))  ) {
		this_->editControl_ = wnd;
		this_->combobox_ = 0; 
	}
	return TRUE;
}


/*LRESULT CALLBACK FileDialogSubclassWindow::MyWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// a thunk substitutes 'this' pointer for HWND parameter on the 
	FileDialogSubclassWindow* pThis = 
		static_cast<FileDialogSubclassWindow*>(reinterpret_cast<TBase*>(hWnd)); 
	HWND wnd  = pThis->m_hWnd;
	if ( uMsg == WM_INITDIALOG ) {
		EnumChildWindows(wnd, EnumChildProc, (LPARAM)pThis);

		bool isWindowVisible = pThis->IsWindowVisible();
		if (pThis->editControl_ ) {


			LOG(INFO) << "isWindowVisible" << isWindowVisible;
			pThis->SetWindowLong( GWL_STYLE, pThis->GetWindowLong(GWL_STYLE) & WS_CHILD & ~ (WS_CAPTION |  WS_SYSMENU | WS_SIZEBOX | WS_MAXIMIZEBOX | 
				WS_MINIMIZEBOX|WS_POPUP|WS_BORDER|WS_VISIBLE));
			pThis->ShowWindow(SW_HIDE);
			pThis->SetWindowPos(HWND_BOTTOM,0,0,1,1,0);
		}

		LRESULT res = TBase::WindowProc(hWnd, uMsg, wParam, lParam);
		pThis->SetTimer(kFillTimerId,400);
		//LOG(ERROR) << "CWebViewWindowOnActivate="<<GuiTools::GetWindowText(wnd);
		
	}
	// forward to original proc if you want 
	return TBase::WindowProc(hWnd, uMsg, wParam, lParam); 
}*/

CDialogHook::CDialogHook(CWebViewWindow* webViewWindow)
{
	webViewWindow_ = webViewWindow;
	Hook();
}

CDialogHook::~CDialogHook()
{
	Hook(0);
}

LRESULT CDialogHook::HookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	
	if(nCode == HCBT_CREATEWND)
	{
		HWND hwnd = (HWND)wParam;
		CBT_CREATEWND *cw = (CBT_CREATEWND*)lParam;
		if ( !lstrcmp(	GetWndClass(wParam, lParam), _T("#32770"))
				&& !lstrcmp(cw->lpcs->lpszName, _T("Открытие"))
			) { 
			webViewWindow_->handleDialogCreation(hwnd, true);
			
		}

		return CCBTHook::HookProc(nCode, wParam, lParam); // calls next hook
	}
}
