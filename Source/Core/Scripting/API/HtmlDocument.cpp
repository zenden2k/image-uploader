/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2018 Sergey Svistunov (zenden2k@gmail.com)

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
#include "Core/Scripting/Squirrelnc.h"
#ifdef _WIN32
#include "HtmlDocumentPrivate_win.h"
#else
// Not implemented
#endif

namespace ScriptAPI {

HtmlDocument::HtmlDocument()
{
//    d_.reset(new HtmlDocumentPrivate());
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

Sqrat::Array HtmlDocument::getElementsByTagName(const std::string& tag)
{
    return d_->getElementsByTagName(tag);
}

Sqrat::Array HtmlDocument::getElementsByName(const std::string& name)
{
    return d_->getElementsByName(name);
}

HtmlElement HtmlDocument::querySelector(const std::string& query)
{
    return d_->querySelector(query);
}

Sqrat::Array HtmlDocument::querySelectorAll(const std::string& query)
{
    return d_->querySelectorAll(query);
}

std::string HtmlDocument::getHTML()
{
    return d_->getHTML();
}

void RegisterHtmlDocumentClass(Sqrat::SqratVM& vm)
{
    using namespace Sqrat;
    vm.GetRootTable().Bind("HtmlDocument", Class<HtmlDocument>(vm.GetVM(), "HtmlDocument")
        .Func("rootElement", &HtmlDocument::rootElement)
        .Func("getElementById", &HtmlDocument::getElementById)
        .Func("getElementsByName", &HtmlDocument::getElementsByName)
        .Func("getElementsByTagName", &HtmlDocument::getElementsByTagName)
        .Func("querySelector", &HtmlDocument::querySelector)
        .Func("querySelectorAll", &HtmlDocument::querySelectorAll)
        .Func("getHTML", &HtmlDocument::getHTML)
    );
}

void RegisterHtmlElementClass(Sqrat::SqratVM& vm)
{
    using namespace Sqrat;
    vm.GetRootTable().Bind("HtmlElement", Class<HtmlElement>(vm.GetVM(), "HtmlElement")
        .Func("getAttribute", &HtmlElement::getAttribute)
        .Func("setAttribute", &HtmlElement::setAttribute)
        .Func("removeAttribute", &HtmlElement::removeAttribute)
        .Func("getId", &HtmlElement::getId)
        .Func("setId", &HtmlElement::setId)
        .Func("getInnerHTML", &HtmlElement::getInnerHTML)
        .Func("setInnerHTML", &HtmlElement::setInnerHTML)
        .Func("getInnerText", &HtmlElement::getInnerText)
        .Func("setInnerText", &HtmlElement::setInnerText)
        .Func("getOuterHTML", &HtmlElement::getOuterHTML)
        .Func("setOuterHTML", &HtmlElement::setOuterHTML)
        .Func("getOuterText", &HtmlElement::getOuterText)
        .Func("setOuterText", &HtmlElement::setOuterText)
        .Func("getTagName", &HtmlElement::getTagName)
        .Func("setValue", &HtmlElement::setValue)
        .Func("getFormElements", &HtmlElement::getFormElements)
        .Func("getParentElement", &HtmlElement::getParentElement)
        .Func("scrollIntoView", &HtmlElement::scrollIntoView)
        .Func("click", &HtmlElement::click)
        .Func("insertHTML", &HtmlElement::insertHTML)
        .Func("insertText", &HtmlElement::insertText)
        .Func("isNull", &HtmlElement::isNull)
        .Func("submitForm", &HtmlElement::submitForm)
        .Func("getChildren", &HtmlElement::getChildren)
    );
}

}

