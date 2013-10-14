#ifndef CORE_UPLOAD_UPLOADTASK_H
#define CORE_UPLOAD_UPLOADTASK_H

#include <string>
#include <Core/Utils/CoreTypes.h>

class UploadTask {
	public:
		virtual ~UploadTask();
		virtual std::string getType() const = 0;
		virtual std::string getMimeType() const = 0;
		virtual int64_t getDataLength() const = 0;
};	

#endif