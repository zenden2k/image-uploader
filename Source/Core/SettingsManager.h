/*
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2011 ZendeN <zenden2k@gmail.com>
	 
    HomePage:    http://zenden.ws/imageuploader

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
		virtual void setValue(const std::string)=0;
		virtual ~SettingsNodeBase(){};
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

template<class T> class SettingsNodeVariant: public SettingsNodeBase
{
	private:
		T* value_;
		bool empty_;
	public:
		SettingsNodeVariant(T* value)
		{
			value_ = value;
		}

		virtual std::string getValue()
		{
			return 
				myToString(*value_);
		}
		virtual void setValue(const std::string text)
		{
			myFromString(text, *value_ );
		}
		virtual ~SettingsNodeVariant()
		{
		}
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
		void saveToXmlNode(ZSimpleXmlNode parentNode, const std::string& name, bool isRoot = false) const;
		void loadFromXmlNode(ZSimpleXmlNode parentNode, const std::string& name, bool isRoot = false);
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
		void saveToXmlNode(ZSimpleXmlNode parentNode) const;
		void loadFromXmlNode(ZSimpleXmlNode parentNode);
	protected:
		SettingsNode root_;
		DISALLOW_COPY_AND_ASSIGN(SettingsManager);
};
#endif