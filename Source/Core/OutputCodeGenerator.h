#ifndef IU_OUTPUTCODEGENERATOR_H
#define IU_OUTPUTCODEGENERATOR_H

#include <string>
#include <vector>
#include "Utils/CoreTypes.h"

struct UploadResultItem
{
   std::string localFilePath;
   std::string directUrl;
   std::string directUrlShortened;
   std::string viewUrl;
   std::string viewUrlShortened;
   std::string thumbUrl;
   std::string thumbUrlShortened;
   std::string serverName;
   std::string displayFileName;
   time_t timeStamp;
   int64_t uploadFileSize;

   std::string getDownloadUrl(bool shortened = false) const {
       return (shortened && !viewUrlShortened.empty()) ? viewUrlShortened : viewUrl;
   }

   std::string getImageUrl(bool shortened = false) const {
       return (shortened && !directUrlShortened.empty()) ? directUrlShortened : directUrl;
   }

   std::string  getThumbUrl(bool shortened = false) const {
       return (shortened && !thumbUrlShortened.empty()) ? thumbUrlShortened : thumbUrl;
   }

   bool isNull() const
   {
       return directUrl.empty() && viewUrl.empty();
   }
};

class OutputCodeGenerator
{
    public:
        enum CodeLang {clHTML, clBBCode, clPlain};
        enum CodeType {ctTableOfThumbnails, ctClickableThumbnails, ctImages, ctLinks};
        OutputCodeGenerator();
        void setLang(CodeLang lang);
        void setType(CodeType type);
        void setRowSize(unsigned int rowSize);
        void setPreferDirectLinks(bool prefer);

        std::string generate(const std::vector<UploadResultItem>& items, const std::string& templateFilePath = {}, bool shortened = false);
    private:
        std::string generateCodeForItem(const UploadResultItem& item, int index);
        std::string image(const std::string& url);
        std::string link(const std::string &url, const std::string &body);
        CodeLang m_lang;
        CodeType m_CodeType;
        bool m_PreferDirectLinks;

        unsigned int rowSize_ = 4;
};

#endif
