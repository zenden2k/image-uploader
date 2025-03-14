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

#ifndef IU_CORE_HTMLELEMENT_H
#define IU_CORE_HTMLELEMENT_H 
#include "Core/Utils/CoreTypes.h"
#include "Core/Scripting/Squirrelnc.h"

namespace ScriptAPI {;

class HtmlElementPrivate;

/**
@since 1.3.1 build 4272.
*/
class HtmlElement {
public:
    HtmlElement();
    HtmlElement(HtmlElementPrivate* pr);

    std::string getAttribute(const std::string& name);
    void setAttribute(const std::string& name, const std::string& value);
    void removeAttribute(const std::string& name);
    std::string getId();
    void setId(const std::string& id);
    std::string getInnerHTML();
    void setInnerHTML(const std::string& html);
    std::string getInnerText();
    void setInnerText(const std::string& text);
    std::string getOuterHTML();
    void setOuterHTML(const std::string& html);
    std::string getOuterText();
    void setOuterText(const std::string& text);
    /**
    Set value of an input element.
    Note: support of <input type="file" /> in this function is considered experimental. 
    It works asynchronously. Do not call this function again until FileInputFilledCallback is called.
    */
    void setValue(const std::string& value);

    /**
    Get value of an input element.
    */
    std::string getValue();
    std::string getTagName();
    HtmlElement getParentElement();
    void scrollIntoView();
    void click();
    void insertHTML(const std::string& name, bool atEnd = false );
    void insertText(const std::string& name, bool atEnd = false );
    HtmlElement querySelector(const std::string& query);
    Sqrat::Array querySelectorAll(const std::string& query);
    Sqrat::Array getFormElements();
    bool submitForm();
    bool isNull();

    /**
    Return an array containg child HtmlElements.
    */
    Sqrat::Array getChildren();
    friend class HtmlDocumentPrivate;
protected:
    std::shared_ptr<HtmlElementPrivate> d_;
    bool checkNull(const char * func);
};

/* @cond PRIVATE */
void RegisterHtmlElementClass(Sqrat::SqratVM& vm);
/* @endcond */

}
#endif
