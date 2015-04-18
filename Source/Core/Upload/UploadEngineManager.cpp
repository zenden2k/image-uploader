#include "UploadEngineManager.h"
#include "../UploadEngineList.h"
#include "ServerProfile.h"
#include "ScriptUploadEngine.h"
#include <Gui/Dialogs/LogWindow.h>
#include "DefaultUploadEngine.h"
#include <Core/Logging.h>

UploadEngineManager::UploadEngineManager(CUploadEngineList* uploadEngineList)
{
	uploadEngineList_ = uploadEngineList;
}

UploadEngineManager::~UploadEngineManager()
{
	UnloadPlugins();
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
	if (ue->UsingPlugin) {
		result = getPlugin(ue->Name, ue->PluginName, serverProfile.serverSettings());
		if (!result) {
			CString errorMessage;
			LOG(ERROR) << "Cannot load plugin '" << ue->PluginName << "'";
		}
	} else {
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
		result = new CDefaultUploadEngine();
		result->setServerSettings(serverProfile.serverSettings());
		result->setUploadData(ue);
		result->onErrorMessage.bind(DefaultErrorHandling::ErrorMessage);
	}
	result->setServerSettings(serverProfile.serverSettings());
	result->setUploadData(ue);
	result->onErrorMessage.bind(DefaultErrorHandling::ErrorMessage);
	return result;
}

CScriptUploadEngine* UploadEngineManager::getScriptUploadEngine(ServerProfile& serverProfile)
{
	return dynamic_cast<CScriptUploadEngine*>(getUploadEngine(serverProfile));
}

CScriptUploadEngine* UploadEngineManager::getPlugin(const Utf8String& serverName, const Utf8String& name, ServerSettingsStruct& params, bool UseExisting) {
	std::lock_guard<std::mutex> lock(pluginsMutex_);
	DWORD curTime = GetTickCount();
	std::thread::id threadId = std::this_thread::get_id();
	CScriptUploadEngine* plugin = m_plugins[threadId][serverName];
	if (plugin && (GetTickCount() - plugin->getCreationTime() < 1000 * 60 * 5))
		UseExisting = true;

	if (plugin && UseExisting && plugin->name() == name && plugin->serverSettings().authData.Login == params.authData.Login) {
		plugin->onErrorMessage.bind(DefaultErrorHandling::ErrorMessage);
		return plugin;
	}

	if (plugin) {
		delete plugin;
		plugin = 0;
		m_plugins[threadId][serverName] = 0;
	}

	CScriptUploadEngine* newPlugin = new CScriptUploadEngine(name);
	newPlugin->onErrorMessage.bind(DefaultErrorHandling::ErrorMessage);
	if (newPlugin->load(m_ScriptsDirectory + name + ".nut", params)) {
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