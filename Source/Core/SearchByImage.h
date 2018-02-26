#ifndef FUNC_SEARCHBYIMAGE_H
#define FUNC_SEARCHBYIMAGE_H

#include <string>
#include <atomic>
#include <Core/3rdpart/FastDelegate.h>

class NetworkClient;

class SearchByImage  {

    public:
        explicit SearchByImage(const std::string& fileName);
        void start();
        void stop();
        bool isRunning() const;

        typedef fastdelegate::FastDelegate2<bool, const std::string&> FinishedDelegate;
        void setOnFinished(FinishedDelegate&& fd);
protected:
    std::string fileName_;
    std::atomic<bool> isRunning_;
    std::atomic<bool> stopSignal_;
    FinishedDelegate onFinished_;
    void run();
    void finish(bool success, const std::string &msg = std::string());
    int progressCallback(NetworkClient *clientp, double dltotal, double dlnow, double ultotal, double ulnow);
    static std::string base64EncodeCompat(const std::string& file);
};

#endif