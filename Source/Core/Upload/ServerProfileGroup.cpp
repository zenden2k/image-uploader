#include "ServerProfileGroup.h"

ServerProfileGroup::ServerProfileGroup() {
    canFirstUseDefaultSettings_ = true;
}

ServerProfileGroup::ServerProfileGroup(const ServerProfile& profile) {
    canFirstUseDefaultSettings_ = true;
    addItem(profile);
}


ServerProfileGroup::ServerProfileGroup(bool canFirstUseDefaultSettings) {
    canFirstUseDefaultSettings_ = canFirstUseDefaultSettings;
}

void ServerProfileGroup::addItem(const ServerProfile& profile) {
    ServerProfile p = profile;
    if (items_.empty() && !canFirstUseDefaultSettings_) {
        p.UseDefaultSettings = false;
    }
    items_.push_back(p);
}

std::vector<ServerProfile>& ServerProfileGroup::getItems() {
    return items_;
}

std::size_t ServerProfileGroup::getCount() const {
    return items_.size();
}

bool ServerProfileGroup::isEmpty() const {
    return items_.empty();
}

ServerProfile& ServerProfileGroup::getByIndex(size_t index) {
    if (items_.empty()) {
        addItem(ServerProfile(canFirstUseDefaultSettings_));
    }
    return items_.at(index);
}
