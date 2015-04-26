#include "ScriptsManager.h"
#include <Gui/Dialogs/LogWindow.h>
#include "UploadFilterScript.h"
#include <Func/Settings.h>

ScriptsManager::ScriptsManager()
{
}

ScriptsManager::~ScriptsManager()
{
    unloadScripts();
}

Script* ScriptsManager::getScript(std::string& fileName, ScriptType type)
{
    std::lock_guard<std::mutex> lock(scriptsMutex_);
    DWORD curTime = GetTickCount();
    bool UseExisting = false;
    std::thread::id threadId = std::this_thread::get_id();
    Script* plugin = scripts_[threadId][fileName];
    if (plugin && (GetTickCount() - plugin->getCreationTime() < (Settings.DeveloperMode ? 3000 : 1000 * 60 * 5 )))
        UseExisting = true;

    if ( plugin && UseExisting ) {
        plugin->switchToThisVM();
        return plugin;
    }

    if (plugin) {
        delete plugin;
        plugin = 0;
       
        scripts_.erase(threadId);
    }
    ServerSync* serverSync = getServerSync(fileName);
    Script* newPlugin;
    if (type == TypeUploadFilterScript)
    {
        newPlugin = new UploadFilterScript(fileName, serverSync);
    } else
    {
        newPlugin  = new Script(fileName, serverSync);
    }
  
    if (newPlugin->isLoaded()) {
        scripts_[threadId][fileName] = newPlugin;
        return newPlugin;
    }
    else {
        delete newPlugin;
    }
    return NULL;
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
    std::thread::id threadId = std::this_thread::get_id();
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