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

#ifndef IU_CORE_UPLOAD_FILEQUEUEUPLOADER_H
#define IU_CORE_UPLOAD_FILEQUEUEUPLOADER_H

#include <string>
#include <memory>

#include "Core/Utils/CoreTypes.h"
#include "Core/Upload/UploadEngine.h"
#include "UploadSession.h"

class IUploadErrorHandler;
class ScriptsManager;
class UploadEngineManager;
class UploadFilter;
class NetworkClient;
class INetworkClientFactory;
class FileQueueUploaderPrivate;

class CFileQueueUploader
{
    public:
        CFileQueueUploader(UploadEngineManager* uploadEngineManager, 
            ScriptsManager* scriptsManager, std::shared_ptr<IUploadErrorHandler> uploadErrorHandler, 
            std::shared_ptr<INetworkClientFactory> networkClientFactory, int maxThreads = 3);
        void addSingleTask(std::shared_ptr<UploadTask> uploadTask);
        void addSession(std::shared_ptr<UploadSession> uploadSession);
        void addTaskToQueue(std::shared_ptr<UploadTask> task);
        void insertTaskAfter(UploadTask* after, std::shared_ptr<UploadTask> task);
        bool removeTaskFromQueue(UploadTask* task);
        void removeSession(std::shared_ptr<UploadSession> uploadSession);
        void retrySession(std::shared_ptr<UploadSession> uploadSession);
        virtual ~CFileQueueUploader();
        bool IsRunning() const;
        void setMaxThreadCount(int threadCount);
        bool isSlotAvailableForServer(const std::string& serverName, int maxThreads);
        void addUploadFilter(UploadFilter* filter);
        void removeUploadFilter(UploadFilter* filter);
        int sessionCount();
        void stopSession(UploadSession* uploadSession);
        std::shared_ptr<UploadSession> session(int index);
        void setOnQueueFinishedCallback(std::function<void(CFileQueueUploader*)> cb);
        void setOnSessionAddedCallback(std::function<void(UploadSession*)> cb);
        void setOnTaskAddedCallback(std::function<void(UploadTask*)> cb);
        void setOnConfigureNetworkClient(std::function<void(CFileQueueUploader*, INetworkClient*)> cb);
        friend class FileQueueUploaderPrivate;
    private:
        DISALLOW_COPY_AND_ASSIGN(CFileQueueUploader);
        FileQueueUploaderPrivate* _impl;
    protected:
        virtual void sessionAdded(UploadSession* session);
        virtual void taskAdded(UploadTask* task);
        bool start();
        void stop();
};

#endif
