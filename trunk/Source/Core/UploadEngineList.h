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

#ifndef IU_CORE_UPLOADENGINELIST_H
#define IU_CORE_UPLOADENGINELIST_H

#pragma once

#include <string>
#include "Upload/UploadEngine.h"

class CUploadEngineList: public CUploadEngineList_Base
{
	public:
		CUploadEngineList();
		bool LoadFromFile(const std::string& filename,std::map <std::string, ServerSettingsStruct>&);
		void setNumOfRetries(int Engine, int Action);
		bool addServer(const CUploadEngineData& data);
	protected:
		int m_EngineNumOfRetries;
		int m_ActionNumOfRetries;	
	private:
		bool static compareEngines(const CUploadEngineData& elem1, const CUploadEngineData& elem2);
};

#endif // IU_CORE_UPLOADENGINELIST_H