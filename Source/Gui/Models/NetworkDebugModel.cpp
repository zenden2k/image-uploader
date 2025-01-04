#include "NetworkDebugModel.h"

#include "Func/MyEngineList.h"

NetworkDebugModel::NetworkDebugModel()

{
  
}

NetworkDebugModel::~NetworkDebugModel() {
}

std::string NetworkDebugModel::getItemText(int row, int column) const {
    const auto& modelData = items_[row];
    if (column == 0) {
        return std::to_string(row + 1);
    } /* else if (column == 1) {
        return modelData->justFileName;
    } else if (column == 2) {
        return modelData->badFileFormat.mimeType;
    } else if (column == 3) {
        return IuCoreUtils::FileSizeToString(modelData->badFileFormat.fileSize);
    } else if (column == 4) {
        return modelData->extension;
    } else if (column == 5) {
        return modelData->badFileFormat.uploadProfile->serverName();
    } */
    return {};
}

uint32_t NetworkDebugModel::getItemColor(int row) const {
    const NetworkDebugModelData& modelData = *items_[row];
     switch (modelData.status()) {
        case NetworkDebugModelData::RowStatus::Skipped: 
            return RGB(160, 160, 160);
        case NetworkDebugModelData::RowStatus::Ignore:
            return RGB(29, 194, 74);
        case NetworkDebugModelData::RowStatus::Error:
            return RGB(230, 0, 0);

        case NetworkDebugModelData::RowStatus::Normal:
        default:
            return GetSysColor(COLOR_WINDOWTEXT);
    } 
    return modelData.color;
}

size_t NetworkDebugModel::getCount() const {
    return items_.size();
}

void NetworkDebugModel::notifyRowChanged(size_t row) {
    if (row < items_.size() && rowChangedCallback_) {
        rowChangedCallback_(row);
    }
}

NetworkDebugModelData* NetworkDebugModel::getDataByIndex(size_t row) {
    if (row >= items_.size()) {
        return nullptr;
    }
    return items_[row].get();
}

void NetworkDebugModel::setOnRowChangedCallback(std::function<void(size_t)> callback) {
    rowChangedCallback_ = std::move(callback);
}

void NetworkDebugModel::resetData() {
    for (auto& it : items_) {
        it->clearInfo();
    }
}

size_t NetworkDebugModel::hasItemsWithStatus(NetworkDebugModelData::RowStatus status) const {
    size_t count = 0;
    for (auto& it : items_) {
        if (it->status() == status) {
            ++count;
        }
    }
    return count;
}
