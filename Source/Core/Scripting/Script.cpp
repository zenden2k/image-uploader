#include "Script.h"
#include "API/ScriptAPI.h"
#include "Core/Upload/ScriptUploadEngine.h"
#include "Core/Logging.h"
#include <Core/ThreadSync.h>
#include <Func/Common.h>

Script::Script(const std::string& fileName, ThreadSync* serverSync, bool doLoad)
{
    m_CreationTime = time(0);
    m_SquirrelScript = 0;
    m_bIsPluginLoaded = false;
    sync_ = serverSync;
    owningThread_ = std::this_thread::get_id();
    fileName_ = fileName;
    if (doLoad) {
        load(fileName);
    }
}

Script::~Script()
{
    ScriptAPI::ClearVmData(vm_);
}

void Script::InitScriptEngine()
{
    sqstd_seterrorhandlers(vm_.GetVM());
    ScriptAPI::SetPrintCallback(vm_, ScriptAPI::PrintCallback(this, &Script::PrintCallback));
}

void Script::DestroyScriptEngine()
{
    ScriptAPI::CleanUp();
}

void Script::FlushSquirrelOutput()
{
    ScriptAPI::FlushSquirrelOutput(vm_);
}

bool Script::preLoad()
{
    networkClient_.reset(new NetworkClient());
    IU_ConfigureProxy(*(networkClient_.get()));
    Sqrat::RootTable& rootTable = vm_.GetRootTable();
    rootTable.SetInstance("Sync", sync_);
    rootTable.SetInstance("nm", networkClient_.get());
    return true;
}

bool Script::postLoad()
{
    return true;
}

bool Script::isLoaded()
{
    return m_bIsPluginLoaded;
}

time_t Script::getCreationTime()
{
    return m_CreationTime;
}

void Script::switchToThisVM()
{
    ScriptAPI::SetCurrentThreadVM(vm_);
}

Sqrat::SqratVM& Script::getVM()
{
    return vm_;
}

bool Script::load(Utf8String fileName)
{
    using namespace Sqrat;
    if (!IuCoreUtils::FileExists(fileName))
    {
        LOG(ERROR) << "Script file doesn't exist " << fileName;
        return false;
    }
       

    using namespace ScriptAPI;
    try
    {
        InitScriptEngine();
        ScriptAPI::RegisterAPI(vm_);

        std::string scriptText;
        if (!IuCoreUtils::ReadUtf8TextFile(fileName, scriptText)) {
            LOG(ERROR) << "Failed to read script from file " << fileName;
            return false;
        }

        preLoad();
 
        ScriptAPI::SetScriptName(vm_, fileName_);
        switchToThisVM();
        m_SquirrelScript = new Sqrat::Script(vm_.GetVM());
        m_SquirrelScript->CompileString(scriptText.c_str(), IuCoreUtils::ExtractFileName(fileName).c_str());

        m_SquirrelScript->Run();
        ScriptAPI::RegisterShortTranslateFunctions(vm_);
        postLoad();
        m_bIsPluginLoaded = true;
    }
    catch (std::exception& e)
    {
        LOG(ERROR)<< "CScriptUploadEngine::Load failed\r\n" + std::string("Error: ") + e.what();
        FlushSquirrelOutput();
        return false;
    }
    FlushSquirrelOutput();
    return true;
}



void Script::PrintCallback(const std::string& output)
{
    std::thread::id threadId = std::this_thread::get_id();
    LOG(WARNING) << IuCoreUtils::ExtractFileName(fileName_) << " [ ThreadId=" << IuCoreUtils::ThreadIdToString(threadId) << "]\r\n" << output;
}

void Script::checkCallingThread()
{
    std::thread::id threadId = std::this_thread::get_id();
    if (threadId != owningThread_)
    {
        throw std::runtime_error("Script methods should be called only in the owning thread.");
    }
}