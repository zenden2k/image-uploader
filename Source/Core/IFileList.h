#pragma once

class IFileListItem {
public:
    virtual ~IFileListItem() = default;
    virtual std::string getFileName() const = 0;
    virtual std::string getMimeType() const = 0;
    virtual void setMimeType(const std::string& mimeType) = 0;
    virtual bool isImage() const = 0;
};

class IFileList {
public:
    virtual ~IFileList() = default;
    virtual size_t getFileCount() const = 0;
    virtual IFileListItem* getFile(size_t index) = 0;
};
