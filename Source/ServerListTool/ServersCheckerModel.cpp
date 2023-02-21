#include "ServersCheckerModel.h"

#include "Func/MyEngineList.h"

namespace ServersListTool {

ServersCheckerModel::ServersCheckerModel(CMyEngineList* engineList) : engineList_(engineList) {
    for (int i = 0; i < engineList_->count(); i++) {
        ServerData *sd = new ServerData();
        CUploadEngineData* ued = engineList_->byIndex(i);
        sd->ued = ued;
        if (ued->hasType(CUploadEngineData::TypeFileServer)) {
            sd->serverType = CUploadEngineData::TypeFileServer;
            items_.push_back(sd);
        } else if (ued->hasType(CUploadEngineData::TypeImageServer)) {
            sd->serverType = CUploadEngineData::TypeImageServer;
            items_.push_back(sd);
        }

        if (ued->hasType(CUploadEngineData::TypeUrlShorteningServer)) {
            ServerData* sd2 = new ServerData();
            sd2->ued = ued;
            sd2->serverType = CUploadEngineData::TypeUrlShorteningServer;
            items_.push_back(sd2);
        }  
    }
}

ServersCheckerModel::~ServersCheckerModel() {
    for (auto* it : items_) {
        delete it;
    }
}

std::string ServersCheckerModel::getItemText(int row, int column) const {
    const ServerData& serverData = *items_[row];
    if (column == 0) {
        return std::to_string(row + 1);
    } else if (column == 1) {
        std::string name = serverData.ued->Name;
        if (serverData.serverType == CUploadEngineData::TypeUrlShorteningServer) {
            name += "  [URL Shortener]";
        }
        return name;
    } else if (column == 2) {
        if (serverData.finished) {
            return serverData.strMark();
        } if (serverData.skip) {
            return "<SKIP>";
        }
        else {
            return serverData.statusText();
        }
    } else if (column == 3) {
        std::string directUrlCellText = serverData.directUrl();
        std::string directUrlInfo = serverData.directUrlInfo();
        if (!directUrlInfo.empty()) {
            directUrlCellText = directUrlInfo + " [" + directUrlCellText + "]";
        }
        return directUrlCellText;
    } else if (column == 4) {
        std::string thumbUrlCellText = serverData.thumbUrl();
        std::string thumbUrlInfo = serverData.thumbUrlInfo();
        if (!thumbUrlInfo.empty()) {
            thumbUrlCellText = thumbUrlInfo + " [" + thumbUrlCellText + "]";
        }
        return thumbUrlCellText;
    } else if (column == 5) {
        std::string viewUrlCellText = serverData.viewurl();
        std::string viewurlInfo = serverData.viewurlInfo();
        if (!viewurlInfo.empty()) {
            viewUrlCellText = viewurlInfo + " [" + viewUrlCellText + "]";
        }
        return viewUrlCellText;
    } else if (column == 6) {
        return serverData.timeStr();
    }
    return std::string();
}

unsigned long ServersCheckerModel::getItemColor(int row) const {
    const ServerData& serverData = *items_[row];
    return serverData.color;
}

size_t ServersCheckerModel::getCount() const {
    return items_.size();
}

void ServersCheckerModel::notifyRowChanged(size_t row) {
    if (row < items_.size() && rowChangedCallback_) {
        rowChangedCallback_(row);
    }
}

ServerData* ServersCheckerModel::getDataByIndex(size_t row) {
    if (row >= items_.size()) {
        return nullptr;
    }
    return items_[row];
}

void ServersCheckerModel::setOnRowChangedCallback(std::function<void(size_t)> callback) {
    rowChangedCallback_ = std::move(callback);
}

void ServersCheckerModel::resetData() {
    for (auto& it : items_) {
        it->clearInfo();
    }
}
}