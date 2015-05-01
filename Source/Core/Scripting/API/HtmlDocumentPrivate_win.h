/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2015 Sergey Svistunov (zenden2k@gmail.com)

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
#ifndef IU_CORE_HTMLDOCUMENTPRIVATE_WIN_H
#define IU_CORE_HTMLDOCUMENTPRIVATE_WIN_H

#pragma once
#include "HtmlElementPrivate_win.h"
#include <ComDef.h>
#include "Core/Utils/CoreUtils.h"

namespace ScriptAPI {
class WebBrowserPrivate;
class HtmlElementPrivate;
class HtmlDocumentPrivate {
    public:
        HtmlDocumentPrivate(IDispatchPtr disp, WebBrowserPrivate *browserPrivate) {
            disp_ = disp;
            doc_ = CComQIPtr<IHTMLDocument2,&IID_IHTMLDocument2>(disp);
            doc3_ = CComQIPtr<IHTMLDocument3,&IID_IHTMLDocument3>(disp);
            browserPrivate_ = browserPrivate;
        }

        HtmlElement rootElement();

        HtmlElement getElementById(const std::string& id);

        Sqrat::Array getElementsByTagName(const std::string& tag);

        Sqrat::Array getElementsByName(const std::string& name);

        HtmlElement querySelector(const std::string& query)
        {
            return rootElement().querySelector(query);
        }    

        Sqrat::Array querySelectorAll(const std::string& query)
        {
            return rootElement().querySelectorAll(query);
        }

        const std::string getHTML() {
            return rootElement().getOuterHTML();
        }
        friend class HtmlElementPrivate;
    protected:
        CComPtr<IHTMLDocument2> doc_;
        CComPtr<IHTMLDocument3> doc3_;
        CComPtr<IElementSelector> selector_;
        HtmlElement rootElement_;
        
        IDispatchPtr disp_;
        WebBrowserPrivate *browserPrivate_;
};

}
#endif