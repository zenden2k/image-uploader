#include "UserFilter.h"

#include "Core/Upload/UrlShorteningTask.h"
#include "Core/Upload/FileUploadTask.h"
#include "Core/Settings.h"
#include "Core/Scripting/UploadFilterScript.h"

UserFilter::UserFilter(ScriptsManager* scriptsManager)
{
    scriptsManager_ = scriptsManager;
}

bool UserFilter::PreUpload(UploadTask* task)
{
    if (Settings.ExecuteScript && !Settings.ScriptFileName.empty())
    {
        UploadFilterScript* script = dynamic_cast<UploadFilterScript*>(scriptsManager_->getScript(Settings.ScriptFileName, ScriptsManager::TypeUploadFilterScript));
        if (!script)
        {
            return false;
        }
        return script->preUpload(task);
    }
    return true;
}

bool UserFilter::PostUpload(UploadTask* task)
{
    
    if (Settings.ExecuteScript && !Settings.ScriptFileName.empty())
    {
        UploadFilterScript* script = dynamic_cast<UploadFilterScript*>(scriptsManager_->getScript(Settings.ScriptFileName, ScriptsManager::TypeUploadFilterScript));
        if (!script)
        {
            return false;
        }
        return script->postUpload(task);
    }
    return true;
}
