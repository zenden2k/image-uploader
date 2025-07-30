
#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <optional>

#include "Core/Upload/UploadEngine.h"
#include "Core/Utils/CoreTypes.h"

class CUploadEngineData;
class CMyEngineList;
class UploadEngineManager;

struct ServerFilter {
    std::string query;
    int64_t fileSize = 0;
    int typeMask = CUploadEngineListBase::ALL_SERVERS;

    bool empty() const {
        return false;
    }
};

class ServerData {
public:
    uint32_t color;
    std::string data;
    CUploadEngineData* ued {};
    int uedIndex = -1;
    mutable std::optional<std::string> formats;
    mutable std::optional<std::string> maxFileSizeString;

    std::string getFormats() const;

    int64_t getMaxFileSize() const;

    std::string getMaxFileSizeString() const;

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
    void applyFilter(const ServerFilter& filter);

protected:
    CMyEngineList* engineList_;
    std::vector<ServerData> items_;
    std::vector<size_t> filteredItemsIndexes_;
    std::function<void(size_t)> rowChangedCallback_;
    std::function<void(size_t)> itemCountChangedCallback_;
    ServerFilter filter_;
    DISALLOW_COPY_AND_ASSIGN(ServerListModel);
};

