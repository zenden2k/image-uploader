#include "HtmlElement.h"

#include <Core/Squirrelnc.h>

#ifdef _WIN32
#include "HtmlElementPrivate_win.h"
#else
// Not implemented
#endif

using namespace ScriptAPI;


namespace ScriptAPI {

HtmlElement::HtmlElement()
{
	//d_.reset(new HtmlElementPrivate());
}

HtmlElement::HtmlElement(HtmlElementPrivate* pr)
{
	d_.reset(pr);
}



const std::string HtmlElement::getAttribute(const std::string& name)
{
	if ( !checkNull("getAttribute") ) {
		return std::string();
	}
	return d_->getAttribute(name);
}

void HtmlElement::setAttribute(const std::string& name)
{
	if ( !checkNull("setAttribute") ) {
		return;
	}
	d_->setAttribute(name);
}

void HtmlElement::removeAttribute(const std::string& name)
{	
	if ( !checkNull("removeAttribute") ) {
		return;
	}
	d_->removeAttribute(name);
}

const std::string HtmlElement::getId()
{
	if ( !checkNull("getId") ) {
		return std::string();
	}
	return d_->getId();
}

void HtmlElement::setId(const std::string& id)
{
	if ( !checkNull("setId") ) {
		return;
	}
	d_->setId(id);
}

const std::string HtmlElement::getInnerHTML()
{
	if ( !checkNull("getInnerHTML") ) {
		return std::string();
	}
	return d_->getInnerHTML();
}

void HtmlElement::setInnerHTML(const std::string& html)
{
	if ( !checkNull("setInnerHTML") ) {
		return;
	}
	d_->setInnerHTML(html);
}

const std::string HtmlElement::getInnerText()
{
	if ( !checkNull("getInnerText") ) {
		return std::string();
	}
	return d_->getInnerText();
}

void HtmlElement::setInnerText(const std::string& text)
{
	if ( !checkNull("setInnerText") ) {
		return;
	}
	d_->setInnerText(text);
}

const std::string HtmlElement::getOuterHTML()
{
	if ( !checkNull("getOuterHTML") ) {
		return std::string();
	}
	return d_->getOuterHTML();
}

void HtmlElement::setOuterHTML(const std::string& html)
{
	if ( !checkNull("setOuterHTML") ) {
		return;
	}
	d_->setOuterHTML(html);
}

const std::string HtmlElement::getOuterText()
{
	if ( !checkNull("getOuterText") ) {
		return std::string();
	}
	return d_->getOuterText();
}

void HtmlElement::setOuterText(const std::string& text)
{
	if ( !checkNull("setOuterText") ) {
		return;
	}
	d_->setOuterText(text);
}

const std::string HtmlElement::getTagName()
{
	if ( !checkNull("getTagName") ) {
		return std::string();
	}
	return d_->getTagName();
}


ScriptAPI::HtmlElement HtmlElement::parentElement()
{
	if ( !checkNull("parentElement") ) {
		return HtmlElement();
	}
	return d_->parentElement();
}

void HtmlElement::scrollIntoView()
{
	if ( !checkNull("scrollIntoView") ) {
		return;
	}
	d_->scrollIntoView();
}

void HtmlElement::click()
{
	if ( !checkNull("click") ) {
		return;
	}
	d_->click();
}

void HtmlElement::insertHTML(const std::string& name, bool atEnd /*= false */)
{
	if ( !checkNull("insertHTML") ) {
		return;
	}
	d_->insertHTML(name, atEnd /*= false */);
}

void HtmlElement::insertText(const std::string& name, bool atEnd /*= false */)
{
	if ( !checkNull("insertText") ) {
		return;
	}
	d_->insertText(name, atEnd /*= false */);
}

bool HtmlElement::isNull()
{
	return !d_;
}

bool HtmlElement::checkNull(const char * func)
{
	if ( isNull() ) {
		LOG(ERROR) << func << " : " << "HtmlElement is null";
		return false;
	}
	return true;
}

}

