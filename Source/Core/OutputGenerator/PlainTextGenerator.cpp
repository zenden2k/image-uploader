#include "PlainTextGenerator.h"

namespace Uptooda::Core::OutputGenerator {

GeneratorID PlainTextGenerator::id() const
{
    return gidPlain;
}

std::string PlainTextGenerator::generateCodeForItem(const UploadObject& item, int index) {
    std::string res;
    if (index) {
        res += lineSeparator();
    }
    res += getItemTargetUrl(item);
    return res;
}

}
