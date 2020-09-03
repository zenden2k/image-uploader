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

#ifndef _IU_ADVANCED_UPLOAD_ENGINE_H
#define _IU_ADVANCED_UPLOAD_ENGINE_H

#include <vector>
#include <string>
#include <memory>

#include "CommonTypes.h"
#include "UploadEngine.h"
#include "Core/Utils/CoreTypes.h"
#include "Core/Upload/FolderList.h"
#include "Core/Network/NetworkClient.h"

class CAdvancedUploadEngine : public CAbstractUploadEngine
{
    public:
        int processTask(std::shared_ptr<UploadTask> task, UploadParams& params) override;
        CAdvancedUploadEngine(ServerSync* serverSync, ServerSettingsStruct* settings, ErrorMessageCallback errorCallback);
        ~CAdvancedUploadEngine();
        void setNetworkClient(INetworkClient* nm) override;
        //bool load(std::string fileName, ServerSettingsStruct& params);
        virtual int getFolderList(CFolderList &FolderList)=0;
        virtual int createFolder(const CFolderItem &parent, CFolderItem &folder)=0;
        virtual int modifyFolder(CFolderItem &folder)=0;
        virtual int getAccessTypeList(std::vector<std::string> &list)=0;
        virtual int getServerParamList(std::map<std::string, std::string> &list)=0;
        virtual int doLogin()=0;
        virtual int doLogout()=0;
        virtual bool isAuthenticated() = 0;
        virtual bool supportsLogout() = 0;

        virtual bool supportsSettings();

        int RetryLimit() override;
        /**
        Beforehand authorization - obtain access token only once then use it for all requests (before upload)
        **/
        virtual bool supportsBeforehandAuthorization();
    protected:
        void log(ErrorInfo::MessageType mt, const std::string& error);
        CFolderList folderList_;
        std::string name_;
        std::string m_ErrorReason;
        std::string m_FileName;
        std::string m_displayFileName;
        LoginInfo li;
        int m_CurrentActionIndex;
        int m_nThumbWidth;
        DISALLOW_COPY_AND_ASSIGN(CAdvancedUploadEngine);
};

#endif