#ifndef IU_GUI_DIALOGS_WEBVIEWWINDOW_H
#define IU_GUI_DIALOGS_WEBVIEWWINDOW_H

#include "atlheaders.h"
#include <atlcrack.h>
#include "resource.h"       // main symbols
#include <Gui/Controls/WTLBrowserView.h>

class CWebViewWindow: public CWindowImpl<CWebViewWindow>
{
public:
	CWebViewWindow();
	~CWebViewWindow();

	DECLARE_WND_CLASS(_T("CWebViewWindow"))
	bool NavigateTo(const CString& url);
protected:
	BEGIN_MSG_MAP(CWebViewWindow)
		
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnResize)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
	END_MSG_MAP()
	// Handler prototypes:
	//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnResize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	CWTLBrowserView view_;
	HWND hWndClient_;

};


#endif