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

class CScriptUploadEngine : public CAbstractUploadEngine, 
                            public Script
{
    public:
        int doUpload(std::shared_ptr<UploadTask> task, UploadParams& params) override;
        CScriptUploadEngine(std::string pluginName, ServerSync* serverSync, ServerSettingsStruct* settings);
        ~CScriptUploadEngine();
        void setNetworkClient(NetworkClient* nm) override;
        //bool load(std::string fileName, ServerSettingsStruct& params);
        int getFolderList(CFolderList &FolderList);
        int createFolder(const CFolderItem &parent, CFolderItem &folder);
        int modifyFolder(CFolderItem &folder);
        int getAccessTypeList(std::vector<std::string> &list);
        int getServerParamList(std::map<std::string, std::string> &list);
        int doLogin();

        bool supportsSettings();
        /**
        Beforehand authorization - obtain access token only once then use it for all requests (before upload)
        **/
        bool supportsBeforehandAuthorization();
        std::string name();
        
        int RetryLimit() override;

        // FIXME: not working
        virtual void stop() override;         
    protected:
        void Log(ErrorInfo::MessageType mt, const std::string& error);
        virtual void PrintCallback(const std::string& output) override;
        bool preLoad() override;
        bool postLoad() override;
        CFolderList folderList_;
        std::string name_;
        bool needStop();
        std::string m_ErrorReason;
        std::string m_FileName;
        std::string m_displayFileName;
        LoginInfo li;
        int m_CurrentActionIndex;
        int m_nThumbWidth;
        DISALLOW_COPY_AND_ASSIGN(CScriptUploadEngine);
};



#endif