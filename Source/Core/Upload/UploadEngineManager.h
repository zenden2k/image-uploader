#ifndef IU_CORE_UPLOADENGINE_MANAGER_H

#pragma once
#include "UploadEngine.h"
#include <thread>
#include <mutex>

class IUploadErrorHandler;
class CScriptUploadEngine;
class CUploadEngineList;
class ServerProfile;

class UploadEngineManager
{
public:
    UploadEngineManager(CUploadEngineList* uploadEngineList, IUploadErrorHandler* uploadErrorHandler);
    ~UploadEngineManager();
    CAbstractUploadEngine* getUploadEngine(ServerProfile &serverProfile);
    CScriptUploadEngine* getScriptUploadEngine(ServerProfile &serverProfile);
    void UnloadPlugins();
    void setScriptsDirectory(const std::string & directory);
    void clearThreadData();
    void resetAuthorization(const ServerProfile& serverProfile);
    void resetFailedAuthorization();
protected:
    CScriptUploadEngine* getPlugin(ServerProfile& serverProfile, const std::string& pluginName, bool UseExisting = false);
    ServerSync* getServerSync(const ServerProfile& serverProfile);
    std::map<std::thread::id, std::map< std::string, CAbstractUploadEngine*>> m_plugins;
    std::mutex pluginsMutex_;
    std::string m_ScriptsDirectory;
    CUploadEngineList* uploadEngineList_;
    typedef std::pair<std::string, std::string> ServerSyncMapKey;
    std::map<ServerSyncMapKey, ServerSync*> serverSyncs_;
    std::mutex serverSyncsMutex_;
    IUploadErrorHandler* uploadErrorHandler_;
};

#endif
