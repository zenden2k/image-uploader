#ifndef IU_OUTPUTCODEGENERATOR_H
#define IU_OUTPUTCODEGENERATOR_H

#include <string>
#include <vector>

struct UploadObject
{
    std::string localFilePath;
    std::string directUrl;
    std::string viewUrl;
    std::string thumbUrl;
    std::string deleteUrl;
    std::string serverName;
    std::string displayFileName;
    time_t timeStamp;
    int64_t uploadFileSize;
};

class OutputCodeGenerator
{
    public:
        enum CodeLang {clHTML, clBBCode, clPlain, clJSON};
        enum CodeType {ctTableOfThumbnails, ctClickableThumbnails, ctImages, ctLinks};
        OutputCodeGenerator();
        void setLang(CodeLang lang);
        void setType(CodeType type);
        void setPreferDirectLinks(bool prefer);
        std::string generate(const std::vector<UploadObject>& items);
    private:
        std::string generateCodeForItem(const UploadObject& item, int index);
        std::string image(const std::string& url);
        std::string link(const std::string &url, const std::string &body);
        std::string generateJson(const std::vector<UploadObject>& items);
        CodeLang m_lang;
        CodeType m_CodeType;
        bool m_PreferDirectLinks;
};

#endif
