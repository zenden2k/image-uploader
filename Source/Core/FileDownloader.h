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

#ifndef IU_CORE_FILEDOWNLOADER_H
#define IU_CORE_FILEDOWNLOADER_H

#include "atlheaders.h"

#include <mutex>
#include <atomic>
#include <thread>
#include <functional>

#include "Core/Network/INetworkClient.h"
#include "Core/Utils/CoreTypes.h"

class CFileDownloader
{
    public:
        struct DownloadFileListItem
        {
            std::string fileName;
            std::string displayName;
            std::string url;
            std::string referer;
            void* id; // pointer to user data
        };
    protected:
        std::function<bool(bool, int, const DownloadFileListItem&)> onFileFinished_;
        std::function<void()> onQueueFinished_;
        std::function<void(INetworkClient*)> onConfigureNetworkClient_;
    public:
        CFileDownloader(std::shared_ptr<INetworkClientFactory> factory, const std::string& tempDirectory, bool createFilesBeforeDownloading = true);
        virtual ~CFileDownloader();
        void addFile(const std::string& url, void* id, const std::string& referer = std::string());
        void start();
        bool waitForFinished();
        void setThreadCount(int n);
        void stop();
        bool isRunning() const;

        void setOnFileFinishedCallback(decltype(onFileFinished_) callback);
        void setOnQueueFinishedCallback(decltype(onQueueFinished_) callback);
        void setOnConfigureNetworkClientCallback(decltype(onConfigureNetworkClient_) callback);

    protected:
        CString errorStr_;
        std::mutex mutex_;
        std::string tempDirectory_;
        std::vector<DownloadFileListItem> fileList_;
        int maxThreads_;
        int runningThreads_;
        std::mutex threadsStatusMutex_;
        std::vector<std::thread> threads_;
        std::atomic<bool> stopSignal_;
        std::atomic<bool> isRunning_;
        std::shared_ptr<INetworkClientFactory> networkClientFactory_;
        bool createFileBeforeDownloading_;
        int ProgressFunc (INetworkClient* userData, double dltotal, double dlnow, double ultotal, double ulnow);
        void memberThreadFunc();
        bool getNextJob(DownloadFileListItem& item);

    private:
        DISALLOW_COPY_AND_ASSIGN(CFileDownloader);
};

#endif
