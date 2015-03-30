#ifndef IU_CORE_HTMLDOCUMENTPRIVATE_WIN_H
#define IU_CORE_HTMLDOCUMENTPRIVATE_WIN_H

#pragma once
#include "HtmlElementPrivate_win.h"
#include <ComDef.h>
#include <Core/Utils/CoreUtils.h>

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

		SquirrelObject getElementsByTagName(const std::string& tag);

		SquirrelObject getElementsByName(const std::string& name);

		HtmlElement querySelector(const std::string& query)
		{
			return rootElement().querySelector(query);
		}	

		SquirrelObject querySelectorAll(const std::string& query)
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