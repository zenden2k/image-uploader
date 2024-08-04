/*

Image Uploader -  free application for uploading images/files to the Internet

Copyright 2007-2024 Sergey Svistunov (zenden2k@gmail.com)

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

#include "ImageSearchFilter.h"

#include "../TempImageUploadTask.h"
#include "../FileUploadTask.h"
#include "../SearchByImageUrlTask.h"

bool ImageSearchFilter::PreUpload(UploadTask* task) {
    return true;
}

bool ImageSearchFilter::PostUpload(UploadTask* task) {
    if (task->parentTask() || (!task->uploadSuccess(false)) /* || task->shorteningStarted()*/ || task->type() != UploadTask::TypeFile) {
        return true;
    }
    auto tempTask = dynamic_cast<TempImageUploadTask*>(task);
    if (!tempTask) {
        return true;
    }
    
    const ServerProfile& server = tempTask->searchServerProfile();
    if (server.isNull()) {
        LOG(ERROR) << "Image search server is not set";
        return true;
    }

    std::string directUrl = tempTask->uploadResult()->directUrl;
    std::string downloadUrl = tempTask->uploadResult()->downloadUrl;

    if (!directUrl.empty()) {
        auto imgSearchTask = std::make_shared<SearchByImageUrlTask>(directUrl, task);
        imgSearchTask->setServerProfile(server);
        //shorteningTask->setRole(UploadTask::UrlShorteningRole);
        //task->setShorteningStarted(true);
        tempTask->addChildTask(imgSearchTask);
    } else {
        LOG(ERROR) << "Cannot search by image. The temporary server did not return direct image link.";
        return false;
    }
    
    return true;
}
