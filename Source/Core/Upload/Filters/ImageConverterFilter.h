#ifndef IU_CORE_IMAGECONVERTERFILTER_H
#define IU_CORE_IMAGECONVERTERFILTER_H

#include "Core/Upload/UploadFilter.h"
#include "Core/Upload/UploadTask.h"

class ImageConverterFilter : public UploadFilter
{
public:
    bool PreUpload(UploadTask* task) override;
    bool PostUpload(UploadTask* task) override;
};
#endif