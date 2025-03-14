#pragma once

#include "Core/IFileList.h"

class CFileListItem: public IFileListItem {
public:
    CString FilePath;
    CString FileName;
    CString VirtualFileName;
    bool selected = false;
    std::string getFileName() const override;
    std::string getMimeType() const override;
    void setMimeType(const std::string& mimeType) override;
    bool isImage() const override;
    void setSkipped(bool enable) override;
    bool isSkipped() const override;

private:
    std::string mimeType_;
    bool skipped_ = false;
};

class CFileList : public CAtlArray<CFileListItem>, public IFileList {
public:
    size_t getFileCount() const override;
    IFileListItem* getFile(size_t index) override;
};
