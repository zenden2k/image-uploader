#include "HTMLGenerator.h"

namespace Uptooda::Core::OutputGenerator {

GeneratorID HTMLGenerator::id() const
{
    return gidHTML;
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
            result += "<br><br>";
        }
        result += fileName + "<br>";
    }

    result += content;
    return result;
}

std::string HTMLGenerator::itemSeparator() const {
    return "  ";
}

std::string HTMLGenerator::lineSeparator() const {
    return "<br>";
}

std::string HTMLGenerator::rowSeparator() const {
    return "\r\n<br>\r\n<br>\r\n";
}

}
