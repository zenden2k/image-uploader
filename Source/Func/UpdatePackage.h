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

#ifndef IU_FUNC_UPDATEPACKAGE_H
#define IU_FUNC_UPDATEPACKAGE_H

#pragma once

#include <string>
#include "atlheaders.h"
#include "Core/Utils/CoreTypes.h"
#include "Core/Network/NetworkClient.h"
#include "Core/Utils/SimpleXml.h"

struct CUpdateItem
{
        CString name;
        CString hash;
        CString saveTo;
        CString action;
        std::string flags;
};

class CUpdateInfo
{
    public:
        CUpdateInfo();
        bool LoadUpdateFromFile(const CString& filename);
        bool LoadUpdateFromBuffer(const CString& buffer);
        bool DoUpdate(const CUpdateInfo& newPackage);
        bool SaveToFile(const CString& filename);
        bool Parse(SimpleXml& xml);    
        bool CheckUpdates();
        const CString getHash();
        bool CanUpdate(const CUpdateInfo& newInfo);
        bool    operator<(const CUpdateInfo& p);

        bool isCoreUpdate() const;
        CString displayName() const;
        CString downloadUrl() const;
        CString fileName() const;
        CString packageName() const;
        CString readableText() const;
        CString updateUrl() const;
        int timeStamp() const;

        void setFileName(const CString & name);
    protected:
        CString m_ReadableText;
        CString m_PackageName, m_DownloadUrl, m_UpdateUrl, m_Hash;
        CString m_FileName;
        bool m_CoreUpdate;
        int m_TimeStamp;
        CString m_DisplayName;
        CString m_Buffer;
};

class CUpdateStatusCallback
{
    public:
        virtual void updateStatus(int packageIndex, const CString& status)=0;
};

class CUpdatePackage
{
    public:
        CUpdatePackage();
        bool LoadUpdateFromFile(const CString& filename);
        bool doUpdate();
        void setUpdateStatusCallback(CUpdateStatusCallback * callback);
        int updatedFileCount() const;
        int totalFileCount() const;
    protected:
        void setStatusText(const CString& text);

        SimpleXml m_xml;
        CString m_PackageName;
        CString m_PackageFolder;
        bool m_CoreUpdate;
        int m_TimeStamp;
        std::vector<CUpdateItem> m_entries;
        int m_nUpdatedFiles;
        int m_nTotalFiles;
        CUpdateStatusCallback* m_statusCallback;
    private:
        DISALLOW_COPY_AND_ASSIGN(CUpdatePackage);
};


class CUpdateManager: public CUpdateStatusCallback
{
    public:
        CUpdateManager();
        bool CheckUpdates();
        bool DoUpdates();
        const CString ErrorString();
        CString generateReport();
        void Clear();
        bool AreUpdatesAvailable();
        bool AreCoreUpdates();
        void setUpdateStatusCallback(CUpdateStatusCallback * callback);
        int successPackageUpdatesCount() const;
        void stop();
        std::vector<CUpdateInfo> m_updateList;
    protected:
        static int progressCallback(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow);
        void updateStatus(int packageIndex, const CString& status);
        bool internal_load_update(CString name);
        bool internal_do_update(CUpdateInfo& ui);

        CString m_ErrorStr;
        int nCurrentIndex;
        CUpdateStatusCallback *m_statusCallback;
        
        NetworkClient nm_;
        int m_nSuccessPackageUpdates;
        bool m_stop;
        int m_nCoreUpdates;
    private:
        DISALLOW_COPY_AND_ASSIGN(CUpdateManager);
};


#endif