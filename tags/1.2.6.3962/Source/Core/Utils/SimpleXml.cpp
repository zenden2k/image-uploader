#include "SimpleXml.h"
#include "CoreUtils.h"

ZSimpleXmlNode::ZSimpleXmlNode(TiXmlElement *el)
{
	m_el = el;
}

ZSimpleXmlNode ZSimpleXmlNode::operator[](const std::string name)
{
	TiXmlElement* rootElem = 0;
	if(m_el)
	{
		TiXmlNode* rootNode = m_el->FirstChild(name.c_str());
		if(rootNode)
			rootElem = rootNode->ToElement();
	}
	return rootElem;
}

bool ZSimpleXmlNode::GetChilds(const std::string name,std::vector<ZSimpleXmlNode> &out)
{
	TiXmlNode * child = 0;
	if(!m_el) return false;
	int count=0;
	while( child = m_el->IterateChildren(name.c_str(), child ) )
	{
		count++;
		TiXmlElement *el = child->ToElement();
		if(el) out.push_back(el);
	}
	return (count != 0);
}

std::string ZSimpleXmlNode::Attribute(const std::string name) const
{
	std::string result;
	if(m_el)
		m_el->QueryStringAttribute(name.c_str(), &result);
	return result;
}

int ZSimpleXmlNode::AttributeInt(const std::string name) const
{
	return atoi(Attribute(name).c_str());
}

bool ZSimpleXmlNode::AttributeBool(const std::string name) const
{
	return atoi(Attribute(name).c_str())!=0;
}

void ZSimpleXmlNode::SetAttribute(const std::string name, const std::string value)
{
	if(!m_el) return;
	m_el->SetAttribute(name, value);
}

void ZSimpleXmlNode::SetAttribute(const std::string name, int value)
{
	if(!m_el) return;
	m_el->SetAttribute(name, value);
}

void ZSimpleXmlNode::SetAttributeBool(const std::string name, bool value)
{
	if(!m_el) return;
	m_el->SetAttribute(name, value? 1: 0);
}

std::string ZSimpleXmlNode::Name()
{
	std::string result;
	if(m_el)
		result = m_el->ValueStr();
	else return "null";
	return result;
}

std::string ZSimpleXmlNode::Text()
{
	std::string result;
	if(m_el)
	{
		const char * str =  m_el->GetText();
		if(str)
			result = str;
	}
	return result;
}
#include <Windows.h>

ZSimpleXml::ZSimpleXml()
{
	docHandle = 0;
}

ZSimpleXml::~ZSimpleXml()
{
	delete docHandle;
}

bool ZSimpleXml::LoadFromFile(const std::string fileName)
{
	Utf8String text;
	if(!IuCoreUtils::ReadUtf8TextFile(fileName, text))
		return false;
	
	doc.Parse(text.c_str());
	docHandle = new TiXmlHandle(&doc);
	return true;
}
	
ZSimpleXmlNode ZSimpleXml::getRoot(const std::string name)
{
	if(docHandle)
	{
		TiXmlElement * root = docHandle->FirstChildElement().ToElement();
		if(root) return root;
	}
	else
	{
		docHandle = new TiXmlHandle(&doc);
	}
	TiXmlElement * el  = new TiXmlElement(name.c_str());
	doc.LinkEndChild(el);
	return el;
}

bool ZSimpleXml::SaveToFile(const std::string fileName)
{
	return doc.SaveFile(fileName.c_str());
}


