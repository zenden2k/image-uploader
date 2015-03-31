#include "WebViewWindow.h"
#include <Func/WinUtils.h>
#include <Gui/GuiTools.h>
#include <Core/Logging.h>


HWND CWebViewWindow::window = 0;
CWebViewWindow::CWebViewWindow() {
	isModal_ = false;
	captureActivate_ = false;
	timerInterval_ = 0;
	combobox_ = 0;
	fileDialogEvent_.Create();
	fileFieldSuccess_ = false;
	messageLoopIsRunning_ = false;
	activeWindowBeforeFill_ = 0;
}
CWebViewWindow::~CWebViewWindow() {

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
	//view_.PutSilent(TRUE); // Supress javascript errors http://stackoverflow.com/questions/7646055/supressing-script-error-in-ie8-c
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

void CWebViewWindow::setUploadFileName(const CString& uploadFileName, CComPtr<IHTMLInputFileElement> inputFileElement)
{
	activeWindowBeforeFill_ = ::GetActiveWindow();
	uploadFileName_ = uploadFileName;
	inputFileElement_ = inputFileElement;
}

bool CWebViewWindow::fillInputFileField()
{
	fileFieldSuccess_ = false;

	//WinUtils::MsgWaitForSingleObject(fileDialogEvent_, 10000);
	messageLoopIsRunning_ = true;
	SetTimer(kMessageLoopTimeoutTimer, 10000);
	WinUtils::TimerWait(5000);
	/*CMessageLoop loop;
	loop.Run();*/
	messageLoopIsRunning_ = false;
	//uploadFileName_.Empty();
	return fileFieldSuccess_;
}

bool CWebViewWindow::compareFileNameWithFileInputField()
{
	CComBSTR res;
	if ( SUCCEEDED( inputFileElement_->get_value(&res) ) && res  ) {
		return res.Length() && res[0] == uploadFileName_[0];
	}
	return false;
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

			fileDialog_ = wnd;
			
			if ( !subclassWindow_.m_hWnd ) {
				subclassWindow_.SubclassWindow(fileDialog_);
			}
			
			
			bool isWindowVisible = ::IsWindowVisible(fileDialog_);
			//LOG(ERROR) << "isWindowVisible" << isWindowVisible;
			::SetWindowLong(fileDialog_, GWL_STYLE, ::GetWindowLong(fileDialog_, GWL_STYLE) & WS_CHILD & ~ (WS_CAPTION |  WS_SYSMENU | WS_SIZEBOX | WS_MAXIMIZEBOX | 
				WS_MINIMIZEBOX|WS_POPUP|WS_BORDER));
			/*::SetWindowPos(0, fileDialog_,0,0,0,0,0);
			::ShowWindow(fileDialog_, SW_HIDE);*/
			//LOG(ERROR) << "CWebViewWindowOnActivate="<<GuiTools::GetWindowText(wnd);
			EnumChildWindows(fileDialog_, EnumChildProc, (LPARAM)this);

			
			/*CComPtr<IAccessible> editControlAccesible;
			VARIANT v;
			v.vt = VT_I4 ;
			v.lVal  = CHILDID_SELF;

			HRESULT hr = ::AccessibleObjectFromWindow(editControl_, OBJID_CLIENT , IID_IAccessible, (void**)(&editControlAccesible)); // 1 - захардкоженный идентификатор ловушк
			*//*if ( editControlAccesible ) {
				editControlAccesible->put_accValue(v, CComBSTR(uploadFileName_));
			}*/
		
		}


		SetTimer(1,500);
		/*::EndDialog(wnd, IDOK);*/
	}
	return 0;
}

LRESULT CWebViewWindow::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if ( wParam == kUserTimer && onTimer ) {
		onTimer();
		return 0;
	} else if ( wParam == kMessageLoopTimeoutTimer ) {
		KillTimer(kMessageLoopTimeoutTimer);
		if ( messageLoopIsRunning_ ) {
			messageLoopIsRunning_ = false;
			PostQuitMessage(0);
		}
	}
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

LRESULT FileDialogSubclassWindow::OnShow(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
	if ( wParam == TRUE ) {
		//::SetWindowLong(m_hWnd, GWL_STYLE, ::GetWindowLong(m_hWnd, GWL_STYLE) & ~ (WS_VISIBLE));
		/*::SetWindowLong(m_hWnd, GWL_STYLE, ::GetWindowLong(m_hWnd, GWL_STYLE) & ~ (WS_VISIBLE|WS_CAPTION |  WS_SYSMENU | WS_SIZEBOX | WS_MAXIMIZEBOX | 
			WS_MINIMIZEBOX));*/
		::SetWindowPos(0, m_hWnd,0,0,1,1,0);
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
	mmi->ptMaxSize.x = 1;
	mmi->ptMaxSize.y = 1;
	mmi->ptMaxPosition.x=1;
	mmi->ptMaxPosition.y=1;
	mmi->ptMaxPosition.x=1;
	mmi->ptMaxPosition.y=1;
	mmi->ptMinTrackSize.x=1;
	mmi->ptMinTrackSize.x=1;
	mmi->ptMaxTrackSize.x=1;
	mmi->ptMaxTrackSize.y=1;
	bHandled = true;
	return 0;
}

LRESULT FileDialogSubclassWindow::OnActivate(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	if ( LOWORD(wParam) == WA_ACTIVE ) {
		HWND old = (HWND)lParam;
		if ( old != m_hWnd ) {
			::SetActiveWindow(old);
		}
	}
	return 0;
}
