#include "HtmlDocumentPrivate_win.h"

using namespace ScriptAPI;
//DECLARE_INSTANCE_TYPE(HtmlDocument);
DECLARE_INSTANCE_TYPE(HtmlElement);

namespace ScriptAPI {;

HtmlElement HtmlDocumentPrivate::rootElement() {
	if ( rootElement_.isNull() ) {
		CComPtr<IHTMLElementCollection> collection;
		CComBSTR tagBstr = _T("html");
		doc3_->getElementsByTagName(tagBstr, &collection);
		long count  = 0;
		if ( !SUCCEEDED(collection->get_length(&count))) {
			return HtmlElement();
		}
		for ( int i = 0; i < count; i ++ ) {
			IDispatchPtr  disp = 0;
			collection->item(CComVariant(i), CComVariant(0), &disp);
			rootElement_ = new HtmlElementPrivate(disp, this); 
			return rootElement_;
		}
	}
	return rootElement_;
}

HtmlElement HtmlDocumentPrivate::getElementById(const std::string& id) {
	CComPtr<IHTMLElement> elem = 0;
	CComBSTR idBstr = IuCoreUtils::Utf8ToWstring(id).c_str();
	if ( SUCCEEDED( doc3_->getElementById(idBstr, &elem))  && elem  ) {
		return new HtmlElementPrivate(elem, this);
	}
	return HtmlElement();
}

SquirrelObject HtmlDocumentPrivate::getElementsByTagName(const std::string& tag) {
	CComPtr<IHTMLElementCollection> collection;
	CComBSTR tagBstr = IuCoreUtils::Utf8ToWstring(tag).c_str();
	doc3_->getElementsByTagName(tagBstr, &collection);
	long count  = 0;
	if ( !SUCCEEDED(collection->get_length(&count))) {
		return SquirrelObject();
	}
	SquirrelObject res = SquirrelVM::CreateArray(count);
	for ( int i = 0; i < count; i ++ ) {
		IDispatchPtr  disp = 0;
		collection->item(CComVariant(i), CComVariant(0), &disp);
		res.SetValue(i, HtmlElement(new HtmlElementPrivate(disp, this)));
	}
	return res;
}

SquirrelObject HtmlDocumentPrivate::getElementsByName(const std::string& name) {
	CComPtr<IHTMLElementCollection> collection;
	CComBSTR tagBstr = IuCoreUtils::Utf8ToWstring(name).c_str();
	doc3_->getElementsByName(tagBstr, &collection);
	long count  = 0;
	if ( !SUCCEEDED(collection->get_length(&count))) {
		return SquirrelObject();
	}
	SquirrelObject res = SquirrelVM::CreateArray(count);
	for ( int i = 0; i < count; i ++ ) {
		IDispatchPtr  disp = 0;
		collection->item(CComVariant(i), CComVariant(0), &disp);
		res.SetValue(i, HtmlElement(new HtmlElementPrivate(disp, this)));
	}
	return res;
}

}