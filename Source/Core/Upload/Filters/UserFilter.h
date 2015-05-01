#ifndef IU_CORE_USERFILTER_H
#define IU_CORE_USERFILTER_H

#include "Core/Upload/UploadFilter.h"
#include "Core/Upload/UploadTask.h"
#include "Core/Scripting/ScriptsManager.h"


class UserFilter : public UploadFilter
{
public:
    UserFilter(ScriptsManager* scriptsManager);
    virtual bool PreUpload(UploadTask* task) override;
    virtual bool PostUpload(UploadTask* task) override;
protected:
    ScriptsManager* scriptsManager_;

};
#endif