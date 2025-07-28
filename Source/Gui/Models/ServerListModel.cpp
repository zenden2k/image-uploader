#include "ServerListModel.h"

#include "Func/MyEngineList.h"
#include "Core/i18n/Translator.h"

ServerListModel::ServerListModel(CMyEngineList* engineList) : engineList_(engineList) {
    for (int i = 0; i < engineList_->count(); i++) {
        CUploadEngineData* ued = engineList_->byIndex(i);
        
        ServerData sd;
        sd.ued = ued;
        sd.uedIndex = i;
        
        items_.push_back(std::move(sd));
    }
}

ServerListModel::~ServerListModel() {
}

std::string ServerListModel::getItemText(int row, int column) const {
    const ServerData& serverData = getDataByIndex(row);
    if (column == 0) {
        return serverData.ued->Name;
    }
    if (column == 1) {
        return serverData.ued->MaxFileSize ?  IuCoreUtils::FileSizeToString(serverData.ued->MaxFileSize) : "";
    }
    if (column == 2) {
        
    }
    if (column == 3) {
        return serverData.ued->NeedAuthorization == 2 ? _("yes") : _("no");
    } else if (column == 4) {
       
    } 
    return {};
}

uint32_t ServerListModel::getItemColor(int row) const {
    const ServerData& serverData = getDataByIndex(row);
    return serverData.color;
}

size_t ServerListModel::getCount() const {
    return items_.size();
}

void ServerListModel::notifyRowChanged(size_t row) {
    if (row < items_.size() && rowChangedCallback_) {
        rowChangedCallback_(row);
    }
}

const ServerData& ServerListModel::getDataByIndex(size_t row) const {
    return items_[row];
}

void ServerListModel::setOnRowChangedCallback(std::function<void(size_t)> callback) {
    rowChangedCallback_ = std::move(callback);
}

void ServerListModel::setOnItemCountChangedCallback(std::function<void(size_t)> callback) {
    itemCountChangedCallback_ = std::move(callback);
}

void ServerListModel::resetData() {
    /* for (auto& it : items_) {
        it.clearInfo();
    }*/
}

void ServerListModel::notifyCountChanged(size_t row) {
    if (itemCountChangedCallback_) {
        itemCountChangedCallback_(row);
    }
}

bool ServerData::acceptFilter(const ServerFilter& filter) const {

    return true;
}
