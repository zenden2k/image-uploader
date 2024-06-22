#include "AbstractCodeGenerator.h"

namespace ImageUploader::Core::OutputGenerator {

std::string AbstractCodeGenerator::generate(const std::vector<UploadObject>& items) {
    std::string result;
    std::vector<std::string> groups;
    std::vector<std::string> fileNames;
    for (size_t i = 0; i < items.size(); i++) {
        auto& item = items[i];
        if (item.isNull()) {
            continue;
        }
        size_t index = item.fileIndex;
        if (groups.size() <= index) {
            groups.resize(index + 1);
            fileNames.resize(index + 1);
        }

        std::string& cur = groups[index];

        if (i) {
            cur += "\r\n";
        }

        fileNames[index] = item.onlyFileName();
        cur += generateCodeForItem(item, i);
    }

    for (size_t i = 0; i < groups.size(); i++) {
        result += group(fileNames[i], groups[i], i);
    }

    return result;
}

std::string AbstractCodeGenerator::generateCodeForItem(const UploadObject& item, int index) {
    std::string url = (!item.getImageUrl(shortenUrl_).empty() && preferDirectLinks_ || item.getDownloadUrl(shortenUrl_).empty()) ? item.getImageUrl(shortenUrl_) : item.getDownloadUrl(shortenUrl_);

    if (lang() == clPlain) {
        return url;
    } else {     
        if ((codeType_ == ctClickableThumbnails || codeType_ == ctTableOfThumbnails) ) {
            if (!item.getThumbUrl(shortenUrl_).empty()) {
                return link(url, image(item.thumbUrl, item.onlyFileName()));
            }  else {
                return link(url, groupByFile_? item.serverName : item.onlyFileName());
            }
        }
        else if (codeType_ == ctImages) {
            if (!item.getImageUrl(shortenUrl_).empty() && (preferDirectLinks_ || item.getDownloadUrl(shortenUrl_).empty())) {
                return image(item.getImageUrl(shortenUrl_), item.onlyFileName());
            }
            else {
                return link(url, item.onlyFileName());
            }
        }

        else
        // if (m_CodeType == ctLinks ||  item.directUrl.empty())
            return link(url, item.onlyFileName());
    }
    return {};
}

std::string AbstractCodeGenerator::image(const std::string& url, const std::string& alt){
    return {};
}

std::string AbstractCodeGenerator::link(const std::string& url, const std::string& body){
    return {};
}

std::string AbstractCodeGenerator::group(const std::string& fileName, const std::string& content, size_t index){
    std::string result;
    if (index) {
        result += "\r\n";
    }
    result += content;
    return result;
}

}
