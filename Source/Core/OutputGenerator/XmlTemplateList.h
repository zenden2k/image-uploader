
#pragma once

#include <string>
#include <vector>

#include "AbstractOutputGenerator.h"

namespace ImageUploader::Core::OutputGenerator {

struct ResultTemplate {
    std::string Name, Items, LineSep, LineStart, ItemSep, LineEnd, TemplateText;
};

class XmlTemplateList {
public:
    XmlTemplateList() = default;

    /**
     * @throws IOException, std::runtime_error
     */
    void loadFromFile(const std::string& fileName);

    /**
     * @throws std::out_of_range
     */
    const ResultTemplate& at(size_t index);
    size_t size() const;
    void clear();
private:
    std::vector<ResultTemplate> templates_;
};

}
