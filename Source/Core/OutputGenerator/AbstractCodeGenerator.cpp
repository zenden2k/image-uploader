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
        std::string res;
        if ((codeType_ == ctClickableThumbnails || codeType_ == ctTableOfThumbnails) ) {
            if (!item.getThumbUrl(shortenUrl_).empty()) {
                res = link(url, image(item.getThumbUrl(shortenUrl_), item.onlyFileName()));
            }  else {
                res = link(url, groupByFile_? item.uploadResult.serverName : item.onlyFileName());
            }

            if (codeType_ == ctTableOfThumbnails) {
                if ((index + 1) % itemsPerLine_) {
                    res += itemSeparator();
                } else {
                    res += rowSeparator();
                }

            }
            return res;
        }
        else if (codeType_ == ctImages) {
            if (index && codeType_ == ctImages) {
                res += lineSeparator();
                res += lineSeparator();
            }
            if (!item.getImageUrl(shortenUrl_).empty() && (preferDirectLinks_ || item.getDownloadUrl(shortenUrl_).empty())) {
                res += image(item.getImageUrl(shortenUrl_), item.onlyFileName());
            }
            else {
                res += link(url, item.onlyFileName());
            }
            
            return res;
        } else {
            if (index) {
                res += lineSeparator();
                res += lineSeparator();
            }
            res += link(url, item.onlyFileName());
        }
        return res;
        // if (m_CodeType == ctLinks ||  item.directUrl.empty())
           
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
        result += lineSeparator();
    }
    result += content;
    return result;
}

std::string AbstractCodeGenerator::itemSeparator() const {
    return "  ";
}

std::string AbstractCodeGenerator::rowSeparator() const {
    return "\r\n\r\n";
}

std::string AbstractCodeGenerator::lineSeparator() const {
    return "\r\n";
}

}
