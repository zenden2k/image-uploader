#ifndef IU_CORE_URLSHORTENINGFILTER_H
#define IU_CORE_URLSHORTENINGFILTER_H

#include "Core/Upload/UploadFilter.h"
#include "Core/Upload/UploadTask.h"

class UrlShorteningFilter : public UploadFilter
{
public:
    bool PreUpload(UploadTask* task) override;
    bool PostUpload(UploadTask* task) override;
};
#endif