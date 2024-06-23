
#pragma once

#include <map>
#include <string>
#include <vector>

#include "AbstractOutputGenerator.h"
#include "XmlTemplateList.h"

namespace ImageUploader::Core::OutputGenerator {

class XmlTemplateGenerator : public AbstractOutputGenerator {
public:
    explicit XmlTemplateGenerator(XmlTemplateList* templateList);
    GeneratorID id() const override;
    std::string generate(const std::vector<UploadObject>& items) override;
    void setTemplateIndex(size_t index);
private:
    std::string replaceVars(const std::string& text, const std::map<std::string, std::string>& m_Vars);
    XmlTemplateList* templateList_;
    size_t templateIndex_ = 0;
};

}
