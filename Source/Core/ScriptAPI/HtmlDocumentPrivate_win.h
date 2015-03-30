#ifndef IU_CORE_HTMLDOCUMENTPRIVATE_WIN_H
#define IU_CORE_HTMLDOCUMENTPRIVATE_WIN_H

#pragma once
#include "HtmlElementPrivate_win.h"
#include <ComDef.h>
#include <Core/Utils/CoreUtils.h>

namespace ScriptAPI {

class HtmlDocumentPrivate {
	public:
		HtmlDocumentPrivate(IDispatchPtr disp) {
			disp_ = disp;
			doc_ = CComQIPtr<IHTMLDocument2,&IID_IHTMLDocument2>(disp);
			doc3_ = CComQIPtr<IHTMLDocument3,&IID_IHTMLDocument3>(disp);
		}

		HtmlElement rootElement() {
			CComPtr<IHTMLElementCollection> collection;
			CComBSTR tagBstr = _T("html");
			doc3_->getElementsByTagName(tagBstr, &collection);
			long count  = 0;
			if ( SUCCEEDED(collection->get_length(&count))) {
				return HtmlElement();
			}
			for ( int i = 0; i < count; i ++ ) {
				IDispatchPtr  disp = 0;
				collection->item(CComVariant(i), CComVariant(0), &disp);
				return  new HtmlElementPrivate(disp);
			}
		}

		HtmlElement getElementById(const std::string& id) {
			CComPtr<IHTMLElement> elem = 0;
			CComBSTR idBstr = IuCoreUtils::Utf8ToWstring(id).c_str();
			if ( SUCCEEDED( doc3_->getElementById(idBstr, &elem))  && elem  ) {
				return new HtmlElementPrivate(elem);
			}
			return HtmlElement();
		}

		SquirrelObject getElementsByTagName(const std::string& tag) {
			CComPtr<IHTMLElementCollection> collection;
			CComBSTR tagBstr = IuCoreUtils::Utf8ToWstring(tag).c_str();
			doc3_->getElementsByTagName(tagBstr, &collection);
			long count  = 0;
			if ( SUCCEEDED(collection->get_length(&count))) {
				return SquirrelObject();
			}
			SquirrelObject res = SquirrelVM::CreateArray(count);
			for ( int i = 0; i < count; i ++ ) {
				IDispatchPtr  disp = 0;
				collection->item(CComVariant(i), CComVariant(0), &disp);
				res.SetValue(i, HtmlElement(new HtmlElementPrivate(disp)));
			}
		}

		SquirrelObject getElementsByName(const std::string& name) {
			CComPtr<IHTMLElementCollection> collection;
			CComBSTR tagBstr = IuCoreUtils::Utf8ToWstring(name).c_str();
			doc3_->getElementsByName(tagBstr, &collection);
			long count  = 0;
			if ( SUCCEEDED(collection->get_length(&count))) {
				return SquirrelObject();
			}
			SquirrelObject res = SquirrelVM::CreateArray(count);
			for ( int i = 0; i < count; i ++ ) {
				IDispatchPtr  disp = 0;
				collection->item(CComVariant(i), CComVariant(0), &disp);
				res.SetValue(i, HtmlElement(new HtmlElementPrivate(disp)));
			}
		}

	protected:
		CComPtr<IHTMLDocument2> doc_;
		CComPtr<IHTMLDocument3> doc3_;
		IDispatchPtr disp_;
};

}
#endif