#include "AbstractOutputGenerator.h"

namespace ImageUploader::Core::OutputGenerator {

void AbstractOutputGenerator::setType(CodeType type) {
    codeType_ = type;
}

void AbstractOutputGenerator::setPreferDirectLinks(bool prefer) {
    preferDirectLinks_ = prefer;
}

void AbstractOutputGenerator::setShortenUrl(bool shorten) {
    shortenUrl_ = shorten;
}

void AbstractOutputGenerator::setGroupByFile(bool group) {
    groupByFile_ = group;
}

}
