#ifndef IU_CORE_HTMLELEMENTPRIVATE_WIN_H
#define IU_CORE_HTMLELEMENTPRIVATE_WIN_H

#pragma once
#include <ComDef.h>
#include "COMUtils.h"
#include <Core/Utils/CoreUtils.h>

namespace ScriptAPI {

class HtmlElementPrivate {
	public:
		HtmlElementPrivate(IHTMLElement* elem) {
			elem_ = elem;
		}
		HtmlElementPrivate(IDispatchPtr disp) {
			disp_  = disp;
			elem_ = CComQIPtr<IHTMLElement,&IID_IHTMLElement>(disp);
		}


		const std::string getAttribute(const std::string& name)
		{
			CComVariant res;
			elem_->getAttribute(CComBSTR(IuCoreUtils::Utf8ToWstring(name).c_str()), 0, &res);
			return ComVariantToString(res);

		}

		void setAttribute(const std::string& name)
		{	
			elem_->setAttribute(CComBSTR(IuCoreUtils::Utf8ToWstring(name).c_str()), CComVariant(IuCoreUtils::Utf8ToWstring(name).c_str()));
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


		HtmlElement parentElement()
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
	protected:
		CComPtr<IHTMLElement> elem_;
		IDispatchPtr disp_;
};

}
#endif