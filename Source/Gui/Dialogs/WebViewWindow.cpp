#include "WebViewWindow.h"

CWebViewWindow::CWebViewWindow() {

}
CWebViewWindow::~CWebViewWindow() {

}

LRESULT CWebViewWindow::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	RECT rc;
	GetWindowRect(&rc);
	hWndClient_ = view_.Create(m_hWnd, rc, _T("about:blank"), WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_HSCROLL | WS_VSCROLL, WS_EX_CLIENTEDGE);
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

LRESULT CWebViewWindow::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	ShowWindow(SW_HIDE);
	bHandled = true;
	return 1;
}