
#pragma once

#include <atomic>
#include <chrono>
#include <string>
#include <vector>
#include <functional>
#include <mutex>
#include <memory>

#include "Core/Utils/CoreTypes.h"

class CUploadEngineData;
class CMyEngineList;
class UploadEngineManager;

struct ServerFilter {
    std::string query;
    int64_t fileSize = 0;

    bool empty() const {
        return query.empty() && !fileSize;
    }
};

class ServerData {
public:
    uint32_t color;
    std::string data;
    CUploadEngineData* ued {};
    int uedIndex = -1;

    bool acceptFilter(const ServerFilter& filter) const;
};


class ServerListModel {
public:
    ServerListModel(CMyEngineList* engineList);
    ~ServerListModel();
    std::string getItemText(int row, int column) const;
    uint32_t getItemColor(int row) const;
    size_t getCount() const;
    void notifyRowChanged(size_t row);
    void notifyCountChanged(size_t row);
    const ServerData& getDataByIndex(size_t row) const;
    void setOnRowChangedCallback(std::function<void(size_t)> callback);
    void setOnItemCountChangedCallback(std::function<void(size_t)> callback);
    void resetData();
    void applyFilter(const std::vector<std::string>& fields);

protected:
    CMyEngineList* engineList_;
    std::vector<ServerData> items_;
    std::vector<size_t> filteredItemsIndexes_;
    std::function<void(size_t)> rowChangedCallback_;
    std::function<void(size_t)> itemCountChangedCallback_;
    DISALLOW_COPY_AND_ASSIGN(ServerListModel);
};

