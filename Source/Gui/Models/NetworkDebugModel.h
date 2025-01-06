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

class NetworkDebugModelData {
public:
    enum class RowStatus { Normal, Error, Skipped, Ignore};

    uint32_t color;
    curl_infotype type;
    RowStatus status_;
    std::string data;
    time_t time_;
    std::string threadId_;

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
    NetworkDebugModelData* getDataByIndex(size_t row);
    void setOnRowChangedCallback(std::function<void(size_t)> callback);
    void setOnItemCountChangedCallback(std::function<void(size_t)> callback);
    void clear();

protected:
    //mutable std::mutex itemsMutex_;
    concurrency::concurrent_vector<NetworkDebugModelData> items_;
    std::function<void(size_t)> rowChangedCallback_;
    std::function<void(size_t)> itemCountChangedCallback_;
    boost::signals2::scoped_connection debugMessageConnection_;
    DISALLOW_COPY_AND_ASSIGN(NetworkDebugModel);
};
#endif
