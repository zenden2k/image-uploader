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

#ifndef IU_CORE_FILEDOWNLOADER_H
#define IU_CORE_FILEDOWNLOADER_H

#include "atlheaders.h"
#include "3rdpart/thread.h"
#include "Core/3rdpart/FastDelegate.h"
#include "Core/Network/NetworkClient.h"
#include "Core/Utils/CoreTypes.h"
#include <memory>

class Object;
class CFileDownloader
{
    public:
        struct DownloadFileListItem
        {
            std::string fileName;
            std::string displayName;
            std::string url;
            std::string referer;
            void* id;
        };

        CFileDownloader();
        virtual ~CFileDownloader();
        void AddFile(const std::string& url,void* userData, const std::string& referer = std::string());
        bool start();
        bool waitForFinished(unsigned int msec = -1);
        void setThreadCount(int n);
        void stop();
        bool IsRunning();

        fastdelegate::FastDelegate0<> onQueueFinished;
        fastdelegate::FastDelegate1<NetworkClient*> onConfigureNetworkClient;
        fastdelegate::FastDelegate3<bool, int, DownloadFileListItem, bool> onFileFinished;
    protected:
        CString m_ErrorStr;
        CAutoCriticalSection m_CS;
        std::vector<DownloadFileListItem> m_fileList;
        int m_nThreadCount;
        int m_nRunningThreads;
        std::vector<HANDLE> m_hThreads;
        bool m_NeedStop;
        volatile bool m_IsRunning;
        static int ProgressFunc (void* userData, double dltotal, double dlnow, double ultotal, double ulnow);
        static unsigned int __stdcall thread_func(void* param);
        void memberThreadFunc();
        bool getNextJob(DownloadFileListItem& item);

    private:
        DISALLOW_COPY_AND_ASSIGN(CFileDownloader);
};

#endif
