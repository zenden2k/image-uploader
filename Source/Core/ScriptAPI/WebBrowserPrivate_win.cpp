#include "WebBrowserPrivate_win.h"
#include <Core/Squirrelnc.h>
using namespace ScriptAPI;
DECLARE_INSTANCE_TYPE(CWebBrowser);
namespace ScriptAPI {;

HtmlDocument WebBrowserPrivate::document() {
	IDispatchPtr doc = webViewWindow_.view_.GetDocument();
	//CComQIPtr<IHTMLDocument3,&IID_IHTMLDocument2> spHTML();

	return new HtmlDocumentPrivate(doc, this);
}

void WebBrowserPrivate::OnPageLoaded(const CString& url) {
	if ( !onUrlChangedCallback_.IsNull() ) {
		try
		{
			SquirrelObject data = SquirrelVM::CreateTable();
			data.SetValue("url", IuCoreUtils::WstringToUtf8((LPCTSTR)url).c_str());
			BindVariable(data, browser_, "browser");
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

void WebBrowserPrivate::OnDocumentComplete(const CString& url) {
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

void WebBrowserPrivate::setFocus()
{
	/*CComQIPtr<IHTMLDocument4> doc4(webViewWindow_.view_.GetDocument());
	if ( doc4 ) {
		doc4->focus();
	}*/
	CComQIPtr<IHTMLDocument2> doc2(webViewWindow_.view_.GetDocument());
	CComQIPtr<IHTMLWindow2>  pWindow;
	doc2->get_parentWindow(&pWindow);
	pWindow->focus();
}

bool WebBrowserPrivate::OnNavigateError(const CString& url, LONG statusCode) {
	if ( !onNavigateErrorCallback_.IsNull() ) {
		try
		{
			SquirrelObject data = SquirrelVM::CreateTable();
			data.SetValue("url", IuCoreUtils::WstringToUtf8((LPCTSTR)url).c_str());
			data.SetValue("statusCode", statusCode);
			//BindVariable(data, new WebBrowser(), "browser");
			BindVariable(data, browser_, "browser");
			SquirrelFunction<void> func(onNavigateErrorCallbackContext_.IsNull() ? *RootTable : onNavigateErrorCallbackContext_, onNavigateErrorCallback_);
			if (func.func.IsNull())
				return false;

			func(data);
		}
		catch (SquirrelError& e)
		{
			LOG(ERROR) << "onNavigateErrorCallback: " << Utf8String(e.desc);
		}
	}
	return false;
}

void WebBrowserPrivate::OnTimer()
{
	if ( !onTimerCallback_.IsNull() ) {
		try
		{
			SquirrelObject data = SquirrelVM::CreateTable();
			//data.SetValue("url", url());
			BindVariable(data, browser_, "browser");
			SquirrelFunction<void> func(onTimerCallbackContext_.IsNull() ? *RootTable : onTimerCallbackContext_, onTimerCallback_);
			if (func.func.IsNull())
				return;

			func(data);
		}
		catch (SquirrelError& e)
		{
			LOG(ERROR) << "onTimerCallback: " << Utf8String(e.desc);
		}
	}
}

void WebBrowserPrivate::OnFileFieldFilled(const CString& fileName)
{
	if ( !onFileFieldFilledCallback_.IsNull() ) {
		try
		{
			SquirrelObject data = SquirrelVM::CreateTable();
			std::string fileNameA = IuCoreUtils::WstringToUtf8((LPCTSTR)fileName);
			data.SetValue("fileName", fileNameA.c_str());
			BindVariable(data, browser_, "browser");
			SquirrelFunction<void> func(onFileFieldFilledCallbackContext_.IsNull() ? *RootTable : onFileFieldFilledCallbackContext_, onFileFieldFilledCallback_);
			if (func.func.IsNull())
				return;

			func(data);
		}
		catch (SquirrelError& e)
		{
			LOG(ERROR) << "onTimerCallback: " << Utf8String(e.desc);
		}
	}
}

}