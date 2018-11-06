#include "SearchGoogleImages.h"

#include <Core/Network/NetworkClient.h>
#include "CoreFunctions.h"
#include "Utils/CryptoUtils.h"
#include "Core/Utils/DesktopUtils.h"

SearchGoogleImages::SearchGoogleImages(const std::string& fileName) :SearchByImage(fileName) {
}

void SearchGoogleImages::run() {
    auto nc = CoreFunctions::createNetworkClient();
    nc->setProgressCallback(NetworkClient::ProgressCallback(this, &SearchGoogleImages::progressCallback));

    try {
        nc->setUrl("https://images.google.com/searchbyimage/upload");
        nc->addQueryParam("filename", IuCoreUtils::ExtractFileName(fileName_));
        nc->addQueryParam("image_content", base64EncodeCompat(fileName_));
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


std::string SearchGoogleImages::base64EncodeCompat(const std::string& file) {
    std::string res = IuCoreUtils::CryptoUtils::Base64Encode(IuCoreUtils::GetFileContents(file));
    for (unsigned int i = 0; i < res.length(); i++) {
        if (res[i] == '+') {
            res[i] = '-';
        } else if (res[i] == '/') {
            res[i] = '_';
        }
    }
    return res;
}