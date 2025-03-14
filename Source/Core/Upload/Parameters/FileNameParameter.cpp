#include "FileNameParameter.h"

FileNameParameter::FileNameParameter(std::string title)
    : TextParameter(std::move(title))
{
}

std::string FileNameParameter::getType() const {
    return TYPE;
}

bool FileNameParameter::directory() const {
    return directory_;
}

void FileNameParameter::setDirectory(bool enable) {
    directory_ = enable;
}
