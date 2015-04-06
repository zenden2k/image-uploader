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

#include "HtmlDocument.h"
#include <Core/Squirrelnc.h>
#ifdef _WIN32
#include "HtmlDocumentPrivate_win.h"
#else
// Not implemented
#endif

using namespace ScriptAPI;
DECLARE_INSTANCE_TYPE(HtmlDocument);
DECLARE_INSTANCE_TYPE(HtmlElement);

namespace ScriptAPI {

HtmlDocument::HtmlDocument()
{
//	d_.reset(new HtmlDocumentPrivate());
}

HtmlDocument::HtmlDocument(HtmlDocumentPrivate* pr)
{
	d_.reset(pr);
}

HtmlElement HtmlDocument::rootElement()
{
	return d_->rootElement();
}

HtmlElement HtmlDocument::getElementById(const std::string& id)
{
	return d_->getElementById(id);
}

SquirrelObject HtmlDocument::getElementsByTagName(const std::string& tag)
{
	return d_->getElementsByTagName(tag);
}

SquirrelObject HtmlDocument::getElementsByName(const std::string& name)
{
	return d_->getElementsByName(name);
}

HtmlElement HtmlDocument::querySelector(const std::string& query)
{
	return d_->querySelector(query);
}

SquirrelObject HtmlDocument::querySelectorAll(const std::string& query)
{
	return d_->querySelectorAll(query);
}

const std::string HtmlDocument::getHTML()
{
	return d_->getHTML();
}

void RegisterHtmlDocumentClass()
{
	using namespace SqPlus;
	SQClassDef<HtmlDocument>("HtmlDocument")
		.func(&HtmlDocument::rootElement, "rootElement")
		.func(&HtmlDocument::getElementById, "getElementById")
		.func(&HtmlDocument::getElementsByName, "getElementsByName")
		.func(&HtmlDocument::getElementsByTagName, "getElementsByTagName")
		.func(&HtmlDocument::querySelector, "querySelector")
		.func(&HtmlDocument::querySelectorAll, "querySelectorAll")
		.func(&HtmlDocument::getHTML, "getHTML")
		;
}

void RegisterHtmlElementClass()
{
	using namespace SqPlus;
	SQClassDef<HtmlElement>("HtmlElement")
		.func(&HtmlElement::getAttribute, "getAttribute")
		.func(&HtmlElement::setAttribute, "setAttribute")
		.func(&HtmlElement::removeAttribute, "removeAttribute")
		.func(&HtmlElement::getId, "getId")
		.func(&HtmlElement::setId, "setId")
		.func(&HtmlElement::getInnerHTML, "getInnerHTML")
		.func(&HtmlElement::setInnerHTML, "setInnerHTML")
		.func(&HtmlElement::getInnerText, "getInnerText")
		.func(&HtmlElement::setInnerText, "setInnerText")
		.func(&HtmlElement::getOuterHTML, "getOuterHTML")
		.func(&HtmlElement::setOuterHTML, "setOuterHTML")
		.func(&HtmlElement::getOuterText, "getOuterText")
		.func(&HtmlElement::setOuterText, "setOuterText")
		.func(&HtmlElement::getTagName, "getTagName")
		.func(&HtmlElement::setValue, "setValue")
		.func(&HtmlElement::getFormElements, "getFormElements")
		.func(&HtmlElement::getParentElement, "getParentElement")
		.func(&HtmlElement::scrollIntoView, "scrollIntoView")
		.func(&HtmlElement::click, "click")
		.func(&HtmlElement::insertHTML, "insertHTML")
		.func(&HtmlElement::insertText, "insertText")
		.func(&HtmlElement::isNull, "isNull")
		.func(&HtmlElement::submitForm, "submitForm")
		.func(&HtmlElement::getChildren, "getChildren")
		;
}

}

