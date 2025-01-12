#include "NetworkDebugModel.h"

#include <iomanip>
#include <sstream>

#include <zlib.h>

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

std::string NetworkDebugModelData::getDecoded() {
    const auto CHUNK_SIZE = 0x4000;
    if (!decoded_.empty()) {
        return decoded_;
    }
    unsigned char in[CHUNK_SIZE];
    unsigned char out[CHUNK_SIZE];
    z_stream strm = { 0 };
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.next_in = in;
    strm.avail_in = 0;
    const auto windowBits = 15;
    int st = inflateInit2(&strm, 16 + MAX_WBITS);
    if (st < 0) {
        LOG(WARNING) << "Failed to decompress gzip";
        return {};
    }
    size_t inPos = 0;
    while (1) {
        int bytesRead;
        int zlibStatus;

        bytesRead = std::min<size_t>(CHUNK_SIZE, data.size() - inPos);
        strm.avail_in = bytesRead;
        strm.next_in = reinterpret_cast<Bytef*>(&data[inPos]);
        do {
            unsigned have;
            strm.avail_out = CHUNK_SIZE;
            strm.next_out = out;
            zlibStatus = inflate(&strm, Z_NO_FLUSH);
            switch (zlibStatus) {
                case Z_OK:
                case Z_STREAM_END:
                case Z_BUF_ERROR:
                    break;

                default:
                    inflateEnd(&strm);
                    LOG(WARNING) << "zlib error " << zlibStatus;
                    return {};
                }
                have = CHUNK_SIZE - strm.avail_out;
                decoded_.append((const char*)out, have);
                inPos += bytesRead;
        } while (strm.avail_out == 0);
        if (inPos >= data.size()) {
            inflateEnd(&strm);
            break;
        }
    }
    return decoded_;
}

NetworkDebugModel::NetworkDebugModel() {
    std::shared_ptr<NetworkDebugger> debugger = ServiceLocator::instance()->networkDebugger();
    debugger->onMessage.connect([this](INetworkClient*, curl_infotype type, char* data, size_t length) -> void {
        NetworkDebugModelData item;
        item.type = type;
        item.time_ = std::time(nullptr);
        item.data.assign(data, length);
        item.threadId_ = IuCoreUtils::ThreadIdToString(std::this_thread::get_id());
        size_t index;
        {
            //std::lock_guard<std::mutex> lk(itemsMutex_);
            items_.push_back(std::move(item));
            index = items_.size();
        }
        notifyCountChanged(index);
    });
}

NetworkDebugModel::~NetworkDebugModel() {
}

std::string NetworkDebugModel::getItemText(int row, int column) const {
    //std::lock_guard<std::mutex> lk(itemsMutex_);
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
    //std::lock_guard<std::mutex> lk(itemsMutex_);
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
    //std::lock_guard<std::mutex> lk(itemsMutex_);
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

void NetworkDebugModel::clear() {
    items_.clear();
    notifyCountChanged(0);
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
