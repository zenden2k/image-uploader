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

#include "Thumbnail.h"
#include "../../3rdpart/parser.h"
#include "../Utils/StringUtils.h"

Thumbnail::Thumbnail()
{
}

bool Thumbnail::LoadFromFile(const std::string& filename)
{
	ZSimpleXml xml;
	 if(!xml.LoadFromFile(filename))
		 return false;



	ZSimpleXmlNode root = xml.getRoot("Thumbnail", false);
	if(root.IsNull()) return false;
	data_.sprite_source_file = root["Definitions"]["Sprite"].Attribute("Source");
	
	if (!data_.sprite_source_file.empty() && IuCoreUtils::ExtractFilePath(data_.sprite_source_file).empty())
	{
		data_.sprite_source_file = IuCoreUtils::ExtractFilePath(filename) + "/"+ data_.sprite_source_file;
	}
	ZSimpleXmlNode colorsNode = root["Definitions"]["Params"];
	std::vector<ZSimpleXmlNode> colorNodes;
	colorsNode.GetChilds("Param", colorNodes);
	for(size_t i=0; i<colorNodes.size(); i++)
	{
		std::string colorName;
		colorName = colorNodes[i].Attribute("Name");
		data_.colors_[colorName] =  colorNodes[i].Attribute("Value");
	}
	ZSimpleXmlNode drawingNode = root["Drawing"];
	if(!drawingNode.IsNull())
	{
		data_.width_addition = drawingNode.Attribute("AddWidth");
		data_.height_addition = drawingNode.Attribute("AddHeight");	
		std::vector<ZSimpleXmlNode> drawOperations;
		drawingNode.GetChilds("Operation", drawOperations);
		for(size_t i=0; i<drawOperations.size(); i++)
		{
			ThumbnailDrawOperation op;
			op.brush = drawOperations[i].Attribute("Brush");
			op.rect = drawOperations[i].Attribute("Rect");
			op.source = drawOperations[i].Attribute("Source");
			op.destination = drawOperations[i].Attribute("Destination");
			op.source_rect = drawOperations[i].Attribute("SourceRect");
			op.type = drawOperations[i].Attribute("Type");
			op.pen = drawOperations[i].Attribute("Pen");
			op.condition = drawOperations[i].Attribute("Condition");
			op.text_colors = drawOperations[i].Attribute("TextColors");
			data_.drawing_operations_.push_back(op);
		}
	}
	file_name_ = filename;
	return true;
}

void Thumbnail::CreateNew()
{
	data_.colors_["FrameColor"] = "#004A6F";
	data_.colors_["StrokeColor"] = "0";
	data_.colors_["GradientColor1"] = "0";
	data_.colors_["GradientColor2"] = "0";
	data_.colors_["TextColor"] = "0";
}

bool Thumbnail::SaveToFile(const std::string& filename)
{
	ZSimpleXml xml;
	std::string fileToSave = filename;
	if(filename.empty())
	{
		fileToSave = file_name_;
	}
	xml.LoadFromFile(fileToSave);
	ZSimpleXmlNode root = xml.getRoot("Thumbnail", false);
	if(root.IsNull()) return false;
	ZSimpleXmlNode colorsNode = root.GetChild("Definitions").GetChild("Params");
	colorsNode.DeleteChilds();
	for(std::map<std::string,std::string>::iterator it=data_.colors_.begin(); it!=data_.colors_.end(); ++it)
	{
		ZSimpleXmlNode colorNode = colorsNode.CreateChild("Param");
		colorNode.SetAttribute("Name", it->first);
		colorNode.SetAttribute("Value", it->second);
	}
	//root.CreateChild("Drawing");
	xml.SaveToFile(fileToSave); 
	return true;
}

std::string Thumbnail::getSpriteFileName() const
{
	return data_.sprite_source_file;
}

std::string Thumbnail::getWidthAddition() const
{
	return data_.width_addition;
}

std::string Thumbnail::getHeightAddition() const
{
	return data_.height_addition;
}

const ThumbnailData* Thumbnail::getData() const
{
	return &data_;
}

unsigned int Thumbnail::getColor(const std::string& name)
{
	std::string res = data_.colors_[name];
	res = IuStringUtils::Replace(res, "#", "0x");
	//MessageBoxA(0, res.c_str(), name.c_str(), 0);
	return strtoul(res.c_str(),0,0);
}

unsigned int Thumbnail::getParam(const std::string& name)
{
	std::string res = data_.colors_[name];
	return atoi(res.c_str());
}

void Thumbnail::setColor(const std::string& name, unsigned int value)
{
	char buf[20];
	sprintf(buf, "#%06x", value);
	data_.colors_[name] = buf;
}

void Thumbnail::setParam(const std::string& name, unsigned int value)
{
	data_.colors_[name] = IuCoreUtils::toString(value);
}

void Thumbnail::setParamString(const std::string& name, const std::string& value)
{
	data_.colors_[name] = value;
}

std::string Thumbnail::getParamString(const std::string& name)
{
	return data_.colors_[name];
}