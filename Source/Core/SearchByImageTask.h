#ifndef CORE_SEARCHBYIMAGETASK_H
#define CORE_SEARCHBYIMAGETASK_H

#include <string>
#include <atomic>
#include <functional>

#include <boost/signals2.hpp>

#include "Core/Utils/CoreTypes.h"
#include "Core/TaskDispatcher.h"

class INetworkClient;

class SearchByImageTask: public CancellableTask {
    public:
        explicit SearchByImageTask(std::string fileName);
        void cancel() override;
        bool isCanceled() override;
        bool isInProgress() override;
        boost::signals2::signal<void(SearchByImageTask*, bool, const std::string&)> onTaskFinished;
    protected:
        std::string fileName_;
        std::atomic<bool> isRunning_;
        std::atomic<bool> stopSignal_;

        void finish(bool success, const std::string& msg = std::string());
        int progressCallback(INetworkClient* clientp, double dltotal, double dlnow, double ultotal, double ulnow);
        
        DISALLOW_COPY_AND_ASSIGN(SearchByImageTask);
};

#endif