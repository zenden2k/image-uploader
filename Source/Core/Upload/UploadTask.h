#ifndef CORE_UPLOAD_UPLOADTASK_H
#define CORE_UPLOAD_UPLOADTASK_H

#include <string>
#include "Core/Utils/CoreTypes.h"
#include "UploadResult.h"
#include "Core/3rdpart/FastDelegate.h"
#include "Core/Network/NetworkClient.h"
#include "Core/Utils/EnumUtils.h"
#include "ServerProfile.h"
#include "Core/TempFileDeleter.h"
#include "CommonTypes.h"
#include <mutex>
#include <deque>

class CAbstractUploadEngine;
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
    StatusType statusType;
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

        DEFINE_MEMBER_ENUM_WITH_STRING_CONVERSIONS(Role, (DefaultRole)(ThumbRole)(UrlShorteningRole));
        DEFINE_MEMBER_ENUM_WITH_STRING_CONVERSIONS(Type, (TypeFile)(TypeUrl));
        //enum Role { DefaultRole, ThumbRole, UrlShorteningRole };
        enum Status { StatusInQueue, StatusRunning, StatusStopped, StatusFinished, StatusFailure, StatusPostponed };
        typedef fastdelegate::FastDelegate2<UploadTask*, bool> TaskFinishedCallback;

        virtual Type type() const = 0;
        virtual std::string getMimeType() const = 0;
        virtual int64_t getDataLength() const = 0;
        UploadTask* parentTask() const;
        std::shared_ptr<UploadTask> child(int index);
        bool isRunning();
        bool isRunningItself();
        void setSession(UploadSession* session);
        UploadSession* session();
        bool isFinished();
        bool isFinishedItself();
        virtual void finishTask(Status status = StatusFinished);
        int getNextTask(UploadTaskAcceptor *acceptor, std::shared_ptr<UploadTask>& outTask);
        int pendingTasksCount(UploadTaskAcceptor* acceptor);
        void addChildTask(std::shared_ptr<UploadTask> child);
        int childCount();
        UploadResult* uploadResult();
        UploadProgress* progress();
        void addTaskFinishedCallback(const TaskFinishedCallback& callback);
        fastdelegate::FastDelegate1<UploadTask*> OnUploadProgress;
        fastdelegate::FastDelegate1<UploadTask*> OnStatusChanged;
        fastdelegate::FastDelegate1<UploadTask*> OnChildTaskAdded;
        fastdelegate::FastDelegate1<UploadTask*> OnFolderUsed;
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
        void stop();
        virtual std::string title() const = 0;
        bool isStopped();
        void setStatus(Status status);
        void setStatusText(const std::string& text);
        Status status() const;
        virtual std::string toString() = 0;
        static std::string UploaderStatusToString(StatusType status, int actionIndex, std::string param);
        TempFileDeleter* tempFileDeleter(bool create = true);
        void addTempFile(const std::string& fileName);
        void deletePostponedChilds();
        bool schedulePostponedChilds();
        friend class CUploader;

    protected:
        UploadTask* parentTask_;
        std::vector<std::shared_ptr<UploadTask>> childTasks_;
        
        //bool isRunning_;
        //bool isFinished_;
        //bool isStopped_;
        UploadResult uploadResult_;
        UploadProgress progress_;
        ServerProfile serverProfile_;
        ServerProfile urlShorteningProfile_;
        CAbstractUploadEngine* currentUploadEngine_;
        void* userData_;
        void init();
        void childTaskFinished(UploadTask* child);
        void uploadProgress(InfoProgress progress);
        void taskFinished();
        void statusChanged();
        void setCurrentUploadEngine(CAbstractUploadEngine* currentUploadEngine);
        bool stopSignal() const;
        UploadSession* session_;
        std::recursive_mutex tasksMutex_;
        Role role_;
        std::vector<TaskFinishedCallback> taskFinishedCallbacks_;
        bool shorteningStarted_;
        volatile bool stopSignal_;
        bool uploadSuccess_;
        Status status_;
        TempFileDeleter* tempFileDeleter_;
};    

#endif