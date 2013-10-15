#ifndef CORE_UPLOAD_FILEUPLOADTASK_H
#define CORE_UPLOAD_FILEUPLOADTASK_H

#include "UploadTask.h"
#include <string>

class FileUploadTask: public UploadTask {
	public:
		FileUploadTask(const std::string& fileName, const std::string& displayName);
		virtual std::string getType() const;
		virtual std::string getMimeType() const;
		virtual int64_t getDataLength() const;
		std::string getFileName() const;
		std::string getDisplayName() const;
	protected:
		std::string fileName_;
		std::string displayName_;
};	

#endif