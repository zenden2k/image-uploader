/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2018 Sergey Svistunov (zenden2k@yandex.ru)

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
#include "Core/Network/NetworkClient.h"
#include "AdvancedUploadEngine.h"

class CScriptUploadEngine : public CAdvancedUploadEngine, 
                            public Script,
                            public NetworkClient::Logger
{
    public:
        int doUpload(std::shared_ptr<UploadTask> task, UploadParams& params) override;
        CScriptUploadEngine(std::string pluginName, ServerSync* serverSync, ServerSettingsStruct* settings);
        ~CScriptUploadEngine();
        void setNetworkClient(INetworkClient* nm) override;
        //bool load(std::string fileName, ServerSettingsStruct& params);
        virtual int getFolderList(CFolderList &FolderList) override;
        virtual int createFolder(const CFolderItem &parent, CFolderItem &folder) override;
        virtual int modifyFolder(CFolderItem &folder) override;
        virtual int getAccessTypeList(std::vector<std::string> &list) override;
        virtual int getServerParamList(std::map<std::string, std::string> &list) override;
        virtual int doLogin() override;

        virtual bool supportsSettings() override;
        /**
        Beforehand authorization - obtain access token only once then use it for all requests (before upload)
        Return true if there is "DoLogin" function in squirrel script
        **/
        virtual bool supportsBeforehandAuthorization() override;
        std::string name();

        // FIXME: not working
        virtual void stop() override;         
    protected:
        void Log(ErrorInfo::MessageType mt, const std::string& error);
        virtual void PrintCallback(const std::string& output) override;
        bool preLoad() override;
        bool postLoad() override;
        virtual void logNetworkError(bool error, const std::string & msg) override;
        CFolderList folderList_;
        std::string name_;
        bool needStop() override;
        std::string m_ErrorReason;
        std::string m_FileName;
        std::string m_displayFileName;
        LoginInfo li;
        DISALLOW_COPY_AND_ASSIGN(CScriptUploadEngine);
};
#endif