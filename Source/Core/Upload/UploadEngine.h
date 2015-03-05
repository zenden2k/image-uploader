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
#ifndef IU_CORE_UPLOADENGINE_H
#define IU_CORE_UPLOADENGINE_H

#pragma once

#include <vector>
#include <string>
#include <map>
#include "Core/3rdpart/FastDelegate.h"
#include "Core/Utils/CoreUtils.h"
#include "Core/Network/NetworkManager.h"
#include "CommonTypes.h"

struct LoginInfo
{
	std::string Login, Password, Cookies;
	bool DoAuth;
};

struct ActionVariable
{
	std::string Name;
	int nIndex;
};

struct UploadAction
{
	int Index;
	bool IgnoreErrors;
	bool OnlyOnce;
	std::string Url;
	std::string Description;
	std::string Referer;
	std::string PostParams;
	std::string Type;
	std::string RegExp;
	std::vector<ActionVariable> Variables;
	int RetryLimit;
	int NumOfTries;
};

struct CFolderItem
{
	CFolderItem()
	{
		accessType = 0;
		itemCount = -1;
	}

	std::string title;
	std::string summary;
	std::string id;
	std::string parentid;
	std::string viewUrl;
	int accessType;
	int itemCount;
	const std::string getTitle() { return title;}
	const std::string getSummary() { return summary;}
	const std::string getId() { return (id);}
	const std::string getParentId() { return (parentid);}
	const int getItemCount() { return itemCount; }
	const int getAccessType() { return accessType; }

	void setTitle(const std::string& str) { title = (str); }
	void setViewUrl(const std::string& str) { viewUrl = (str); }

	void setSummary(const std::string& str) { summary = (str); }
	void setId(const std::string& str) { id = (str); }
	void setParentId(const std::string& str) { parentid = (str); }
	void setAccessType(const int type) {  accessType=type; }
	void setItemCount(const int count) {  itemCount=count; }	
};

struct ServerSettingsStruct
{
	ServerSettingsStruct(){authData.DoAuth=0;}
	std::map<std::string, std::string> params;
	LoginInfo authData;
	CFolderItem newFolder;
	const std::string getParam(const std::string& name)
	{
		std::string result;
		std::string pname = name;
		if(pname == "Password" && authData.DoAuth)
			result = authData.Password;
		else if(pname == "Login" && authData.DoAuth)
			result = authData.Login;
		else result = params[name.c_str()];
		return result;
	}

	void setParam(const std::string& name, const std::string& value)
	{
		params[name] = value;
	}
};

class CUploadEngineData
{
	public:
		enum ServerType { TypeImageServer, TypeFileServer, TypeUrlShorteningServer, TypeTextServer};
		std::string Name;
		std::string PluginName;
		bool SupportsFolders;
		bool UsingPlugin;
		bool Debug;
		bool ImageHost;
		bool SupportThumbnails;
		int NeedAuthorization;
		bool NeedPassword;
		int64_t MaxFileSize;
		std::string RegistrationUrl;
		std::string CodedLogin;
		std::string CodedPassword;
		std::string ThumbUrlTemplate, ImageUrlTemplate, DownloadUrlTemplate;
		std::vector<UploadAction> Actions;
		std::string LoginLabel;
		int RetryLimit;
		int NumOfTries;
		ServerType Type;
		CUploadEngineData();
};

struct CIUUploadParams
{
	int apiVersion;
	int thumbWidth;
	int thumbHeight;
	Utf8String serverName;
	Utf8String data;
	Utf8String folderId;
	Utf8String DirectUrl;

	Utf8String ThumbUrl;
	Utf8String ViewUrl;
	Utf8String ServerFileName;
	Utf8String temp_;
	const std::string getParam(const std::string& name)
	{
		temp_ = "";
		if(name == "THUMBWIDTH")
			temp_= IuCoreUtils::toString(thumbWidth);
		else if (name == "THUMBHEIGHT")
			temp_ =  IuCoreUtils::toString(thumbHeight);
		return temp_;
	}

	const std::string getFolderID() { return (folderId); }
	void setDirectUrl(const std::string& url) { DirectUrl = (url);}
	void setThumbUrl(const std::string& url) { ThumbUrl = (url);}
	void setViewUrl(const std::string& url) { ViewUrl = (url);}
	const std::string getServerFileName() { return (ServerFileName); }
};

class  CUploadEngineList_Base
{
	protected:
		std::vector<CUploadEngineData> m_list;
	public:
		CUploadEngineList_Base();
		CUploadEngineData* byIndex(size_t index);
		CUploadEngineData* byName(const std::string &name);
		CUploadEngineData*  firstEngineOfType(CUploadEngineData::ServerType type);
		int count();
		int getRandomImageServer();
		int getRandomFileServer();
		int GetUploadEngineIndex(const std::string Name);
};

class UploadTask;

class CAbstractUploadEngine
{
	public:
		CAbstractUploadEngine();
		virtual ~CAbstractUploadEngine();
		void setThumbnailWidth(int width);
		virtual bool doUpload(UploadTask* task, CIUUploadParams &params) = 0;
		void setServerSettings(ServerSettingsStruct settings);
		virtual int RetryLimit()=0;
		virtual void setNetworkManager(NetworkManager* nm);
		void setUploadData(CUploadEngineData* data);
		CUploadEngineData* getUploadData() const;
		// Events
		fastdelegate::FastDelegate0<bool> onNeedStop;
		fastdelegate::FastDelegate1<InfoProgress> onProgress;
		fastdelegate::FastDelegate3<StatusType, int, std::string> onStatusChanged;
		fastdelegate::FastDelegate2< const std::string&, bool> onDebugMessage;
		fastdelegate::FastDelegate1<ErrorInfo> onErrorMessage;
	protected:
		bool m_bShouldStop;
		NetworkManager * m_NetworkManager;
		CUploadEngineData * m_UploadData;
		ServerSettingsStruct m_ServersSettings;
		int m_ThumbnailWidth;
		bool DebugMessage(const std::string& message, bool isServerResponseBody = false);
		bool ErrorMessage(ErrorInfo);
		bool needStop();
		void SetStatus(StatusType status, std::string param = "");		
};

#endif // IU_CORE_UPLOADENGINE_H