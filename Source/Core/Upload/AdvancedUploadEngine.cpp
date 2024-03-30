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

#include "AdvancedUploadEngine.h"


#include "FolderTask.h"
#include "Core/Upload/ServerSync.h"

CAdvancedUploadEngine::CAdvancedUploadEngine(ServerSync* serverSync, ServerSettingsStruct* settings, ErrorMessageCallback errorCallback) :
                        CAbstractUploadEngine(serverSync, std::move(errorCallback)),
                            m_CurrentActionIndex(0), 
                            m_nThumbWidth(0)
{
    setServerSettings(settings);
}

CAdvancedUploadEngine::~CAdvancedUploadEngine()
{
}

int CAdvancedUploadEngine::processFolderTask(std::shared_ptr<UploadTask> task) {
    auto folderTask = std::dynamic_pointer_cast<FolderTask>(task);
    assert(folderTask != nullptr);
    if (folderTask->operationType() == FolderOperationType::foGetFolders) {
        return getFolderList(folderTask->folderList());
    } else if (folderTask->operationType() == FolderOperationType::foCreateFolder) {
        return createFolder(CFolderItem(), folderTask->folder());
    } else if (folderTask->operationType() == FolderOperationType::foModifyFolder) {
        return modifyFolder(folderTask->folder());
    } else {
        LOG(ERROR) << "Not implemented";
    }

    return 0;
}

int CAdvancedUploadEngine::processTask(std::shared_ptr<UploadTask> task, UploadParams& params)
{
    int res = doProcessTask(task, params);
    if (res == -2) {
        // Clear authorization flag and try again
        serverSync_->resetAuthorization();
    }
    /*  res = doProcessTask(task, params);
        if (res == -2) {
            return -1;
        }
    }*/
    return res;
}

int CAdvancedUploadEngine::doProcessTask(std::shared_ptr<UploadTask> task, UploadParams& params) {
    return 0;
}

void CAdvancedUploadEngine::setNetworkClient(INetworkClient* nm)
{
    CAbstractUploadEngine::setNetworkClient(nm);
}

bool CAdvancedUploadEngine::supportsSettings()
{
    return false;
}

bool CAdvancedUploadEngine::supportsBeforehandAuthorization()
{
    return false;
}

void CAdvancedUploadEngine::log(ErrorInfo::MessageType mt, const std::string& error)
{
    ErrorInfo ei;
    ei.ActionIndex = -1;
    ei.messageType = mt;
    ei.errorType = etUserError;
    ei.error = error;
    ei.sender = "CAdvancedUploadEngine";
    if (m_UploadData) {
        ei.ServerName = m_UploadData->Name;
    }
    if (currentTask_) {
        ei.FileName = currentTask_->toString();
    }
    
    ErrorMessage(ei);
}

int CAdvancedUploadEngine::RetryLimit()
{
    return m_UploadData->RetryLimit;
}
