#ifndef IU_GUI_DIALOGS_WEBVIEWWINDOW_H
#define IU_GUI_DIALOGS_WEBVIEWWINDOW_H

#include <functional>

#include "atlheaders.h"
#include "resource.h"       // main symbols
#include "Gui/Controls/WTLBrowserView.h"
#include "3rdpart/thread.h"
#ifdef IU_ENABLE_WEBVIEW2
    #include "WebView2.h"
#endif

class CWebViewWindow: public CWindowImpl<CWebViewWindow>//, public TimerAdapter<CWebViewWindow>
{
public:

    CWebViewWindow();
    ~CWebViewWindow();

    DECLARE_WND_CLASS(_T("CWebViewWindow"))
    bool NavigateTo(const CString& url);
    int DoModal(HWND parent, bool show = true);
    int exec();
    void close(int retCode = 1);
    void abortFromAnotherThread();
    void destroyFromAnotherThread();
    void setSilent(bool silent);
    bool displayHTML(const CString& html);
    CWTLBrowserView view_;
    static HWND window;
protected:
    enum { kUserTimer = 400, kMessageLoopTimeoutTimer = 401, WM_SETFILLTIMER = WM_USER +55, WM_FILLINPUTFIELD, WM_DESTROYWEBVIEWWINDOW};
    BEGIN_MSG_MAP(CWebViewWindow)
        
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_SIZE, OnResize)
        MESSAGE_HANDLER(WM_CLOSE, OnClose) 
        MESSAGE_HANDLER(WM_DESTROYWEBVIEWWINDOW, OnDestroyFromAnotherThread)
         
    END_MSG_MAP()
    // Handler prototypes:
    //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnResize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnDestroyFromAnotherThread(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

    HWND hWndClient_;
    bool isModal_;
    CIcon icon_;
    CIcon iconSmall_;
    CString uploadFileName_;
    CString initialUrl_;
    bool messageLoopIsRunning_;
    #ifdef IU_ENABLE_WEBVIEW2
    CComPtr<ICoreWebView2Controller> webviewController_;
    CComPtr<ICoreWebView2> webviewWindow_;
    #endif
    
    void setOnUrlChanged(std::function<void(const CString&)> cb);
    void setOnDocumentComplete(std::function<void(const CString&)> cb);
    void setOnNavigateError(std::function<bool(const CString&, LONG)> cb);

protected:
    std::function<void(const CString&)> onUrlChanged_;
    std::function<void(const CString&)> onDocumentComplete_;
    std::function<bool(const CString&, LONG)> onNavigateError_;


//    CDialogHook * dialogHook_;
    CComPtr<IAccessible> accesible_;
    //static CWebViewWindow* instance;
};


#endif