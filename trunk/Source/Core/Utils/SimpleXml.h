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


#ifndef IU_CORE_UTILS_SIMPLEXML_H
#define IU_CORE_UTILS_SIMPLEXML_H

#pragma once
#define TIXML_USE_STL
#include <tinyxml.h>
#include <vector>
#include "CoreUtils.h"

class ZSimpleXmlNode
{
	public:
		ZSimpleXmlNode();
		ZSimpleXmlNode(TiXmlElement *el);
		ZSimpleXmlNode operator[](const std::string& name);
		std::string Attribute(const std::string& name) const;
		bool GetAttribute(const std::string& name, std::string& value) const;
		int AttributeInt(const std::string& name) const;
		zint64 AttributeInt64(const std::string& name) const;
		bool AttributeBool(const std::string& name) const;
		std::string Name() const;
		std::string Text() const;

		// Write
		ZSimpleXmlNode CreateChild(const std::string& name);
		ZSimpleXmlNode GetChild(const std::string& name, bool create = true);
		void SetAttribute(const std::string& name, const std::string& value);
		void SetAttribute(const std::string& name, int value);
		void SetAttribute(const std::string& name, zint64 value);
		void SetAttributeBool(const std::string& name, bool value);
		void SetText(const std::string& value);

		bool IsNull() const;
		bool GetChilds(const std::string& name,std::vector<ZSimpleXmlNode> &out) const;
		bool GetAttributes(std::vector<std::string> &out) const;
		void DeleteChilds();
	private:
		TiXmlElement *m_el;
};

/* A simple tiny wrapper of TinyXML library which covers all xml needs of the app.
   Unlike TinyXML classes or some other wrappers I met over Internet, usage of this class doesn't imply
	pointer manipulations, exceptions handling or usage of iterators (I actually hate them)
*/

class ZSimpleXml
{
	public:
		ZSimpleXml();
		virtual ~ZSimpleXml();
		bool LoadFromFile(const std::string& fileName);
		bool LoadFromString(const std::string& string);
		bool SaveToFile(const std::string& fileName) const;
		ZSimpleXmlNode getRoot(const std::string& name, bool create = true);
	private:
		TiXmlDocument doc;
		TiXmlHandle *docHandle;
};

#endif