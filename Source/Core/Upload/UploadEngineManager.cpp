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

#include "UploadEngineManager.h"

#include "../UploadEngineList.h"
#include "ServerProfile.h"
#include "ScriptUploadEngine.h"
#include "DefaultUploadEngine.h"
#include "Core/Logging.h"
#include "ServerSync.h"
#include "Core/Settings/BasicSettings.h"
#include "Core/Upload/UploadErrorHandler.h"
#include "Core/ServiceLocator.h"
#ifndef IU_DISABLE_MEGANZ
#include "MegaNzUploadEngine.h"
#endif

UploadEngineManager::UploadEngineManager(CUploadEngineList* uploadEngineList, IUploadErrorHandler* uploadErrorHandler, 
    std::shared_ptr<INetworkClientFactory> factory)
{
    uploadEngineList_ = uploadEngineList;
    uploadErrorHandler_ = uploadErrorHandler;
    networkClientFactory_ = factory;
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
        return nullptr;
    }
    CUploadEngineData *ue = uploadEngineList_->byName(serverProfile.serverName());
    if (!ue)
    {
        LOG(ERROR) << "No such server " << serverProfile.serverName();
        return nullptr;
    }
    CAbstractUploadEngine* result = nullptr;
    std::string serverName = serverProfile.serverName();
    std::thread::id threadId = std::this_thread::get_id();

    BasicSettings* Settings = ServiceLocator::instance()->basicSettings();
    ServerSettingsStruct* serverSettings = Settings->getServerSettings(serverProfile, true);
    std::string authDataLogin = serverSettings ? serverSettings->authData.Login : std::string();
    if (ue->UsingPlugin) {
        // Try to load Squirrel (.nut) script
        result = getPlugin(serverProfile, ue->PluginName);
        if (!result) {
            LOG(ERROR) << "Cannot load plugin '" << ue->PluginName << "'";
            return nullptr;
        }
    } else {
        std::lock_guard<std::mutex> guard(pluginsMutex_);
        CAbstractUploadEngine* plugin = nullptr;
        auto it = m_plugins.find(threadId);
        if (it != m_plugins.end()) {
            auto it2 = it->second.find(serverName);
            if (it2 != it->second.end()) {
                plugin = it2->second;
            }
        }

        if (plugin &&  plugin->serverSettings()->authData.Login == authDataLogin) {
            return plugin;
        }

        delete plugin;
        ServerSync* serverSync = getServerSync(serverProfile);
        CAbstractUploadEngine::ErrorMessageCallback errorCallback(uploadErrorHandler_, &IUploadErrorHandler::ErrorMessage);
#ifndef IU_DISABLE_MEGANZ
        if (ue->Engine == "MegaNz") {
            result = new CMegaNzUploadEngine(serverSync, serverSettings, errorCallback);
        } 
		else
#endif
		{
            result = new CDefaultUploadEngine(serverSync, errorCallback);
        }
        result->setServerSettings(serverSettings);
        result->setUploadData(ue);
        
        m_plugins[threadId][serverName] = result;
    }
    
    result->setServerSettings(serverSettings);
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

    BasicSettings* basicSettings = ServiceLocator::instance()->basicSettings();
    ServerSettingsStruct* params = basicSettings->getServerSettings(serverProfile, true);

    std::thread::id threadId = std::this_thread::get_id();
    CScriptUploadEngine* plugin = nullptr;

    auto it = m_plugins.find(threadId);
    if (it != m_plugins.end()) {
        auto it2 = it->second.find(serverName);
        if (it2 != it->second.end()) {
            plugin = dynamic_cast<CScriptUploadEngine*>(it2->second);;
        }
    }

    BasicSettings& Settings = *ServiceLocator::instance()->basicSettings();
    if (plugin && (time(0)- plugin->getCreationTime() <(Settings.DeveloperMode ? 3000 : 1000 * 60 * 5)))
        UseExisting = true;

    if (plugin) {
        ServerSettingsStruct* serverSettings = plugin->serverSettings();
        if (UseExisting && plugin->name() == pluginName && serverSettings->authData.Login == params->authData.Login) {
            plugin->onErrorMessage.bind(uploadErrorHandler_, &IUploadErrorHandler::ErrorMessage);
            plugin->switchToThisVM();
            return plugin;
        }
    }

    if (plugin) {
        delete plugin;
        plugin = 0;
        m_plugins[threadId][serverName] = nullptr;
    }
    ServerSync* serverSync = getServerSync(serverProfile);
    std::string fileName = scriptsDirectory_ + pluginName + ".nut";
    CScriptUploadEngine* newPlugin = new CScriptUploadEngine(fileName, serverSync, params, networkClientFactory_, 
        CAbstractUploadEngine::ErrorMessageCallback(uploadErrorHandler_, &IUploadErrorHandler::ErrorMessage));

    if (newPlugin->isLoaded()) {
        m_plugins[threadId][serverName] = newPlugin;
        return newPlugin;
    }
    else {
        delete newPlugin;
    }
    return nullptr;
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
