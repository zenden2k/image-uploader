/*

Uptooda - free application for uploading images/files to the Internet

Copyright 2007-2025 Sergey Svistunov (zenden2k@gmail.com)

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

#include "ScriptsManager.h"

#include "UploadFilterScript.h"
#include "Core/Settings/BasicSettings.h"
#include "Core/ServiceLocator.h"

ScriptsManager::ScriptsManager(std::shared_ptr<INetworkClientFactory> networkClientFactory) :
    networkClientFactory_(std::move(networkClientFactory))
{
}

ScriptsManager::~ScriptsManager()
{
    unloadScripts();
    for (auto& it : serverSyncs_) {
        delete it.second;
    }
    serverSyncs_.clear();
}

Script* ScriptsManager::getScript(const std::string& fileName, ScriptType type)
{
    std::lock_guard<std::mutex> lock(scriptsMutex_);
    bool useExisting = false;
    const std::thread::id threadId = std::this_thread::get_id();
    Script* plugin = nullptr;
    auto it = scripts_.find(threadId);
    if (it != scripts_.end()) {
        auto it2 = it->second.find(fileName);
        if (it2 != it->second.end()) {
            plugin = it2->second;
        }
    }
    const auto settings = ServiceLocator::instance()->basicSettings();
    if (plugin && (time(nullptr) - plugin->getCreationTime() < (settings->DeveloperMode ? 3000 : 1000 * 60 * 5))) {
        useExisting = true;
    }

    if ( plugin && useExisting ) {
        plugin->switchToThisVM();
        return plugin;
    }

    if (plugin) {
        delete plugin;
        plugin = nullptr;

        scripts_.erase(threadId);
    }
    ServerSync* serverSync = getServerSync(fileName);
    Script* newPlugin;
    if (type == ScriptType::TypeUploadFilterScript)
    {
        newPlugin = new UploadFilterScript(fileName, serverSync, networkClientFactory_);
    } else
    {
        newPlugin  = new Script(fileName, serverSync, networkClientFactory_);
    }

    if (newPlugin->isLoaded()) {
        scripts_[threadId][fileName] = newPlugin;
        return newPlugin;
    }
    else {
        delete newPlugin;
    }
    return nullptr;
}

void ScriptsManager::unloadScripts()
{
    std::lock_guard<std::mutex> lock(scriptsMutex_);
    for (auto it = scripts_.begin(); it != scripts_.end(); ++it) {
        for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
            delete it2->second;
        }
    }
    scripts_.clear();
}

void ScriptsManager::clearThreadData()
{
    std::lock_guard<std::mutex> lock(scriptsMutex_);
    const std::thread::id threadId = std::this_thread::get_id();
    auto it = scripts_.find(threadId);
    if (it != scripts_.end()) {
        for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
            delete it2->second;
        }
        scripts_.erase(it);
    }
}

ServerSync* ScriptsManager::getServerSync(const std::string& fileName)
{
    std::lock_guard<std::mutex> lock(serverSyncsMutex_);
    auto it = serverSyncs_.find(fileName);
    if (it == serverSyncs_.end())
    {
        ServerSync *sync = new ServerSync();
        serverSyncs_[fileName] = sync;
        return sync;
    }
    return it->second;
}
