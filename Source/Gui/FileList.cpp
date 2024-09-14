#include "FileList.h"

#include "Core/Utils/CoreUtils.h"
#include "Func/IuCommonFunctions.h"

std::string CFileListItem::getFileName() const
{
    return W2U(FileName);
}

std::string CFileListItem::getMimeType() const
{
    return mimeType_;
}

void CFileListItem::setMimeType(const std::string& mimeType)
{
    mimeType_ = mimeType;
}

bool CFileListItem::isImage() const
{
    return IuCommonFunctions::IsImage(FileName);
}

size_t CFileList::getFileCount() const
{
    return GetCount();
}

IFileListItem* CFileList::getFile(size_t index)
{
    return &GetAt(index);
}
