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

#ifndef _IU_MEGANZ_UPLOAD_ENGINE_H
#define _IU_MEGANZ_UPLOAD_ENGINE_H


#include <vector>
#include <string>

#include "CommonTypes.h"
#include "UploadEngine.h"
#include "Core/Utils/CoreTypes.h"
#include "Core/Upload/FolderList.h"
#include "AdvancedUploadEngine.h"
#include "FileUploadTask.h"

#include <atomic>
class MyListener;
class MyGfxProcessor;
namespace mega
{
class MegaApi;
class MegaProxy;
}

class CMegaNzUploadEngine : public CAdvancedUploadEngine
                    
{
    public:
        int doUpload(std::shared_ptr<UploadTask> task, UploadParams& params) override;
        CMegaNzUploadEngine(ServerSync* serverSync, ServerSettingsStruct* settings);
        ~CMegaNzUploadEngine();
        //bool load(std::string fileName, ServerSettingsStruct& params);
        int getFolderList(CFolderList &FolderList) override;
        int createFolder(const CFolderItem &parent, CFolderItem &folder) override;
        int modifyFolder(CFolderItem &folder) override;
        int getAccessTypeList(std::vector<std::string> &list) override;
        int getServerParamList(std::map<std::string, std::string> &list) override;
        int doLogin() override;

        bool supportsSettings() override;
        /**
        Beforehand authorization - obtain access token only once then use it for all requests (before upload)
        **/
        virtual bool supportsBeforehandAuthorization() override;
        friend class MyListener;
    protected:
        DISALLOW_COPY_AND_ASSIGN(CMegaNzUploadEngine);

        

        std::unique_ptr<mega::MegaApi> megaApi_;
#ifdef _WIN32
        std::unique_ptr<MyGfxProcessor> proc_;
#endif
        std::unique_ptr<MyListener> listener_;
        CFolderList* folderList_;
        std::unique_ptr<mega::MegaProxy> proxy_;
        std::shared_ptr<FileUploadTask> fileTask_;
        std::atomic<bool> loginFinished_, loginSuccess_,
            fetchNodesFinished_, fetchNodesSuccess_, uploadFinished_, uploadSuccess_, exportFinished_, exportSuccess_,
            createFolderFinished_, createFolderSuccess_,renameFolderFinished_, renameFolderSuccess_;
        std::string publicLink_;
        bool ensureNodesFetched();

};

#endif