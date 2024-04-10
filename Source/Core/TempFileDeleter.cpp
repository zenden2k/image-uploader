#include "TempFileDeleter.h"

#include "Core/Utils/CoreUtils.h"

TempFileDeleter::~TempFileDeleter()
{
    try {
        cleanup();
    } catch (const std::exception& ex) {
        LOG(ERROR) << ex.what();
    }
}

void TempFileDeleter::addFile(const std::string& fileName)
{
    m_files.push_back(fileName);
}

bool TempFileDeleter::cleanup()
{
    bool result = true;
    for (const auto& fileName: m_files)
    {
        result = IuCoreUtils::RemoveFile(fileName) && result;
    }
    m_files.clear();
    return result;
}
