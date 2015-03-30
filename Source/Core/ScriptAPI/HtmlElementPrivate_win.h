#ifndef IU_CORE_HTMLELEMENTPRIVATE_WIN_H
#define IU_CORE_HTMLELEMENTPRIVATE_WIN_H

#pragma once
#include <ComDef.h>
#include "COMUtils.h"
#include <Core/Utils/CoreUtils.h>
#include <Core/Utils/StringUtils.h>

namespace ScriptAPI {

class HtmlElementPrivate {
	public:
		
		HtmlElementPrivate(IHTMLElement* elem) {
			elem_ = elem;
			selector_ = CComQIPtr<IElementSelector>(elem);
		}
		HtmlElementPrivate(IDispatchPtr disp) {
			disp_  = disp;
			elem_ = CComQIPtr<IHTMLElement,&IID_IHTMLElement>(disp);
			selector_ = CComQIPtr<IElementSelector>(disp);
		}

		const std::string getAttribute(const std::string& name)
		{
			if ( name == "class" || !IuStringUtils::stricmp(name.c_str(), "name")) {
				return getClassName();
			}
			CComVariant res;
			if ( SUCCEEDED( elem_->getAttribute(CComBSTR(IuCoreUtils::Utf8ToWstring(name).c_str()), 0, &res) ) ) {
				return ComVariantToString(res);
			}
			return std::string();
		}

		const std::string getClassName() {

			CComBSTR res;
			if ( SUCCEEDED( elem_->get_className(&res) ) ) {
				return ComVariantToString(res);
			}
			return std::string();
		}

		void setClassName(const std::string& name) {
			elem_->put_className(CComBSTR(IuCoreUtils::Utf8ToWstring(name).c_str()));
		}
		void setAttribute(const std::string& name, const std::string& value)
		{	
			if ( name == "class" || !IuStringUtils::stricmp(name.c_str(), "name")) {
				return setClassName(value);
			}
			elem_->setAttribute(CComBSTR(IuCoreUtils::Utf8ToWstring(name).c_str()), CComVariant(IuCoreUtils::Utf8ToWstring(value).c_str()));
		}

		void removeAttribute(const std::string& name)
		{
			VARIANT_BOOL res;
			elem_->removeAttribute(CComBSTR(IuCoreUtils::Utf8ToWstring(name).c_str()), 0, &res);
		}

		const std::string getId()
		{
			CComBSTR res;
			if ( SUCCEEDED ( elem_->get_id(&res) ) ) {
				return IuCoreUtils::WstringToUtf8((OLECHAR*)res);
			}
			return std::string();
		}

		void setId(const std::string& id)
		{
			elem_->put_id((CComBSTR(IuCoreUtils::Utf8ToWstring(id).c_str())));
		}

		const std::string getInnerHTML()
		{
			CComBSTR res;
			if ( SUCCEEDED( elem_->get_innerHTML(&res) )  ) {
				return IuCoreUtils::WstringToUtf8((OLECHAR*)res);
			}
			return std::string();
		}

		void setInnerHTML(const std::string& html)
		{
			elem_->put_innerHTML(CComBSTR(IuCoreUtils::Utf8ToWstring(html).c_str()));
		}

		const std::string getInnerText()
		{
			CComBSTR res;
			if ( SUCCEEDED( elem_->get_innerText(&res) )  ) {
				return IuCoreUtils::WstringToUtf8((OLECHAR*)res);
			}
			return std::string();
		}

		void setInnerText(const std::string& text)
		{
			elem_->put_innerText(CComBSTR(IuCoreUtils::Utf8ToWstring(text).c_str()));
		}

		const std::string getOuterHTML()
		{
			CComBSTR res;
			if ( SUCCEEDED( elem_->get_outerHTML(&res) )  ) {
				return IuCoreUtils::WstringToUtf8((OLECHAR*)res);
			}
			return std::string();
		}

		void setOuterHTML(const std::string& html)
		{
			elem_->put_outerHTML(CComBSTR(IuCoreUtils::Utf8ToWstring(html).c_str()));
		}

		const std::string getOuterText()
		{
			CComBSTR res;
			if ( SUCCEEDED( elem_->get_outerText(&res) )  ) {
				return IuCoreUtils::WstringToUtf8((OLECHAR*)res);
			}
			return std::string();
		}

		void setOuterText(const std::string& text)
		{
			elem_->put_outerText(CComBSTR(IuCoreUtils::Utf8ToWstring(text).c_str()));
		}

		const std::string getTagName()
		{
			CComBSTR res;
			if ( SUCCEEDED( elem_->get_outerText(&res) )  ) {
				return IuCoreUtils::WstringToUtf8((OLECHAR*)res);
			}
			return std::string();
		}
		bool isNull() {
			return !elem_;
		}

		HtmlElement getParentElement()
		{
			IHTMLElement* res;
			if ( SUCCEEDED( elem_->get_parentElement(&res) ) ) {
				return new HtmlElementPrivate(res);
			}
			return HtmlElement();
		}

		void scrollIntoView()
		{
			elem_->scrollIntoView(CComVariant(TRUE));
		}

		void click()
		{
			elem_->click();
		}

		void insertHTML(const std::string& html, bool atEnd /*= false */)
		{
			elem_->insertAdjacentHTML(atEnd ? L"afterBegin" : L"beforeEnd", CComBSTR(IuCoreUtils::Utf8ToWstring(html).c_str()));
		}

		void insertText(const std::string& text, bool atEnd /*= false */)
		{
			elem_->insertAdjacentText(atEnd ? L"afterBegin" : L"beforeEnd", CComBSTR(IuCoreUtils::Utf8ToWstring(text).c_str()));
		}

		SquirrelObject getChildren() {
			IDispatchPtr disp;
			elem_->get_children(&disp);
			CComQIPtr<IHTMLElementCollection> collection(disp);
			long count  = 0;
			if ( !SUCCEEDED(collection->get_length(&count))) {
				return SquirrelObject();
			}
			SquirrelObject res = SquirrelVM::CreateArray(count);
			for ( int i = 0; i < count; i ++ ) {
				IDispatchPtr  disp = 0;
				collection->item(CComVariant(i), CComVariant(0), &disp);
				res.SetValue(i, HtmlElement(new HtmlElementPrivate(disp)));
			}
			return res;
		}

		HtmlElement querySelector(const std::string& query)
		{
			if ( !selector_ ) {
				LOG(WARNING) << "This Internet Explorer version does not support querySelector functions.";
				return  HtmlElement();
			}
			CComPtr<IHTMLElement> elem;
			CComBSTR queryBstr = IuCoreUtils::Utf8ToWstring(query).c_str();
			if ( SUCCEEDED( selector_->querySelector(queryBstr, &elem))  ) {
				if ( elem ) {
					return new HtmlElementPrivate(elem);
				}
			}
			return HtmlElement();
		}	

		SquirrelObject querySelectorAll(const std::string& query)
		{
			if ( !selector_ ) {
				LOG(WARNING) << "This Internet Explorer version does not support querySelector functions.";
				return  SquirrelObject();
			}
			CComPtr<IHTMLDOMChildrenCollection> collection;
			CComBSTR queryBstr = IuCoreUtils::Utf8ToWstring(query).c_str();
			if ( !SUCCEEDED(selector_->querySelectorAll(queryBstr, &collection)) ) {
				return SquirrelObject();
			}
			long count  = 0;
			if ( !SUCCEEDED(collection->get_length(&count))) {
				return SquirrelObject();
			}
			SquirrelObject res = SquirrelVM::CreateArray(count);
			for ( int i = 0; i < count; i ++ ) {
				IDispatchPtr  disp = 0;
				collection->item(i,/*, CComVariant(0),*/ &disp);
				// Check if object does  provide IHTMLElement interface  
				if ( CComQIPtr<IHTMLElement>(disp)) {
					res.SetValue(i, HtmlElement(new HtmlElementPrivate(disp)));
				}
			}
			return res;
		}
	protected:
		CComPtr<IHTMLElement> elem_;
		IDispatchPtr disp_;
		CComPtr<IElementSelector> selector_;
};

}
#endif