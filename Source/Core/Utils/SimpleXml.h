/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2024 Sergey Svistunov (zenden2k@gmail.com)

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

#ifndef IU_CORE_UTILS_SIMPLEXML_H
#define IU_CORE_UTILS_SIMPLEXML_H

#pragma once

#include <vector>
#include <functional>
#include "CoreTypes.h"

namespace tinyxml2 {
class XMLElement;
}

namespace Sqrat{
class Array;
class Function;
}

class SimpleXml_impl;
class SimpleXmlNode_impl;
class SimpleXmlExtend;

/**

SimpleXmlNode class

*/
class SimpleXmlNode
{
    public:
        explicit SimpleXmlNode();
        /*! @cond PRIVATE */
        SimpleXmlNode(const SimpleXmlNode& node);
        /*! @endcond */
        virtual ~SimpleXmlNode();
        
        std::string Attribute(const std::string& name) const;
        /*! @cond PRIVATE */
        SimpleXmlNode operator[](const std::string& name);
        bool GetAttribute(const std::string& name, std::string& value) const;
        bool GetAttributeBool(const std::string& name, bool & value) const;
        bool GetAttributeInt(const std::string& name, int & value) const;
        /*! @endcond */

        int AttributeInt(const std::string& name) const;
        /*! @cond PRIVATE */
        int64_t AttributeInt64(const std::string& name) const;
        /*! @endcond */
        bool AttributeBool(const std::string& name) const;
        std::string Name() const;
        std::string Text() const;
       
        // Write
        SimpleXmlNode CreateChild(const std::string& name);
        SimpleXmlNode GetChild(const std::string& name, bool create = true);

        /**
        *  Slow function (it iterates over children)
         * @deprecated since 1.4.1
         */
        SimpleXmlNode GetChildByIndex(int index);

        /**
        *  Slow function (it iterates over children)
        */
        int GetChildCount();
        void SetAttribute(const std::string& name, const std::string& value);
        void SetAttributeString(const std::string& name, const std::string& value);
        void SetAttribute(const std::string& name, int value);
        void SetAttributeInt(const std::string& name, int value);
        void SetAttribute(const std::string& name, int64_t value);
        void SetAttributeBool(const std::string& name, bool value);
        void SetText(const std::string& value);

        bool IsNull() const;
        /*! @cond PRIVATE */
        bool GetChilds(const std::string& name,std::vector<SimpleXmlNode> &out) const;
        bool GetAttributes(std::vector<std::string> &out) const;
        SimpleXmlNode& each(std::function<bool(int, SimpleXmlNode&)> callback);

        /*! @endcond */
        int GetAttributeCount();
        void DeleteChilds();
        /*! @cond PRIVATE */
        SimpleXmlNode& operator = (const SimpleXmlNode& node);
        /*! @endcond */

        /**
        Iterate over childs
        @since 1.3.2

        Example:
        @include xml_each.nut
        */
        SimpleXmlNode& Each(Sqrat::Function callback); // this declaration is only for docs, should not be implemented

        /**
        Get children by tag name
        @since 1.3.2

        Example:
        @include xml_children.nut
        */
        Sqrat::Array GetChilds(const std::string& name);
    protected:
        SimpleXmlNode(tinyxml2::XMLElement *el);
    private:
        SimpleXmlNode_impl *impl_;
        friend class SimpleXml;
};

/**
   A simple tiny wrapper of TinyXML-2 library which covers all xml needs of the app.
   Unlike TinyXML classes or some other wrappers I met over Internet, usage of this class doesn't imply
    pointer manipulations, exceptions handling or usage of iterators (I actually hate them)
*/
class SimpleXml
{
    public:
        SimpleXml();
        virtual ~SimpleXml();
        bool LoadFromFile(const std::string& fileName);
        bool LoadFromString(const std::string& string);
        bool SaveToFile(const std::string& fileName) const;
        SimpleXmlNode getRoot(const std::string& name, bool create = true);
        std::string ToString();
    private:
        SimpleXml_impl *impl_;
        //DISALLOW_COPY_AND_ASSIGN(SimpleXml);
};

#endif
