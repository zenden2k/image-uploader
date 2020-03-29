/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2018 Sergey Svistunov (zenden2k@yandex.ru)

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

#include <boost/format.hpp>

#ifndef TIXML_USE_STL
    #define TIXML_USE_STL
#endif

#include "SimpleXml.h"
#include "Core/3rdpart/tinyxml.h"
#include "CoreUtils.h"

class SimpleXml_impl
{
    public:
        SimpleXml_impl()
        {
            // doc = 0;
            docHandle = 0;
        }

        virtual ~SimpleXml_impl()
        {
            delete docHandle;
        }

        TiXmlDocument doc;
        TiXmlHandle* docHandle;
};

class SimpleXmlNode_impl
{
    public:
        SimpleXmlNode_impl()
        {
            m_el = 0;
        }

        TiXmlElement* m_el;

    protected:

    private:
};

SimpleXmlNode::SimpleXmlNode()
{
    impl_ = new SimpleXmlNode_impl();
    impl_->m_el = 0;
}

SimpleXmlNode::SimpleXmlNode(const SimpleXmlNode& node)
{
    impl_ = new SimpleXmlNode_impl();
    *this = node;
}

SimpleXmlNode::~SimpleXmlNode()
{
    delete impl_;
}

SimpleXmlNode::SimpleXmlNode(TiXmlElement* el)
{
    impl_ = new SimpleXmlNode_impl();
    impl_->m_el = el;
}

SimpleXmlNode SimpleXmlNode::operator[](const std::string& name)
{
    TiXmlElement* rootElem = 0;
    if (impl_->m_el)
    {
        TiXmlNode* rootNode = impl_->m_el->FirstChild(name.c_str());
        if (rootNode)
            rootElem = rootNode->ToElement();
    }
    return rootElem;
}

SimpleXmlNode SimpleXmlNode::GetChild(const std::string& name, bool create )
{
    TiXmlElement* rootElem = 0;
    if (impl_->m_el)
    {
        TiXmlNode* rootNode = impl_->m_el->FirstChild(name.c_str());
        if (rootNode)
            rootElem = rootNode->ToElement();
        else if (create)
        {
            return CreateChild(name);
        }
    }
    return rootElem;
}

SimpleXmlNode SimpleXmlNode::GetChildByIndex(int index)
{
    TiXmlElement* rootElem = 0;
    if (impl_->m_el)
    {
        TiXmlNode* rootNode = TiXmlHandle(impl_->m_el).Child(index).Element();
        if (rootNode)
            rootElem = rootNode->ToElement();
    }
    return rootElem;
}

int SimpleXmlNode::GetChildCount()
{
    int count = 0;
    TiXmlNode * child = 0;
    while ( (child = impl_->m_el->IterateChildren(child )) != 0 )
    {
        count++;
    }
    return count;
}

bool SimpleXmlNode::IsNull() const
{
    return (impl_->m_el == 0);
}

bool SimpleXmlNode::GetChilds(const std::string& name, std::vector<SimpleXmlNode>& out) const
{
    TiXmlNode* child = 0;
    if (!impl_->m_el)
        return false;
    int count = 0;
    while ( (child = impl_->m_el->IterateChildren(name.c_str(), child )) != 0 )
    {
        count++;
        TiXmlElement* el = child->ToElement();
        if (el)
            out.push_back(el);
    }
    return (count != 0);
}

void SimpleXmlNode::DeleteChilds()
{
    if (!impl_->m_el)
        return;
    impl_->m_el->Clear();
}

const std::string SimpleXmlNode::Attribute(const std::string& name) const
{
    std::string result;
    if (impl_->m_el)
        impl_->m_el->QueryStringAttribute(name.c_str(), &result);
    return result;
}

int SimpleXmlNode::AttributeInt(const std::string& name) const
{
    return atoi(Attribute(name).c_str());
}

bool SimpleXmlNode::AttributeBool(const std::string& name) const
{
    return atoi(Attribute(name).c_str()) != 0;
}

int64_t SimpleXmlNode::AttributeInt64(const std::string& name) const
{
    return IuCoreUtils::stringToInt64(Attribute(name).c_str());
}

void SimpleXmlNode::SetAttribute(const std::string& name, const std::string& value)
{
    if (!impl_->m_el)
        return;
    impl_->m_el->SetAttribute(name, value);
}

void SimpleXmlNode::SetAttribute(const std::string& name, int value)
{
    if (!impl_->m_el)
        return;
    impl_->m_el->SetAttribute(name, value);
}

void SimpleXmlNode::SetAttribute(const std::string& name, int64_t value)
{
    std::string str = IuCoreUtils::int64_tToString(value);
    if (!impl_->m_el)
        return;
    impl_->m_el->SetAttribute(name, str);
}

void SimpleXmlNode::SetAttributeInt(const std::string& name, int value)
{
    if (!impl_->m_el)
        return;
    impl_->m_el->SetAttribute(name, value);
}

void SimpleXmlNode::SetAttributeString(const std::string& name, const std::string& value)
{
    SetAttribute(name,value);
}

void SimpleXmlNode::SetAttributeBool(const std::string& name, bool value)
{
    if (!impl_->m_el)
        return;
    impl_->m_el->SetAttribute(name, value ? 1 : 0);
}

const std::string SimpleXmlNode::Name() const
{
    std::string result;
    if (impl_->m_el)
        result = impl_->m_el->ValueStr();
    else
        return "null";
    return result;
}

const std::string SimpleXmlNode::Text() const
{
    std::string result;
    if (impl_->m_el)
    {
        const char* str =  impl_->m_el->GetText();
        if (str)
            result = str;
    }
    return result;
}

SimpleXmlNode& SimpleXmlNode::each(std::function<bool(int, SimpleXmlNode&)> callback) {
    int i = 0;
    TiXmlNode * child = 0;
    while ((child = impl_->m_el->IterateChildren(child)) != 0) {
        TiXmlElement* el = child->ToElement();
        SimpleXmlNode node(el);
        bool res = callback(i, node);
        if (res ) {
            break;
        }
        i++;
    }
    return *this;
}

SimpleXmlNode SimpleXmlNode::CreateChild(const std::string& name)
{
    if (!impl_->m_el)
        return 0;
    return impl_->m_el->InsertEndChild(TiXmlElement(name))->ToElement();
}

SimpleXml::SimpleXml()
{
    impl_ = new SimpleXml_impl();
    impl_->docHandle = 0;
    TiXmlBase::SetCondenseWhiteSpace(false);
}

SimpleXmlNode& SimpleXmlNode::operator = (const SimpleXmlNode& node)
{
    impl_->m_el = node.impl_->m_el;
    return *this;
}

SimpleXml::~SimpleXml()
{
    delete impl_;
}

bool SimpleXml::LoadFromFile(const std::string& fileName)
{
    std::string text;
    if (!IuCoreUtils::ReadUtf8TextFile(fileName, text))
        return false;

    return LoadFromString(text);
}

bool SimpleXml::LoadFromString(const std::string& string)
{
    impl_->doc.Parse(string.c_str());
    impl_->docHandle = new TiXmlHandle(&impl_->doc);
    return true;
}

SimpleXmlNode SimpleXml::getRoot(const std::string& name,  bool create)
{
    if (impl_->docHandle)
    {
        TiXmlElement* root = impl_->docHandle->FirstChildElement().ToElement();
        if (root)
            return root;
    }
    else if (!create)
        return 0;
    else
    {
        impl_->docHandle = new TiXmlHandle(&impl_->doc);
    }
    /* We do not need to delete this objects later because tinyxml does free memory for us */
    TiXmlDeclaration* decl = new TiXmlDeclaration( "1.0", "UTF-8", "" );
    impl_->doc.LinkEndChild( decl );
    TiXmlElement* el  = new TiXmlElement(name.c_str());
    impl_->doc.LinkEndChild(el);
    return el;
}

const std::string SimpleXml::ToString()
{
    TiXmlPrinter printer;
    printer.SetIndent( "    " );
    impl_->doc.Accept( &printer );
    return printer.CStr();
}

bool SimpleXml::SaveToFile(const std::string& fileName) const
{
    FILE* f = IuCoreUtils::fopen_utf8(fileName.c_str(), "wb");
    if (!f) {
        LOG(ERROR) << boost::format("Could not save xml to file '%s'.") % fileName << std::endl << "Reason: " << strerror(errno);
        return false;
    }
    bool res =  impl_->doc.SaveFile(f);
    fclose(f);
    return res;
}

void SimpleXmlNode::SetText(const std::string& value)
{
    if (!impl_->m_el)
        return;
    impl_->m_el->Clear();
    TiXmlText* el = new TiXmlText(value);
    impl_->m_el->LinkEndChild(el);
}

bool SimpleXmlNode::GetAttribute(const std::string& name, std::string& value) const
{
    if (impl_->m_el)
        if (impl_->m_el->QueryStringAttribute(name.c_str(), &value) == TIXML_NO_ATTRIBUTE)
            return false;
    return true;
}

bool SimpleXmlNode::GetAttributes(std::vector<std::string>& out) const
{
    if (!impl_->m_el)
        return false;
    TiXmlAttribute* attr = impl_->m_el->FirstAttribute();
    while (attr != 0)
    {
        out.push_back(attr->Name());
        attr = attr->Next();
    }
    return true;
}

int SimpleXmlNode::GetAttributeCount()
{
    if (!impl_->m_el)
        return 0;
    int count = 0;
    TiXmlAttribute* attr = impl_->m_el->FirstAttribute();
    while (attr != 0)
    {
        count++;
        attr = attr->Next();
    }
    return count;
}

bool SimpleXmlNode::GetAttributeBool(const std::string& name, bool& value) const
{
    std::string v;
    if (impl_->m_el)
        if (impl_->m_el->QueryStringAttribute(name.c_str(), &v) == TIXML_NO_ATTRIBUTE)
            return false;
    value = AttributeBool(name);
    return true;
}

bool SimpleXmlNode::GetAttributeInt(const std::string& name, int& value) const
{
    std::string v;
    if (impl_->m_el)
        if (impl_->m_el->QueryStringAttribute(name.c_str(), &v) == TIXML_NO_ATTRIBUTE)
            return false;
    value = AttributeInt(name);
    return true;
}
