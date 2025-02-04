/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2018 Sergey Svistunov (zenden2k@gmail.com)

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

*/

#include "Uploader.h"

#include <cmath>

#include "Core/Upload/FileUploadTask.h"

CUploader::CUploader(std::shared_ptr<INetworkClientFactory> networkClientFactory)
{
    m_bShouldStop = false;
    m_CurrentStatus = stNone;
    m_CurrentEngine = nullptr;
    m_PrInfo.IsUploading = false;
    m_PrInfo.Total = 0;
    m_PrInfo.Uploaded = 0;
    isFatalServerError_ = false;
    m_NetworkClient = networkClientFactory->create();
}

CUploader::~CUploader()
{
}

void CUploader::Cleanup()
{
    m_CurrentEngine->setOnDebugMessageCallback(nullptr);
    m_CurrentEngine->setOnNeedStopCallback(nullptr);
    m_CurrentEngine->setOnStatusChangedCallback(nullptr);
    m_CurrentEngine->setOnErrorMessageCallback(nullptr);
}

int CUploader::pluginProgressFunc (INetworkClient* nc, double dltotal, double dlnow, double ultotal, double ulnow)
{
    CUploader* uploader = this;
    NetworkClient* networkClient = dynamic_cast<NetworkClient*>(nc);

    if (!uploader || !networkClient)
        return 0;

    if (uploader->needStop())
        return -1;

    if (ultotal < 0 || ulnow < 0)
        return 0;

    /*CString format;
    format.Format(L"Total =  %d Current = %d\r\n", (int)ultotal,(int)ulnow );
    OutputDebugStringW(format);*/

    if ( ultotal != 0 && ulnow == 0 && uploader->m_CurrentStatus == stWaitingAnswer ) {
            uploader->SetStatus(stUploading);
    }
    if (fabs(ultotal - ulnow) < 1)
    {
        uploader->m_PrInfo.IsUploading = false;
        uploader->m_PrInfo.Total = static_cast<uint64_t>(ultotal);
        uploader->m_PrInfo.Uploaded = static_cast<uint64_t>(ulnow);

        if (ultotal != 0 && uploader->m_CurrentStatus == stUploading) {
            //OutputDebugStringW(L"Set status waiting\r\n");
            uploader->SetStatus(stWaitingAnswer);
        } else {
            /*format.Format(L"CurrentStatus = %d\r\n", uploader->m_CurrentStatus );
            OutputDebugStringW(format);*/
        }
    }
    else
    {
        uploader->m_PrInfo.IsUploading = true;
        uploader->m_PrInfo.Total = static_cast<uint64_t>(ultotal);
        uploader->m_PrInfo.Uploaded = static_cast<uint64_t>(ulnow);
    }
    uploader->m_PrInfo.IsUploading = networkClient->currrentActionType() == NetworkClient::ActionType::atUpload;
    uploader->currentTask_->uploadProgress(uploader->m_PrInfo);

    if (uploader->onProgress_) {
        uploader->onProgress_(uploader, uploader->m_PrInfo);
    }
    return 0;
}

bool CUploader::UploadFile(const std::string& FileName, const std::string& displayFileName) {
    return Upload(std::make_shared<FileUploadTask>(FileName, displayFileName));
}

bool CUploader::Upload(std::shared_ptr<UploadTask> task) {
    isFatalServerError_ = false;
    if (!m_CurrentEngine) {
        Error(true, "Cannot proceed: m_CurrentEngine is NULL!");
        return false;
    }
    std::string FileName;
    currentTask_ = task;

    if (task->type() == UploadTask::TypeFile) {
        FileName = dynamic_cast<FileUploadTask*>(task.get())->getFileName();
        if ( FileName.empty() ) {
            Error(true, "Empty filename!");
            return false;
        }

        if ( ! IuCoreUtils::FileExists (FileName) ) {
            Error(true, "File \""+FileName+"\" doesn't exist!");
            return false;
        }
    }
    UploadTask* topLevelTask = task->parentTask() ? task->parentTask() : task.get();
    auto* topLevelFileTask = dynamic_cast<FileUploadTask*>(topLevelTask);
    std::string topLevelFileName;
    if (topLevelFileTask) {
        topLevelFileName = topLevelFileTask->getFileName();
    }
    m_PrInfo.IsUploading = false;
    m_PrInfo.Total = 0;
    m_PrInfo.Uploaded = 0;
    m_FileName = FileName;
    m_bShouldStop = false;
    if (onConfigureNetworkClient_) {
        onConfigureNetworkClient_(this, m_NetworkClient.get());
    }
    m_NetworkClient->setLogger(nullptr);

    m_CurrentEngine->setNetworkClient(m_NetworkClient.get());
    using namespace std::placeholders;
    m_CurrentEngine->setOnDebugMessageCallback(std::bind(&CUploader::DebugMessage, this, _1, _2));
    m_CurrentEngine->setOnNeedStopCallback(std::bind(&CUploader::needStop, this));
    m_CurrentEngine->setOnStatusChangedCallback(std::bind(&CUploader::SetStatus, this, _1, _2, _3));
    m_CurrentEngine->setOnErrorMessageCallback(std::bind(&CUploader::ErrorMessage, this, _1));
    m_CurrentEngine->setCurrentUploader(this);

    task->setCurrentUploadEngine(m_CurrentEngine);

    UploadParams uparams;

    ImageUploadParams imageUploadParams = task->serverProfile().getImageUploadParams();
    ThumbCreatingParams& tcParams = imageUploadParams.getThumbRef();

    if (tcParams.ResizeMode == ThumbCreatingParams::trByBoth || tcParams.ResizeMode == ThumbCreatingParams::trByWidth) {
        uparams.thumbWidth = tcParams.Width;
    }
    if (tcParams.ResizeMode == ThumbCreatingParams::trByBoth || tcParams.ResizeMode == ThumbCreatingParams::trByHeight) {
        uparams.thumbHeight = tcParams.Height;
    }

    /*if (task->type() == UploadTask::TypeFile) {
        FileUploadTask* fileTask = dynamic_cast<FileUploadTask*>(task.get());
    }*/
    m_NetworkClient->setProgressCallback(std::bind(&CUploader::pluginProgressFunc, this, _1, _2, _3, _4, _5));
    ResultCode EngineRes = ResultCode::Failure;
    int retryLimit = task->retryLimit();
    if (!retryLimit) {
        retryLimit = m_CurrentEngine->RetryLimit();
    }
    int i = 0;
    do
    {
        if (needStop())
        {
            Cleanup();
            return false;
        }
        EngineRes = static_cast<ResultCode>(m_CurrentEngine->processTask(task, uparams));
        task->setCurrentUploadEngine(nullptr);

        switch (EngineRes) {
        case ResultCode::FatalServerError:
            isFatalServerError_ = true;
            Cleanup();
            return false;
        case ResultCode::TryAgain:
            if (i == retryLimit) {
                retryLimit++;
            }
            break;
        case ResultCode::FatalError:
            i = std::max(i, retryLimit - 1);
            break;
        default:
            break;
        }

        i++;
        if (needStop())
        {
            Cleanup();
            return false;
        }
        if (EngineRes < ResultCode::Success && i != retryLimit)
        {
            Error(false, "", etRepeating, i, task.get(), topLevelFileName);
        }
    } while (EngineRes < ResultCode::Success && i < retryLimit);

    if (EngineRes == ResultCode::Failure || EngineRes == ResultCode::FatalError) {
        Error(true, "", etRetriesLimitReached, -1, task.get(), topLevelFileName);
        Cleanup();
        return false;
    }
    if (task->type() == UploadTask::TypeFile || task->type() == UploadTask::TypeUrl) {
        UploadResult* result = task->uploadResult();
        result->directUrl = uparams.DirectUrl;
        result->downloadUrl = uparams.ViewUrl;
        if (result->thumbUrl.empty()) {
            result->thumbUrl = uparams.ThumbUrl;
        }
        result->deleteUrl = uparams.DeleteUrl;
        result->editUrl = uparams.EditUrl;

        return !(result->directUrl.empty() && result->downloadUrl.empty() && result->editUrl.empty());
    } else {
        return EngineRes == ResultCode::Success;
    }
}

bool CUploader::setUploadEngine(CAbstractUploadEngine* UploadEngine)
{
    if (m_CurrentEngine == UploadEngine)
        return true;
    m_CurrentEngine = UploadEngine;
    return true;
}

void CUploader::SetStatus(StatusType status, int param1, const std::string& param)
{
    m_CurrentStatus = status;
    if (onStatusChanged_) {
        onStatusChanged_(this, status, param1, param);
    }
}

StatusType CUploader::GetStatus() const
{
    return m_CurrentStatus;
}

bool CUploader::isFatalError() const
{
    return isFatalServerError_;
}

CAbstractUploadEngine* CUploader::getUploadEngine()
{
    return m_CurrentEngine;
}

void CUploader::stop()
{
    m_bShouldStop = true;
}

bool CUploader::needStop()
{
    if (m_bShouldStop)
        return m_bShouldStop;
    if (currentTask_->stopSignal())
    {
        m_bShouldStop = true;
        return m_bShouldStop;
    }
    if (onNeedStop_) {
    m_bShouldStop = onNeedStop_();  // delegate call
    }
    return m_bShouldStop;
}

std::shared_ptr<UploadTask> CUploader::currentTask() const
{
    return currentTask_;
}

void CUploader::setOnNeedStopCallback(std::function<bool()> cb) {
    onNeedStop_ = cb;
}

void CUploader::setOnProgress(std::function<void(CUploader*, InfoProgress)> cb) {
    onProgress_ = cb;
}

void CUploader::setOnStatusChanged(std::function<void(CUploader*, StatusType, int, std::string)> cb) {
    onStatusChanged_ = cb;
}

void CUploader::setOnDebugMessage(std::function<void(CUploader*, const std::string&, bool)> cb) {
    onDebugMessage_ = cb;
}

void CUploader::setOnErrorMessage(std::function<void(CUploader*, ErrorInfo)> cb) {
    onErrorMessage_ = cb;
}

void CUploader::setOnConfigureNetworkClient(std::function<void(CUploader*, INetworkClient*)> cb) {
    onConfigureNetworkClient_ = cb;
}

void CUploader::DebugMessage(const std::string& message, bool isServerResponseBody)
{
    if (onDebugMessage_) {
        onDebugMessage_(this, message, isServerResponseBody);
    }
}

void CUploader::ErrorMessage(const ErrorInfo& error)
{
    if (onErrorMessage_) {
        onErrorMessage_(this, error);
    }
}

void CUploader::Error(bool error, std::string message, ErrorType type, int retryIndex, UploadTask* uploadTask, const std::string& topLevelFileName)
{
    ErrorInfo err;
    err.ActionIndex  = -1;
    err.messageType = error ? ErrorInfo::mtError : ErrorInfo::mtWarning;
    err.error = message;
    err.FileName = m_FileName;
    err.errorType = type;
    err.sender = "CUploader";
    err.RetryIndex = retryIndex;
    err.TopLevelFileName = topLevelFileName;
    if (uploadTask) {
        err.ServerName = uploadTask->serverName();
    }
    ErrorMessage(err);
}
