#include "UploadEngineManager.h"
#include "../UploadEngineList.h"
#include "ServerProfile.h"
#include "ScriptUploadEngine.h"
#include "Gui/Dialogs/LogWindow.h"
#include "DefaultUploadEngine.h"
#include "Core/Logging.h"
#include "ServerSync.h"
#include "Core/Scripting/API/ScriptAPI.h"
#include <Func/Settings.h>

UploadEngineManager::UploadEngineManager(CUploadEngineList* uploadEngineList)
{
	uploadEngineList_ = uploadEngineList;
}

UploadEngineManager::~UploadEngineManager()
{
	UnloadPlugins();
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
	CAbstractUploadEngine* result = NULL;
    std::string serverName = serverProfile.serverName();
    std::thread::id threadId = std::this_thread::get_id();
    ServerSettingsStruct& params = serverProfile.serverSettings();
    
	if (ue->UsingPlugin) {
		result = getPlugin(serverProfile, ue->PluginName);
		if (!result) {
			CString errorMessage;
			LOG(ERROR) << "Cannot load plugin '" << ue->PluginName << "'";
			return NULL;
		}
		
	} else {
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
		result = new CDefaultUploadEngine(serverSync);
		result->setServerSettings(&serverProfile.serverSettings());
		result->setUploadData(ue);
		result->onErrorMessage.bind(DefaultErrorHandling::ErrorMessage);
        
        m_plugins[threadId][serverName] = result;

	}
	
	result->setServerSettings(&serverProfile.serverSettings());
	result->setUploadData(ue);
	result->onErrorMessage.bind(DefaultErrorHandling::ErrorMessage);
	return result;
}

CScriptUploadEngine* UploadEngineManager::getScriptUploadEngine(ServerProfile& serverProfile)
{
	return dynamic_cast<CScriptUploadEngine*>(getUploadEngine(serverProfile));
}

CScriptUploadEngine* UploadEngineManager::getPlugin(ServerProfile& serverProfile, const std::string& pluginName, bool UseExisting) {
	std::lock_guard<std::mutex> lock(pluginsMutex_);
	DWORD curTime = GetTickCount();
	std::string serverName = serverProfile.serverName();
	ServerSettingsStruct& params = serverProfile.serverSettings();
	std::thread::id threadId = std::this_thread::get_id();
    CScriptUploadEngine* plugin = dynamic_cast<CScriptUploadEngine*>(m_plugins[threadId][serverName]);
    if (plugin && (GetTickCount() - plugin->getCreationTime() <(Settings.DeveloperMode ? 3000 : 1000 * 60 * 5)))
		UseExisting = true;

	if (plugin && UseExisting && plugin->name() == pluginName && plugin->serverSettings()->authData.Login == params.authData.Login) {
		plugin->onErrorMessage.bind(DefaultErrorHandling::ErrorMessage);
        plugin->switchToThisVM();
		return plugin;
	}

	if (plugin) {
		delete plugin;
		plugin = 0;
		m_plugins[threadId][serverName] = 0;
	}
	ServerSync* serverSync = getServerSync(serverProfile);
    std::string fileName = m_ScriptsDirectory + pluginName + ".nut";
    CScriptUploadEngine* newPlugin = new CScriptUploadEngine(fileName, serverSync, &params);
	newPlugin->onErrorMessage.bind(DefaultErrorHandling::ErrorMessage);
	if (newPlugin->isLoaded()) {
		m_plugins[threadId][serverName] = newPlugin;
		return newPlugin;
	}
	else {
		delete newPlugin;
	}
	return NULL;
}


void UploadEngineManager::UnloadPlugins() {
	std::lock_guard<std::mutex> lock(pluginsMutex_);
	for (auto it = m_plugins.begin(); it != m_plugins.end(); ++it) {
		for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
			delete it2->second;
		}
	}
	m_plugins.clear();
}

void UploadEngineManager::setScriptsDirectory(const Utf8String & directory) {
	m_ScriptsDirectory = directory;
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
