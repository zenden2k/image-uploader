#include "UserFilter.h"

#include "Core/Upload/UrlShorteningTask.h"
#include "Core/Settings/BasicSettings.h"
#include "Core/Scripting/UploadFilterScript.h"
#include "Core/ServiceLocator.h"

UserFilter::UserFilter(ScriptsManager* scriptsManager)
{
    scriptsManager_ = scriptsManager;
}

bool UserFilter::PreUpload(UploadTask* task)
{
    BasicSettings& Settings = *ServiceLocator::instance()->basicSettings();
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
    BasicSettings& Settings = *ServiceLocator::instance()->basicSettings();
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
