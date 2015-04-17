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


#ifndef IU_MY_ENGINE_LIST_H
#define IU_MY_ENGINE_LIST_H

#include <atlbase.h>
#include <atlapp.h>
#include <atlmisc.h>
#include "Core/UploadEngineList.h"
#include "Core/Upload/DefaultUploadEngine.h"

class CMyEngineList: public CUploadEngineList
{
	public:
		CMyEngineList();
		~CMyEngineList();
		CString m_ErrorStr;
		const CString ErrorStr();
		CUploadEngineData* byName(const CString &name);
		int GetUploadEngineIndex(const CString Name);
		/*CAbstractUploadEngine* getUploadEngine(CUploadEngineData* data, ServerSettingsStruct& serverSettings);
		CAbstractUploadEngine* getUploadEngine(std::string name, ServerSettingsStruct& serverSettings);
		CAbstractUploadEngine* getUploadEngine(int index, ServerSettingsStruct& serverSettings);*/
public:
		bool LoadFromFile(const CString& filename);
		/*bool DestroyCachedEngine(const std::string& name, const std::string& profileName);**/
		HICON CMyEngineList::getIconForServer(const std::string& name);
		CString CMyEngineList::getIconNameForServer(const std::string& name);
		static char DefaultServer[];
		static char RandomServer[];
	private:
		std::map<std::string, HICON> serverIcons_;
};
#endif