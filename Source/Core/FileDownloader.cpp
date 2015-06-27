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

#include "Core/FileDownloader.h"
#include <algorithm>

#include "Func/Common.h"
#include "Func/IuCommonFunctions.h"
#include "Func/WinUtils.h"

// TODO:
// 2. remove dependency from non-core headers ( "Common.h")
// 3. Use pimpl
CFileDownloader::CFileDownloader()
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

void CFileDownloader::AddFile(const std::string& url, void* id, const std::string& referer) {
    if (stopSignal_)
        return;  // Cannot add file to queue while stopping process
    mutex_.lock();

    DownloadFileListItem newItem;
    newItem.url = url;
    newItem.id = reinterpret_cast <void*>(id);
    newItem.referer = referer;
    m_fileList.push_back(newItem);
    mutex_.unlock();
}

void CFileDownloader::setThreadCount(int n)
{
    maxThreads_ = n;
}

bool CFileDownloader::start() {
    stopSignal_ = false;
    std::lock_guard<std::mutex> guard(mutex_);

    size_t numThreads = min(maxThreads_ - runningThreads_, int(m_fileList.size()));
    for (size_t i = 0; i < numThreads; i++) {
        runningThreads_++;
        isRunning_ = true;
        threads_.push_back(std::thread(&CFileDownloader::memberThreadFunc, this));
    }

    return 0;
}

void CFileDownloader::memberThreadFunc()
{
    NetworkClient nm;

    // Providing callback function to stop downloading
    nm.setProgressCallback(CFileDownloader::ProgressFunc, this);
    mutex_.lock();
    if (onConfigureNetworkClient)
        onConfigureNetworkClient(&nm);
    mutex_.unlock();

    for (;; )
    {
        DownloadFileListItem curItem;
        if (!getNextJob(curItem))
            break;

        std::string url = curItem.url;
        if (url.empty())
            break;

        nm.setOutputFile(curItem.fileName);
        if ( !curItem.referer.empty() ) {
            nm.setReferer(curItem.referer);
        }
        try {
            nm.doGet(url);
        } catch (NetworkClient::AbortedException&) {
            break;
        }
        
        if (stopSignal_)
            break;

        mutex_.lock();
        bool success = nm.responseCode() >= 200 && nm.responseCode() <= 299;

        if (!onFileFinished.empty()) {
            onFileFinished(success, nm.responseCode(), curItem);
        }

        if (stopSignal_)
            m_fileList.clear();
        mutex_.unlock();
    }

    mutex_.lock();
    runningThreads_--;

    if (stopSignal_)
        m_fileList.clear();
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
    mutex_.lock();
    if (!m_fileList.empty() && !stopSignal_)
    {
        item = *m_fileList.begin();

        std::string url;
        url = item.url;
        m_fileList.erase(m_fileList.begin());

        std::string ext = IuCoreUtils::ExtractFileExt(url);
        std::string fileName = IuCoreUtils::ExtractFileNameFromUrl(url);
        CString wFileName =
            GetUniqFileName(IuCommonFunctions::IUTempFolder + Utf8ToWstring(fileName.c_str()).c_str());
        std::string filePath = WCstringToUtf8(wFileName);

        // Creating file
        FILE* f = _tfopen(wFileName, L"wb");
        if (f)
            fclose(f);
        item.fileName = filePath;
        result = true;
    }

    mutex_.unlock();
    return result;
}

void CFileDownloader::stop()
{
    stopSignal_ = true;
}

bool CFileDownloader::IsRunning()
{
    return isRunning_;
}

bool CFileDownloader::waitForFinished()
{
    if (threads_.empty()) {
        return true;
    }
       
    for (auto& th : threads_) {
        th.join();
    }
    return true;
}

int CFileDownloader::ProgressFunc(void* userData, double dltotal, double dlnow,
                                  double ultotal,
                                  double ulnow)
{
    CFileDownloader* fd = reinterpret_cast<CFileDownloader*>(userData);
    if (fd->stopSignal_)
        return -1;
    return 0;
}
