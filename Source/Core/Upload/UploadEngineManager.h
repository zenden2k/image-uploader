#ifndef IU_CORE_UPLOADENGINE_MANAGER_H

#pragma once
#include "UploadEngine.h"

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
protected:
	CScriptUploadEngine* getPlugin(const Utf8String& serverName, const Utf8String& name, ServerSettingsStruct& params, bool UseExisting = false);
	std::map<Utf8String, CScriptUploadEngine*> m_plugins;
	Utf8String m_ScriptsDirectory;
	CUploadEngineList* uploadEngineList_;
};

#endif
