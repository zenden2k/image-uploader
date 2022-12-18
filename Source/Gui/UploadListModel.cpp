#include "UploadListModel.h"

#include "Core/Upload/UploadSession.h"
#include "Core/Upload/FileUploadTask.h"
#include "Core/ServiceLocator.h"
#include "Core/i18n/Translator.h"
#include "Core/Upload/UrlShorteningTask.h"

UploadListModel::UploadListModel(std::shared_ptr<UploadSession> session) {
    int n = session->taskCount();
    for (int i = 0; i < n; i++) {
        auto task = session->getTask(i);
        auto fileTask = std::dynamic_pointer_cast<FileUploadTask>(task);
        using namespace std::placeholders;
        task->setOnUploadProgressCallback(std::bind(&UploadListModel::onTaskUploadProgress, this, _1));
        task->setOnStatusChangedCallback(std::bind(&UploadListModel::onTaskStatusChanged, this, _1));
        task->onTaskFinished.connect(std::bind(&UploadListModel::onTaskFinished, this, _1, _2));
        task->onChildTaskAdded.connect(std::bind(&UploadListModel::onChildTaskAdded, this, _1));
        UploadListItem *sd = new UploadListItem();
        sd->tableRow = i;
        sd->setStatusText(TR("In queue"));
        sd->setFileName(U2W(fileTask->getFileName()));
        sd->setDisplayName(U2W(fileTask->getDisplayName()));
        task->setUserData(sd);
        items_.push_back(sd);
    }
}

UploadListModel::~UploadListModel() {
    for (auto* it : items_) {
        delete it;
    }
}

CString UploadListModel::getItemText(int row, int column) const {
    if (row >= static_cast<int>(items_.size())) {
        return CString();
    }
    UploadListItem* serverData = items_[row];
    if (column == 0) {
        return serverData->displayName();
    } 
    if (column == 1) {
        return serverData->statusText();
    } 
    if (column == 2) {
        return serverData->thumbStatusText();
    }
    return CString();
}

COLORREF UploadListModel::getItemTextColor(int row) const {
    const UploadListItem& serverData = *items_[row];
    return serverData.color;
}

size_t UploadListModel::getCount() const {
    return items_.size();
}

void UploadListModel::notifyRowChanged(size_t row) {
    if (row < items_.size() && rowChangedCallback_) {
        rowChangedCallback_(row);
    }
}

UploadListItem* UploadListModel::getDataByIndex(size_t row) {
    if (row >= items_.size()) {
        return nullptr;
    }
    return items_[row];
}

void UploadListModel::setOnRowChangedCallback(std::function<void(size_t)> callback) {
    rowChangedCallback_ = std::move(callback);
}

void UploadListModel::resetData() {
    for (auto& it : items_) {
        it->clearInfo();
    }
}

// This callback is being executed in worker thread
void UploadListModel::onTaskUploadProgress(UploadTask* task) {
    auto* fps = static_cast<UploadListItem*>(task->role() == UploadTask::DefaultRole ? task->userData() : task->parentTask()->userData());
    if (!fps) {
        return;
    }
    auto* fileTask = dynamic_cast<FileUploadTask*>(task);
    if (fileTask) {
        bool isThumb = task->role() == UploadTask::ThumbRole;
        int percent = 0;
        UploadProgress* progress = task->progress();
        if (progress->totalUpload) {
            percent = static_cast<int>(100 * ((float)progress->uploaded) / progress->totalUpload);
        }
        CString uploadSpeed = U2W(progress->speed);
        CString progressText;
        progressText.Format(TR("%s of %s (%d%%) %s"), (LPCTSTR)U2W(IuCoreUtils::fileSizeToString(progress->uploaded)),
            (LPCTSTR)U2W(IuCoreUtils::fileSizeToString(progress->totalUpload)), percent, uploadSpeed.GetString());

        if (isThumb) {
            fps->setThumbStatusText(progressText);
        } else {
            fps->setStatusText(progressText);
        }
        notifyRowChanged(fps->tableRow);
    }
}

void UploadListModel::onTaskStatusChanged(UploadTask* task) {
    UploadProgress* progress = task->progress();
    UploadListItem* fps = static_cast<UploadListItem*>(task->role() == UploadTask::DefaultRole ? task->userData() : task->parentTask()->userData());
    if (!fps) {
        return;
    }

    FileUploadTask* fileTask = dynamic_cast<FileUploadTask*>(task);
    if (fileTask) {
        CString statusText = IuCoreUtils::Utf8ToWstring(progress->statusText).c_str();

        bool isThumb = task->role() == UploadTask::ThumbRole;
        if (isThumb) {
            fps->setThumbStatusText(statusText);
        } else {
            fps->setStatusText(statusText);
        }
        notifyRowChanged(fps->tableRow);
    }

    UrlShorteningTask* urlTask = dynamic_cast<UrlShorteningTask*>(task);
    if (urlTask) {
        UploadTask* parentTask = urlTask->parentTask();
        if (urlTask->isFinished() && parentTask && parentTask->isFinished()) {
            CString statusText = U2W(parentTask->progress()->statusText);
            fps->setStatusText(statusText);
            notifyRowChanged(fps->tableRow);
        }
    }
}

// This callback is being executed in worker thread
void UploadListModel::onTaskFinished(UploadTask* task, bool ok) {
    FileUploadTask* fileTask = dynamic_cast<FileUploadTask*>(task);
    if (!fileTask) {
        return;
    }
    if (fileTask->role() == UploadTask::ThumbRole) {
        UploadListItem* fps = static_cast<UploadListItem*>(task->parentTask()->userData());
        if (!fps) {
            return;
        }
        fps->setThumbStatusText(TR("Finished"));
        notifyRowChanged(fps->tableRow);
    }
}

void UploadListModel::onChildTaskAdded(UploadTask* child) {
    if (child->role() == UploadTask::UrlShorteningRole) {
        UploadListItem* fps = static_cast<UploadListItem*>(child->parentTask()->userData());
        fps->setStatusText(TR("Shortening link..."));
        notifyRowChanged(fps->tableRow);
    }
    using namespace std::placeholders;
    child->onTaskFinished.connect(std::bind(&UploadListModel::onTaskFinished, this, _1, _2));
    child->setOnUploadProgressCallback(std::bind(&UploadListModel::onTaskUploadProgress, this, _1));
    child->setOnStatusChangedCallback(std::bind(&UploadListModel::onTaskStatusChanged, this, _1));
}