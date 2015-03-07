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
		CAbstractUploadEngine* getUploadEngine(CUploadEngineData* data, ServerSettingsStruct& serverSettings);
		CAbstractUploadEngine* getUploadEngine(std::string name, ServerSettingsStruct& serverSettings);
		CAbstractUploadEngine* getUploadEngine(int index, ServerSettingsStruct& serverSettings);
public:
		bool LoadFromFile(const CString& filename);
		bool DestroyCachedEngine(const std::string& name, const std::string& profileName);
		HICON CMyEngineList::getIconForServer(const std::string& name);
		CString CMyEngineList::getIconNameForServer(const std::string& name);
		static char DefaultServer[];
		static char RandomServer[];
	private:
		CDefaultUploadEngine * m_prevUpEngine;
		std::map<std::string, HICON> serverIcons_;
};
#endif