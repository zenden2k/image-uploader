#ifndef IU_CORE_SCRIPTING_SCRIPT_H
#define IU_CORE_SCRIPTING_SCRIPT_H

#include "Squirrelnc.h"

#pragma once
#include <thread>
#include <Core/ThreadSync.h>

class ServerSync;
class NetworkClient;
class Script {
    public:
        Script(const std::string& fileName, ThreadSync* serverSync, bool load = true);
        virtual ~Script();
        Sqrat::SqratVM& getVM();
        bool isLoaded();
        void InitScriptEngine();
        static void DestroyScriptEngine();
        time_t getCreationTime();
        /**
        Using multiple Squirrel VMs in the SAME thread is not supported. 
        This function should be called each time current script is changed.
        */
        void switchToThisVM();
    protected:
        virtual void PrintCallback(const std::string& output);
        void checkCallingThread();
        bool load(std::string fileName);
        void FlushSquirrelOutput();
        virtual bool preLoad();
        virtual bool postLoad();
        std::string fileName_;
        Sqrat::SqratVM vm_;
        Sqrat::Script* m_SquirrelScript;
        time_t m_CreationTime;
        bool m_bIsPluginLoaded;
        std::thread::id owningThread_;
        ThreadSync* sync_;
        std::unique_ptr<NetworkClient> networkClient_;
    private:
        DISALLOW_COPY_AND_ASSIGN(Script);

};

#endif