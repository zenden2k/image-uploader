#include "NetworkDebugModel.h"

#include <iomanip>
#include <sstream>

#include "Core/ServiceLocator.h"
#include "Core/Network/NetworkDebugger.h"

#include "Func/MyEngineList.h"

namespace {

std::string CurlTypeToString(curl_infotype type)
{
    switch (type) {
    case CURLINFO_TEXT:
        return "== Info: ";
    default:
        return "<unknown>";

    case CURLINFO_HEADER_OUT:
        return "=> Send header";

    case CURLINFO_DATA_OUT:
        return "=> Send data";

    case CURLINFO_SSL_DATA_OUT:
        return "=> Send SSL data";

    case CURLINFO_HEADER_IN:
        return "<= Recv header";

    case CURLINFO_DATA_IN:
        return "<= Recv data";

    case CURLINFO_SSL_DATA_IN:
        return "<= Recv SSL data";
    };
}

}

NetworkDebugModel::NetworkDebugModel() {
    std::shared_ptr<NetworkDebugger> debugger = ServiceLocator::instance()->networkDebugger();
    debugger->onMessage.connect([this](INetworkClient*, curl_infotype type, char* data, size_t length) -> void {
        NetworkDebugModelData item;
        item.type = type;
        item.time_ = std::time(nullptr);
        item.data.assign(data, length);
        item.threadId_ = IuCoreUtils::ThreadIdToString(std::this_thread::get_id());
        size_t index = -1;
        {
            std::lock_guard<std::mutex> lk(itemsMutex_);
            items_.push_back(std::move(item));
            index = items_.size() - 1;
        }
        notifyCountChanged(index+1);
    });
}

NetworkDebugModel::~NetworkDebugModel() {
}

std::string NetworkDebugModel::getItemText(int row, int column) const {
    std::lock_guard<std::mutex> lk(itemsMutex_);
    const auto& modelData = items_[row];
    if (column == 0) {
       // return std::to_string(row + 1);
    } else if (column == 1) {
        return modelData.threadId_;
    }
    else if (column == 2) {
        std::stringstream ss;
        ss << std::put_time(std::localtime(&modelData.time_), "%F %T");
        return ss.str();
    }  else if (column == 3) {
        return CurlTypeToString(modelData.type);
    } else if (column == 4) {
        if (modelData.type == CURLINFO_HEADER_IN || modelData.type == CURLINFO_HEADER_OUT || modelData.type == CURLINFO_TEXT) {
            return modelData.data.substr(0, modelData.data.find_first_of("\r\n"));
        }
    } /* else if (column == 4) {
        return modelData->extension;
    } else if (column == 5) {
        return modelData->badFileFormat.uploadProfile->serverName();
    }*/
    return {};
}

uint32_t NetworkDebugModel::getItemColor(int row) const {
    std::lock_guard<std::mutex> lk(itemsMutex_);
    const auto& modelData = items_[row];
    switch (modelData.type) {
        case CURLINFO_TEXT: 
            return RGB(160, 160, 160);
        default:
            return GetSysColor(COLOR_WINDOWTEXT);
    } 
    return modelData.color;
}

size_t NetworkDebugModel::getCount() const {
    std::lock_guard<std::mutex> lk(itemsMutex_);
    return items_.size();
}

void NetworkDebugModel::notifyRowChanged(size_t row) {
    if (row < items_.size() && rowChangedCallback_) {
        rowChangedCallback_(row);
    }
}


void NetworkDebugModel::notifyCountChanged(size_t row)
{
    if (itemCountChangedCallback_) {
        itemCountChangedCallback_(row);
    }
}

NetworkDebugModelData* NetworkDebugModel::getDataByIndex(size_t row) {
    if (row >= items_.size()) {
        return nullptr;
    }
    return &items_[row];
}

void NetworkDebugModel::setOnRowChangedCallback(std::function<void(size_t)> callback) {
    rowChangedCallback_ = std::move(callback);
}


void NetworkDebugModel::setOnItemCountChangedCallback(std::function<void(size_t)> callback) {
    itemCountChangedCallback_ = std::move(callback);
}

void NetworkDebugModel::resetData() {
    for (auto& it : items_) {
        it.clearInfo();
    }
}
/*
size_t NetworkDebugModel::hasItemsWithStatus(NetworkDebugModelData::RowStatus status) const {
    size_t count = 0;
    for (auto& it : items_) {
        if (it->status() == status) {
            ++count;
        }
    }
    return count;
}*/
