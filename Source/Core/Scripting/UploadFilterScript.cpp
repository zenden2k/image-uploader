#include "UploadFilterScript.h"

#include "Script.h"
#include "API/ScriptAPI.h"
#include "API/UploadTaskWrappers.h"

UploadFilterScript::UploadFilterScript(const std::string& fileName, ThreadSync* serverSync) : Script(fileName, serverSync)
{
}

bool UploadFilterScript::preUpload(UploadTask* task)
{
    using namespace Sqrat;
    try
    {
        checkCallingThread();

        Function func(vm_.GetRootTable(), "PreUpload");
        if (func.IsNull()) {
            return true;
        }
        bool res = ScriptAPI::GetValue(func.Evaluate<bool>(ScriptAPI::UploadTaskWrapper(task), 0));
        FlushSquirrelOutput();
        return res;
    }
    catch (std::exception& e)
    {
        LOG(ERROR) << "UploadFilterScript::preUpload\r\n"  << std::string(e.what());
    }
    FlushSquirrelOutput();
    return false;
}

bool UploadFilterScript::postUpload(UploadTask* task)
{
    using namespace Sqrat;
    try
    {
        checkCallingThread();

        Function func(vm_.GetRootTable(), "PostUpload");
        if (func.IsNull()) {
            return true;
        }
        bool res = ScriptAPI::GetValue(func.Evaluate<bool>(ScriptAPI::UploadTaskWrapper(task), 0));
        FlushSquirrelOutput();
        return res;
    }
    catch (std::exception& e)
    {
        LOG(ERROR) << "UploadFilterScript::postUpload\r\n" << std::string(e.what());
    }
    FlushSquirrelOutput();
    return false;
}