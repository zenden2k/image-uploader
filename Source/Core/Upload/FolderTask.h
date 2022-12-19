#ifndef IU_CORE_UPLOAD_FOLDERTASK_H
#define IU_CORE_UPLOAD_FOLDERTASK_H
#pragma once

#include <string>


#include "FolderList.h"
#include "UploadTask.h"

enum class FolderOperationType
{
    foGetFolders, foCreateFolder, foModifyFolder
};

class FolderTask: public UploadTask {
    public:
        explicit FolderTask(FolderOperationType opType);
        Type type() const override;
        std::string getMimeType() const override;
        int64_t getDataLength() const override;
        std::string toString() override;
        std::string title() const override;
        int retryLimit() override;
        CFolderList& folderList();
        FolderOperationType operationType() const;
        CFolderItem& folder();
        void setFolder(const CFolderItem& item);
private:
    FolderOperationType operationType_;
    CFolderList folderList_;
    CFolderItem folder_;
};   

#endif 
