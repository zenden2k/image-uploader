#include "FolderTask.h"

FolderTask::FolderTask(FolderOperationType opType): operationType_(opType){
}

UploadTask::Type FolderTask::type() const {
    return TypeFolder;
}

std::string FolderTask::getMimeType() const {
    return {};
}

int64_t FolderTask::getDataLength() const {
    return 0;
}

std::string FolderTask::toString() {
    return "FolderTask";
}

std::string FolderTask::title() const {
    return "FolderTask";
}

int FolderTask::retryLimit() {
    return 1;
}

CFolderList& FolderTask::folderList() {
    return folderList_;
}

FolderOperationType FolderTask::operationType() const {
    return operationType_;
}

CFolderItem& FolderTask::folder() {
    return folder_;
}

void FolderTask::setFolder(const CFolderItem& item) {
    folder_ = item;
}