#ifndef IU_CORE_SCRIPTAPI_WEBBROWSERPRIVATE_WIN_H
#define IU_CORE_SCRIPTAPI_WEBBROWSERPRIVATE_WIN_H

#include "ScriptAPI.h"

#include <Gui/Dialogs/WebViewWindow.h>
#include <Core/Utils/CoreUtils.h>
#include <Core/Logging.h>
#include <Core/Squirrelnc.h>
#include <Gui/Dialogs/WizardDlg.h>
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
		}

		return webViewWindow_.DoModal(parent);

		//DestroyWindow();
		return true;
	}

	bool exec() {
		create();
		if ( !initialUrl_.IsEmpty() ) {
			webViewWindow_.NavigateTo(initialUrl_);
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

	const std::string getDocumentBody() {
		return IuCoreUtils::WstringToUtf8((LPCTSTR)webViewWindow_.view_.GetHTML());
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
	CWebBrowser* browser_;
	int initialWidth_;
	int initialHeight_;

	void create(HWND parent = 0 ) {
		if ( webViewWindow_.m_hWnd) {
			return;
		}
	
		

		CRect r(0,0, initialWidth_, initialWidth_);
		webViewWindow_.Create(0,CWindow::rcDefault,_T("Web Browser"), WS_POPUP | WS_OVERLAPPEDWINDOW	);
		CDC hdc = ::GetDC(webViewWindow_.m_hWnd);
		float dpiScaleX_ = GetDeviceCaps(hdc, LOGPIXELSX) / 96.0f;
		float dpiScaleY_ = GetDeviceCaps(hdc, LOGPIXELSY) / 96.0f;

		if ( initialWidth_ && initialHeight_ ) {
			webViewWindow_.ResizeClient(initialWidth_ * dpiScaleX_, initialHeight_ * dpiScaleY_);
		}
	}
	void OnPageLoaded(const CString& url) {
		if ( !onUrlChangedCallback_.IsNull() ) {
			try
			{
				SquirrelObject data = SquirrelVM::CreateTable();
				data.SetValue("url", IuCoreUtils::WstringToUtf8((LPCTSTR)url).c_str());
				data.SetValue("browser", browser_);
				SquirrelFunction<void> func(onUrlChangedCallbackContext_.IsNull() ? *RootTable : onUrlChangedCallbackContext_, onUrlChangedCallback_);
				if (!func.func.IsNull() ) {
					func(data);
				}
			}
			catch (SquirrelError& e)
			{
				LOG(ERROR) << "onUrlChangedCallback: "<<Utf8String(e.desc);
			}
		}
	}

	void OnDocumentComplete(const CString& url) {
		if ( !onLoadFinishedCallback_.IsNull() ) {
			try
			{
				SquirrelObject data = SquirrelVM::CreateTable();
				data.SetValue("url", IuCoreUtils::WstringToUtf8((LPCTSTR)url).c_str());
				BindVariable(data, browser_, "browser");
				//data.SetValue("browser", browser_);
				SquirrelFunction<void> func(onLoadFinishedCallbackContext_.IsNull() ? *RootTable : onLoadFinishedCallbackContext_, onLoadFinishedCallback_);
				if (!func.func.IsNull() ) {
					func(data);
				}
			}
			catch (SquirrelError& e)
			{
				LOG(ERROR) << "onLoadFinishedCallback: " << Utf8String(e.desc);
			}
		}
	}


	bool OnNavigateError(const CString& url, LONG statusCode) {
		if ( !onNavigateErrorCallback_.IsNull() ) {
			try
			{
				SquirrelObject data = SquirrelVM::CreateTable();
				data.SetValue("url", IuCoreUtils::WstringToUtf8((LPCTSTR)url).c_str());
				data.SetValue("statusCode", statusCode);
				//BindVariable(data, new WebBrowser(), "browser");
				data.SetValue("browser", browser_);
				SquirrelFunction<void> func(onNavigateErrorCallbackContext_.IsNull() ? *RootTable : onNavigateErrorCallbackContext_, onNavigateErrorCallback_);
				if (func.func.IsNull())
					return false;

				func(data);
			}
			catch (SquirrelError& e)
			{
				LOG(ERROR) << "OnNavigateError: " << Utf8String(e.desc);
			}
		}
		return false;
	}
};

}
#endif
