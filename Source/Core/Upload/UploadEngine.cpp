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

#include "UploadEngine.h"

#include <cstdlib>

#include "Core/Utils/StringUtils.h"
#include "Core/Upload/ServerSync.h"

CUploadEngineData::CUploadEngineData()
{
    SupportsFolders = false;
    UsingPlugin = false;
    Debug = false;
    SupportThumbnails = false;
    BeforehandAuthorization = false;
    NeedAuthorization = naNotAvailable;
    NeedPassword = true; 
    MaxFileSize = 0;
    RetryLimit = 0;
    MaxThreads = 0;
    TypeMask = 0;
}

bool CUploadEngineData::hasType(ServerType type) const
{
    return (TypeMask & type) == type;
}

CUploadEngineListBase::CUploadEngineListBase()
{
}

CUploadEngineData* CUploadEngineListBase::byIndex(size_t index) {
    if ( index < m_list.size() ) {
        return &m_list[index];
    } 
    return nullptr;
}

int CUploadEngineListBase::count() const
{
    return m_list.size();
}

CUploadEngineData* CUploadEngineListBase::byName(const std::string& name)
{
    for (size_t i = 0; i < m_list.size(); i++)
    {
        if (!IuStringUtils::stricmp(m_list[i].Name.c_str(), name.c_str()))
            return &m_list[i];
    }
    return nullptr;
}

CUploadEngineData*  CUploadEngineListBase::firstEngineOfType(CUploadEngineData::ServerType type) {
    for (size_t i = 0; i < m_list.size(); i++)
    {
        if ( m_list[i].hasType(type)) {
            return &m_list[i];
        }
    }
    return nullptr;
}

int CUploadEngineListBase::getRandomImageServer() const
{
    std::vector<int> m_suitableServers;
    for (size_t i = 0; i < m_list.size(); i++)
    {
        if (m_list[i].NeedAuthorization != CUploadEngineData::naObligatory && m_list[i].hasType(CUploadEngineData::TypeImageServer)) {
            m_suitableServers.push_back(i);
        }
    }
    if (m_suitableServers.empty()) {
        return -1;
    }
    return m_suitableServers[rand() % (m_suitableServers.size())];
}

int CUploadEngineListBase::getRandomFileServer() const
{
    std::vector<size_t> m_suitableServers;
    for (size_t i = 0; i < m_list.size(); i++)
    {
        if (m_list[i].NeedAuthorization != CUploadEngineData::naObligatory && m_list[i].hasType(CUploadEngineData::TypeFileServer)) {
            m_suitableServers.push_back(i);
        }
    }
    if (m_suitableServers.empty()) {
        return -1;
    }
    return m_suitableServers[rand() % m_suitableServers.size()];
}

int CUploadEngineListBase::getUploadEngineIndex(const std::string& Name) const
{
    for (size_t i = 0; i < m_list.size(); i++)
    {
        if (m_list[i].Name == Name)
            return i;
    }
    return -1;
}

std::vector<CUploadEngineData>::const_iterator CUploadEngineListBase::begin() const {
    return m_list.begin();
}
std::vector<CUploadEngineData>::const_iterator CUploadEngineListBase::end() const {
    return m_list.end();
}

/* CAbstractUploadEngine */

CAbstractUploadEngine::~CAbstractUploadEngine()
{
}

bool CAbstractUploadEngine::DebugMessage(const std::string& message, bool isServerResponseBody)
{
    if (onDebugMessage)
        onDebugMessage(message, isServerResponseBody);
    return true;
}

bool CAbstractUploadEngine::ErrorMessage(ErrorInfo ei)
{
    if (onErrorMessage) {
        onErrorMessage(ei);
    } else
    {
        (ei.messageType == ErrorInfo::mtError ? LOG(ERROR) : LOG(WARNING)) << ei.error;
    }
    return true;
}

void CAbstractUploadEngine::setServerSettings(ServerSettingsStruct* settings)
{
    m_ServersSettings = settings;
}

ServerSettingsStruct * CAbstractUploadEngine::serverSettings() const
{
    return m_ServersSettings;
}

bool CAbstractUploadEngine::needStop()
{
    if (m_bShouldStop)
        return m_bShouldStop;
    if (onNeedStop)
        m_bShouldStop = onNeedStop();  // delegate call
    return m_bShouldStop;
}

void CAbstractUploadEngine::SetStatus(StatusType status, const std::string& param)
{
    if (onStatusChanged)
        onStatusChanged(status, 0,  param);
}

void CAbstractUploadEngine::setNetworkClient(INetworkClient* nm)
{
    m_NetworkClient = nm;
    m_NetworkClient->setCurlShare(serverSync_->getCurlShare());
}

void CAbstractUploadEngine::setUploadData(CUploadEngineData* data)
{
    m_UploadData = data;
}

CAbstractUploadEngine::CAbstractUploadEngine(ServerSync* serverSync, ErrorMessageCallback errorCallback)
{
    m_bShouldStop = 0;
    m_NetworkClient = nullptr;
    m_UploadData = nullptr;
    currUploader_ = nullptr;
    serverSync_ = serverSync;
    m_ServersSettings = nullptr;
    onErrorMessage = errorCallback;
}


CUploadEngineData* CAbstractUploadEngine::getUploadData() const
{
    return m_UploadData;
}

void CAbstractUploadEngine::setServerSync(ServerSync* sync)
{
    serverSync_ = sync;
}

void CAbstractUploadEngine::setCurrentUploader(CUploader* uploader) {
    currUploader_ = uploader;
}

CUploader* CAbstractUploadEngine::currentUploader() const{
    return currUploader_;
}

void CAbstractUploadEngine::stop()
{
    m_bShouldStop = true;
    //serverSync_->stop();
}

ServerSync* CAbstractUploadEngine::serverSync() const
{
    return serverSync_;
}

const std::string CFolderItem::NewFolderMark = "_iu_create_folder_";