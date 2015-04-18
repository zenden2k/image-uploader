#ifndef CORE_UPLOAD_UPLOADTASK_H
#define CORE_UPLOAD_UPLOADTASK_H

#include <string>
#include <Core/Utils/CoreTypes.h>
#include "UploadResult.h"
#include <Core/3rdpart/FastDelegate.h>
#include <Core/Network/NetworkClient.h>
#include "ServerProfile.h"
#include "CommonTypes.h"
#include <mutex>
#include <deque>

class UploadTask;
class UploadSession;
struct UploadProgressTimeInfo
{
	int64_t ms; //time
	int64_t bytes;
};
class UploadProgress {
public:
	std::string statusText;
	int stage;
	int64_t uploaded;
	int64_t totalUpload;
	int64_t lastUpdateTime;
	bool isUploading;
	std::string speed;
	std::deque<UploadProgressTimeInfo> timeBytes;
	UploadProgress()
	{
		stage = 0;
		uploaded = 0;
		totalUpload = 0;
		lastUpdateTime = 0;
		isUploading = false;
	}
};

class UploadTaskAcceptor
{
public:
	virtual bool canAcceptUploadTask(UploadTask* task) = 0;
};

class UploadTask {
	public:
		UploadTask();
		UploadTask(UploadTask* parentTask);
		virtual ~UploadTask();

		enum Role { DefaultRole, ThumbRole, UrlShorteningRole };
		typedef fastdelegate::FastDelegate2<UploadTask*, bool> TaskFinishedCallback;

		virtual std::string getType() const = 0;
		virtual std::string getMimeType() const = 0;
		virtual int64_t getDataLength() const = 0;
		UploadTask* parentTask() const;
		bool isRunning();
		bool isRunningItself();
		void setSession(UploadSession* session);
		UploadSession* session();
		bool isFinished();
		bool isFinishedItself();
		virtual void setFinished(bool finished);
		void setRunning(bool running);
		int getNextTask(UploadTaskAcceptor *acceptor, std::shared_ptr<UploadTask>& outTask);
		int pendingTasksCount(UploadTaskAcceptor* acceptor);
		void addChildTask(std::shared_ptr<UploadTask> child);
		UploadResult* uploadResult();
		UploadProgress* progress();
		void addTaskFinishedCallback(const TaskFinishedCallback& callback);
		fastdelegate::FastDelegate1<UploadTask*> OnUploadProgress;
		fastdelegate::FastDelegate1<UploadTask*> OnStatusChanged;
		fastdelegate::FastDelegate1<UploadTask*> OnChildTaskAdded;
		std::string serverName() const;
		ServerProfile& serverProfile();
		void setServerProfile(ServerProfile profile);
		ServerProfile& urlShorteningServer();
		void setUrlShorteningServer(ServerProfile profile);
		void setUserData(void* data);
		void* userData() const;
		bool uploadSuccess(bool withChilds = true);
		void setUploadSuccess(bool success);
		Role role() const;
		void setRole(Role role);
		bool shorteningStarted() const;
		void setShorteningStarted(bool started);
		friend class CUploader;

	protected:
		UploadTask* parentTask_;
		std::vector<std::shared_ptr<UploadTask>> childTasks_;
		
		bool isRunning_;
		bool isFinished_;
		UploadResult uploadResult_;
		UploadProgress progress_;
		ServerProfile serverProfile_;
		ServerProfile urlShorteningProfile_;
		void* userData_;
		void init();
		void childTaskFinished(UploadTask* child);
		void uploadProgress(InfoProgress progress);
		void taskFinished();
		bool uploadSuccess_;
		UploadSession* session_;
		std::mutex tasksMutex_;
		Role role_;
		std::vector<TaskFinishedCallback> taskFinishedCallbacks_;
		bool shorteningStarted_;
};	

#endif