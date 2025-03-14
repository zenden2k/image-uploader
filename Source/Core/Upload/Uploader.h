/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2018 Sergey Svistunov (zenden2k@gmail.com)

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

#ifndef _UPLOADER_H_
#define _UPLOADER_H_

#include <functional>
#include <string>
#include <memory>

#include "Core/Utils/CoreTypes.h"
#include "Core/Network/NetworkClient.h"
#include "Core/Upload/UploadEngine.h"

class CUploader
{
    public:
        explicit CUploader(std::shared_ptr<INetworkClientFactory> networkClientFactory);
        ~CUploader();
        
        bool setUploadEngine(CAbstractUploadEngine* UploadEngine);
        CAbstractUploadEngine * getUploadEngine();
        
        bool UploadFile(const std::string & FileName, const std::string& displayFileName);
        bool Upload(std::shared_ptr<UploadTask> task);
        void stop();
        bool needStop();
        std::shared_ptr<UploadTask> currentTask() const;
       
        void setOnNeedStopCallback(std::function<bool()> cb);
        void setOnProgress(std::function<void(CUploader*, InfoProgress)> cb);
        void setOnStatusChanged(std::function<void(CUploader*, StatusType, int, std::string)> cb);
        void setOnDebugMessage(std::function<void(CUploader*, const std::string&, bool)> cb);
        void setOnErrorMessage(std::function<void(CUploader*, ErrorInfo)> cb);
        void setOnConfigureNetworkClient(std::function<void(CUploader*, INetworkClient*)> cb);

        void DebugMessage(const std::string& message, bool isServerResponseBody = false);
        void SetStatus(StatusType status, int param1=0, const std::string& param="");
        StatusType GetStatus() const;
        bool isFatalError() const;
    protected:
        InfoProgress m_PrInfo;
        int pluginProgressFunc(INetworkClient* userData, double dltotal, double dlnow, double ultotal, double ulnow);

        bool m_bShouldStop;
        StatusType m_CurrentStatus;
        std::string m_FileName;
        std::string m_displayFileName;
        std::string m_ErrorReason;
        bool isFatalServerError_;
        
        void Error(bool error, std::string message, ErrorType type = etOther, int retryIndex = -1, UploadTask* uploadTask = nullptr, const std::string& topLevelFileName = std::string());
        void ErrorMessage(const ErrorInfo&);
        std::unique_ptr<INetworkClient> m_NetworkClient;
        CAbstractUploadEngine *m_CurrentEngine;
        std::shared_ptr<UploadTask> currentTask_;
        // events
        std::function<bool()> onNeedStop_;
        std::function<void(CUploader*, InfoProgress)> onProgress_;
        std::function<void(CUploader*, StatusType, int, std::string)> onStatusChanged_;
        std::function<void(CUploader*, const std::string&, bool)> onDebugMessage_;
        std::function<void(CUploader*, ErrorInfo)> onErrorMessage_;
        std::function<void(CUploader*, INetworkClient*)> onConfigureNetworkClient_;
        void Cleanup();
    private:
        DISALLOW_COPY_AND_ASSIGN(CUploader);
};

#endif
