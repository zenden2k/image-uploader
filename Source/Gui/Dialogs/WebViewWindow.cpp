#include "WebViewWindow.h"

#include <UrlMon.h>

#include "Func/WinUtils.h"
#include "Gui/GuiTools.h"
#include "Core/Logging.h"
#ifdef IU_ENABLE_WEBVIEW2
    #include <wrl.h>
    #include <wil/com.h>
    #include "WebView2.h"
#endif

HWND CWebViewWindow::window = nullptr;

CWebViewWindow::CWebViewWindow()
        {
    //instance = this;
    isModal_ = false;
    messageLoopIsRunning_ = false;
    hWndClient_ = nullptr;
    //dialogHook_ = 0;
}
CWebViewWindow::~CWebViewWindow() {


    //instance = 0;
    //delete dialogHook_;
}

LRESULT CWebViewWindow::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    window = m_hWnd;
    icon_ = GuiTools::LoadBigIcon(IDR_MAINFRAME);
    iconSmall_ = GuiTools::LoadSmallIcon(IDR_MAINFRAME);

    SetIcon(icon_, TRUE);
    SetIcon(iconSmall_, FALSE);

    HWND hWnd = m_hWnd;
    ::CoInitialize(NULL);
    HRESULT hr = -1;
#ifdef IU_ENABLE_WEBVIEW2
    using namespace Microsoft::WRL;
    hr = CreateCoreWebView2EnvironmentWithOptions(nullptr, nullptr, nullptr,
    Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
        [hWnd, this](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
            
            // Create a CoreWebView2Controller and get the associated CoreWebView2 whose parent is the main window hWnd
            env->CreateCoreWebView2Controller(hWnd, Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                [hWnd, this](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
                if (controller != nullptr) {
                    webviewController_ = controller;
                    webviewController_->get_CoreWebView2(&webviewWindow_);
                }
                
                // Add a few settings for the webview
                // The demo step is redundant since the values are the default settings
                /*ICoreWebView2Settings* Settings;
                webviewWindow_->get_Settings(&Settings);
                Settings->put_IsScriptEnabled(TRUE);
                Settings->put_AreDefaultScriptDialogsEnabled(TRUE);
                Settings->put_IsWebMessageEnabled(TRUE);*/
                
                // Resize WebView to fit the bounds of the parent window
                RECT bounds;
                ::GetClientRect(hWnd, &bounds);
                HRESULT hr2 = webviewController_->put_Bounds(bounds);
                if (FAILED(hr2)) {
                    LOG(ERROR) << "Failed to put bounds on webview";
                }
                // Schedule an async task to navigate to Bing
                    if (!initialUrl_.IsEmpty()) {
                                        webviewWindow_->Navigate(initialUrl_);
                    }

                  EventRegistrationToken token;
                    webviewWindow_->add_NavigationStarting(Callback<ICoreWebView2NavigationStartingEventHandler>(
                        [this](ICoreWebView2* webview, ICoreWebView2NavigationStartingEventArgs * args) -> HRESULT {
                            PWSTR uri;
                            args->get_Uri(&uri);
                            std::wstring source(uri);
                            if (onUrlChanged_) {
                                onUrlChanged_(source.c_str());
                            }
  
                            CoTaskMemFree(uri);
                            return S_OK;
                 }).Get(), &token);

                 /*webviewWindow_->add_NavigationCompleted(Callback<ICoreWebView2NavigationCompletedEventHandler>(
                        [this](ICoreWebView2* webview, ICoreWebView2NavigationCompletedEventArgs * args) -> HRESULT {
                            PWSTR uri;
                            args->(&uri);
                            std::wstring source(uri);
                            if (onUrlChanged_) {
                                onUrlChanged_(source.c_str());
                            }
  
                            CoTaskMemFree(uri);
                            return S_OK;
                 }).Get(), &token); */ 
                
                return S_OK;
            }).Get());
        return S_OK;
    }).Get());
#endif
    if (FAILED(hr)) {
        WinUtils::UseLatestInternetExplorerVersion(false);

        CoInternetSetFeatureEnabled(FEATURE_DISABLE_NAVIGATION_SOUNDS, SET_FEATURE_ON_PROCESS, true);
        
        RECT rc;
        GetWindowRect(&rc);
        hWndClient_ = view_.Create(m_hWnd, rc, _T("about:blank"), WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_HSCROLL | WS_VSCROLL, 0);
        view_.PutSilent(TRUE); // Suppress javascript errors http://stackoverflow.com/questions/7646055/supressing-script-error-in-ie8-c
        view_.SetFocus();
    }
    
    return 0;
}

LRESULT CWebViewWindow::OnResize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    RECT clientRect;
    GetClientRect(&clientRect);
     if (view_.m_hWnd) {
        view_.SetWindowPos(NULL, &clientRect, SWP_NOMOVE);
    }
#ifdef IU_ENABLE_WEBVIEW2
    else if (webviewController_) {
         HRESULT hr2 = webviewController_->put_Bounds(clientRect);
    }
#endif
   
    return 0;
}

bool CWebViewWindow::NavigateTo(const CString& url) {
    if (view_.m_hWnd) {
        view_.Navigate(url);
    }
#ifdef IU_ENABLE_WEBVIEW2
    else {
        if (webviewWindow_) {
            webviewWindow_->Navigate(url);
        } else {
            initialUrl_ = url;
        }
    }
#endif     
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
    window = 0;
    return res;
}

int CWebViewWindow::exec()
{
    return DoModal(0, false);
}

void CWebViewWindow::close(int retCode)
{
    PostQuitMessage(retCode);
}

void CWebViewWindow::abortFromAnotherThread()
{
    PostMessage(WM_CLOSE);
}

void CWebViewWindow::destroyFromAnotherThread()
{
    SendMessage(WM_DESTROYWEBVIEWWINDOW);
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

LRESULT CWebViewWindow::OnDestroyFromAnotherThread(UINT, WPARAM, LPARAM, BOOL&)
{
    DestroyWindow();
    m_hWnd = 0;
    return 0;
}

void CWebViewWindow::setSilent(bool silent) {
    if (view_.m_hWnd) {
        view_.PutSilent(silent);
    } else {
        //initialSilent_ = silent;
    }
}

bool CWebViewWindow::displayHTML(const CString& html) {
    return view_.displayHTML(html) == ERROR_SUCCESS;
}

void CWebViewWindow::setOnUrlChanged(std::function<void(const CString&)> cb) {
    //if (view_.m_hWnd) {
        view_.setOnNavigateComplete2(std::move(cb));
    /*} else {
        onUrlChanged_ = cb;
    }*/

}

void CWebViewWindow::setOnDocumentComplete(std::function<void(const CString&)> cb) {
    onDocumentComplete_ = cb;
}

void CWebViewWindow::setOnNavigateError(std::function<bool(const CString&, LONG)> cb) {
    onNavigateError_ = cb;
}
