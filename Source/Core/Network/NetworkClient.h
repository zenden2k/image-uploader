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
#ifndef IU_CLI
#include <zthread/Mutex.h>
#endif

#include "Core/Utils/CoreUtils.h"

#define NString std::string

std::string nm_trimStr(const std::string& str);
void nm_splitString(const std::string& str, const std::string& delimiters, std::vector<std::string>& tokens, int maxCount = -1);

class NetworkClient
{
	public:
		enum ActionType {
			atNone = 0, atPost, atUpload, atGet
		};
		NetworkClient(void);
		~NetworkClient(void);
		void addQueryParam(const NString& name, const NString& value);
		void addQueryParamFile(const NString& name, const NString& fileName, const NString& displayName = "", const NString& contentType = "");
		void addQueryHeader(const NString& name, const NString& value);
		void setUrl(const NString& url);
		bool doPost(const NString& data="");
		bool doUploadMultipartData();
		bool doUpload(const NString& fileName,const NString &data);
		bool doGet(const std::string &url="");
		const std::string responseBody();
		int responseCode();
		const NString errorString();
		void setUserAgent(const NString& userAgentStr);
		const NString responseHeaderText();
		const NString responseHeaderByName(const NString& name);
		NString responseHeaderByIndex(const int index, NString& name);
		int responseHeaderCount();
		void setProgressCallback(curl_progress_callback func, void *data);
		const NString urlEncode(const NString& str);
		const NString getCurlResultString();
		void setCurlOption(int option, const NString &value);
		void setCurlOptionInt(int option, long value);
		void setMethod(const NString &str);
		void setProxy(const NString &host, int port, int type);
		void setProxyUserPassword(const NString &username, const NString password);
		void setReferer(const NString &str);
		void setOutputFile(const NString &str);
		void setUploadBufferSize(const int size);
		void setChunkOffset(double offset);
		void setChunkSize(double size);
		void setTreatErrorsAsWarnings(bool treat);
		int getCurlResult();
		CURL* getCurlHandle();
		static void Uninitialize();
		void enableResponseCodeChecking(bool enable);
	private:
		enum CallBackFuncType{funcTypeBody,funcTypeHeader};

		struct CallBackData
		{
			CallBackFuncType funcType;
			NetworkClient* nmanager;
		};

		struct CustomHeaderItem
		{
			NString name;
			NString value;
		};

		struct QueryParam
		{
			bool isFile;
			NString name;
			NString value; // also filename
			NString displayName; 
			NString contentType;
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
		static void curl_cleanup();

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
		NString m_url;
		void* m_progressData;
		CURLcode curl_result;
		int64_t m_CurrentFileSize;
		int64_t m_currentUploadDataSize;
		std::vector<QueryParam> m_QueryParams;
		std::vector<CustomHeaderItem> m_QueryHeaders;
		std::vector<CustomHeaderItem> m_ResponseHeaders;
		std::string internalBuffer;
		std::string m_headerBuffer;
		NString m_userAgent;
		char m_errorBuffer[CURL_ERROR_SIZE];;
		std::string m_method;
		struct curl_slist * chunk_;
		bool enableResponseCodeChecking_;
		int64_t chunkOffset_;
		int64_t chunkSize_;
		bool treatErrorsAsWarnings_;
               #ifndef IU_CLI
		static ZThread::Mutex _mutex;
#endif
		static bool _curl_init;
		static bool _is_openssl;
		static char CertFileName[1024];
};

#endif
