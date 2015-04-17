#ifndef CORE_UPLOAD_UPLOADTASK_H
#define CORE_UPLOAD_UPLOADTASK_H

#include <string>
#include <Core/Utils/CoreTypes.h>
#include "UploadResult.h"
#include <Core/3rdpart/FastDelegate.h>
#include <Core/Network/NetworkClient.h>
#include "ServerProfile.h"
#include <mutex>

class UploadSession;

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
		void setSession(UploadSession* session);
		UploadSession* session();
		bool isFinished();
		void setFinished(bool finished);
		void setRunning(bool running);
		void addChildTask(std::shared_ptr<UploadTask> child);
		UploadResult* uploadResult();
		UploadProgress* progress();
		fastdelegate::FastDelegate2<UploadTask*, bool> OnFileFinished;
		fastdelegate::FastDelegate1<UploadTask*> OnUploadProgress;
		std::string serverName() const;
		ServerProfile& serverProfile();
		void setServerProfile(ServerProfile profile);
		void setUserData(void* data);
		void* userData() const;
		bool uploadSuccess();
		void setUploadSuccess(bool success);

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
		void childTaskFinished(UploadTask* child);
		void taskFinished();
		bool uploadSuccess_;
		UploadSession* session_;
		std::mutex tasksMutex_;
};	

#endif