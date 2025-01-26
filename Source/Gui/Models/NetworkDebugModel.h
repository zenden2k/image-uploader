#ifndef IU_SERVERLISTTOOL_NetworkDebugModel_H
#define IU_SERVERLISTTOOL_NetworkDebugModel_H

#pragma once

#include <atomic>
#include <chrono>
#include <string>
#include <vector>
#include <functional>
#include <mutex>
#include <memory>

#include <ppl.h>
#include <concurrent_vector.h>

#include <curl/curl.h>

#include "Core/Utils/CoreTypes.h"
#include "Core/Network/NetworkClient.h"
#include "Core/Network/NetworkDebugger.h"

namespace NetworkDebugModelNS {

constexpr auto COLUMN_COUNT = 5;
constexpr auto COLUMN_N = 0;
constexpr auto COLUMN_THREAD_ID = 1;
constexpr auto COLUMN_TIME = 2;
constexpr auto COLUMN_TYPE = 3;
constexpr auto COLUMN_TEXT = 4;
}

class NetworkDebugModelData {
public:
    enum class RowStatus { Normal, Error, Skipped, Ignore};

    uint32_t color;
    curl_infotype type;
    RowStatus status_;
    std::string data;
    time_t time_;
    std::string threadId_;
    std::string decoded_;

public:
    NetworkDebugModelData()
    {
        color = 0;
    }

    void clearInfo() {
        color = 0;
    }

    RowStatus status() const {
        return status_;
    }

    bool hasDecoded() const {
        return !decoded_.empty();
    }

    bool acceptFilter(const std::vector<std::string>& fields) const;
    std::string getDecoded();
};

class NetworkDebugModel {
public:
    NetworkDebugModel();
    ~NetworkDebugModel();
    std::string getItemText(int row, int column) const;
    uint32_t getItemColor(int row) const;
    size_t getCount() const;
    void notifyRowChanged(size_t row);
    void notifyCountChanged(size_t row);
    const NetworkDebugModelData* getDataByIndex(size_t row) const;
    void setOnRowChangedCallback(std::function<void(size_t)> callback);
    void setOnItemCountChangedCallback(std::function<void(size_t)> callback);
    void clear();
    void applyFilter(const std::vector<std::string>& fields);
    /**
     * @throws IOException
     */
    void saveToFile(const std::string& fileName);

protected:
    //mutable std::mutex itemsMutex_;
    concurrency::concurrent_vector<NetworkDebugModelData> items_;
    concurrency::concurrent_vector<size_t> filteredItemsIndexes_;
    std::function<void(size_t)> rowChangedCallback_;
    std::function<void(size_t)> itemCountChangedCallback_;
    boost::signals2::scoped_connection debugMessageConnection_;
    std::string getCell(const NetworkDebugModelData& item, int row, int column) const;
    DISALLOW_COPY_AND_ASSIGN(NetworkDebugModel);
    std::vector<std::string> filter_;
};
#endif
