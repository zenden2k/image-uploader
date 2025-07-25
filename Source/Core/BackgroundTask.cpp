#include "BackgroundTask.h"

#include "Core/Utils/CoreUtils.h"

bool BackgroundTask::isInProgress() {
    return isRunning_;
}

BackgroundTaskResult BackgroundTask::result() const {
    return result_;
}

void BackgroundTask::cancel() {
    isCanceled_ = true;
}

bool BackgroundTask::isCanceled() {
    return isCanceled_;
}

void BackgroundTask::run() {
	BackgroundTaskResult result = BackgroundTaskResult::Failed;
	defer d([&] { // Run at function exit
		isRunning_ = false;
        result_ = result;
        onTaskFinished(this, result);
	});
	if (isCanceled_) {
		result = BackgroundTaskResult::Canceled;
		return;
	}
	isRunning_ = true;
	result = doJob();
}
