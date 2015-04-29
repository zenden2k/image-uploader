#ifndef IU_CORE_UPLOADENGINE_MANAGER_H

#pragma once
#include "UploadEngine.h"
#include <thread>
#include <mutex>

class CScriptUploadEngine;
class CUploadEngineList;
class ServerProfile;

class UploadEngineManager
{
public:
	UploadEngineManager(CUploadEngineList* uploadEngineList);
	~UploadEngineManager();
	CAbstractUploadEngine* getUploadEngine(ServerProfile &serverProfile);
	CScriptUploadEngine* getScriptUploadEngine(ServerProfile &serverProfile);
	void UnloadPlugins();
	void setScriptsDirectory(const Utf8String & directory);
	void clearThreadData();
protected:
	CScriptUploadEngine* getPlugin(ServerProfile& serverProfile, const std::string& pluginName, bool UseExisting = false);
	ServerSync* getServerSync(const ServerProfile& serverProfile);
	std::map<std::thread::id, std::map< Utf8String, CAbstractUploadEngine*>> m_plugins;
	std::mutex pluginsMutex_;
	Utf8String m_ScriptsDirectory;
	CUploadEngineList* uploadEngineList_;
	typedef std::pair<std::string, std::string> ServerSyncMapKey;
	std::map<ServerSyncMapKey, ServerSync*> serverSyncs_;
	std::mutex serverSyncsMutex_;
};

#endif
