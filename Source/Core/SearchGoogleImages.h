#ifndef FUNC_SEARCHGOOGLEIMAGES_H
#define FUNC_SEARCHGOOGLEIMAGES_H

#include <string>

#include "SearchByImage.h"
#include "Network/INetworkClient.h"

class SearchGoogleImages: public SearchByImageTask  {
    public:
        explicit SearchGoogleImages(std::shared_ptr<INetworkClientFactory> networkClientFactory, std::string fileName);
protected:
    BackgroundTaskResult doJob() override;
    static bool base64EncodeCompat(const std::string& file, std::string& output);
    std::shared_ptr<INetworkClientFactory> networkClientFactory_;
};

#endif
