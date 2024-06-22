#include "HTMLGenerator.h"

namespace ImageUploader::Core::OutputGenerator {

CodeLang HTMLGenerator::lang() const{
    return clHTML;
}

std::string HTMLGenerator::image(const std::string& url, const std::string& alt) {
    return "<img src=\"" + url + "\" alt=\"" + alt + "\"/>";
}

std::string HTMLGenerator::link(const std::string& url, const std::string& body) {
    return "<a href=\"" + url + "\">" + body + "</a>";
}

std::string HTMLGenerator::group(const std::string& fileName, const std::string& content, size_t index) {
    std::string result;
    if (groupByFile_ && !content.empty()) {
        if (index) {
            result += "<br>";
        }
        result += fileName + "<br><br>";
    }

    result += content;
    return result;

}


}