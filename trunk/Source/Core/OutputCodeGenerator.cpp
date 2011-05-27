/*
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2010 ZendeN <zenden2k@gmail.com>

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

#include "OutputCodeGenerator.h"
#include "Utils/CoreUtils.h"

ZOutputCodeGenerator::ZOutputCodeGenerator()
{
	m_PreferDirectLinks = true;
	m_lang = clPlain;
	m_CodeType = ctLinks;
}

void ZOutputCodeGenerator::setLang(CodeLang lang)
{
	m_lang = lang;
}

void ZOutputCodeGenerator::setType(CodeType type)
{
	m_CodeType = type;
}

std::string ZOutputCodeGenerator::generate(const std::vector<ZUploadObject>& items)
{
   std::string result ="";
	for(int i=0; i < items.size(); i++)
	{
		if(i) result += "\r\n";
		result += generateCodeForItem(items[i], i);
	}
	return result;
}

std::string ZOutputCodeGenerator::generateCodeForItem(const ZUploadObject& item, int index)
{
	if(m_lang == clPlain)
      return item.directUrl.empty()?item.viewUrl:item.directUrl;
	else
	{
      if((m_CodeType == ctClickableThumbnails || m_CodeType == ctTableOfThumbnails) && !item.thumbUrl.empty())
         return link(item.directUrl, image(item.thumbUrl));
      else if (m_CodeType == ctImages && !item.directUrl.empty())
         return image(item.directUrl);
      else
        // if (m_CodeType == ctLinks ||  item.directUrl.empty())
            return link(item.directUrl.empty()?item.viewUrl:item.directUrl, IuCoreUtils::ExtractFileName(item.displayFileName));
	}
   return "";
}

std::string ZOutputCodeGenerator::image(std::string url)
{
	if (m_lang == clBBCode)
		return "[img]" + url +"[/img]";
	else  return "<img border='0' src='" + url+ "'/>";
}

std::string ZOutputCodeGenerator::link(std::string url, std::string body)
{
	if (m_lang == clBBCode)
		return "[url=" + url +"]"+body+"[/url]";
	else  return "<a href='"+url+"'>"+body+"</a>";
}

void ZOutputCodeGenerator::setPreferDirectLinks(bool prefer)
{
	m_PreferDirectLinks = prefer;
}
