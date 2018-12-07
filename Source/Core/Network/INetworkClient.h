/*

Image Uploader -  free application for uploading images/files to the Internet

Copyright 2007-2018 Sergey Svistunov (zenden2k@yandex.ru)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

#ifndef SRC_CORE_NETWORK_INETWORK_CLIENT_H_
#define SRC_CORE_NETWORK_INETWORK_CLIENT_H_

//
// NetworkClient interface
//
#include <string>
#include <Core/Utils/CoreTypes.h>

typedef void CURL;
class CurlShare;

class INetworkClient {
    public:
        virtual ~INetworkClient() {}
        /*! @cond PRIVATE */
        enum ActionType {
            atNone = 0, atPost, atUpload, atGet
        };

        virtual void addQueryParam(const std::string& name, const std::string& value) = 0;
        virtual void addQueryParamFile(const std::string& name, const std::string& fileName, const std::string& displayName = "", const std::string& contentType = "") = 0;
        virtual void addQueryHeader(const std::string& name, const std::string& value) = 0;
        virtual void setUrl(const std::string& url) = 0;
        virtual bool doPost(const std::string& data = "") = 0;
        virtual bool doUploadMultipartData() = 0;
        virtual bool doUpload(const std::string& fileName, const std::string &data) = 0;
        virtual bool doGet(const std::string &url = "") = 0;
        virtual const std::string responseBody() = 0;
        virtual int responseCode() = 0;
        virtual const std::string errorString() = 0;
        virtual void setUserAgent(const std::string& userAgentStr) = 0;
        virtual const std::string responseHeaderText() = 0;
        virtual const std::string responseHeaderByName(const std::string& name) = 0;
        virtual std::string responseHeaderByIndex(const int index, std::string& name) = 0;
        virtual int responseHeaderCount() = 0;
        virtual const std::string getCurlResultString() = 0;
        virtual void setCurlOption(int option, const std::string &value) = 0;
        virtual void setCurlOptionInt(int option, long value) = 0;
        virtual const std::string getCurlInfoString(int option) = 0;
        virtual int getCurlInfoInt(int option) = 0;
        virtual double getCurlInfoDouble(int option) = 0;
        virtual void setMethod(const std::string &str) = 0;
        virtual void setProxy(const std::string &host, int port = 0, int type = 0) = 0;
        virtual void setProxyUserPassword(const std::string &username, const std::string& password) = 0;
        virtual void clearProxy() = 0;
        virtual void setReferer(const std::string &str) = 0;
        virtual void setOutputFile(const std::string &str) = 0;
        virtual void setChunkOffset(double offset) = 0;
        virtual void setChunkSize(double size) = 0;
        virtual int getCurlResult() = 0;
        virtual CURL* getCurlHandle() = 0;
        virtual void setCurlShare(CurlShare* share) = 0;
        virtual void setTimeout(uint32_t timeout) = 0;
        virtual void setConnectionTimeout(uint32_t connection_timeout) = 0;
        virtual void enableResponseCodeChecking(bool enable) = 0;
        virtual void setErrorLogId(const std::string &str) = 0;
};

#endif