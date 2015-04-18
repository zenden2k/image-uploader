#ifndef CORE_UPLOAD_FILEUPLOADTASK_H
#define CORE_UPLOAD_FILEUPLOADTASK_H

#include "UploadTask.h"
#include <string>

class FileUploadTask: public UploadTask {
	public:
		FileUploadTask(const std::string& fileName, const std::string& displayName, UploadTask* parentTask = 0);
		virtual std::string getType() const;
		virtual std::string getMimeType() const;
		virtual int64_t getDataLength() const;
		std::string getFileName() const;
		void setFileName(const std::string& fileName);
		std::string getDisplayName() const;
		void setDisplayName(const std::string& name);
		std::string originalFileName() const;
		void setFinished(bool finished) override;
	protected:
		std::string fileName_;
		std::string originalFileName_;
		std::string displayName_;
};	

#endif