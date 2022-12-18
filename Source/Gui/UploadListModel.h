#ifndef IU_GUI_UPLOADLISTMODEL_H
#define IU_GUI_UPLOADLISTMODEL_H

#pragma once

#include <string>
#include <vector>
#include <functional>
#include <mutex>

#include "atlheaders.h"
#include "Core/Utils/CoreTypes.h"

class UploadSession;
class UploadTask;

class UploadListItem {
public:
    COLORREF color;
    bool finished;
    int tableRow;
protected:
    CString fileName_;
    CString displayName_;
    CString statusText_;
    CString thumbStatusText_;
  
    mutable std::mutex dataMutex;
public:
    UploadListItem() {
        color = GetSysColor(COLOR_WINDOWTEXT); 
        finished = false;
        tableRow = -1;
    }

    void clearInfo() {
        std::lock_guard<std::mutex> lk(dataMutex);
        color = 0;
        fileName_.Empty();
        displayName_.Empty();
        statusText_.Empty();
        thumbStatusText_.Empty(); 
    }

    CString fileName() const {
        std::lock_guard<std::mutex> lk(dataMutex);
        return fileName_;
    }

    void setFileName(CString name) {
        std::lock_guard<std::mutex> lk(dataMutex);
        fileName_ = name;
    }

    CString displayName() const {
        std::lock_guard<std::mutex> lk(dataMutex);
        return displayName_;
    }

    void setDisplayName(CString name) {
        std::lock_guard<std::mutex> lk(dataMutex);
        displayName_ = name;
    }

    CString statusText() const {
        std::lock_guard<std::mutex> lk(dataMutex);
        return statusText_;
    }

    void setStatusText(CString text) {
        std::lock_guard<std::mutex> lk(dataMutex);
        statusText_ = text;
    }

    CString thumbStatusText() const {
        std::lock_guard<std::mutex> lk(dataMutex);
        return thumbStatusText_;
    }

    void setThumbStatusText(CString text) {
        std::lock_guard<std::mutex> lk(dataMutex);
        thumbStatusText_ = text;
    }
};


class UploadListModel {
public:
    explicit UploadListModel(std::shared_ptr<UploadSession> session);
    ~UploadListModel();
    CString getItemText(int row, int column) const;
    COLORREF getItemTextColor(int row) const;
    size_t getCount() const;
    void notifyRowChanged(size_t row);
    UploadListItem* getDataByIndex(size_t row);
    void setOnRowChangedCallback(std::function<void(size_t)> callback);
    void resetData();
protected:
    std::vector<UploadListItem*> items_;
    std::function<void(size_t)> rowChangedCallback_;

    void onTaskUploadProgress(UploadTask* task);
    void onTaskStatusChanged(UploadTask* task);
    void onTaskFinished(UploadTask* task, bool ok);
    void onChildTaskAdded(UploadTask* child);
    DISALLOW_COPY_AND_ASSIGN(UploadListModel);
};

#endif