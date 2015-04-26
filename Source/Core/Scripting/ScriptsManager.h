#ifndef IU_CORE_SCRIPTING_SCRIPTSMANAGER_H
#define IU_CORE_SCRIPTING_SCRIPTSMANAGER_H

#pragma once
#include <thread>
#include <mutex>
#include "Script.h"
#include "Core/Upload/ServerSync.h"



class ScriptsManager {
public:
    ScriptsManager();
    ~ScriptsManager();
    Script* getScript(std::string &fileName);
    void unloadScripts();
    void clearThreadData();
    ServerSync* getServerSync(const std::string& fileName);
protected:
    std::map<std::thread::id, Script*> scripts_;
    std::mutex scriptsMutex_;
    std::string scriptsDirectory_;
    typedef std::string ServerSyncMapKey;
    std::map<ServerSyncMapKey, ServerSync*> serverSyncs_;
    std::mutex serverSyncsMutex_;
};

#endif