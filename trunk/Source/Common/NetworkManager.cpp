/*
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2010 ZendeN <zenden2k@gmail.com>
	 
    HomePage:    http://zenden.ws/imageuploader

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdafx.h>
#include "NetworkManager.h"
#include <sys/stat.h>
#include <fcntl.h>

#ifndef _DEBUG
	#pragma comment(lib,"libcurl.lib")
#else
	#pragma comment(lib,"libcurld_imp.lib")
#endif
#pragma comment(lib, "Ws2_32.lib")
size_t simple_read_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
	return  fread(ptr, size, nmemb, (FILE*)stream);
}

int NetworkManager::set_sockopts(void * clientp, curl_socket_t sockfd, curlsocktype purpose) 
{
	// See http://support.microsoft.com/kb/823764
	NetworkManager* nm = reinterpret_cast<NetworkManager*>(clientp);
	int val = nm->m_UploadBufferSize + 32;
	setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (const char *)&val, sizeof(val));
	return 0;
}

int NetworkManager::private_static_writer(char *data, size_t size, size_t nmemb, void *buffer_in)
{
	CallBackData* cbd = reinterpret_cast<CallBackData*>(buffer_in);
	NetworkManager* nm = cbd->nmanager;
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

 
void NetworkManager::setProxy(const NString &host, int port, int type)
{
	curl_easy_setopt(curl_handle, CURLOPT_PROXY, host.c_str());
	curl_easy_setopt(curl_handle, CURLOPT_PROXYPORT, port);	
	curl_easy_setopt(curl_handle, CURLOPT_PROXYTYPE, type);
	curl_easy_setopt(curl_handle, CURLOPT_NOPROXY, "localhost,127.0.0.1"); // test
}

void NetworkManager::setProxyUserPassword(const NString &username, const NString password)
{
	if(username.empty());
		//curl_easy_setopt(curl_handle, CURLOPT_PROXYUSERPWD,"");
	else
	{
		std::string authStr = username+":"+password;
		curl_easy_setopt(curl_handle, CURLOPT_PROXYUSERPWD, authStr.c_str());
	}
}

int NetworkManager::private_writer(char *data, size_t size, size_t nmemb)
{
	if(!m_OutFileName.empty())
			{
				if(!m_hOutFile)
					if(!(m_hOutFile = _wfopen(Utf8ToWstring(m_OutFileName).c_str(), L"wb")))
						return 0;
				fwrite(data, size,nmemb, m_hOutFile);
			}
	else
		internalBuffer.append(data, size * nmemb);
		return size * nmemb;
	
}

int NetworkManager::private_header_writer(char *data, size_t size, size_t nmemb)
{

	m_headerBuffer.append(data, size * nmemb);
	return size * nmemb;
}


int NetworkManager::ProgressFunc(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)
{
	NetworkManager *nm = reinterpret_cast<NetworkManager*>(clientp);
	if(nm && nm->m_progressCallbackFunc)
	{
		if(ultotal<0 && nm->m_CurrentFileSize>0)
			ultotal = nm->m_CurrentFileSize;
		return nm->m_progressCallbackFunc(nm->m_progressData, dltotal, dlnow, ultotal, ulnow);
	}
	return 0;
}

void NetworkManager::setMethod(const NString &str)
{
	m_method = str;
}
bool private_Init = false;

NetworkManager::NetworkManager(void)
{
	if(!private_Init)
	{
		curl_global_init(CURL_GLOBAL_ALL);	
	}
	m_hOutFile = 0;
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
	curl_easy_setopt(curl_handle, CURLOPT_COOKIELIST, "");
	setUserAgent("Mozilla/5.0");
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, private_static_writer);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &m_bodyFuncData);	
	curl_easy_setopt(curl_handle, CURLOPT_WRITEHEADER, &m_headerFuncData);
	curl_easy_setopt(curl_handle, CURLOPT_ERRORBUFFER, m_errorBuffer);
	
	curl_easy_setopt(curl_handle, CURLOPT_PROGRESSFUNCTION, &ProgressFunc);
	curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 0);
	curl_easy_setopt(curl_handle, CURLOPT_PROGRESSDATA, this);
	curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, TRUE);
	curl_easy_setopt(curl_handle, CURLOPT_ENCODING, "");
	curl_easy_setopt(curl_handle, CURLOPT_SOCKOPTFUNCTION, &set_sockopts);
	curl_easy_setopt(curl_handle, CURLOPT_SOCKOPTDATA, this);
	
   curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0L);
   curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 0L); 
	//We want the referrer field set automatically when following locations
	curl_easy_setopt(curl_handle, CURLOPT_AUTOREFERER, 1L); 
	curl_easy_setopt(curl_handle, CURLOPT_BUFFERSIZE, 32768L);
}

NetworkManager::~NetworkManager(void)
{
	curl_easy_setopt(curl_handle, CURLOPT_PROGRESSFUNCTION, NULL);
	curl_easy_cleanup(curl_handle);
}

void NetworkManager::addQueryParam(const NString& name, const NString& value)
{
	QueryParam newParam;
	newParam.name = name;
	newParam.value = value;
	newParam.isFile = false;
	m_QueryParams.push_back(newParam);
}

void NetworkManager::addQueryParamFile(const NString& name, const NString& fileName, const NString& displayName, const NString& contentType)
{
	QueryParam newParam;
	newParam.name = name;
	newParam.value = fileName;
	newParam.isFile = true;
	newParam.contentType = contentType;
	newParam.displayName= displayName;
	m_QueryParams.push_back(newParam);
}

void NetworkManager::setUrl(const NString& url)
{
	m_url = url;
	curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
}

void CloseFileList(std::vector<FILE *>& files)
{
	for(int i=0; i<files.size(); i++)
	{
		fclose(files[i]);
	}
	files.clear();
}

bool NetworkManager::doUploadMultipartData()
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
					FILE * curFile = _wfopen(Utf8ToWstring(it->value).c_str(), L"rb"); /* open file to upload */
					if(!curFile) 
					{
						CloseFileList(openedFiles);
						return false; /* can't continue */
					}
					openedFiles.push_back(curFile);
					struct stat file_info; 
				 /* to get the file size */
					if(fstat(fileno(curFile), &file_info) != 0) {
						
						return false; /* can't continue */
						} 
					int  curFileSize = file_info.st_size;

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
	curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 2L);
	curl_result = curl_easy_perform(curl_handle);
	CloseFileList(openedFiles);
	return private_on_finish_request();
}

bool NetworkManager::private_on_finish_request()
{
	private_cleanup_after();
	private_parse_headers();
	if (curl_result != CURLE_OK)
	{
		return false; //fail
	}

	return true;
}

const std::string NetworkManager::responseBody()
{
	return internalBuffer;
}

int NetworkManager::responseCode()
{
	int result=-1;
	curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &result);
	return result;
}

void NetworkManager::addQueryHeader(const NString& name, const NString& value)
{
	CustomHeaderItem chi;
	chi.name = name;
	chi.value = nm_trimStr(value);
	m_QueryHeaders.push_back(chi);
}

bool NetworkManager::doGet(const std::string & url)
{
	if(!url.empty())
		setUrl(url);

	private_initTransfer();
	if(!private_apply_method())
		curl_easy_setopt(curl_handle, CURLOPT_HTTPGET, 1);
	curl_result = curl_easy_perform(curl_handle);
	return private_on_finish_request();

}

bool NetworkManager::doPost(const NString& data)
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
	if(data.empty())
		curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, postData.c_str());
	else
		curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, data.c_str());
		
	curl_result = curl_easy_perform(curl_handle);
	return private_on_finish_request();
}

const NString NetworkManager::urlEncode(const NString& str)
{
	char * encoded = curl_easy_escape(curl_handle, str.c_str() , str.length() );
	std::string res = encoded;
	res+="";
	curl_free(encoded);
	return res;
}
const NString NetworkManager::errorString()
{
	return m_errorBuffer;
}

void NetworkManager::setUserAgent(const NString& userAgentStr)
{
	m_userAgent = userAgentStr;
}

void NetworkManager::private_initTransfer()
{
	private_cleanup_before();
	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, m_userAgent.c_str());

	std::vector<CustomHeaderItem>::iterator it, end = m_QueryHeaders.end();
	struct curl_slist *chunk = NULL;

	for(it = m_QueryHeaders.begin(); it!=end; it++)
	{
		chunk = curl_slist_append(chunk, (it->name + ": " + it->value).c_str());
	}

	curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, chunk);

}

NString NetworkManager::responseHeaderText()
{
	return m_headerBuffer;
}

void nm_splitString(const std::string& str, const std::string& delimiters , std::vector<std::string>& tokens, int maxCount=-1)
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
void NetworkManager::setProgressCallback(curl_progress_callback func, void *data)
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
void NetworkManager::private_parse_headers()
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

NString NetworkManager::responseHeaderByName(const NString& name)
{
	std::vector<CustomHeaderItem>::iterator it, end = m_ResponseHeaders.end();
	
	for(it = m_ResponseHeaders.begin(); it!=end; it++)
	{
		if(it->name == name)
			return it->value;
	}
	return "";

}

int NetworkManager::responseHeaderCount()
{
	return m_ResponseHeaders.size();
}

NString NetworkManager::responseHeaderByIndex(const int index, NString& name)
{
	name = m_ResponseHeaders[index].name;
	return m_ResponseHeaders[index].value;
}

void NetworkManager::private_cleanup_before()
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
void NetworkManager::private_cleanup_after()
{
	m_QueryHeaders.clear();
	m_QueryParams.clear();
	if(m_hOutFile)
	{
		fclose(m_hOutFile);
		m_hOutFile = 0;
	}
	m_OutFileName.clear();
	m_method = "";

	m_uploadData = "";
}

size_t NetworkManager::read_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
	NetworkManager* nm = reinterpret_cast<NetworkManager*>(stream);;
	if(!nm) return 0;
	return nm->private_read_callback(ptr, size, nmemb, stream);
} 

size_t NetworkManager::private_read_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
	size_t retcode;
	int wantsToRead = size * nmemb;
	if(m_uploadingFile)
		retcode = fread(ptr, size, nmemb, m_uploadingFile);
	else
	{
		
		int canRead = min(m_uploadData.size()-m_nUploadDataOffset, wantsToRead);
		memcpy(ptr, m_uploadData.c_str(),canRead);
		m_nUploadDataOffset+=canRead;
		retcode = canRead;
	}
	return retcode;
}


bool NetworkManager::doUpload(const NString& fileName, const NString &data)
{
	struct stat file_info; 
	double speed_upload, total_time;
  
	if(data.empty())
	{
		m_uploadingFile = _wfopen(Utf8ToWstring(fileName).c_str(), L"rb"); /* open file to upload */
		if(!m_uploadingFile) 
		{
			return false; /* can't continue */
		}

	  /* to get the file size */
	  if(fstat(fileno(m_uploadingFile), &file_info) != 0) {

		 return false; /* can't continue */
	  } 
	  m_CurrentFileSize = file_info.st_size;
	}
	else
	{
		m_nUploadDataOffset =0;
		m_uploadData = data;
		m_CurrentFileSize =data.size();
	}
	curl_easy_setopt(curl_handle, CURLOPT_READFUNCTION, read_callback);
		if(!private_apply_method())
	curl_easy_setopt(curl_handle, CURLOPT_POST, 1L);
	curl_easy_setopt(curl_handle,CURLOPT_POSTFIELDS, NULL);
	
	curl_easy_setopt(curl_handle, CURLOPT_READDATA, this);


	char buf[25];
	sprintf(buf, "%d", (int)m_CurrentFileSize);
	addQueryHeader("Content-Length", buf);
	
	private_initTransfer();
	
	curl_easy_setopt(curl_handle, CURLOPT_INFILESIZE_LARGE,
                     (curl_off_t)m_CurrentFileSize); 
	curl_result = curl_easy_perform(curl_handle);

	std::cout<<m_errorBuffer;

	if(m_uploadingFile)
		 fclose(m_uploadingFile);
	bool res = private_on_finish_request();
	return res;

}


bool NetworkManager::private_apply_method()
{
	if(m_method=="POST")
		curl_easy_setopt(curl_handle, CURLOPT_POST, 1L);
	else 	if(m_method=="GET")
		curl_easy_setopt(curl_handle, CURLOPT_HTTPGET, 1L);
	else if (m_method=="PUT")
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

void NetworkManager::setReferer(const NString &str)
{
	curl_easy_setopt(curl_handle, CURLOPT_REFERER, str.c_str());
}

int NetworkManager::getCurlResult()
{
	return curl_result;
}
CURL* NetworkManager::getCurlHandle()
{
	return curl_handle;
}

void NetworkManager::setOutputFile(const NString &str)
{
	m_OutFileName = str;
}

void NetworkManager::setUploadBufferSize(const int size)
{
	m_UploadBufferSize = size;
}