#ifndef IU_GUI_DIALOGS_WEBVIEWWINDOW_H
#define IU_GUI_DIALOGS_WEBVIEWWINDOW_H

#include <functional>

#include "atlheaders.h"
#include "resource.h"       // main symbols
#include "Gui/Controls/WTLBrowserView.h"
#include "3rdpart/thread.h"
#include "3rdpart/MemberFunctionCallback.h"

class CWebViewWindow;
class FileDialogSubclassWindow : public CWindowImpl<FileDialogSubclassWindow>
{
public:
    typedef public CWindowImpl<FileDialogSubclassWindow> TBase;
    void SubclassWindow(HWND wnd);
    enum { kTimerId = 99342, kFillTimerId };


public: 
    FileDialogSubclassWindow(CWebViewWindow* webViewWindow);
    // this is a virtual function you can override 
    //WNDPROC GetWindowProc( ) {return MyWindowProc;} 

    DECLARE_WND_CLASS(_T("FileDialogSubclassWindow"))

    BEGIN_MSG_MAP(FileDialogSubclassWindow)
    //    MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_SHOWWINDOW, OnShow)
        MESSAGE_HANDLER(WM_GETMINMAXINFO, OnGetMinMaxInfo)
        MESSAGE_HANDLER(WM_ACTIVATE, OnActivate)
        MESSAGE_HANDLER(WM_TIMER, OnTimer)
    //    MESSAGE_HANDLER(WM_WINDOWPOSCHANGING , OnWindowPosChanging)

        //MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
    END_MSG_MAP()
    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnShow(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnGetMinMaxInfo(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnActivate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnWindowPosChanging(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    static LRESULT CALLBACK MyWindowProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) ;
    static BOOL CALLBACK EnumChildProc(HWND hwnd, LPARAM lParam);
    HWND editControl_;
    HWND combobox_; 
    CWebViewWindow* webViewWindow_;

};
class CWebViewWindow;

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
    void setTimerInterval(int interval);
    CWTLBrowserView view_;
    
    void fillInputFileField(const CString& uploadFileName, CComPtr<IHTMLInputFileElement> inputFileElement, CComPtr<IAccessible> accesible );
    //bool fillInputFileField();
    bool compareFileNameWithFileInputField();
    void handleDialogCreation(HWND wnd, bool fromHook = false);
    void setOnTimerCallback(std::function<void()> cb);
    void setOnFileFieldFilledCallback(std::function<void(const CString&)> cb);
    void SetFillTimer();
    static HWND window;
protected:
    enum { kUserTimer = 400, kMessageLoopTimeoutTimer = 401, WM_SETFILLTIMER = WM_USER +55, WM_FILLINPUTFIELD, WM_DESTROYWEBVIEWWINDOW};
    BEGIN_MSG_MAP(CWebViewWindow)
        
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_SIZE, OnResize)
        MESSAGE_HANDLER(WM_CLOSE, OnClose)
        MESSAGE_HANDLER(WM_ENABLE, OnEnable)
        //MESSAGE_HANDLER(WM_ACTIVATE, OnActivate)
        MESSAGE_HANDLER(WM_TIMER, OnTimer) 
        MESSAGE_HANDLER(WM_SETFILLTIMER, OnSetFillTimer) 
        MESSAGE_HANDLER(WM_FILLINPUTFIELD, OnFillInputField) 
        MESSAGE_HANDLER(WM_DESTROYWEBVIEWWINDOW, OnDestroyFromAnotherThread)
         
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
    LRESULT OnSetFillTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnFillInputField(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnDestroyFromAnotherThread(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

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
    std::vector<FileDialogSubclassWindow*> subclassedWindows_;
    CComPtr<IHTMLInputFileElement> inputFileElement_;
    static HHOOK hook_;
    static BOOL CALLBACK EnumChildProc(HWND hwnd, LPARAM lParam);
    /*static */LRESULT /*CALLBACK */CBTHook(int nCode, WPARAM wParam, LPARAM lParam);

//    CDialogHook * dialogHook_;
    CComPtr<IAccessible> accesible_;
    //static CWebViewWindow* instance;
    CBTHookMemberFunctionCallback callback_;
    HMODULE urlmonDll_;
    std::function<void()> onTimer_;
    std::function<void(const CString&)> onFileFieldFilled_;
};


#endif