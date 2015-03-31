#ifndef IU_GUI_DIALOGS_WEBVIEWWINDOW_H
#define IU_GUI_DIALOGS_WEBVIEWWINDOW_H

#include "atlheaders.h"
#include <atlcrack.h>
#include "resource.h"       // main symbols
#include <Gui/Controls/WTLBrowserView.h>
#include "3rdpart/thread.h"
class FileDialogSubclassWindow : public CWindowImpl<FileDialogSubclassWindow>
{
public:
	DECLARE_WND_CLASS(_T("FileDialogSubclassWindow"))

	BEGIN_MSG_MAP(ImageEditorWindow)
		MESSAGE_HANDLER(WM_SHOWWINDOW, OnShow)
		MESSAGE_HANDLER(WM_GETMINMAXINFO, OnGetMinMaxInfo)
		MESSAGE_HANDLER(WM_ACTIVATE, OnActivate)
		//MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
	END_MSG_MAP()
	LRESULT OnShow(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnGetMinMaxInfo(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnActivate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

};

class CWebViewWindow: public CWindowImpl<CWebViewWindow>
{
public:

	CWebViewWindow();
	~CWebViewWindow();

	DECLARE_WND_CLASS(_T("CWebViewWindow"))
	bool NavigateTo(const CString& url);
	int DoModal(HWND parent, bool show = true);
	int exec();
	void close();
	void setTimerInterval(int interval);
	CWTLBrowserView view_;
	
	fastdelegate::FastDelegate0<void> onTimer;
	fastdelegate::FastDelegate1<const CString&> onFileFieldFilled;
	void setUploadFileName(const CString& uploadFileName, CComPtr<IHTMLInputFileElement> inputFileElement);
	bool fillInputFileField();
	bool compareFileNameWithFileInputField();
	static HWND window;
protected:
	enum { kUserTimer = 400, kMessageLoopTimeoutTimer = 401};
	BEGIN_MSG_MAP(CWebViewWindow)
		
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnResize)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_ENABLE, OnEnable)
		MESSAGE_HANDLER(WM_ACTIVATE, OnActivate)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)                     
		 
	END_MSG_MAP()
	// Handler prototypes:
	//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnResize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnEnable(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	
	HWND hWndClient_;
	bool isModal_;
	CIcon icon_;
	CIcon iconSmall_;
	bool captureActivate_;
	HWND combobox_; 
	HWND editControl_;
	HWND fileDialog_;
	HWND activeWindowBeforeFill_;
	int timerInterval_;
	CEvent fileDialogEvent_;
	bool fileFieldSuccess_;
	CString uploadFileName_;
	bool messageLoopIsRunning_;
	FileDialogSubclassWindow subclassWindow_;
	CComPtr<IHTMLInputFileElement> inputFileElement_;
	static BOOL CALLBACK EnumChildProc(HWND hwnd, LPARAM lParam);
};


#endif