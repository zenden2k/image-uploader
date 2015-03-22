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

#ifndef _IU_SCRIPT_UPLOAD_ENGINE_H
#define _IU_SCRIPT_UPLOAD_ENGINE_H

#include <vector>
#include <string>

#include "../Squirrelnc.h"

#include "CommonTypes.h"
#include "UploadEngine.h"
#include "Core/Utils/CoreTypes.h"

extern const Utf8String IuNewFolderMark;
class CFolderList
{
	public:
		std::vector<CFolderItem> m_folderItems;
		void Clear() {m_folderItems.clear();}
		const int GetCount() { return m_folderItems.size();}
		CFolderItem& operator [] (int index) {  return m_folderItems[index]; }
		void AddFolder(const std::string& title, const std::string& summary, const std::string& id, const std::string& parentid, int accessType);
		void AddFolderItem(const CFolderItem& item);
};

class CScriptUploadEngine: public CAbstractUploadEngine
{
	public:
		bool doUpload(UploadTask* task, CIUUploadParams &params);
	protected:
		bool needStop();
		Utf8String m_ErrorReason;
		Utf8String m_FileName;
		Utf8String m_displayFileName;
		LoginInfo li;
		int m_CurrentActionIndex;
		int m_nThumbWidth;
	
	public:
		CScriptUploadEngine(Utf8String pluginName);
		~CScriptUploadEngine();
		static void InitScriptEngine();
		static void DestroyScriptEngine();
		void FlushSquirrelOutput();
		void setNetworkManager(NetworkManager* nm);
		bool load(Utf8String fileName, ServerSettingsStruct& params);
		int getFolderList(CFolderList &FolderList);
		int  createFolder(CFolderItem &parent, CFolderItem &folder);
		int  modifyFolder(CFolderItem &folder);
		int getAccessTypeList(std::vector<Utf8String> &list);
		int getServerParamList(std::map<Utf8String, Utf8String> &list);
		int doLogin();
		bool isLoaded();
		bool supportsSettings();
		bool supportsBeforehandAuthorization();
		Utf8String name();
		time_t getCreationTime();
		int RetryLimit();
		
		SquirrelObject m_Object; 		
	protected:
		void Log(ErrorInfo::MessageType mt, const std::string& error);
		CFolderList m_FolderList;
		Utf8String m_sName;
		SquirrelObject m_SquirrelScript;
		time_t m_CreationTime;
		bool m_bIsPluginLoaded;
		DISALLOW_COPY_AND_ASSIGN(CScriptUploadEngine);
};

// You must implement this function
const std::string Impl_AskUserCaptcha(NetworkManager *nm, const std::string& url);
const std::string Impl_InputDialog(const std::string& text, const std::string& defaultValue);

#endif