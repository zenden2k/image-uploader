#ifndef CORE_UPLOAD_URLSHORTENINGTASK_H
#define CORE_UPLOAD_URLSHORTENINGTASK_H

#include "UploadTask.h"
#include <string>

class UrlShorteningTask: public UploadTask {
	public:
		UrlShorteningTask(const std::string& url, UploadTask* parentTask = 0);

		enum ParentUrlType
		{
			None, DirectUrl, DownloadUrl
		};
		virtual std::string getType() const;
		virtual std::string getMimeType() const;
		virtual int64_t getDataLength() const;
		std::string getUrl() const;
		void setFinished(bool finished) override;
		void setParentUrlType(ParentUrlType type);
		ParentUrlType parentUrlType();
		std::string toString() override;
protected:
		std::string url_;
		ParentUrlType parentUrlType_;
};	

#endif