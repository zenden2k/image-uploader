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

#include "Core/FileDownloader.h"

#include <errno.h>
#include <algorithm>

#include "Func/WinUtils.h"
#include "Core/3rdpart/UriParser.h"

// TODO:
// 2. remove dependency from non-core headers ( "Common.h")
// 3. Use pimpl
CFileDownloader::CFileDownloader(std::shared_ptr<INetworkClientFactory> factory, const std::string& tempDirectory, bool createFilesBeforeDownloading)
    : tempDirectory_(tempDirectory), networkClientFactory_(factory), createFileBeforeDownloading_(createFilesBeforeDownloading)
{
    maxThreads_ = 3;
    stopSignal_ = false;
    isRunning_ = false;
    runningThreads_ = 0;
}

CFileDownloader::~CFileDownloader()
{
    waitForFinished();
}

void CFileDownloader::addFile(const std::string& url, void* id, const std::string& referer) {
    if (stopSignal_)
        return;  // Cannot add file to queue while stopping process
    std::lock_guard<std::mutex> guard(mutex_);

    DownloadFileListItem newItem;
    newItem.url = url;
    newItem.id = id;
    newItem.referer = referer;
    newItem.displayName = IuCoreUtils::ExtractFileNameFromUrl(url);
    fileList_.push_back(newItem);
}

void CFileDownloader::setThreadCount(int n)
{
    maxThreads_ = n;
}

bool CFileDownloader::start() {
    stopSignal_ = false;
    std::lock_guard<std::mutex> guard(mutex_);

    size_t numThreads = std::min(maxThreads_ - runningThreads_, int(fileList_.size()));
    for (size_t i = 0; i < numThreads; i++) {
        runningThreads_++;
        isRunning_ = true;
        threads_.push_back(std::thread(&CFileDownloader::memberThreadFunc, this));
    }

    return 0;
}

void CFileDownloader::memberThreadFunc()
{
    std::unique_ptr<INetworkClient> nm(networkClientFactory_->create());

    // Providing callback function to stop downloading
    nm->setProgressCallback(NetworkClient::ProgressCallback(this, &CFileDownloader::ProgressFunc));
    mutex_.lock();
    if (onConfigureNetworkClient)
        onConfigureNetworkClient(nm.get());
    mutex_.unlock();

    for (;; )
    {
        DownloadFileListItem curItem;
        if (!getNextJob(curItem))
            break;

        std::string url = curItem.url;
        if (url.empty())
            break;

        nm->setOutputFile(curItem.fileName);
        if ( !curItem.referer.empty() ) {
            nm->setReferer(curItem.referer);
        }
        try {
            nm->doGet(url);
        } catch (NetworkClient::AbortedException&) {
            break;
        }
        
        if (stopSignal_)
            break;

        mutex_.lock();
        bool success = nm->responseCode() >= 200 && nm->responseCode() <= 299;

        if (onFileFinished_) {
            onFileFinished_(success, nm->responseCode(), curItem);
        }

        if (stopSignal_)
            fileList_.clear();
        mutex_.unlock();
    }

    mutex_.lock();
    runningThreads_--;

    if (stopSignal_)
        fileList_.clear();
    mutex_.unlock();  // We need to release  mutex before calling  onQueueFinished()

    threadsStatusMutex_.lock();
    // otherwise we may get a deadlock
    if (!runningThreads_ && isRunning_){
        isRunning_ = false;
        stopSignal_ = false;
        if (onQueueFinished)
            onQueueFinished();
    }
    threadsStatusMutex_.unlock();
}

bool CFileDownloader::getNextJob(DownloadFileListItem& item)
{
    bool result = false;
    std::lock_guard<std::mutex> guard(mutex_);

    if (!fileList_.empty() && !stopSignal_)
    {
        item = *fileList_.begin();

        std::string url;
        url = item.url;
        fileList_.erase(fileList_.begin());

        uriparser::Uri uri(url);
        std::string path = uri.path();
        NetworkClient nc;
        std::string fileName = nc.urlDecode(IuCoreUtils::ExtractFileName(path));

        if (fileName.length() > 30) {
            fileName = fileName.substr(0, 30);
        }

        
        CString possiblePath = U2W(tempDirectory_) + Utf8ToWstring(fileName).c_str();
        if (possiblePath.GetLength() > MAX_PATH - 4) {
            possiblePath = possiblePath.Left(MAX_PATH - 4);
        }
        CString wFileName = WinUtils::GetUniqFileName(possiblePath);
        std::string filePath = WCstringToUtf8(wFileName);

        if (createFileBeforeDownloading_) {
            // Creating file
            FILE* f = _tfopen(wFileName, L"wb");
            if (f) {
                fclose(f);
            } else {
                LOG(ERROR) << "Unable to create file:" << std::endl << wFileName << std::endl
                    << "Error: " << strerror(errno);
                return false;
            }
        }
        item.fileName = filePath;
        result = true;
    }

    return result;
}

void CFileDownloader::stop()
{
    stopSignal_ = true;
}

bool CFileDownloader::isRunning()
{
    return isRunning_;
}

void CFileDownloader::setOnFileFinishedCallback(decltype(onFileFinished_) callback) {
    onFileFinished_ = std::move(callback);
}

bool CFileDownloader::waitForFinished()
{
    if (threads_.empty()) {
        return true;
    }
       
    for (auto& th : threads_) {
        if (th.joinable()) {
            th.join();
        }
    }
    return true;
}

int CFileDownloader::ProgressFunc(INetworkClient* userData, double dltotal, double dlnow, double ultotal, double ulnow) {
    if (stopSignal_) {
        return -1;
    }
       
    return 0;
}
