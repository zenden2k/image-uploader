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

#ifndef NOMINMAX
#define NOMINMAX
#endif 

#include "NetworkClient.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <memory.h>
#include <cstdio>
#include <algorithm>
#include <iostream>
#include "Core/Utils/CoreUtils.h"
#include <Core/Logging.h>

char NetworkClient::CertFileName[1024]= "";

size_t simple_read_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
	return  fread(ptr, size, nmemb, (FILE*)stream);
}

int NetworkClient::set_sockopts(void * clientp, curl_socket_t sockfd, curlsocktype purpose) 
{
	#ifdef _WIN32
		// See http://support.microsoft.com/kb/823764
		NetworkClient* nm = reinterpret_cast<NetworkClient*>(clientp);
		int val = nm->m_UploadBufferSize + 32;
		setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (const char *)&val, sizeof(val));
	#endif
	return 0;
}

int NetworkClient::private_static_writer(char *data, size_t size, size_t nmemb, void *buffer_in)
{
	CallBackData* cbd = reinterpret_cast<CallBackData*>(buffer_in);
	NetworkClient* nm = cbd->nmanager;
	if(nm)
	{
		if(cbd->funcType == funcTypeBody)
		{
			return nm->private_writer(data, size, nmemb);
		}
		else
			return nm->private_header_writer(data, size, nmemb);
	}
	return 0;
}

void NetworkClient::setProxy(const NString &host, int port, int type)
{
	curl_easy_setopt(curl_handle, CURLOPT_PROXY, host.c_str());
	curl_easy_setopt(curl_handle, CURLOPT_PROXYPORT, (long)port);	
	curl_easy_setopt(curl_handle, CURLOPT_PROXYTYPE, (long)type);
	curl_easy_setopt(curl_handle, CURLOPT_NOPROXY, "localhost,127.0.0.1"); // test
} 

void NetworkClient::setProxyUserPassword(const NString &username, const NString password)
{
	if(username.empty());
		//curl_easy_setopt(curl_handle, CURLOPT_PROXYUSERPWD,"");
	else
	{
		std::string authStr = username+":"+password;
		curl_easy_setopt(curl_handle, CURLOPT_PROXYUSERPWD, authStr.c_str());
	}
}

int NetworkClient::private_writer(char *data, size_t size, size_t nmemb)
{
	if(!m_OutFileName.empty())
	{
		if(!m_hOutFile)
			if(!(m_hOutFile = IuCoreUtils::fopen_utf8(m_OutFileName.c_str(), "wb")))
				return 0;
		fwrite(data, size,nmemb, m_hOutFile);
	}
	else
		internalBuffer.append(data, size * nmemb);
	return size * nmemb;
}

int NetworkClient::private_header_writer(char *data, size_t size, size_t nmemb)
{
	m_headerBuffer.append(data, size * nmemb);
	return size * nmemb;
}

int NetworkClient::ProgressFunc(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)
{
	NetworkClient *nm = reinterpret_cast<NetworkClient*>(clientp);
	if(nm && nm->m_progressCallbackFunc)
	{
		if  (nm->chunkOffset_>=0 && nm->chunkSize_>0 && nm->m_currentActionType == atUpload) {
			ultotal = nm->m_CurrentFileSize;
			ulnow = nm->chunkOffset_ + ulnow;
		} else if( ((ultotal<=0 && nm->m_CurrentFileSize>0)) && nm->m_currentActionType == atUpload)
			ultotal = double(nm->m_CurrentFileSize);
		return nm->m_progressCallbackFunc(nm->m_progressData, dltotal, dlnow, ultotal, ulnow);
	}
	return 0;
}

void NetworkClient::setMethod(const NString &str)
{
	m_method = str;
}

bool  NetworkClient::_curl_init = false;
bool  NetworkClient::_is_openssl = false;
#ifndef IU_CLI
ZThread::Mutex NetworkClient::_mutex;
#endif
NetworkClient::NetworkClient(void)
{
    #ifndef IU_CLI
	_mutex.acquire();
#endif
	if(!_curl_init)
	{
		enableResponseCodeChecking_ = true;
		curl_global_init(CURL_GLOBAL_ALL);
		curl_version_info_data * infoData = curl_version_info(CURLVERSION_NOW);
		_is_openssl =  strstr(infoData->ssl_version, "WinSSL")!=infoData->ssl_version;
#ifdef WIN32
		GetModuleFileNameA(0, CertFileName, 1023);
		int i, len = lstrlenA(CertFileName);
		for(i=len; i>=0; i--)
		{
			if(CertFileName[i] == _T('\\')) {
				CertFileName[i+1] = 0;
				break;
			}
		}
		StrCatA(CertFileName, "curl-ca-bundle.crt");
#endif
		atexit(&curl_cleanup);
		_curl_init = true;
	}
    #ifndef IU_CLI
	_mutex.release();
#endif
	m_hOutFile = 0;
	chunkOffset_ = -1;
	chunkSize_ = -1;
	chunk_ = 0;
	m_CurrentFileSize = -1;
	m_uploadingFile = NULL;
	*m_errorBuffer = 0;
	m_progressCallbackFunc = NULL;
	curl_handle = curl_easy_init(); // Initializing libcurl
	m_bodyFuncData.funcType = funcTypeBody;
	m_bodyFuncData.nmanager = this;
	m_UploadBufferSize = 65536;
	m_headerFuncData.funcType = funcTypeHeader;
	m_headerFuncData.nmanager = this;
	m_nUploadDataOffset = 0;
	treatErrorsAsWarnings_ = false;
	curl_easy_setopt(curl_handle, CURLOPT_COOKIELIST, "");
	setUserAgent("Mozilla/5.0");

	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, private_static_writer);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &m_bodyFuncData);	
	curl_easy_setopt(curl_handle, CURLOPT_WRITEHEADER, &m_headerFuncData);
	curl_easy_setopt(curl_handle, CURLOPT_ERRORBUFFER, m_errorBuffer);
	
	curl_easy_setopt(curl_handle, CURLOPT_PROGRESSFUNCTION, &ProgressFunc);
	curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 0L);
	curl_easy_setopt(curl_handle, CURLOPT_PROGRESSDATA, this);
        curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl_handle, CURLOPT_ENCODING, "");
	curl_easy_setopt(curl_handle, CURLOPT_SOCKOPTFUNCTION, &set_sockopts);
	curl_easy_setopt(curl_handle, CURLOPT_SOCKOPTDATA, this);
	 
#ifdef _WIN32
	curl_easy_setopt(curl_handle, CURLOPT_CAINFO, CertFileName);
#endif
	curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 1L); 
	curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 2L);
	//curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0L); 
	//curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 0L);

	//We want the referrer field set automatically when following locations
	curl_easy_setopt(curl_handle, CURLOPT_AUTOREFERER, 1L); 
	curl_easy_setopt(curl_handle, CURLOPT_BUFFERSIZE, 32768L);
	   curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 0L);
}

NetworkClient::~NetworkClient(void)
{
	curl_easy_setopt(curl_handle, CURLOPT_PROGRESSFUNCTION, (long)NULL);
	curl_easy_cleanup(curl_handle);
}

void NetworkClient::addQueryParam(const NString& name, const NString& value)
{
	QueryParam newParam;
	newParam.name = name;
	newParam.value = value;
	newParam.isFile = false;
	m_QueryParams.push_back(newParam);
}

void NetworkClient::addQueryParamFile(const NString& name, const NString& fileName, const NString& displayName, const NString& contentType)
{
	QueryParam newParam;
	newParam.name = name;
	newParam.value = fileName;
	newParam.isFile = true;
	newParam.contentType = contentType;
	newParam.displayName= displayName;
	m_QueryParams.push_back(newParam);
}

void NetworkClient::setUrl(const NString& url)
{
	m_url = url;
	curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
}

void CloseFileList(std::vector<FILE *>& files)
{
	for(size_t i=0; i<files.size(); i++)
	{
		fclose(files[i]);
	}
	files.clear();
}

bool NetworkClient::doUploadMultipartData()
{
	private_initTransfer();
	std::vector<FILE *> openedFiles;

	struct curl_httppost *formpost=NULL;
	struct curl_httppost *lastptr=NULL;

	{
		std::vector<QueryParam>::iterator it, end = m_QueryParams.end();

		for(it=m_QueryParams.begin(); it!=end; it++)
		{
			if(it->isFile)
			{
					curl_easy_setopt(curl_handle, CURLOPT_READFUNCTION, simple_read_callback);
                                        std::string fileName = it->value;
					FILE * curFile = IuCoreUtils::fopen_utf8(it->value.c_str(), "rb"); /* open file to upload */
					if(!curFile) 
					{
						CloseFileList(openedFiles);
						return false; /* can't continue */
					}
					openedFiles.push_back(curFile);
					// FIXME: 64bit file size support!
                    long  curFileSize = IuCoreUtils::getFileSize(fileName);

				if(it->contentType.empty())
					curl_formadd(&formpost,
						&lastptr,
						CURLFORM_COPYNAME, it->name.c_str(),
						CURLFORM_FILENAME, it->displayName.c_str(),
						CURLFORM_STREAM, /*it->value.c_str()*/curFile,
						CURLFORM_CONTENTSLENGTH, curFileSize,
						CURLFORM_END);
				else 
					curl_formadd(&formpost,
						&lastptr,
						CURLFORM_COPYNAME, it->name.c_str(),
						CURLFORM_FILENAME, it->displayName.c_str(),
						CURLFORM_STREAM, /*it->value.c_str()*/curFile,
						CURLFORM_CONTENTSLENGTH, curFileSize,
						CURLFORM_CONTENTTYPE, it->contentType.c_str(),
						CURLFORM_END);
			}
			else
			{
				 curl_formadd(&formpost,
						&lastptr,
						CURLFORM_COPYNAME, it->name.c_str(),
						CURLFORM_COPYCONTENTS, it->value.c_str(),
						CURLFORM_END);
			}
		}
	}

	curl_easy_setopt(curl_handle, CURLOPT_HTTPPOST, formpost);
	m_currentActionType = atUpload;
	curl_result = curl_easy_perform(curl_handle);
	CloseFileList(openedFiles);
	curl_formfree(formpost);
	return private_on_finish_request();
}

bool NetworkClient::private_on_finish_request()
{
	private_checkResponse();
	private_cleanup_after();
	private_parse_headers();
	if (curl_result != CURLE_OK)
	{
		return false; //fail
	}

	return true;
}

const std::string NetworkClient::responseBody()
{
	return internalBuffer;
}

int NetworkClient::responseCode()
{
    long result=-1;
	curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &result);
	return result;
}

void NetworkClient::addQueryHeader(const NString& name, const NString& value)
{
	CustomHeaderItem chi;
	chi.name = name;
	chi.value = /*nm_trimStr*/(value);
	m_QueryHeaders.push_back(chi);
}

bool NetworkClient::doGet(const std::string & url)
{
	if(!url.empty())
		setUrl(url);

	private_initTransfer();
	if(!private_apply_method())
		curl_easy_setopt(curl_handle, CURLOPT_HTTPGET, 1);
	m_currentActionType = atGet;
	curl_result = curl_easy_perform(curl_handle);
	return private_on_finish_request();

}

bool NetworkClient::doPost(const NString& data)
{
	private_initTransfer();
	if(!private_apply_method())
   curl_easy_setopt(curl_handle, CURLOPT_POST, 1L);
	std::string postData;
	std::vector<QueryParam>::iterator it, end = m_QueryParams.end();

		for(it=m_QueryParams.begin(); it!=end; it++)
		{
			if(!it->isFile)
			{
				postData+= urlEncode(it->name)+"="+urlEncode(it->value)+"&";
			}
		}

	if(data.empty()) {
		curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, postData.c_str());
	}
	else {
		curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, data.c_str());
	}

	m_currentActionType = atPost;	
	curl_result = curl_easy_perform(curl_handle);
	return private_on_finish_request();
}

const NString NetworkClient::urlEncode(const NString& str)
{
	char * encoded = curl_easy_escape(curl_handle, str.c_str() , str.length() );
	std::string res = encoded;
	res+="";
	curl_free(encoded);
	return res;
}
const NString NetworkClient::errorString()
{
	return m_errorBuffer;
}

void NetworkClient::setUserAgent(const NString& userAgentStr)
{
	m_userAgent = userAgentStr;
}
 
void NetworkClient::private_initTransfer()
{
	private_cleanup_before();
	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, m_userAgent.c_str());

	std::vector<CustomHeaderItem>::iterator it, end = m_QueryHeaders.end();
	chunk_ = NULL; 

	for(it = m_QueryHeaders.begin(); it!=end; it++)
	{
		if ( it->value == "\n" ) {
			chunk_ = curl_slist_append(chunk_, (it->name + ";" + it->value).c_str());
		} else {
			chunk_ = curl_slist_append(chunk_, (it->name + ": " + it->value).c_str());
		} 
		
	}

	curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, chunk_);
}

void NetworkClient::private_checkResponse()
{
	if ( !enableResponseCodeChecking_ )  {
		return;
	}
	int code = responseCode();
	if ( ( !code || (code>= 400 && code<=499)) && errorString() != "Callback aborted" ) {
		(treatErrorsAsWarnings_ ? LOG(WARNING) : LOG(ERROR) ) << "Request to the URL '"<<m_url<<"' failed. \r\nResponse code: "<<code<<"\r\n"<<errorString()<<"\r\n"<<internalBuffer;
	}
}

void NetworkClient::curl_cleanup()
{
	curl_global_cleanup();
}

const NString NetworkClient::responseHeaderText()
{
	return m_headerBuffer;
}

void nm_splitString(const std::string& str, const std::string& delimiters, std::vector<std::string>& tokens, int maxCount)
{
    // Skip delimiters at beginning.
	std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    std::string::size_type pos     = str.find_first_of(delimiters, lastPos);
	int counter =0;
    while (std::string::npos != pos || std::string::npos != lastPos)
    {
		 counter++;
		 if(counter == maxCount){
			 tokens.push_back(str.substr(lastPos, str.length()));break;
		 }
		 else

        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
}
void NetworkClient::setProgressCallback(curl_progress_callback func, void *data)
{
	m_progressCallbackFunc = func;
	m_progressData = data;
}

std::string nm_trimStr(const std::string& str)
{
	std::string res;
	// Trim Both leading and trailing spaces
	size_t startpos = str.find_first_not_of(" \t\r\n"); // Find the first character position after excluding leading blank spaces
	size_t endpos = str.find_last_not_of(" \t\r\n"); // Find the first character position from reverse af

	// if all spaces or empty return an empty string
	if(( std::string::npos == startpos ) || ( std::string::npos == endpos))
	{
       res = "";
	}
   else
       res = str.substr( startpos, endpos-startpos+1 );
	return res;
}

#include <iostream>
void NetworkClient::private_parse_headers()
{
	std::vector<std::string> headers;
	nm_splitString(m_headerBuffer, "\n",headers);
	std::vector<std::string>::iterator it;

	for(it=headers.begin(); it!=headers.end(); it++)
	{
		std::vector<std::string> thisHeader;
		nm_splitString(*it, ":",thisHeader, 2);

		if(thisHeader.size() == 2)
		{
			CustomHeaderItem chi;
			chi.name = nm_trimStr(thisHeader[0]);
			chi.value = nm_trimStr(thisHeader[1]);
			m_ResponseHeaders.push_back(chi);
		}
	}
}

const NString NetworkClient::responseHeaderByName(const NString& name)
{
	std::vector<CustomHeaderItem>::iterator it, end = m_ResponseHeaders.end();
	
	for(it = m_ResponseHeaders.begin(); it!=end; it++)
	{
		if(it->name == name)
			return it->value;
	}
	return "";

}

int NetworkClient::responseHeaderCount()
{
	return m_ResponseHeaders.size();
}

NString NetworkClient::responseHeaderByIndex(const int index, NString& name)
{
	name = m_ResponseHeaders[index].name;
	return m_ResponseHeaders[index].value;
}

void NetworkClient::private_cleanup_before()
{
	std::vector<CustomHeaderItem>::iterator it, end = m_QueryHeaders.end();

	bool add = true;
	for(it = m_QueryHeaders.begin(); it!=end; it++)
	{
		if(it->name == "Expect" ) { add = false; break; }
	}
	addQueryHeader("Expect", "");
	m_ResponseHeaders.clear();
	internalBuffer.clear();
	m_headerBuffer.clear();
}

void NetworkClient::private_cleanup_after()
{
	m_currentActionType = atNone;
	m_QueryHeaders.clear();
	m_QueryParams.clear();
	if(m_hOutFile)
	{
		fclose(m_hOutFile);
		m_hOutFile = 0;
	}
	m_OutFileName.clear();
	m_method = "";
	curl_easy_setopt(curl_handle, CURLOPT_INFILESIZE_LARGE, (curl_off_t )-1);

	m_uploadData.clear();
	m_uploadingFile = NULL;
	chunkOffset_ = -1;
	chunkSize_ = -1;
	enableResponseCodeChecking_ = true;
	m_nUploadDataOffset = 0;
	/*curl_easy_setopt(curl_handle, CURLOPT_READFUNCTION, 0L);
	curl_easy_setopt(curl_handle, CURLOPT_READDATA, 0L);*/
	if(chunk_)
	{
		curl_slist_free_all(chunk_);
		chunk_ = 0;
	}
}

size_t NetworkClient::read_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
	NetworkClient* nm = reinterpret_cast<NetworkClient*>(stream);;
	if(!nm) return 0;
	return nm->private_read_callback(ptr, size, nmemb, stream);
} 

size_t NetworkClient::private_read_callback(void *ptr, size_t size, size_t nmemb, void *)
{
	size_t retcode;
	int wantsToRead = size * nmemb;
	if(m_uploadingFile)
		retcode = fread(ptr, size, nmemb, m_uploadingFile);
	else
	{
		// dont event try to remove "<>" brackets!!
		int canRead = std::min<>((int)m_uploadData.size()-m_nUploadDataOffset, (int)wantsToRead);
		memcpy(ptr, m_uploadData.c_str(),canRead);
		m_nUploadDataOffset+=canRead;
		retcode = canRead;
	}
	return retcode;
}


bool NetworkClient::doUpload(const NString& fileName, const NString &data)
{
	if(!fileName.empty())
	{
		m_uploadingFile = IuCoreUtils::fopen_utf8(fileName.c_str(), "rb"); /* open file to upload */
		if(!m_uploadingFile) 
		{
			LOG(ERROR)<< "Failed to open file '" << fileName << "'";
			return false; /* can't continue */
		}
		m_CurrentFileSize = IuCoreUtils::getFileSize(fileName);
		m_currentUploadDataSize = m_CurrentFileSize;
		if(m_CurrentFileSize < 0) 
			return false;
		if ( chunkSize_  >0 && chunkOffset_ >= 0 ) {
			m_currentUploadDataSize = chunkSize_;
			if ( fseek(m_uploadingFile, chunkOffset_, SEEK_SET)) {
			}
		} 
		m_currentActionType = atUpload;
	}
	else
	{
		m_nUploadDataOffset =0;
		m_uploadData = data;
		m_CurrentFileSize = data.length();
		m_currentUploadDataSize = m_CurrentFileSize;
		m_currentActionType = atPost;
	}
	curl_easy_setopt(curl_handle, CURLOPT_READFUNCTION, read_callback);
		if(!private_apply_method())
	curl_easy_setopt(curl_handle, CURLOPT_POST, 1L);
	curl_easy_setopt(curl_handle,CURLOPT_POSTFIELDS, NULL);
	curl_easy_setopt(curl_handle, CURLOPT_READDATA, this);
	
	if ( m_method != "PUT" ) {
		addQueryHeader("Content-Length", IuCoreUtils::int64_tToString(m_currentUploadDataSize));
	}
	private_initTransfer();
	curl_easy_setopt(curl_handle, CURLOPT_INFILESIZE_LARGE, (curl_off_t)m_currentUploadDataSize);
	
	curl_result = curl_easy_perform(curl_handle);
	if(m_uploadingFile)
		 fclose(m_uploadingFile);
	bool res = private_on_finish_request();
	return res;
}

bool NetworkClient::private_apply_method()
{
	curl_easy_setopt(curl_handle, CURLOPT_CUSTOMREQUEST,NULL);
	curl_easy_setopt(curl_handle, CURLOPT_UPLOAD, 0L);
	if(m_method == "POST")
		curl_easy_setopt(curl_handle, CURLOPT_POST, 1L);
	else 	if(m_method == "GET")
		curl_easy_setopt(curl_handle, CURLOPT_HTTPGET, 1L);
	else if (m_method == "PUT")
			curl_easy_setopt(curl_handle, CURLOPT_UPLOAD, 1L);
	else if (!m_method.empty())
	{
		curl_easy_setopt(curl_handle, CURLOPT_CUSTOMREQUEST,m_method.c_str());
	}
	else 
	{
		curl_easy_setopt(curl_handle, CURLOPT_CUSTOMREQUEST,NULL);
		return false;
	}
	return true;

}

void NetworkClient::setReferer(const NString &str)
{
	curl_easy_setopt(curl_handle, CURLOPT_REFERER, str.c_str());
}

int NetworkClient::getCurlResult()
{
	return curl_result;
}
CURL* NetworkClient::getCurlHandle()
{
	return curl_handle;
}

void NetworkClient::setOutputFile(const NString &str)
{
	m_OutFileName = str;
}

void NetworkClient::setUploadBufferSize(const int size)
{
	m_UploadBufferSize = size;
}

void NetworkClient::setChunkOffset(double offset)
{
	chunkOffset_ = offset;
}

void NetworkClient::setChunkSize(double size)
{
	chunkSize_ = size;
}

void NetworkClient::setTreatErrorsAsWarnings(bool treat)
{
	treatErrorsAsWarnings_ = treat;
}

const NString  NetworkClient::getCurlResultString()
{
	const char * str = curl_easy_strerror(curl_result);
	std::string res = str;
	res+="";
	//curl_free(str);
	return res;
}

void NetworkClient::Uninitialize()
{
	if(_curl_init)
	{
		curl_global_cleanup();	
	}
}


void NetworkClient::enableResponseCodeChecking(bool enable)
{
	enableResponseCodeChecking_ = enable;
}

void NetworkClient::setCurlOption(int option, const NString &value) {
	curl_easy_setopt(curl_handle, (CURLoption)option, value.c_str());
}

void NetworkClient::setCurlOptionInt(int option, long value) {
	curl_easy_setopt(curl_handle, (CURLoption)option, value);
}