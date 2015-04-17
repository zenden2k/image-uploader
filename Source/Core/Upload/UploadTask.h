#ifndef CORE_UPLOAD_UPLOADTASK_H
#define CORE_UPLOAD_UPLOADTASK_H

#include <string>
#include <Core/Utils/CoreTypes.h>
#include "UploadResult.h"
#include <Core/3rdpart/FastDelegate.h>
#include <Core/Network/NetworkClient.h>
#include "ServerProfile.h"

class UploadProgress {
public:
	std::string statusText;
	int stage;
	int64_t uploaded;
	int64_t totalUpload;
	bool isUploading;
};

class UploadTask {
	public:
		UploadTask();
		UploadTask(UploadTask* parentTask);
		virtual ~UploadTask();
		virtual std::string getType() const = 0;
		virtual std::string getMimeType() const = 0;
		virtual int64_t getDataLength() const = 0;
		UploadTask* parentTask() const;
		bool isRunning();
		bool isFinished();
		void setFinished(bool finished);
		void setRunning(bool running);
		void addChildTask(std::shared_ptr<UploadTask> child);
		UploadResult* uploadResult();
		UploadProgress* progress();
		fastdelegate::FastDelegate2<std::shared_ptr<UploadTask>, bool> OnFileFinished;
		fastdelegate::FastDelegate1<std::shared_ptr<UploadTask>> OnUploadProgress;
		std::string serverName() const;
		ServerProfile& serverProfile();
		void setServerProfile(ServerProfile profile);
		void setUserData(void* data);
		void* userData() const;
	protected:
		UploadTask* parentTask_;
		std::vector<std::shared_ptr<UploadTask>> childTasks_;
		bool isRunning_;
		bool isFinished_;
		UploadResult uploadResult_;
		UploadProgress progress_;
		ServerProfile serverProfile_;
		void* userData_;
		void init();
};	

#endif