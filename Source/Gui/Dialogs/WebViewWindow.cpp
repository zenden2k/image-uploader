#include "WebViewWindow.h"
#include <Func/WinUtils.h>
#include <Gui/GuiTools.h>

CWebViewWindow::CWebViewWindow() {
	isModal_ = false;
}
CWebViewWindow::~CWebViewWindow() {

}

LRESULT CWebViewWindow::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	icon_ = GuiTools::LoadBigIcon(IDR_MAINFRAME);
	iconSmall_ = GuiTools::LoadSmallIcon(IDR_MAINFRAME);

	SetIcon(icon_, TRUE);
	SetIcon(iconSmall_, FALSE);

	WinUtils::UseLatestInternetExplorerVersion(false);
	RECT rc;
	GetWindowRect(&rc);
	hWndClient_ = view_.Create(m_hWnd, rc, _T("about:blank"), WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_HSCROLL | WS_VSCROLL, WS_EX_CLIENTEDGE);
	view_.PutSilent(TRUE); // Supress javascript errors http://stackoverflow.com/questions/7646055/supressing-script-error-in-ie8-c
	//view_.GoHome();
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
	if ( show ) {
		if ( parent ) {
			::EnableWindow(parent, true);
		}
		ShowWindow(SW_HIDE);
		if ( parent ) {
			::SetActiveWindow(parent);
		}
	}
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

LRESULT CWebViewWindow::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	if ( isModal_ ) {
		PostQuitMessage(0);
	} else {
		ShowWindow(SW_HIDE);
	}
	bHandled = true;
	return 1;
}
