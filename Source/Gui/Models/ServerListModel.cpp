#include "ServerListModel.h"

#include "Func/MyEngineList.h"
#include "Core/i18n/Translator.h"
#include "Core/Utils/StringUtils.h"

namespace {

size_t StringSearch(const std::string& str1, const std::string& str2) {
    auto loc = std::locale();
    auto it = std::search(str1.begin(), str1.end(),
        str2.begin(), str2.end(), [&loc](char ch1, char ch2) -> bool {
            return std::toupper(ch1, loc) == std::toupper(ch2, loc);
        });
    if (it != str1.end()) {
        return it - str1.begin();
    } else {
        return std::string::npos;
    }
}

}

ServerListModel::ServerListModel(CMyEngineList* engineList) : engineList_(engineList) {
    updateEngineList();
}

ServerListModel::~ServerListModel() {
}

void ServerListModel::updateEngineList() {
    filteredItemsIndexes_.clear();
    items_.clear();

    for (int i = 0; i < engineList_->count(); i++) {
        CUploadEngineData* ued = engineList_->byIndex(i);

        ServerData sd;
        sd.ued = ued;
        sd.uedIndex = i;
        sd.engineList = engineList_;

        items_.push_back(std::move(sd));
    }
}

std::string ServerListModel::getItemText(int row, int column) const {
    const ServerData& serverData = getDataByIndex(row);
    if (column == 0) {
        return serverData.getServerDisplayName();
    }
    if (column == 1) {
        return serverData.getMaxFileSizeString();
    }
    if (column == 2) {
        
    }
    if (column == 3) {
        return serverData.ued->NeedAuthorization == 2 ? _("yes") : _("no");
    } else if (column == 4) {
        return serverData.getFormats();
    } 
    return {};
}

uint32_t ServerListModel::getItemColor(int row) const {
    const ServerData& serverData = getDataByIndex(row);
    return serverData.color;
}

size_t ServerListModel::getCount() const {
    if (filter_.empty()) {
        return items_.size();
    }
    return filteredItemsIndexes_.size();
}

void ServerListModel::notifyRowChanged(size_t row) {
    if (row < items_.size() && rowChangedCallback_) {
        rowChangedCallback_(row);
    }
}

const ServerData& ServerListModel::getDataByIndex(size_t row) const {
    if (!filter_.empty()) {
        row = filteredItemsIndexes_[row];
    }
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


void ServerListModel::applyFilter(const ServerFilter& filter) {
    filter_ = filter;
    filteredItemsIndexes_.clear();
    size_t i = 0;
    for (const auto& item : items_) {
        if (item.acceptFilter(filter_)) {
            filteredItemsIndexes_.push_back(i);
        }
        i++;
    }
    notifyCountChanged(getCount());
}

void ServerListModel::notifyCountChanged(size_t row) {
    if (itemCountChangedCallback_) {
        itemCountChangedCallback_(row);
    }
}

std::string ServerData::getFormats() const {
    if (!formats.has_value()) {
        std::string result;
        std::map<int, std::set<std::string>> extensions;

        for (const auto& formatGroup : ued->SupportedFormatGroups) {
            extensions[formatGroup.MinUserRank].insert(formatGroup.Extensions.begin(), formatGroup.Extensions.end());   
        }

        for (const auto& [k, v] : extensions) {
            if (!result.empty()) {
                result += "/ ";
            }
            if (!v.empty()) {
                result += IuStringUtils::Join(v, ",");
                result += " ";
            }
        }   
        
        formats = result;
    }

    return *formats;
}

int64_t ServerData::getMaxFileSize() const {
    return ued->SupportedFormatGroups.empty() ? ued->MaxFileSize : ued->SupportedFormatGroups[0].MaxFileSize;
}

std::string ServerData::getMaxFileSizeString() const {
    if (!maxFileSizeString.has_value()) {
        std::string result;
        std::map<int, int64_t> fileSizes;

        for (const auto& formatGroup : ued->SupportedFormatGroups) {
            
            if (formatGroup.MaxFileSize > fileSizes[formatGroup.MinUserRank]) {
                fileSizes[formatGroup.MinUserRank] = formatGroup.MaxFileSize;
                
            }
        }

        for (const auto [k, fileSize] : fileSizes) {
            if (!result.empty()) {
                result += "/ ";
            }
            if (fileSize) {
                result += IuCoreUtils::FileSizeToString(fileSize);
                result += " ";
            }  
        }

        if (result.empty() && ued->MaxFileSize > 0) {
            result += IuCoreUtils::FileSizeToString(ued->MaxFileSize);
        }
        maxFileSizeString = result;
    }
    return *maxFileSizeString;
}


std::string ServerData::getServerDisplayName() const {
    if (!serverDisplayName.has_value()) {
        serverDisplayName = engineList->getServerDisplayName(ued);
    }

    return *serverDisplayName;
}

bool ServerData::acceptFilter(const ServerFilter& filter) const {
    if (!filter.query.empty()) {
        if (StringSearch(getServerDisplayName(), filter.query) == std::string::npos) {
            return false;
        }
    }

    return (ued->TypeMask & filter.typeMask) != 0;
}
