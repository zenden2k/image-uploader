#ifndef CORE_SEARCHBYIMAGETASK_H
#define CORE_SEARCHBYIMAGETASK_H

#include <string>
#include <atomic>
#include <functional>

#include "BackgroundTask.h"
#include "Core/Utils/CoreTypes.h"

class INetworkClient;

class SearchByImageTask: public BackgroundTask {
    public:
        explicit SearchByImageTask(std::string fileName);
        std::string message() const;
    protected:
        std::string fileName_, message_;

        /*std::atomic<bool> isRunning_;
        std::atomic<bool> stopSignal_;*/

        void finish(const std::string& msg = {});
        int progressCallback(INetworkClient* clientp, double dltotal, double dlnow, double ultotal, double ulnow);
        
        DISALLOW_COPY_AND_ASSIGN(SearchByImageTask);
};

#endif