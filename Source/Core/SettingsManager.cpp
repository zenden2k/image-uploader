/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2015 Sergey Svistunov (zenden2k@gmail.com)

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