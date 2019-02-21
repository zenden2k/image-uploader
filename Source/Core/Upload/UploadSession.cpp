#include "UploadSession.h"

#include <algorithm>
#include "UploadTask.h"

UploadSession::UploadSession()
{
    finishedSignalSent_ = false;
    stopSignal_ = true;
    finishedCount_ = 0;
    isStopped_ = false;
}

void UploadSession::addTask(std::shared_ptr<UploadTask> task)
{
    //tasksMutex_.lock();
    task->setSession(this);
    tasks_.push_back(task);
    //tasksMutex_.unlock();
    notifyTaskAdded(task.get());
}

void UploadSession::removeTask(std::shared_ptr<UploadTask> task)
{
    //std::lock_guard<std::recursive_mutex> lock(tasksMutex_);
    auto it = std::find(tasks_.begin(), tasks_.end(), task);
    if (it != tasks_.end() )
        tasks_.erase(it);
}

int  UploadSession::getNextTask(UploadTaskAcceptor *acceptor, std::shared_ptr<UploadTask>& outTask)
{
    int count = 0;
    //LOG(ERROR) << "UploadSession::getNextTask()";
//    std::lock_guard<std::recursive_mutex> lock(tasksMutex_);
    for (auto it = tasks_.begin(); it != tasks_.end(); ++it)
    {
        UploadTask* uploadTask = it->get();
        if (uploadTask->status() == UploadTask::StatusInQueue) {
            count++;
            if (acceptor->canAcceptUploadTask(uploadTask))
            {
                outTask = *it;
                it->get()->setStatus(UploadTask::StatusPostponed); // FIXME: add new property instead of settings status = StatusPostponed
                return count;
            }
            
        }
        std::shared_ptr<UploadTask> task;
        int childCount = it->get()->getNextTask(acceptor, task);
        count += childCount;
        if (task)
        {
            task->setStatus(UploadTask::StatusPostponed);
            outTask = task;
            return count;
        }
    }
    return count;
}

bool UploadSession::isRunning()
{
    //std::lock_guard<std::recursive_mutex> lock(tasksMutex_);
    for (auto it = tasks_.begin(); it != tasks_.end(); ++it)
    {
        if (it->get()->isRunning())
        {
            return true;
        }
    }
    return false;
}

bool UploadSession::isFinished()
{
    try {
//        std::lock_guard<std::recursive_mutex> lock(tasksMutex_);
        //return tasks_.size() == finishedCount_;
        if (tasks_.empty())
        {
            return false;
        }
        for (size_t i = 0; i < tasks_.size(); i++)
        {
            auto it = tasks_[i];
            //tasksMutex_.unlock(); // FIXME!!! How to avoid deadlocks???
            if (!it->isFinished())
            {
                //tasksMutex_.lock();
                return false;
            }
            //tasksMutex_.lock();
        }
        return true;
    }
    catch (std::exception& ex)
    {
        LOG(ERROR) << ex.what();
    }
    return false;
    //return isFinished_;
}

int UploadSession::pendingTasksCount(UploadTaskAcceptor* acceptor)
{
    //std::lock_guard<std::recursive_mutex> lock(tasksMutex_);
    int res = 0;
    for (auto it = tasks_.begin(); it != tasks_.end(); it++)
    {
        UploadTask* uploadTask = it->get();
        if (uploadTask->status() == UploadTask::StatusInQueue && acceptor->canAcceptUploadTask(uploadTask)) {
            res++;
        }
        res += it->get()->pendingTasksCount(acceptor);
    }
    return res;
}

int UploadSession::taskCount()
{
    /*try {
        std::lock_guard<std::recursive_mutex> lock(tasksMutex_);
        return tasks_.size();
    } catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
    }*/
    return tasks_.size();
}

int UploadSession::finishedTaskCount(UploadTask::Status status)
{
    int res = 0;
    try {
        //std::lock_guard<std::recursive_mutex> lock(tasksMutex_);
    
        for (auto it = tasks_.begin(); it != tasks_.end(); ++it)
        {
            if (it->get()->isFinished() && it->get()->status() == status)
            {
                res++;
            }
        }
    }
    catch (std::exception& ex) {
        LOG(ERROR) << ex.what();
    }
    return res;
}

bool UploadSession::isStopped()
{
    return isStopped_;
}

void UploadSession::addSessionFinishedCallback(const SessionFinishedCallback& callback)
{
    sessionFinishedCallbacks_.push_back(callback);
}

void UploadSession::addTaskAddedCallback(const TaskAddedCallback& callback)
{
    taskAddedCallbacks_.push_back(callback);
}

void UploadSession::stop()
{
    stopSignal_ = true;
    //std::lock_guard<std::recursive_mutex> lock(tasksMutex_);
    for (auto it = tasks_.begin(); it != tasks_.end(); ++it)
    {
        it->get()->stop();

    }
}

void UploadSession::clearStopFlag() {
    stopSignal_ = false;
    for (auto it = tasks_.begin(); it != tasks_.end(); ++it) {
        it->get()->clearStopFlag();

    }
}

void UploadSession::restartFailedTasks() {
    if (isRunning()) {
        return;
    }
    finishedSignalSent_ = false;
    for (auto it = tasks_.begin(); it != tasks_.end(); ++it) {
        auto status = it->get()->status();

        if (status == UploadTask::StatusFailure || status == UploadTask::StatusStopped) {
            it->get()->setStatus(UploadTask::StatusInQueue);
        }
    }
}

bool UploadSession::isFatalErrorSet(const std::string& serverName, const std::string& profileName)
{
    std::lock_guard<std::mutex> lock(serverFatalErrorsMutex_);
    auto it = serverFatalErrors_.find(std::make_pair(serverName, profileName));
    if (it != serverFatalErrors_.end())
    {
        return it->second;
    }
    return false;
}

void UploadSession::setFatalErrorForServer(const std::string& serverName, const std::string& profileName)
{
    std::lock_guard<std::mutex> lock(serverFatalErrorsMutex_);
    serverFatalErrors_[std::make_pair(serverName, profileName)] = true;
}

void UploadSession::clearErrorsForServer(const std::string& serverName, const std::string& profileName)
{
    std::lock_guard<std::mutex> lock(serverFatalErrorsMutex_);
    auto it = serverFatalErrors_.find(std::make_pair(serverName, profileName));
    if (it != serverFatalErrors_.end())
    {
        serverFatalErrors_.erase(it);
    }
}

std::shared_ptr<UploadTask> UploadSession::getTask(int index)
{
    //std::lock_guard<std::recursive_mutex> lock(tasksMutex_);
    return tasks_[index];
}

void UploadSession::taskFinished(UploadTask* task)
{
    ++finishedCount_;
    std::lock_guard<std::mutex> lock(finishMutex_);
    if (!finishedSignalSent_ && isFinished()) {
        // TODO: use std::call_once
        for (const auto& it : sessionFinishedCallbacks_) {
            it(this);
        }
        finishedSignalSent_ = true;
    }
}

void UploadSession::childTaskAdded(UploadTask* task)
{
    finishedSignalSent_ = false;
    notifyTaskAdded(task);
}

bool UploadSession::stopSignal()
{
    return stopSignal_;
}

void UploadSession::notifyTaskAdded(UploadTask* task)
{
    for (size_t i = 0; i < taskAddedCallbacks_.size(); i++)
    {
        taskAddedCallbacks_[i](this, task);
    }
}