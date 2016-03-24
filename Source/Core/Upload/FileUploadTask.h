#ifndef CORE_UPLOAD_FILEUPLOADTASK_H
#define CORE_UPLOAD_FILEUPLOADTASK_H

#include "UploadTask.h"
#include <string>


class FileUploadTask: public UploadTask {
    public:
        FileUploadTask(const std::string& fileName, const std::string& displayName, UploadTask* parentTask = 0);
        ~FileUploadTask();
        virtual Type type() const;
        virtual std::string getMimeType() const override;
        virtual int64_t getDataLength() const override;
        std::string getFileName() const;
        int64_t getFileSize() const;
        void setFileName(const std::string& fileName);
        std::string getDisplayName() const;
        void setDisplayName(const std::string& name);
        std::string originalFileName() const;
        virtual void finishTask(Status status = StatusFinished) override;
        std::string toString() override;
        std::string title() const override;
        bool isImage();
        void setIsImage(bool image);
protected:
        std::string fileName_;
        std::string originalFileName_;
        std::string displayName_;
        bool isImage_;
        
};    

#endif