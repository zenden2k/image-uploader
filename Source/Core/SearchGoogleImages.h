#ifndef FUNC_SEARCHGOOGLEIMAGES_H
#define FUNC_SEARCHGOOGLEIMAGES_H

#include <string>

#include "SearchByImage.h"
#include "Network/INetworkClient.h"

class NetworkClient;

class SearchGoogleImages: public SearchByImage  {

    public:
        explicit SearchGoogleImages(std::shared_ptr<INetworkClientFactory> networkClientFactory, const std::string& fileName);
protected:
    void run() override;
    static bool base64EncodeCompat(const std::string& file, std::string& output);
    std::shared_ptr<INetworkClientFactory> networkClientFactory_;
};

#endif