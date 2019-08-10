#ifndef IU_CORE_USERFILTER_H
#define IU_CORE_USERFILTER_H

#include "Core/Upload/UploadFilter.h"
#include "Core/Scripting/ScriptsManager.h"

class UserFilter : public UploadFilter
{
public:
    UserFilter(ScriptsManager* scriptsManager);
    bool PreUpload(UploadTask* task) override;
    bool PostUpload(UploadTask* task) override;
protected:
    ScriptsManager* scriptsManager_;

};
#endif