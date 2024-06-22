
#pragma once

#include <string>
#include <vector>

#include "Core/Utils/CoreUtils.h"

namespace ImageUploader::Core::OutputGenerator {

struct UploadObject {
    std::string localFilePath;
    std::string directUrl;
    std::string directUrlShortened;
    std::string viewUrl;
    std::string viewUrlShortened;
    std::string thumbUrl;
    std::string thumbUrlShortened;
    std::string deleteUrl;
    std::string serverName;
    std::string displayFileName;
    time_t timeStamp = 0;
    int64_t uploadFileSize = -1;
    // Index in original file list
    size_t fileIndex = -1;

    std::string getDownloadUrl(bool shortened = false) const {
        return (shortened && !viewUrlShortened.empty()) ? viewUrlShortened : viewUrl;
    }

    std::string getImageUrl(bool shortened = false) const {
        return (shortened && !directUrlShortened.empty()) ? directUrlShortened : directUrl;
    }

    std::string getThumbUrl(bool shortened = false) const {
        return (shortened && !thumbUrlShortened.empty()) ? thumbUrlShortened : thumbUrl;
    }

    bool isNull() const {
        return directUrl.empty() && viewUrl.empty();
    }

    std::string onlyFileName() const {
        return IuCoreUtils::ExtractFileName(displayFileName);
    }
};

enum CodeLang { clHTML, clBBCode, clPlain, clJSON, clMarkdown };
enum CodeType { ctTableOfThumbnails, ctClickableThumbnails, ctImages, ctLinks };

class AbstractOutputGenerator
{
public:
    AbstractOutputGenerator() = default;
    virtual ~AbstractOutputGenerator() = default;
    void setType(CodeType type);
    void setPreferDirectLinks(bool prefer);
    void setShortenUrl(bool shorten);
    void setGroupByFile(bool group);
    virtual CodeLang lang() const = 0;
    virtual std::string generate(const std::vector<UploadObject>& items) = 0;
protected:
    CodeType codeType_;
    bool preferDirectLinks_ = true, shortenUrl_ = false, groupByFile_ = false;
};

}
