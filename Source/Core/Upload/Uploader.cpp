/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2018 Sergey Svistunov (zenden2k@yandex.ru)

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
#include <cstdlib>
#include <ctime>
#include "Core/Upload/FileUploadTask.h"
#include <cmath>

CUploader::CUploader(std::shared_ptr<INetworkClientFactory> networkClientFactory)
{
    srand((unsigned int)time(0));
    m_bShouldStop = false;
    m_CurrentStatus = stNone;
    m_CurrentEngine = NULL;
    m_PrInfo.IsUploading = false;
    m_PrInfo.Total = 0;
    m_PrInfo.Uploaded = 0;
    isFatalError_ = false;
    m_NetworkClient = networkClientFactory->create();
}

CUploader::~CUploader(void)
{
}

void CUploader::Cleanup()
{
    m_CurrentEngine->onDebugMessage.clear();
    m_CurrentEngine->onNeedStop.clear();
    m_CurrentEngine->onStatusChanged.clear();
    m_CurrentEngine->onErrorMessage.clear();
}

int CUploader::pluginProgressFunc (INetworkClient* nc, double dltotal, double dlnow, double ultotal, double ulnow)
{
    CUploader* uploader = this;

    if (!uploader)
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
    uploader->currentTask_->uploadProgress(uploader->m_PrInfo);

    if (uploader->onProgress)
        uploader->onProgress(uploader, uploader->m_PrInfo);
    return 0;
}

bool CUploader::UploadFile(const std::string& FileName, const std::string displayFileName) {
    return Upload(std::make_shared<FileUploadTask>(FileName, displayFileName));
}

bool CUploader::Upload(std::shared_ptr<UploadTask> task) {
    isFatalError_ = false;
    if (!m_CurrentEngine) {
        Error(true, "Cannot proceed: m_CurrentEngine is NULL!");
        return false;
    }
    std::string FileName;
    currentTask_ = task;


    if (task->type() == UploadTask::TypeFile) {
        FileName = static_cast<FileUploadTask*>(task.get())->getFileName();
        if ( FileName.empty() ) {
            Error(true, "Empty filename!");
            return false;
        }

        if ( ! IuCoreUtils::FileExists (FileName) ) {
            Error(true, "File \""+FileName+"\" doesn't exist!");
            return false;
        }
    }
    m_PrInfo.IsUploading = false;
    m_PrInfo.Total = 0;
    m_PrInfo.Uploaded = 0;
    m_FileName = FileName;
    m_bShouldStop = false;
    if (onConfigureNetworkClient)
        onConfigureNetworkClient(this, m_NetworkClient.get());
    m_NetworkClient->setLogger(nullptr);

    m_CurrentEngine->setNetworkClient(m_NetworkClient.get());
    m_CurrentEngine->onDebugMessage.bind(this, &CUploader::DebugMessage);
    m_CurrentEngine->onNeedStop.bind(this, &CUploader::needStop);
    m_CurrentEngine->onStatusChanged.bind(this, &CUploader::SetStatus);
    m_CurrentEngine->onErrorMessage.bind(this, &CUploader::ErrorMessage);
    m_CurrentEngine->setCurrentUploader(this);

    task->setCurrentUploadEngine(m_CurrentEngine);

    UploadParams uparams;
#ifdef IU_WTL
    ImageUploadParams imageUploadParams = task->serverProfile().getImageUploadParams();
    ThumbCreatingParams& tcParams = imageUploadParams.getThumbRef();
    if (tcParams.Size) {
        if (tcParams.ResizeMode != ThumbCreatingParams::trByHeight) {
            uparams.thumbWidth = tcParams.Size;
        } else {
            uparams.thumbHeight = tcParams.Size;
        }
    }
#else
    uparams.thumbWidth = 180;
#endif
    /*if (task->type() == UploadTask::TypeFile) {
        FileUploadTask* fileTask = dynamic_cast<FileUploadTask*>(task.get());
    }*/
    m_NetworkClient->setProgressCallback(INetworkClient::ProgressCallback(this, &CUploader::pluginProgressFunc));
    int EngineRes = 0;
    int i = 0;
    do
    {
        if (needStop())
        {
            Cleanup();
            return false;
        }
        EngineRes = m_CurrentEngine->doUpload(task, uparams);
        task->setCurrentUploadEngine(nullptr);

        if ( EngineRes == -1 ) {
            isFatalError_ = true;
            Cleanup();
            return false;
        }
        i++;
        if (needStop())
        {
            Cleanup();
            return false;
        }
        if (!EngineRes && i != m_CurrentEngine->RetryLimit())
        {
            Error(false, "", etRepeating, i);
        }
    }
    while (!EngineRes && i < m_CurrentEngine->RetryLimit());

    if (!EngineRes)
    {
        Error(true, "", etRetriesLimitReached);
        Cleanup();
        return false;
    }
    UploadResult* result = task->uploadResult();
    result->directUrl = uparams.DirectUrl;
    result->downloadUrl = uparams.ViewUrl;
    if (result->thumbUrl.empty()) {
        result->thumbUrl = uparams.ThumbUrl;
    }
    result->deleteUrl = uparams.DeleteUrl;
    result->editUrl = uparams.EditUrl;

    if (result->directUrl.empty() && result->downloadUrl.empty() && result->editUrl.empty())
    {
        return false;
    }
    return true;
}

bool CUploader::setUploadEngine(CAbstractUploadEngine* UploadEngine)
{
    if (m_CurrentEngine == UploadEngine)
        return true;
    m_CurrentEngine = UploadEngine;
    return true;
}

void CUploader::SetStatus(StatusType status, int param1, std::string param)
{
    m_CurrentStatus = status;
    if (onStatusChanged)
        onStatusChanged(this, status, param1,  param);
}

StatusType CUploader::GetStatus() const
{
    return m_CurrentStatus;
}

bool CUploader::isFatalError() const
{
    return isFatalError_;
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
    if (onNeedStop)
        m_bShouldStop = onNeedStop();  // delegate call
    return m_bShouldStop;
}

std::shared_ptr<UploadTask> CUploader::currentTask() const
{
    return currentTask_;
}

void CUploader::DebugMessage(const std::string& message, bool isServerResponseBody)
{
    if (onDebugMessage)
        onDebugMessage(this, message, isServerResponseBody);
}

void CUploader::ErrorMessage(const ErrorInfo& error)
{
    if (onErrorMessage)
        onErrorMessage(this, error);
}

void CUploader::Error(bool error, std::string message, ErrorType type, int retryIndex)
{
    ErrorInfo err;
    err.ActionIndex  = -1;
    err.messageType = error ? ErrorInfo::mtError : ErrorInfo::mtWarning;
    err.error = message;
    err.FileName = m_FileName;
    err.errorType = type;
    err.sender = "CUploader";
    err.RetryIndex = retryIndex;
    ErrorMessage(err);
}
