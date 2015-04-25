#ifndef CORE_UPLOAD_URLSHORTENINGTASK_H
#define CORE_UPLOAD_URLSHORTENINGTASK_H

#include "UploadTask.h"
#include <string>
#include "Core/Utils/EnumUtils.h"

class UrlShorteningTask: public UploadTask {
	public:
		UrlShorteningTask(const std::string& url, UploadTask* parentTask = 0);

        DEFINE_MEMBER_ENUM_WITH_STRING_CONVERSIONS(ParentUrlType, 
            (None)(DirectUrl)(DownloadUrl));

		virtual Type type() const override;
		virtual std::string getMimeType() const override;
		virtual int64_t getDataLength() const override;
		std::string getUrl() const;
		virtual void finishTask(Status status = StatusFinished) override;
		void setParentUrlType(ParentUrlType type);
		ParentUrlType parentUrlType();
		std::string toString() override;
		std::string title() const override;
protected:
		std::string url_;
		ParentUrlType parentUrlType_;
};	

#endif