#include "SearchGoogleImages.h"

#include "Core/Network/NetworkClient.h"
#include "Utils/CryptoUtils.h"
#include "Core/Utils/DesktopUtils.h"

SearchGoogleImages::SearchGoogleImages(std::shared_ptr<INetworkClientFactory> networkClientFactory, const std::string& fileName) :SearchByImage(fileName),
    networkClientFactory_(std::move(networkClientFactory))
{
}

void SearchGoogleImages::run() {
    auto nc = networkClientFactory_->create();
    using namespace std::placeholders;
    nc->setProgressCallback(std::bind(&SearchGoogleImages::progressCallback, this, _1, _2, _3, _4, _5));

    try {
        nc->setUrl("https://images.google.com/searchbyimage/upload");
        nc->addQueryParam("filename", IuCoreUtils::ExtractFileName(fileName_));
        std::string encoded_file;
        if (!base64EncodeCompat(fileName_, encoded_file)) {
            finish(false, "Unable to encode file.");
            return;
        }

        nc->addQueryParam("image_content", encoded_file);
        nc->setCurlOptionInt(CURLOPT_FOLLOWLOCATION, 0);
        nc->doUploadMultipartData();

        if (stopSignal_) {
            finish(false, "Aborted by user.");
            return;
        }
        if (nc->responseCode() != 302) {
            finish(false, "Server sent unexpected result.");
            return;
        }

        std::string url = nc->responseHeaderByName("Location");

        if (url.empty()) {
            finish(false, "Server sent unexpected result.");
            return;
        }

        if (!DesktopUtils::ShellOpenUrl(url)) {
            finish(false, "Unable to launch default web browser.");
            return;
        }
    } catch (NetworkClient::AbortedException&){
        finish(false, "Aborted by user.");
        return;
    }
    
    finish(true);
}


bool SearchGoogleImages::base64EncodeCompat(const std::string& file, std::string& output) {
    if (!IuCoreUtils::CryptoUtils::Base64EncodeFile(file, output)) {
        return false;
    }

    for (unsigned int i = 0; i < output.length(); i++) {
        if (output[i] == '+') {
            output[i] = '-';
        } else if (output[i] == '/') {
            output[i] = '_';
        }
    }
    return true;
}