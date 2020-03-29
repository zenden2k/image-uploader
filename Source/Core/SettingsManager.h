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

#ifndef IU_CORE_SETTINGSMANAGER_H
#define IU_CORE_SETTINGSMANAGER_H

#include <string>
#include <map>
#include <iostream>
#include <sstream>
#include "Core/Utils/CoreTypes.h"
#include "Core/Utils/SimpleXml.h"

#define n_bind(a) operator[]( #a ).bind(a)
#define nm_bind(b,a) operator[]( #a ).bind(b.a)

class SettingsNodeBase
{
    public:
        virtual std::string getValue()=0;
        virtual void setValue(const std::string&)=0;
        virtual ~SettingsNodeBase() = default;
};

template<class T> std::string myToString(const T& value)
{
        std::stringstream str;
        str << value;
        return str.str();
}


template<class T> void myFromString(const std::string& text, T & value)
{
   std::stringstream str(text);
   str >> value;
}

template<class T, class T2> void myFromString(const std::string& text, T & value)
{
   std::stringstream str(text);
   str >> value;
}


inline void myFromString(const std::string& text, std::string & value)
{
    value = text;
}

template<class T> class SettingsNodeVariant: public SettingsNodeBase
{
    private:
        T* value_;
    public:
        explicit SettingsNodeVariant(T* value)
        {
            value_ = value;
        }

        std::string getValue() override {
            return myToString(*value_);
        }

        void setValue(const std::string& text) override {
            myFromString(text, *value_ );
        }

        virtual ~SettingsNodeVariant() = default;
};

class SettingsNode
{
    public:
        SettingsNode();
        virtual ~SettingsNode();
        template<class T> void bind(T& var)
        {
            delete binded_value_;
            binded_value_ = new SettingsNodeVariant<T>(&var);
        }
        SettingsNode& operator[](const std::string&);
        void saveToXmlNode(SimpleXmlNode parentNode, const std::string& name, bool isRoot = false) const;
        void loadFromXmlNode(SimpleXmlNode parentNode, const std::string& name, bool isRoot = false);
    protected:
        SettingsNodeBase * binded_value_;
        std::map<std::string, SettingsNode*> childs_; 
        DISALLOW_COPY_AND_ASSIGN(SettingsNode);
};

class SettingsManager
{
    public:
        SettingsManager();
        SettingsNode& operator[](const std::string&);
        SettingsNode& root();
        void saveToXmlNode(SimpleXmlNode parentNode) const;
        void loadFromXmlNode(SimpleXmlNode parentNode);
    protected:
        SettingsNode root_;
        DISALLOW_COPY_AND_ASSIGN(SettingsManager);
};
#endif