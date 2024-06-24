
#pragma once

#include <string>
#include <vector>

#include "AbstractOutputGenerator.h"

namespace ImageUploader::Core::OutputGenerator {

class AbstractCodeGenerator: public AbstractOutputGenerator {
    public:
        AbstractCodeGenerator() = default;
        std::string generate(const std::vector<UploadObject>& items) override;
    protected:
        virtual std::string generateCodeForItem(const UploadObject& item, int index);
        virtual std::string image(const std::string& url, const std::string& alt);
        virtual std::string link(const std::string& url, const std::string& body);
        virtual std::string group(const std::string& fileName, const std::string& content, size_t index);
        virtual std::string itemSeparator() const;
        virtual std::string rowSeparator() const;
        virtual std::string lineSeparator() const;
        std::string getItemTargetUrl(const UploadObject& item) const;
};

}
