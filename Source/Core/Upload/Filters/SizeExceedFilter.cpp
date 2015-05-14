#include "SizeExceedFilter.h"

#include "Gui/Dialogs/SizeExceed.h"
#include "Core/Upload/UploadEngine.h"
#include "Core/Upload/FileUploadTask.h"
#include "Core/ServiceLocator.h"
#include "Core/Scripting/DialogProvider.h"

SizeExceedFilter::SizeExceedFilter(CUploadEngineList* engineList, UploadEngineManager* uploadEngineManager) {
    engineList_ = engineList;
    uploadEngineManager_ = uploadEngineManager;
}

bool SizeExceedFilter::PreUpload(UploadTask* task) {
    if (task->type() == UploadTask::TypeFile) {
        FileUploadTask * fileTask = dynamic_cast<FileUploadTask*>(task);
        CUploadEngineData* ue = fileTask->serverProfile().uploadEngineData();
        if (ue) {
            if (ue->MaxFileSize && fileTask->getFileSize() > ue->MaxFileSize) {
                std::lock_guard<std::mutex> guard(ServiceLocator::instance()->dialogProvider()->dialogMutex_);
                CSizeExceed SE(fileTask, engineList_, uploadEngineManager_);
                HWND parent = ServiceLocator::instance()->programWindow()->getNativeHandle();
                int res = SE.DoModal(parent);
                if (res == IDOK || res == 3) {
                    if (res == 3) {
                        //InitialParams = iss; // if user choose button USE FOR ALL
                    }

                }
            }
        }
    }
    
    return true;
}

bool SizeExceedFilter::PostUpload(UploadTask* task) {
    return true;
}