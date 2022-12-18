#include "SearchByImageTask.h"

#include "Network/INetworkClient.h"

SearchByImageTask::SearchByImageTask(std::string fileName): fileName_(std::move(fileName)) {
    isRunning_ = false;
}

void SearchByImageTask::finish(const std::string& msg) {
    message_ = msg;
}

int SearchByImageTask::progressCallback(INetworkClient* clientp, double dltotal, double dlnow, double ultotal, double ulnow) {
    if (isCanceled_) {
        return -1;
    }
    return 0;
}

std::string  SearchByImageTask::message() const {
    return message_;
}

