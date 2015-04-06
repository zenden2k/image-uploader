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
		int doUpload(UploadTask* task, CIUUploadParams &params);
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
		void setNetworkClient(NetworkClient* nm);
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
const std::string Impl_AskUserCaptcha(NetworkClient *nm, const std::string& url);
const std::string Impl_InputDialog(const std::string& text, const std::string& defaultValue);

#endif