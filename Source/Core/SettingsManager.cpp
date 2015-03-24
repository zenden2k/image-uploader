/*
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2015 ZendeN <zenden2k@gmail.com>
	 
    HomePage:    http://zenden.ws/imageuploader

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "SettingsManager.h"

SettingsNode::SettingsNode()
{
	binded_value_ = 0;
}

SettingsManager::SettingsManager()
{
}

SettingsNode& SettingsManager::operator[](const std::string& name)
{
	return root_[name];
}

SettingsNode& SettingsNode::operator[](const std::string& name)
{
	if(childs_.count(name) == 0)
	{
		childs_[name] = new SettingsNode();
	}
	return *childs_[name];
}

void SettingsNode::saveToXmlNode(SimpleXmlNode parentNode, const std::string& name, bool isRoot) const
{
	int namelen = name.length();
	if(namelen>0 && name[0] == '@')
	{
		parentNode.SetAttribute(name.substr(1, namelen - 1), binded_value_->getValue());
	}
	else
	{
		SimpleXmlNode child = parentNode;
         
      if(!isRoot)
      {
         child = parentNode.GetChild(name);
		   if(binded_value_)
		      child.SetText(binded_value_->getValue());
      }
	
	for(std::map<std::string, SettingsNode*>::const_iterator it=childs_.begin(); it!=childs_.end(); ++it)
	{
		it->second->saveToXmlNode(child, it->first);
	}
	}
}

void SettingsNode::loadFromXmlNode(SimpleXmlNode parentNode, const std::string& name, bool isRoot)
{
	int namelen = name.length();
	if(namelen>0 && name[0] == '@')
	{
		std::string attribValue;
		if(parentNode.GetAttribute(name.substr(1, namelen - 1), attribValue))
			binded_value_->setValue(attribValue);
	}
	else
	{
		SimpleXmlNode child = parentNode;
      if(!isRoot)
      {
         child = parentNode.GetChild(name, false);
      }
		if(!child.IsNull())
		{
			if(binded_value_ )
				binded_value_->setValue(child.Text());
			for(std::map<std::string, SettingsNode*>::const_iterator it=childs_.begin(); it!=childs_.end(); ++it)
			{
				it->second->loadFromXmlNode(child, it->first);
			}
		}
	}
}

SettingsNode::~SettingsNode()
{
	delete binded_value_;
	for(std::map<std::string, SettingsNode*>::const_iterator it=childs_.begin(); it!=childs_.end(); ++it)
	{
		delete it->second;
	}
}

SettingsNode& SettingsManager::root()
{
	return root_;
}

void SettingsManager::saveToXmlNode(SimpleXmlNode parentNode) const
{
	root_.saveToXmlNode(parentNode, "Settings", true);
}

void SettingsManager::loadFromXmlNode(SimpleXmlNode parentNode)
{
	root_.loadFromXmlNode(parentNode, "Settings", true);
}