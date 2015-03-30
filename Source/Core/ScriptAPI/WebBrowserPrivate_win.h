#ifndef IU_CORE_SCRIPTAPI_WEBBROWSERPRIVATE_WIN_H
#define IU_CORE_SCRIPTAPI_WEBBROWSERPRIVATE_WIN_H

#include "ScriptAPI.h"

#include <Gui/Dialogs/WebViewWindow.h>
#include <Core/Utils/CoreUtils.h>
#include <Core/Logging.h>
#include <Core/Squirrelnc.h>
#include <Gui/Dialogs/WizardDlg.h>
#include "HtmlDocumentPrivate_win.h"
namespace ScriptAPI {
using namespace SqPlus;
class WebBrowserPrivate {
public:
	WebBrowserPrivate(CWebBrowser * browser ) {
		browser_ = browser;
		webViewWindow_.view_.onNavigateComplete2.bind(this, &WebBrowserPrivate::OnPageLoaded);
		webViewWindow_.view_.onNavigateError.bind(this, &WebBrowserPrivate::OnNavigateError);
		webViewWindow_.view_.onDocumentComplete.bind(this, &WebBrowserPrivate::OnDocumentComplete);
		initialWidth_ = 800;
		initialHeight_ = 600;
		initialTitle_ = _T("Web Browser");
	}

	~WebBrowserPrivate() {
		if ( webViewWindow_.m_hWnd ) {
			webViewWindow_.DestroyWindow();
		}
	}

	bool navigateToUrl(const std::string& url) {
		if ( webViewWindow_.m_hWnd) {
			return webViewWindow_.NavigateTo(IuCoreUtils::Utf8ToWstring(url).c_str());
		} else {
			initialUrl_ = IuCoreUtils::Utf8ToWstring(url).c_str();
			return true;
		}
	}

	void setOnUrlChangedCallback(SquirrelObject callBack, SquirrelObject context) {
		onUrlChangedCallback_ = callBack;
		onUrlChangedCallbackContext_ = context;
	}

	void setOnNavigateErrorCallback(SquirrelObject callBack, SquirrelObject context) {
		onNavigateErrorCallback_ = callBack;
		onNavigateErrorCallbackContext_ = context;
	}

	void setOnLoadFinishedCallback(SquirrelObject callBack, SquirrelObject context) {
		onLoadFinishedCallback_ = callBack;
		onLoadFinishedCallbackContext_ = context;
	}

	bool showModal() {
		HWND parent = 
#ifndef IU_CLI
			pWizardDlg->m_hWnd;
#else 
			0;
#endif
			create();
		
		if ( !initialUrl_.IsEmpty() ) {
			webViewWindow_.NavigateTo(initialUrl_);
		} else if ( !initialHtml_.IsEmpty()) {
			webViewWindow_.view_.displayHTML(initialHtml_);
		}

		return webViewWindow_.DoModal(parent);

		//DestroyWindow();
		return true;
	}

	bool exec() {
		create();
		if ( !initialUrl_.IsEmpty() ) {
			webViewWindow_.NavigateTo(initialUrl_);
		} else if ( !initialHtml_.IsEmpty()) {
			webViewWindow_.view_.displayHTML(initialHtml_);
		}

		return webViewWindow_.exec();
	}

	void show() {
		create();
		webViewWindow_.ShowWindow(SW_SHOW);
	}

	void hide() {
		webViewWindow_.ShowWindow(SW_HIDE);
	}

	void close() {
		webViewWindow_.close();
	}

	bool setHtml(const std::string& html) {
		if ( webViewWindow_.m_hWnd) {
			return webViewWindow_.view_.displayHTML(IuCoreUtils::Utf8ToWstring(html).c_str()) == ERROR_SUCCESS;
		} else {
			initialHtml_ = IuCoreUtils::Utf8ToWstring(html).c_str();
			return true;
		}
	}

	void resize(int w, int h) {
		if ( webViewWindow_.m_hWnd) {
			CDC hdc = ::GetDC(webViewWindow_.m_hWnd);
			float dpiScaleX_ = GetDeviceCaps(hdc, LOGPIXELSX) / 96.0f;
			float dpiScaleY_ = GetDeviceCaps(hdc, LOGPIXELSY) / 96.0f;
			webViewWindow_.ResizeClient(w * dpiScaleX_, h * dpiScaleY_);

		} else {
			initialWidth_ = w;
			initialHeight_ = h;
		}
	}

	void setTitle(const std::string& title) {
		if ( webViewWindow_.m_hWnd) {
			webViewWindow_.SetWindowText(IuCoreUtils::Utf8ToWstring(title).c_str());
		} else {
			initialTitle_ = IuCoreUtils::Utf8ToWstring(title).c_str();
		}
	}

	const std::string url() {
		if ( webViewWindow_.m_hWnd) {
			return IuCoreUtils::WstringToUtf8((LPCTSTR)webViewWindow_.view_.GetLocationURL());
		} 
		return std::string();
	}

	const std::string title() {
		if ( webViewWindow_.m_hWnd) {
			return IuCoreUtils::WstringToUtf8((LPCTSTR)webViewWindow_.view_.GetLocationName());
		} 
		return IuCoreUtils::WstringToUtf8((LPCTSTR)initialTitle_);
	}

	const std::string getDocumentContents() {
		return /*IuCoreUtils::WstringToUtf8((LPCTSTR)webViewWindow_.view_.GetHTML());*/document().getHTML();
	}

	HtmlDocument document();

	IWebBrowser2* getBrowserInterface() {
		return  webViewWindow_.view_.GetBrowserInterface();
	}

	bool runJavaScript(const std::string& code) {
		if ( webViewWindow_.m_hWnd) {
			CComVariant res;
			/*CComBSTR strSource();
			CComVariant bstrTarget;
			
			bstrTarget.vt = VT_BSTR;
			bstrTarget.bstrVal = strSource.Copy();*/
			return webViewWindow_.view_.CallJScript(_T("eval"), IuCoreUtils::Utf8ToWstring(code).c_str(), &res);
		} else {
			LOG(ERROR) << "injectJavaScript: WebBrowser control is not created yet.";
		}
	}

	SquirrelObject callJavaScriptFunction(const std::string& funcName, SquirrelObject args) {
		if ( webViewWindow_.m_hWnd) {
			CComVariant res;
			webViewWindow_.view_.CallJScript(IuCoreUtils::Utf8ToWstring(funcName).c_str(), &res);
		} else {
			LOG(ERROR) << "injectJavaScript: WebBrowser control is not created yet.";
		}
		return SquirrelObject();
	}

protected:
	CWebViewWindow webViewWindow_;
	SquirrelObject onUrlChangedCallback_;
	SquirrelObject onUrlChangedCallbackContext_;
	SquirrelObject onNavigateErrorCallback_;
	SquirrelObject onNavigateErrorCallbackContext_;
	SquirrelObject onLoadFinishedCallback_;
	SquirrelObject onLoadFinishedCallbackContext_;
	CString initialUrl_;
	CString initialTitle_;
	CString initialHtml_;
	CWebBrowser* browser_;
	int initialWidth_;
	int initialHeight_;

	void create(HWND parent = 0 ) {
		if ( webViewWindow_.m_hWnd) {
			return;
		}
	
		

		CRect r(0,0, initialWidth_, initialWidth_);
		webViewWindow_.Create(0,CWindow::rcDefault,initialTitle_, WS_POPUP | WS_OVERLAPPEDWINDOW	);
		CDC hdc = ::GetDC(webViewWindow_.m_hWnd);
		float dpiScaleX_ = GetDeviceCaps(hdc, LOGPIXELSX) / 96.0f;
		float dpiScaleY_ = GetDeviceCaps(hdc, LOGPIXELSY) / 96.0f;

		if ( initialWidth_ && initialHeight_ ) {
			webViewWindow_.ResizeClient(initialWidth_ * dpiScaleX_, initialHeight_ * dpiScaleY_);
		}
	}
	void OnPageLoaded(const CString& url);

	void OnDocumentComplete(const CString& url);


	bool OnNavigateError(const CString& url, LONG statusCode);
};

}
#endif
