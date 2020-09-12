/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2018 Sergey Svistunov (zenden2k@yandex.ru)

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

*/

#include "WebBrowserPrivate_win.h"
#include "Core/Scripting/Squirrelnc.h"
#include "WebBrowser.h"

namespace ScriptAPI {;


void WebBrowserPrivate::OnUrlChanged(const CString& url) {
    if ( !onUrlChangedCallback_.IsNull() ) {
        try
        {
            Sqrat::Table data(GetCurrentThreadVM());
            data.SetValue("url", IuCoreUtils::WstringToUtf8((LPCTSTR)url).c_str());
            data.SetInstance("browser", browser_);
            //SquirrelFunction<void> func(onUrlChangedCallbackContext_.IsNull() ? *RootTable : onUrlChangedCallbackContext_, onUrlChangedCallback_);
            if (!onUrlChangedCallback_.IsNull() ) {
                onUrlChangedCallback_.Execute(data);
            }
        }
        catch (std::exception& e)
        {
            LOG(ERROR) << "onUrlChangedCallback: "<<std::string(e.what());
            FlushSquirrelOutput(GetCurrentThreadVM());
        }
    }
}

void WebBrowserPrivate::OnDocumentComplete(const CString& url) {
    if ( !onLoadFinishedCallback_.IsNull() ) {
        try
        {
            Sqrat::Table data(GetCurrentThreadVM());
            data.SetValue("url", IuCoreUtils::WstringToUtf8((LPCTSTR)url).c_str());
            data.SetInstance("browser", browser_);
            //data.SetValue("browser", browser_);
            //SquirrelFunction<void> func(onLoadFinishedCallbackContext_.IsNull() ? *RootTable : onLoadFinishedCallbackContext_, onLoadFinishedCallback_);
            if ( !onLoadFinishedCallback_.IsNull() ) {
                onLoadFinishedCallback_.Execute(data);
            }
        }
        catch (std::exception& e)
        {
            LOG(ERROR) << "onLoadFinishedCallback: " << std::string(e.what());
            FlushSquirrelOutput(GetCurrentThreadVM());
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
            Sqrat::Table data(GetCurrentThreadVM());
            data.SetValue("url", IuCoreUtils::WstringToUtf8((LPCTSTR)url).c_str());
            data.SetValue("statusCode", statusCode);
            //BindVariable(data, new WebBrowser(), "browser");
            data.SetInstance("browser", browser_);
            //SquirrelFunction<void> func(onNavigateErrorCallbackContext_.IsNull() ? *RootTable : onNavigateErrorCallbackContext_, onNavigateErrorCallback_);
            if (onNavigateErrorCallback_.IsNull())
                return false;

            onNavigateErrorCallback_.Execute(data);
        }
        catch (std::exception& e)
        {
            LOG(ERROR) << "onNavigateErrorCallback: " << std::string(e.what());
            FlushSquirrelOutput(GetCurrentThreadVM());
        }
    }
    return false;
}

}