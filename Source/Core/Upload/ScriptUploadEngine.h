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

#include "Core/Scripting/Squirrelnc.h"

#include "CommonTypes.h"
#include "UploadEngine.h"
#include "Core/Utils/CoreTypes.h"
#include "Core/Upload/FolderList.h"
#include "Core/Scripting/Script.h"

extern const Utf8String IuNewFolderMark;

class CScriptUploadEngine : public CAbstractUploadEngine, 
                            public Script
{
	public:
		int doUpload(std::shared_ptr<UploadTask> task, CIUUploadParams& params);
	protected:
		bool needStop();
		Utf8String m_ErrorReason;
		Utf8String m_FileName;
		Utf8String m_displayFileName;
		LoginInfo li;
		int m_CurrentActionIndex;
		int m_nThumbWidth;
	
	public:
        CScriptUploadEngine(Utf8String pluginName, ServerSync* serverSync, ServerSettingsStruct* settings);
		~CScriptUploadEngine();
		void setNetworkClient(NetworkClient* nm);
		//bool load(Utf8String fileName, ServerSettingsStruct& params);
		int getFolderList(CFolderList &FolderList);
		int createFolder(CFolderItem &parent, CFolderItem &folder);
		int modifyFolder(CFolderItem &folder);
		int getAccessTypeList(std::vector<Utf8String> &list);
		int getServerParamList(std::map<Utf8String, Utf8String> &list);
		int doLogin();

		bool supportsSettings();
		bool supportsBeforehandAuthorization();
		Utf8String name();
		
		int RetryLimit();
		
		void stop() override;
	//Sqrat::Table m_Object; 		
	protected:
		void Log(ErrorInfo::MessageType mt, const std::string& error);
        virtual void PrintCallback(const std::string& output) override;
        bool preLoad() override;
        bool postLoad() override;
        void clearSqratError();
		CFolderList m_FolderList;
       
		Utf8String m_sName;
      
		DISALLOW_COPY_AND_ASSIGN(CScriptUploadEngine);
};



#endif