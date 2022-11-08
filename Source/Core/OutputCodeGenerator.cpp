/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2018 Sergey Svistunov (zenden2k@gmail.com)

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

#include "OutputCodeGenerator.h"

#include "Core/3rdpart/inja/inja.hpp"
#include <nlohmann/json.hpp>
#include "Utils/CoreUtils.h"


OutputCodeGenerator::OutputCodeGenerator()
{
	m_PreferDirectLinks = true;
	m_lang = clPlain;
	m_CodeType = ctLinks;
}

void OutputCodeGenerator::setLang(CodeLang lang)
{
	m_lang = lang;
}

void OutputCodeGenerator::setType(CodeType type)
{
	m_CodeType = type;
}

std::string OutputCodeGenerator::generate(const std::vector<UploadResultItem>& items, const std::string& templateFilePath, bool shortened)
{
	if (!templateFilePath.empty()) {
		using namespace inja;
		json data;
		data["rowSize"] = rowSize_;
		std::string templateBody = IuCoreUtils::GetFileContents(templateFilePath);

		if (templateBody.empty()) {
			LOG(ERROR) << "Unable to load template " << templateFilePath;
		}
		json arr = nlohmann::json::array();
		int i = 0;
		for(const auto& item: items) {
			if (item.isNull()) {
			    continue;
			}
			std::string url;
			if (!item.getImageUrl(shortened).empty() && (m_PreferDirectLinks || item.getDownloadUrl(shortened).empty()))
				url = item.getImageUrl(shortened);
			else
				url = item.getDownloadUrl(shortened);

			arr[i++] = {
			    {"url", url },
				{"thumbUrl", item.getThumbUrl(shortened)},
				{"imageUrl", item.getImageUrl(shortened)},
				{"downloadUrl", item.getDownloadUrl(shortened)},
				{"filePath", item.localFilePath},
				{"fileName", item.displayFileName},
				{"fileNameWithoutExt", IuCoreUtils::ExtractFileNameNoExt(item.displayFileName)},
				{"rowStart", !(i % rowSize_)},
				{"rowEnd", !((i + 1) % rowSize_)},
				//{"index", 
			};
		}
		data["items"] = arr;
		try {
			return render(templateBody, data);
		} catch (const std::exception& ex) {
			LOG(ERROR) << "Error while rendering template " << templateFilePath << std::endl << ex.what();
		}
	} else {
		std::string result;
		for (size_t i = 0; i < items.size(); i++)
		{
			if (i) result += "\r\n";
			result += generateCodeForItem(items[i], i);
		}
		return result;
	}
	return {};
}

std::string OutputCodeGenerator::generateCodeForItem(const UploadResultItem& item, int index)
{
	if(m_lang == clPlain)
      return item.directUrl.empty()?item.viewUrl:item.directUrl;
	else
	{
      if((m_CodeType == ctClickableThumbnails || m_CodeType == ctTableOfThumbnails) && !item.thumbUrl.empty())
          return link(item.directUrl.empty() ? item.viewUrl : item.directUrl, image(item.thumbUrl));
      else if (m_CodeType == ctImages && !item.directUrl.empty())
         return image(item.directUrl);
      else
        // if (m_CodeType == ctLinks ||  item.directUrl.empty())
            return link(item.directUrl.empty()?item.viewUrl:item.directUrl, IuCoreUtils::ExtractFileName(item.displayFileName));
	}
   return "";
}

std::string OutputCodeGenerator::image(const std::string& url)
{
	if (m_lang == clBBCode)
		return "[img]" + url +"[/img]";
	else  return "<img border='0' src='" + url+ "'/>";
}

std::string OutputCodeGenerator::link(const std::string &url, const std::string &body)
{
	if (m_lang == clBBCode)
		return "[url=" + url +"]"+body+"[/url]";
	else  return "<a href='"+url+"'>"+body+"</a>";
}

void OutputCodeGenerator::setPreferDirectLinks(bool prefer)
{
	m_PreferDirectLinks = prefer;
}

void OutputCodeGenerator::setRowSize(unsigned int rowSize) {
	rowSize_ = rowSize;
}