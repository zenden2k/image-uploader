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

bool AbstractOutputGenerator::loadTemplate(const std::string& templateFileName){
    if (templateHead_ || templateFoot_) {
        return true;
    }

    std::string fileContents = IuCoreUtils::GetFileContents(templateFileName);

    if (fileContents.empty()) {
        return false;
    }

    std::string pattern = "%images%";
    auto it = fileContents.find(pattern);

    if (it != std::string::npos) {
        templateHead_ = fileContents.substr(0, it);
        templateFoot_ = fileContents.substr(it + pattern.length());
    }

    return true;
}

std::string AbstractOutputGenerator::generate(const std::vector<UploadObject>& items, bool withTemplate /*= true*/) {
    std::string res;
    if (withTemplate && templateHead_) {
        res += *templateHead_;
    }

    res += doGenerate(items);

    if (withTemplate && templateFoot_) {
        res += *templateFoot_;
    }
    return res;
}

}
