
#pragma once

#include <optional>
#include <string>
#include <vector>

#include "Core/Upload/UploadResult.h"
#include "Core/Utils/CoreUtils.h"

class UploadTask;

namespace Uptooda::Core::OutputGenerator {

struct UploadObject {
    UploadResult uploadResult;
    std::string localFilePath;
    /*std::string directUrl;
    std::string directUrlShortened;
    std::string viewUrl;
    std::string viewUrlShortened;
    std::string thumbUrl;
    std::string thumbUrlShortened;
    std::string deleteUrl;
    std::string serverName;*/
    std::string displayFileName;
    time_t timeStamp = 0;
    int64_t uploadFileSize = -1;
    // Index in original file list
    size_t fileIndex = -1;

    void fillFromUploadResult(UploadResult* res, UploadTask* task);

    std::string getDownloadUrl(bool shortened = false) const {
        return (shortened && !uploadResult.downloadUrlShortened.empty()) ? uploadResult.downloadUrlShortened : uploadResult.downloadUrl;
    }

    std::string getImageUrl(bool shortened = false) const {
        return (shortened && !uploadResult.directUrlShortened.empty()) ? uploadResult.directUrlShortened : uploadResult.directUrl;
    }

    std::string getThumbUrl(bool shortened = false) const {
        return uploadResult.thumbUrl;
        //return (shortened && !thumbUrlShortened.empty()) ? thumbUrlShortened : thumbUrl;
    }

    bool isNull() const {
        return uploadResult.directUrl.empty() && uploadResult.downloadUrl.empty();
    }

    std::string onlyFileName() const {
        return displayFileName;
    }
};

enum CodeLang { clUnknown = -1, clBBCode, clHTML,  clPlain, clMarkdown, clJSON };
enum GeneratorID { gidBBCode, gidHTML, gidPlain, gidMarkdown, gidJSON, gidXmlTemplate};
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
    void setItemsPerLine(int n);
    virtual GeneratorID id() const = 0;
    bool loadTemplate(const std::string& templateFileName);
    std::string generate(const std::vector<UploadObject>& items, bool withTemplate = true);
protected:
    CodeType codeType_ = ctTableOfThumbnails;
    bool preferDirectLinks_ = true, shortenUrl_ = false, groupByFile_ = false;
    int itemsPerLine_ = 4;
    std::optional<std::string> templateHead_, templateFoot_;
    virtual std::string doGenerate(const std::vector<UploadObject>& items) = 0;
};

}
