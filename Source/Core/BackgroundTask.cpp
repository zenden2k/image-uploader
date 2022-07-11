#include "BackgroundTask.h"

#include "Core/Utils/CoreUtils.h"

bool BackgroundTask::isInProgress() {
    return isRunning_;
}

void BackgroundTask::cancel() {
    isCanceled_ = true;
}

bool BackgroundTask::isCanceled() {
    return isCanceled_;
}

void BackgroundTask::run() {
	BackgroundTaskResult result = BackgroundTaskResult::Failed;
	defer<void> d([&] { // Run at function exit
		isRunning_ = false;
		onTaskFinished(this, result);
	});
	if (isCanceled_) {
		result = BackgroundTaskResult::Canceled;
		return;
	}
	isRunning_ = true;
	result = doJob();
}