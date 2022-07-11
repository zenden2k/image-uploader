#ifndef IU_CORE_BACKGROUNDTASK
#define IU_CORE_BACKGROUNDTASK

#include <boost/signals2.hpp>
#include <string>

#include "TaskDispatcher.h"
#include "Utils/CoreTypes.h"

enum class BackgroundTaskResult { Success, Failed, Canceled };

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
	DISALLOW_COPY_AND_ASSIGN(BackgroundTask);
protected:
	bool isRunning_ = false;
	bool isCanceled_ = false;
};

#endif
