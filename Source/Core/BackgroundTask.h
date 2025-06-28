#ifndef IU_CORE_BACKGROUNDTASK
#define IU_CORE_BACKGROUNDTASK

#include <atomic>
#include <string>
#include <boost/signals2.hpp>

#include "TaskDispatcher.h"
#include "Utils/CoreTypes.h"

enum class BackgroundTaskResult { Invalid, Success, Failed, Canceled };

class BackgroundTask: public CancellableTask {
public:
	BackgroundTask() = default;
	boost::signals2::signal<void(BackgroundTask*, BackgroundTaskResult)> onTaskFinished;
	boost::signals2::signal<void(BackgroundTask*, int, int, const std::string&)> onProgress;
	virtual BackgroundTaskResult doJob() = 0;
	void run() override final;
	void cancel() override;
	bool isCanceled() override;
	bool isInProgress() override;
    BackgroundTaskResult result() const;
	DISALLOW_COPY_AND_ASSIGN(BackgroundTask);
protected:
	std::atomic<bool> isRunning_ = false;
    std::atomic<bool> isCanceled_ = false;
    std::atomic<BackgroundTaskResult> result_ = BackgroundTaskResult::Invalid;
};

#endif
