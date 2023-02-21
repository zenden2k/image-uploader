#ifndef IU_SERVERLISTTOOL_SERVERSCHECKERMODEL_H
#define IU_SERVERLISTTOOL_SERVERSCHECKERMODEL_H

#pragma once

#include <string>
#include <vector>
#include <functional>
#include <mutex>
#include "Core/Utils/CoreTypes.h"

class CUploadEngineData;
class CMyEngineList;
class UploadEngineManager;

namespace ServersListTool {

class ServerData {
public:
    bool skip;
    int stars[3]; // 0 - direct url, 1 - thumbnail url, 2 - download (view) url
    unsigned long color;
    int fileToCheck;
    int filesChecked;
    int timeElapsed;
    bool finished;
    CUploadEngineData* ued;
    int serverType;
protected:
    std::string directUrl_;
    std::string directUrlInfo_;
    std::string viewurl_;
    std::string viewurlInfo_;
    std::string thumbUrl_;
    std::string thumbUrlInfo_;
    std::string statusText_;
    std::string timeStr_;
    std::string strMark_;


    mutable std::mutex dataMutex;

public:

    ServerData() {
        memset(stars, 0, sizeof(stars));
        color = 0;
        fileToCheck = 0;
        timeElapsed = 0;
        filesChecked = 0;
        finished = false;
        skip = false;
        ued = nullptr;
        serverType = 0;
    }

    void setLinkInfo(int columnId, const std::string& info) {
        std::lock_guard<std::mutex> lk(dataMutex);
        if (columnId == 0) {
            directUrlInfo_ = info;
        } else if (columnId == 1) {
            thumbUrlInfo_ = info;
        } else if (columnId == 2) {
            viewurlInfo_ = info;
        }
    }

    void clearInfo() {
        std::lock_guard<std::mutex> lk(dataMutex);
        color = 0;
        fileToCheck = 0;
        timeElapsed = 0;
        filesChecked = 0;
        finished = false;
        directUrl_.clear();
        directUrlInfo_.clear();
        viewurl_.clear();
        viewurlInfo_.clear();
        thumbUrl_.clear();
        thumbUrlInfo_.clear();
        statusText_.clear();
        timeStr_.clear();
        strMark_.clear();
    }

    std::string directUrl() const {
        std::lock_guard<std::mutex> lk(dataMutex);
        return directUrl_;
    }

    void setDirectUrl(const std::string& url) {
        std::lock_guard<std::mutex> lk(dataMutex);
        directUrl_ = url;
    }

    std::string directUrlInfo() const {
        std::lock_guard<std::mutex> lk(dataMutex);
        return directUrlInfo_;
    }

    void setDirectUrlInfo(const std::string& info) {
        std::lock_guard<std::mutex> lk(dataMutex);
        directUrlInfo_ = info;
    }

    std::string viewurl() const {
        std::lock_guard<std::mutex> lk(dataMutex);
        return viewurl_;
    }

    void setViewUrl(const std::string& viewUrl) {
        std::lock_guard<std::mutex> lk(dataMutex);
        viewurl_ = viewUrl;
    }

    std::string viewurlInfo() const {
        std::lock_guard<std::mutex> lk(dataMutex);
        return viewurlInfo_;
    }

    void setViewUrlInfo(const std::string& info) {
        std::lock_guard<std::mutex> lk(dataMutex);
        viewurlInfo_ = info;
    }

    std::string thumbUrl() const {
        std::lock_guard<std::mutex> lk(dataMutex);
        return thumbUrl_;
    }

    void setThumbUrl(const std::string& thumbUrl) {
        std::lock_guard<std::mutex> lk(dataMutex);
        thumbUrl_ = thumbUrl;
    }

    std::string thumbUrlInfo() const {
        std::lock_guard<std::mutex> lk(dataMutex);
        return thumbUrlInfo_;
    }

    void setThumbUrlInfo(const std::string& info) {
        std::lock_guard<std::mutex> lk(dataMutex);
        thumbUrlInfo_ = info;
    }

    std::string statusText() const {
        std::lock_guard<std::mutex> lk(dataMutex);
        return statusText_;
    }

    void setStatusText(const std::string& text) {
        std::lock_guard<std::mutex> lk(dataMutex);
        statusText_ = text;
    }

    std::string timeStr() const {
        std::lock_guard<std::mutex> lk(dataMutex);
        return timeStr_;
    }

    void setTimeStr(const std::string& str) {
        std::lock_guard<std::mutex> lk(dataMutex);
        timeStr_ = str;
    }

    std::string strMark() const {
        std::lock_guard<std::mutex> lk(dataMutex);
        return strMark_;
    }

    void setStrMark(const std::string& str) {
        std::lock_guard<std::mutex> lk(dataMutex);
        strMark_ = str;
    }

    void setStars(int index, int value) {
        std::lock_guard<std::mutex> lk(dataMutex);
        stars[index] = value;
    }
};


class ServersCheckerModel {
public:
    ServersCheckerModel(CMyEngineList* engineList);
    ~ServersCheckerModel();
    std::string getItemText(int row, int column) const;
    unsigned long getItemColor(int row) const;
    size_t getCount() const;
    void notifyRowChanged(size_t row);
    ServerData* getDataByIndex(size_t row);
    void setOnRowChangedCallback(std::function<void(size_t)> callback);
    void resetData();
protected:
    CMyEngineList* engineList_;
    std::vector<ServerData*> items_;
    std::function<void(size_t)> rowChangedCallback_;
    DISALLOW_COPY_AND_ASSIGN(ServersCheckerModel);
};

}
#endif