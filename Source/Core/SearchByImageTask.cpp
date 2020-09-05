#include "SearchByImageTask.h"

#include "Network/INetworkClient.h"

SearchByImageTask::SearchByImageTask(std::string fileName): fileName_(std::move(fileName)) {
    isRunning_ = false;
    stopSignal_ = false;
}

void SearchByImageTask::cancel() {
    stopSignal_ = true;
}

bool SearchByImageTask::isCanceled() {
    return stopSignal_;
}

bool SearchByImageTask::isInProgress() {
    return isRunning_;
}

void SearchByImageTask::finish(bool success, const std::string& msg) {
    onTaskFinished(this, success, msg);
    isRunning_ = false;
}

int SearchByImageTask::progressCallback(INetworkClient* clientp, double dltotal, double dlnow, double ultotal, double ulnow) {
    if (stopSignal_) {
        return -1;
    }
    return 0;
}


