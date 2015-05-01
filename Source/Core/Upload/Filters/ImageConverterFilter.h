#ifndef IU_CORE_IMAGECONVERTERFILTER_H
#define IU_CORE_IMAGECONVERTERFILTER_H

#include "Core/Upload/UploadFilter.h"
#include "Core/Upload/UploadTask.h"

class ImageConverterFilter : public UploadFilter
{
public:
    virtual bool PreUpload(UploadTask* task) override;
    virtual bool PostUpload(UploadTask* task) override;
protected:
    //void OnFileFinished(UploadTask* task, bool ok);
};
#endif