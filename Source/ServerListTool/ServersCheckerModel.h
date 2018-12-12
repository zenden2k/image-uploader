#ifndef IU_SERVERLISTTOOL_SERVERSCHECKERMODEL_H
#define IU_SERVERLISTTOOL_SERVERSCHECKERMODEL_H

#pragma once

#include <string>
#include <vector>
#include <functional>

class CUploadEngineData;
class CMyEngineList;
class UploadEngineManager;

namespace ServersListTool {

struct ServerData {
    bool skip;
    int stars[3];
    unsigned long color;
    int fileToCheck;
    int filesChecked;
    int timeElapsed;
    bool finished;
    std::string directUrl;
    std::string directUrlInfo;
    std::string viewurl;
    std::string viewurlInfo;
    std::string thumbUrl;
    std::string thumbUrlInfo;
    std::string statusText;
    std::string timeStr;
    std::string strMark;
    CUploadEngineData* ued;

    ServerData() {
        memset(stars, 0, sizeof(stars));
        color = 0;
        fileToCheck = 0;
        timeElapsed = 0;
        filesChecked = 0;
        finished = false;
        skip = false;
        ued = nullptr;
    }

    void setLinkInfo(int columnId, const std::string& info) {
        if (columnId == 0) {
            directUrlInfo = info;
        } else if (columnId == 1) {
            thumbUrlInfo = info;
        } else if (columnId == 2) {
            viewurlInfo = info;
        }
    }

    void clearInfo() {
        color = 0;
        fileToCheck = 0;
        timeElapsed = 0;
        filesChecked = 0;
        finished = false;
        directUrl.clear();
        directUrlInfo.clear();
        viewurl.clear();
        viewurlInfo.clear();
        thumbUrl.clear();
        thumbUrlInfo.clear();
        statusText.clear();
        timeStr.clear();
        strMark.clear();
    }
};


class ServersCheckerModel {
public:
    ServersCheckerModel(CMyEngineList* engineList);
    std::string getItemText(int row, int column) const;
    unsigned long getItemColor(int row) const;
    size_t getCount() const;
    void notifyRowChanged(size_t row);
    ServerData* getDataByIndex(size_t row);
    void setOnRowChangedCallback(std::function<void(size_t)> callback);
    void resetData();
protected:
    CMyEngineList* engineList_;
    std::vector<ServerData> items_;
    std::function<void(size_t)> rowChangedCallback_;
};

}
#endif