#include "AbstractOutputGenerator.h"

#include "Core/Upload/FileUploadTask.h"

namespace ImageUploader::Core::OutputGenerator {

void UploadObject::fillFromUploadResult(UploadResult* res, UploadTask* task) {
    uploadResult = *res;
    auto* fileTask = dynamic_cast<FileUploadTask*>(task);
    if (fileTask) {
        localFilePath = fileTask->getFileName();
        displayFileName = fileTask->getDisplayName();
        if (displayFileName.empty()) {
            displayFileName = IuCoreUtils::ExtractFileName(localFilePath);
        }
        fileIndex = fileTask->index();
        //it.ServerName = U2W(lastUploadedItem_->serverName());
    }
    uploadFileSize = task->getDataLength();
}

void AbstractOutputGenerator::setType(CodeType type) {
    codeType_ = type;
}

void AbstractOutputGenerator::setPreferDirectLinks(bool prefer) {
    preferDirectLinks_ = prefer;
}

void AbstractOutputGenerator::setShortenUrl(bool shorten) {
    shortenUrl_ = shorten;
}

void AbstractOutputGenerator::setGroupByFile(bool group) {
    groupByFile_ = group;
}

void AbstractOutputGenerator::setItemsPerLine(int n) {
    itemsPerLine_ = n;
}

}
