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
 
#include "Base.h"
#include "Func/Settings.h"
#include "Core/Utils/CoreUtils.h"
#include "Func/LocalFileCache.h"

ZBase* ZBase::m_base = 0;

void ZBase::cleanUp()
{
	delete m_base;
}

ZBase::ZBase()
{
	m_hm.setHistoryFileName(Settings.SettingsFolder+"/History/","history");
}

ZBase* ZBase::get()
{
	if(m_base == 0) {
		m_base = new ZBase();
		atexit(&cleanUp);
	}
	return m_base;
}

CHistoryManager* ZBase::historyManager() 
{
	return &m_hm;
}


void ZBase::addToGlobalCache(const std::string fileName, const std::string url)
{
	LocalFileCache::instance().addFile(url, fileName);
	//m_UrlCache[url] = fileName;
}

std::string ZBase::getFromCache(const std::string url)
{
	return LocalFileCache::instance().get(url);

}