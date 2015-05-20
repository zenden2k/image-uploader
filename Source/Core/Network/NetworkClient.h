/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2015 Sergey Svistunov (zenden2k@gmail.com)

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

#ifndef _NETWORK_MANAGER_H_
#define _NETWORK_MANAGER_H_

#include <string>
#include <vector>

#include <curl/curl.h>
//#include <curl/types.h>
#include <mutex>

#include "Core/Utils/CoreUtils.h"
#include "Core/Utils/CoreTypes.h"
#include "CurlShare.h"

std::string nm_trimStr(const std::string& str);
void nm_splitString(const std::string& str, const std::string& delimiters, std::vector<std::string>& tokens, int maxCount = -1);

/**
@brief  HTTP/FTP client (libcurl wrapper).

Note: After each completed request, most of the options are set to default values.
*/
class NetworkClient
{
    public:
        /*! @cond PRIVATE */
        enum ActionType {
            atNone = 0, atPost, atUpload, atGet
        };
        /*! @endcond */
        NetworkClient(void);
        ~NetworkClient(void);
        void addQueryParam(const std::string& name, const std::string& value);
        void addQueryParamFile(const std::string& name, const std::string& fileName, const std::string& displayName = "", const std::string& contentType = "");
        /**
        Example:
        @include networkclient_header.nut
        */
        void addQueryHeader(const std::string& name, const std::string& value);
        void setUrl(const std::string& url);
        /**
        Example 1
        @include networkclient_post.nut
        Example 2
        @include networkclient_post_raw.nut
        */
        bool doPost(const std::string& data="");
        /**
        Example
        @include networkclient_upload.nut
        */
        bool doUploadMultipartData();

        /**
        Example 1
        @include networkclient_put.nut
        Example 2
        @include networkclient_ftp.nut
        */
        bool doUpload(const std::string& fileName,const std::string &data);

        /**
        Example 1
        @include networkclient_get.nut
        Example 2 
        @include networkclient_get_file.nut
        */
        bool doGet(const std::string &url="");
        const std::string responseBody();
        int responseCode();
        const std::string errorString();
        void setUserAgent(const std::string& userAgentStr);
        const std::string responseHeaderText();
        const std::string responseHeaderByName(const std::string& name);
        std::string responseHeaderByIndex(const int index, std::string& name);
        int responseHeaderCount();
        /*! @cond PRIVATE */
        void setProgressCallback(curl_progress_callback func, void *data);
        /*! @endcond */
        const std::string urlEncode(const std::string& str);
        const std::string getCurlResultString();
        void setCurlOption(int option, const std::string &value);
        void setCurlOptionInt(int option, long value);
        const std::string getCurlInfoString(int option);
        int getCurlInfoInt(int option);
        double getCurlInfoDouble(int option);
        void setMethod(const std::string &str);
        /*! @cond PRIVATE */
        void setProxy(const std::string &host, int port, int type);
        void setProxyUserPassword(const std::string &username, const std::string password);
        /*! @endcond */
        void setReferer(const std::string &str);
        void setOutputFile(const std::string &str);
        /*! @cond PRIVATE */
        void setUploadBufferSize(const int size);
        /*! @endcond */
        /**
        Set the byte offset of current chunk, relative to the beginning of the full file.
        @since 1.3.0
        */
        void setChunkOffset(double offset);

        /**
        Sets size of current chunk.
        @since 1.3.0
        */
        void setChunkSize(double size);
        /*! @cond PRIVATE */
        void setTreatErrorsAsWarnings(bool treat);
        /*! @endcond */
        int getCurlResult();
        /*! @cond PRIVATE */
        CURL* getCurlHandle();
        void setCurlShare(CurlShare* share);
        /*! @endcond */
        void enableResponseCodeChecking(bool enable);

        class AbortedException : public std::runtime_error {
        public:
            AbortedException(const std::string& msg) : std::runtime_error(msg) {}
            AbortedException(const AbortedException& ex) : std::runtime_error(ex) {}
        };
    private:
        //DISALLOW_COPY_AND_ASSIGN(NetworkClient);
        enum CallBackFuncType{funcTypeBody,funcTypeHeader};

        struct CallBackData
        {
            CallBackFuncType funcType;
            NetworkClient* nmanager;
        };

        struct CustomHeaderItem
        {
            std::string name;
            std::string value;
        };

        struct QueryParam
        {
            bool isFile;
            std::string name;
            std::string value; // also filename
            std::string displayName; 
            std::string contentType;
        };

        static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *stream);
        static int ProgressFunc(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow);
        static int private_static_writer(char *data, size_t size, size_t nmemb, void *buffer_in);
        int private_writer(char *data, size_t size, size_t nmemb);
        int private_header_writer(char *data, size_t size, size_t nmemb);
        size_t private_read_callback(void *ptr, size_t size, size_t nmemb, void *stream);
        static int set_sockopts(void * clientp, curl_socket_t sockfd, curlsocktype purpose);
        bool private_apply_method();
        void private_parse_headers();
        void private_cleanup_before();
        void private_cleanup_after();
        bool private_on_finish_request();
        void private_initTransfer();
        void private_checkResponse();
        public:
        static void curl_init();
        static void curl_cleanup();
        protected:

        int m_UploadBufferSize;
        CURL *curl_handle;
        FILE *m_hOutFile;
        std::string m_OutFileName;
        FILE *m_uploadingFile;
        std::string m_uploadData;
        ActionType m_currentActionType;
        int m_nUploadDataOffset;
        CallBackData m_bodyFuncData;
        curl_progress_callback m_progressCallbackFunc;
        CallBackData m_headerFuncData;
        std::string m_url;
        void* m_progressData;
        CURLcode curl_result;
        int64_t m_CurrentFileSize;
        int64_t m_currentUploadDataSize;
        std::vector<QueryParam> m_QueryParams;
        std::vector<CustomHeaderItem> m_QueryHeaders;
        std::vector<CustomHeaderItem> m_ResponseHeaders;
        std::string internalBuffer;
        std::string m_headerBuffer;
        std::string m_userAgent;
        char m_errorBuffer[CURL_ERROR_SIZE];
        std::string m_method;
        struct curl_slist * chunk_;
        bool enableResponseCodeChecking_;
        int64_t chunkOffset_;
        int64_t chunkSize_;
        bool treatErrorsAsWarnings_;
        CurlShare* curlShare_;
        static std::mutex _mutex;
        static bool _curl_init;
//        static bool _is_openssl;
        static char CertFileName[1024];
};

#endif
