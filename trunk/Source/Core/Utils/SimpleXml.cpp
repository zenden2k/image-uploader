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

#ifndef TIXML_USE_STL
#define TIXML_USE_STL
#endif

#include "SimpleXml.h"
#include "Core/3rdpart/tinyxml.h"
#include "CoreUtils.h"

class ZSimpleXml_impl
{
	public:
		ZSimpleXml_impl()
		{
			//doc = 0;
			docHandle = 0;
		}	
		virtual ~ZSimpleXml_impl()
		{
			delete docHandle;
		}
		TiXmlDocument doc;
		TiXmlHandle *docHandle;
};

class ZSimpleXmlNode_impl
{
	public:
		ZSimpleXmlNode_impl()
		{
			m_el = 0;
		}
		TiXmlElement *m_el;
	protected:
	private:
};

ZSimpleXmlNode::ZSimpleXmlNode()
{
	impl_ = new ZSimpleXmlNode_impl();
	impl_->m_el = 0;
}

ZSimpleXmlNode::ZSimpleXmlNode(const ZSimpleXmlNode& node)
{
	impl_ = new ZSimpleXmlNode_impl();
	*this = node;
}
ZSimpleXmlNode::~ZSimpleXmlNode()
{
	delete impl_;
}

ZSimpleXmlNode::ZSimpleXmlNode(void *el)
{
	impl_ = new ZSimpleXmlNode_impl();
	impl_->m_el = reinterpret_cast<TiXmlElement*>(el);
}

ZSimpleXmlNode ZSimpleXmlNode::operator[](const std::string& name)
{
	TiXmlElement* rootElem = 0;
	if(impl_->m_el)
	{
		
		TiXmlNode* rootNode = impl_->m_el->FirstChild(name.c_str());
		if(rootNode)
			rootElem = rootNode->ToElement();
	}
	return rootElem;
}

ZSimpleXmlNode ZSimpleXmlNode::GetChild(const std::string& name, bool create )
{
	TiXmlElement* rootElem = 0;
	if(impl_->m_el)
	{
		
		TiXmlNode* rootNode = impl_->m_el->FirstChild(name.c_str());
		if(rootNode)
			rootElem = rootNode->ToElement();
		else if(create)
		{
			return CreateChild(name);
		}
	}
	return rootElem;
}

bool ZSimpleXmlNode::IsNull() const
{
	return (impl_->m_el==0);
}

bool ZSimpleXmlNode::GetChilds(const std::string& name, std::vector<ZSimpleXmlNode> &out) const
{
	TiXmlNode * child = 0;
	if(!impl_->m_el) return false;
	int count=0;
	while( child = impl_->m_el->IterateChildren(name.c_str(), child ) )
	{
		count++;
		TiXmlElement *el = child->ToElement();
		if(el) out.push_back(el);
	}
	return (count != 0);
}

void ZSimpleXmlNode::DeleteChilds()
{
	if(!impl_->m_el) return ;
	impl_->m_el->Clear();
}
std::string ZSimpleXmlNode::Attribute(const std::string& name) const
{
	std::string result;
	if(impl_->m_el)
		impl_->m_el->QueryStringAttribute(name.c_str(), &result);
	return result;
}

int ZSimpleXmlNode::AttributeInt(const std::string& name) const
{
	return atoi(Attribute(name).c_str());
}

bool ZSimpleXmlNode::AttributeBool(const std::string& name) const
{
	return atoi(Attribute(name).c_str())!=0;
}

zint64 ZSimpleXmlNode::AttributeInt64(const std::string& name) const
{
	return IuCoreUtils::stringTozint64(Attribute(name).c_str());
}

void ZSimpleXmlNode::SetAttribute(const std::string& name, const std::string& value)
{
	if(!impl_->m_el) return;
	impl_->m_el->SetAttribute(name, value);
}

void ZSimpleXmlNode::SetAttribute(const std::string& name, int value)
{
	if(!impl_->m_el) return;
	impl_->m_el->SetAttribute(name, value);
}

void ZSimpleXmlNode::SetAttribute(const std::string& name, zint64 value)
{
	std::string str = IuCoreUtils::zint64ToString(value);
	if(!impl_->m_el) return;
	impl_->m_el->SetAttribute(name, str);
}

void ZSimpleXmlNode::SetAttributeBool(const std::string& name, bool value)
{
	if(!impl_->m_el) return;
	impl_->m_el->SetAttribute(name, value? 1: 0);
}

std::string ZSimpleXmlNode::Name() const
{
	std::string result;
	if(impl_->m_el)
		result = impl_->m_el->ValueStr();
	else return "null";
	return result;
}

std::string ZSimpleXmlNode::Text() const
{
	std::string result;
	if(impl_->m_el)
	{
		const char * str =  impl_->m_el->GetText();
		if(str)
			result = str;
	}
	return result;
}

ZSimpleXmlNode ZSimpleXmlNode::CreateChild(const std::string& name)
{
	if(!impl_->m_el) return 0;
	return impl_->m_el->InsertEndChild(TiXmlElement(name))->ToElement();
}

ZSimpleXml::ZSimpleXml()
{
	impl_ = new ZSimpleXml_impl();
	impl_->docHandle = 0;
	TiXmlBase::SetCondenseWhiteSpace(false);
}

ZSimpleXmlNode& ZSimpleXmlNode::operator = (const ZSimpleXmlNode& node)
{
	impl_->m_el = node.impl_->m_el;
	return *this;
}
ZSimpleXml::~ZSimpleXml()
{
	delete impl_;
}

bool ZSimpleXml::LoadFromFile(const std::string& fileName)
{
	Utf8String text;
	if(!IuCoreUtils::ReadUtf8TextFile(fileName, text))
		return false;
	
	return LoadFromString(text);
}

bool ZSimpleXml::LoadFromString(const std::string& string)
{
	impl_->doc.Parse(string.c_str());
	impl_->docHandle = new TiXmlHandle(&impl_->doc);
	return true;
}
	
ZSimpleXmlNode ZSimpleXml::getRoot(const std::string& name,  bool create)
{
	if(impl_->docHandle)
	{
		TiXmlElement * root = impl_->docHandle->FirstChildElement().ToElement();
		if(root) return root;
	}
	else if(!create)
		return 0;
	else
	{
		impl_->docHandle = new TiXmlHandle(&impl_->doc);
	}
	/* We do not need to delete this objects later because tinyxml does free memory for us */
	TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "UTF-8", "" );
   impl_->doc.LinkEndChild( decl );
	TiXmlElement * el  = new TiXmlElement(name.c_str());
	impl_->doc.LinkEndChild(el);
	return el;
}

bool ZSimpleXml::SaveToFile(const std::string& fileName) const
{
	FILE * f = IuCoreUtils::fopen_utf8(fileName.c_str(), "wb");
	if(!f) return false;
	
	bool res =  impl_->doc.SaveFile(f);
	fclose(f);
	return res;
}

void ZSimpleXmlNode::SetText(const std::string& value)
{
	if(!impl_->m_el) return;
	impl_->m_el->Clear();
	TiXmlText* el = new TiXmlText(value);
	impl_->m_el->LinkEndChild(el);
}

bool ZSimpleXmlNode::GetAttribute(const std::string& name, std::string& value) const
{
	if(impl_->m_el)
		if(impl_->m_el->QueryStringAttribute(name.c_str(), &value) == TIXML_NO_ATTRIBUTE) return false;
	return true;
}

bool ZSimpleXmlNode::GetAttributes(std::vector<std::string> &out) const
{
	if(!impl_->m_el) return false;
	TiXmlAttribute* attr = impl_->m_el->FirstAttribute();
	while(attr!=0)
	{
		out.push_back(attr->Name());
		attr = attr->Next();
	}
	return true;
}

bool ZSimpleXmlNode::GetAttributeBool(const std::string& name, bool & value) const
{
   std::string v;
   if(impl_->m_el)
		if(impl_->m_el->QueryStringAttribute(name.c_str(), &v) == TIXML_NO_ATTRIBUTE) return false;
   value = AttributeBool(name);
	return true;
}

bool ZSimpleXmlNode::GetAttributeInt(const std::string& name, int & value) const
{
   std::string v;
   if(impl_->m_el)
		if(impl_->m_el->QueryStringAttribute(name.c_str(), &v) == TIXML_NO_ATTRIBUTE) return false;
	value = AttributeInt(name);
   return true;
}
