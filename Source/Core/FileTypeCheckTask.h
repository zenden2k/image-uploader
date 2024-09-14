#pragma once

#include "Core/BackgroundTask.h"
#include "Core/Upload/ServerProfileGroup.h"
#include "IFileList.h"

class FileTypeCheckTask: public BackgroundTask {
public:
    explicit FileTypeCheckTask(IFileList* fileList, const ServerProfileGroup& sessionImageServer, const ServerProfileGroup& sessionFileServer);
	
	BackgroundTaskResult doJob() override;

	std::string message() const;
	
	DISALLOW_COPY_AND_ASSIGN(FileTypeCheckTask);

    private:
        IFileList* fileList_;
        ServerProfileGroup sessionImageServer_;
        ServerProfileGroup sessionFileServer_;
        std::string message_;

};
