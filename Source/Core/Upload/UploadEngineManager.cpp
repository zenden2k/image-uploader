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

#include "UploadEngineManager.h"

#include "../UploadEngineList.h"
#include "ServerProfile.h"
#include "ScriptUploadEngine.h"
#include "DefaultUploadEngine.h"
#include "Core/Logging.h"
#include "ServerSync.h"
#include "Core/Settings.h"
#include "Core/Upload/UploadErrorHandler.h"
#include "MegaNzUploadEngine.h"

UploadEngineManager::UploadEngineManager(CUploadEngineList* uploadEngineList, IUploadErrorHandler* uploadErrorHandler)
{
    uploadEngineList_ = uploadEngineList;
    uploadErrorHandler_ = uploadErrorHandler;
}

UploadEngineManager::~UploadEngineManager()
{
    unloadUploadEngines();
    for (auto sync : serverSyncs_)
    {
        delete sync.second;
    }
    serverSyncs_.clear();
}

CAbstractUploadEngine* UploadEngineManager::getUploadEngine(ServerProfile &serverProfile)
{
    if (serverProfile.serverName().empty())
    {
        LOG(ERROR) << "UploadEngineManager::getUploadEngine" << " empty server name";
        return 0;
    }
    CUploadEngineData *ue = uploadEngineList_->byName(serverProfile.serverName());
    if (!ue)
    {
        LOG(ERROR) << "No such server " << serverProfile.serverName();
        return 0;
    }
    CAbstractUploadEngine* result = NULL;
    std::string serverName = serverProfile.serverName();
    std::thread::id threadId = std::this_thread::get_id();
    ServerSettingsStruct& params = serverProfile.serverSettings();
    
    if (ue->UsingPlugin) {
        result = getPlugin(serverProfile, ue->PluginName);
        if (!result) {
            LOG(ERROR) << "Cannot load plugin '" << ue->PluginName << "'";
            return NULL;
        }
        
    } else {
        std::lock_guard<std::mutex> guard(pluginsMutex_);
        CAbstractUploadEngine* plugin = m_plugins[threadId][serverName];
        if (plugin &&  plugin->serverSettings()->authData.Login == params.authData.Login) {
            return plugin;
        }
        /*if (m_prevUpEngine) {
            if (m_prevUpEngine->getUploadData()->Name == data->Name &&
                m_prevUpEngine->serverSettings().authData.Login == serverSettings.authData.Login

                )
                result = m_prevUpEngine;
            else
            {
                delete m_prevUpEngine;
                m_prevUpEngine = 0;
            }
        }
        if (!m_prevUpEngine)*/
        delete plugin;
        ServerSync* serverSync = getServerSync(serverProfile);
        if (ue->Engine == "MegaNz") {
            result = new CMegaNzUploadEngine(serverSync, &serverProfile.serverSettings());
        } else {
            result = new CDefaultUploadEngine(serverSync);
        }
        result->setServerSettings(&serverProfile.serverSettings());
        result->setUploadData(ue);
        result->onErrorMessage.bind(uploadErrorHandler_, &IUploadErrorHandler::ErrorMessage);
        
        m_plugins[threadId][serverName] = result;
    }
    
    result->setServerSettings(&serverProfile.serverSettings());
    result->setUploadData(ue);
    result->onErrorMessage.bind(uploadErrorHandler_, &IUploadErrorHandler::ErrorMessage);
    return result;
}

CScriptUploadEngine* UploadEngineManager::getScriptUploadEngine(ServerProfile& serverProfile)
{
    return dynamic_cast<CScriptUploadEngine*>(getUploadEngine(serverProfile));
}

CScriptUploadEngine* UploadEngineManager::getPlugin(ServerProfile& serverProfile, const std::string& pluginName, bool UseExisting) {
    std::lock_guard<std::mutex> lock(pluginsMutex_);
    std::string serverName = serverProfile.serverName();
    ServerSettingsStruct& params = serverProfile.serverSettings();
    std::thread::id threadId = std::this_thread::get_id();
    CScriptUploadEngine* plugin = dynamic_cast<CScriptUploadEngine*>(m_plugins[threadId][serverName]);
    if (plugin && (time(0)- plugin->getCreationTime() <(Settings.DeveloperMode ? 3000 : 1000 * 60 * 5)))
        UseExisting = true;

    if (plugin && UseExisting && plugin->name() == pluginName && plugin->serverSettings()->authData.Login == params.authData.Login) {
        plugin->onErrorMessage.bind(uploadErrorHandler_, &IUploadErrorHandler::ErrorMessage);
        plugin->switchToThisVM();
        return plugin;
    }

    if (plugin) {
        delete plugin;
        plugin = 0;
        m_plugins[threadId][serverName] = 0;
    }
    ServerSync* serverSync = getServerSync(serverProfile);
    std::string fileName = scriptsDirectory_ + pluginName + ".nut";
    CScriptUploadEngine* newPlugin = new CScriptUploadEngine(fileName, serverSync, &params);
    newPlugin->onErrorMessage.bind(uploadErrorHandler_, &IUploadErrorHandler::ErrorMessage);
    if (newPlugin->isLoaded()) {
        m_plugins[threadId][serverName] = newPlugin;
        return newPlugin;
    }
    else {
        delete newPlugin;
    }
    return NULL;
}

void UploadEngineManager::unloadUploadEngines() {
    std::lock_guard<std::mutex> lock(pluginsMutex_);
    for (auto it = m_plugins.begin(); it != m_plugins.end(); ++it) {
        for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
            delete it2->second;
        }
    }
    m_plugins.clear();
}

void UploadEngineManager::setScriptsDirectory(const std::string & directory) {
    scriptsDirectory_ = directory;
}

void UploadEngineManager::clearThreadData()
{
    std::lock_guard<std::mutex> lock(pluginsMutex_);
    std::thread::id threadId = std::this_thread::get_id();
    auto it = m_plugins.find(threadId);
    if (it != m_plugins.end()) {
        for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
            delete it2->second;
        }
        m_plugins.erase(it);
    }
}

void UploadEngineManager::resetAuthorization(const ServerProfile& serverProfile)
{
    ServerSync* sync = getServerSync(serverProfile);
    sync->resetAuthorization();
}

void UploadEngineManager::resetFailedAuthorization()
{
    std::lock_guard<std::mutex> lock(serverSyncsMutex_);
    for (auto sync : serverSyncs_)
    {
        sync.second->resetFailedAuthorization();
    }
}

ServerSync* UploadEngineManager::getServerSync(const ServerProfile& serverProfile)
{
    std::lock_guard<std::mutex> lock(serverSyncsMutex_);
    ServerSyncMapKey key = std::make_pair(serverProfile.serverName(), serverProfile.profileName());
    auto it = serverSyncs_.find(key);
    if (it == serverSyncs_.end())
    {
        ServerSync *sync = new ServerSync();
        serverSyncs_[key] = sync;
        return sync;
    }
    return it->second;
}
