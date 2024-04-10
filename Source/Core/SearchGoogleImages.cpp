#include "SearchGoogleImages.h"

#include "Core/Network/NetworkClient.h"
#include "Utils/CryptoUtils.h"
#include "Core/Utils/DesktopUtils.h"

SearchGoogleImages::SearchGoogleImages(std::shared_ptr<INetworkClientFactory> networkClientFactory, std::string fileName): SearchByImageTask(std::move(fileName)),
    networkClientFactory_(std::move(networkClientFactory))
{
}

BackgroundTaskResult SearchGoogleImages::doJob() {
    if (isCanceled_) {
        finish("Aborted by user.");
        return BackgroundTaskResult::Canceled;
    }
    auto nc = networkClientFactory_->create();
    using namespace std::placeholders;
    nc->setProgressCallback(std::bind(&SearchGoogleImages::progressCallback, this, _1, _2, _3, _4, _5));

    try {
        nc->setUrl("https://images.google.com/searchbyimage/upload");
        nc->addQueryParam("filename", IuCoreUtils::ExtractFileName(fileName_));
        std::string encoded_file;
        if (!base64EncodeCompat(fileName_, encoded_file)) {
            finish("Unable to encode file.");
            return BackgroundTaskResult::Failed;
        }

        nc->addQueryParam("image_content", encoded_file);
        nc->setCurlOptionInt(CURLOPT_FOLLOWLOCATION, 0);
        nc->doUploadMultipartData();

        if (isCanceled_) {
            finish("Aborted by user.");
            return BackgroundTaskResult::Canceled;
        }
        if (nc->responseCode() != 302) {
            finish("Server sent unexpected result.");
            return BackgroundTaskResult::Failed;
        }

        const std::string url = nc->responseHeaderByName("Location");

        if (url.empty()) {
            finish("Server sent unexpected result.");
            return BackgroundTaskResult::Failed;
        }

        if (!DesktopUtils::ShellOpenUrl(url)) {
            finish("Unable to launch default web browser.");
            return BackgroundTaskResult::Failed;
        }
    } catch (NetworkClient::AbortedException&){
        finish("Aborted by user.");
        return BackgroundTaskResult::Canceled;
    }
    
    finish();
    return BackgroundTaskResult::Success;
}


bool SearchGoogleImages::base64EncodeCompat(const std::string& file, std::string& output) {
    if (!IuCoreUtils::CryptoUtils::Base64EncodeFile(file, output)) {
        return false;
    }

    for (size_t i = 0; i < output.length(); i++) {
        if (output[i] == '+') {
            output[i] = '-';
        } else if (output[i] == '/') {
            output[i] = '_';
        }
    }
    return true;
}
