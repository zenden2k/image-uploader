#ifndef CORE_UPLOAD_URLSHORTENINGTASK_H
#define CORE_UPLOAD_URLSHORTENINGTASK_H

#include "UploadTask.h"
#include <string>

class UrlShorteningTask: public UploadTask {
	public:
		UrlShorteningTask(const std::string& url);
		virtual std::string getType() const;
		virtual std::string getMimeType() const;
		virtual int64_t getDataLength() const;
		std::string getUrl() const;
	protected:
		std::string url_;
};	

#endif