#ifndef IU_CORE_UPLOADENGINE_MANAGER_H
#define IU_CORE_UPLOADENGINE_MANAGER_H

#pragma once
#include <thread>
#include <mutex>

#include "UploadEngine.h"

// Forward class declarations
class IUploadErrorHandler;
class CScriptUploadEngine;
class CUploadEngineList;
class ServerProfile;

/** UploadEngineManager class manages upload engines (instances of classes derivated from CAbstractUploadEngine).
    and their lifetime
**/
class UploadEngineManager
{
public:
    UploadEngineManager(CUploadEngineList* uploadEngineList, IUploadErrorHandler* uploadErrorHandler);
    ~UploadEngineManager();

    /**
    Load and create upload engine. Object is owned by UploadEngineManager.
    CScriptUploadEngine function can be called only in the thread it has been created.
    **/
    CAbstractUploadEngine* getUploadEngine(ServerProfile &serverProfile);
    CScriptUploadEngine* getScriptUploadEngine(ServerProfile &serverProfile);
    
    /** 
    Force unload all cached upload engines
    */
    void unloadUploadEngines();
    void setScriptsDirectory(const std::string & directory);

    /** 
    This function destroys all upload engines owned by current thread and all associated Scripting API data.
    */
    void clearThreadData();

    /**
    Reset authorization on this server (failed and succeeded).
    It is called when user starts new upload
    */
    void resetAuthorization(const ServerProfile& serverProfile);

    /**
    Reset failed authorization on ALL servers
    */
    void resetFailedAuthorization();
protected:
    CScriptUploadEngine* getPlugin(ServerProfile& serverProfile, const std::string& pluginName, bool UseExisting = false);
    ServerSync* getServerSync(const ServerProfile& serverProfile);
    std::map<std::thread::id, std::map< std::string, CAbstractUploadEngine*>> m_plugins;
    std::mutex pluginsMutex_;
    std::string scriptsDirectory_;
    CUploadEngineList* uploadEngineList_;
    typedef std::pair<std::string, std::string> ServerSyncMapKey;
    std::map<ServerSyncMapKey, ServerSync*> serverSyncs_;
    std::mutex serverSyncsMutex_;
    IUploadErrorHandler* uploadErrorHandler_;
};

#endif
