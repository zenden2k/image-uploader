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

#pragma once
#include <windows.h>
#undef _UNICODE
	#include <sqplus.h>
#define _UNICODE

#include "Common/NetworkManager.h"

CString IU_md5_file(const CString& filename);


struct CFolderItem
{
	CFolderItem()
	{
		accessType = 0;
		itemCount = -1;
	}

	std::wstring title;
	std::wstring summary;
	std::wstring id;
	std::wstring parentid;
	std::wstring viewUrl;
	int accessType;
	int itemCount;
	const std::string getTitle() { return WstringToUtf8(title);}
	const std::string getSummary() { return WstringToUtf8(summary);}
	const std::string getId() { return WstringToUtf8(id);}
	const std::string getParentId() { return WstringToUtf8(parentid);}
	const int getItemCount() { return itemCount; }
	const int getAccessType() { return accessType; }

	void setTitle(const std::string& str) { title = Utf8ToWstring(str); }
	void setViewUrl(const std::string& str) { viewUrl = Utf8ToWstring(str); }
	
	void setSummary(const std::string& str) { summary = Utf8ToWstring(str); }
	void setId(const std::string& str) { id = Utf8ToWstring(str); }
	void setParentId(const std::string& str) { parentid = Utf8ToWstring(str); }
	void setAccessType(const int type) {  accessType=type; }
	void setItemCount(const int count) {  itemCount=count; }	
};


class CFolderList
{
	
public:
	std::vector<CFolderItem> m_folderItems;
	void Clear() {m_folderItems.clear();}
	const int GetCount() { return m_folderItems.size();}
	CFolderItem& operator [] (int index) {  return m_folderItems[index]; }
	void AddFolder(const std::string& title, const std::string& summary, const std::string& id, const std::string& parentid, int accessType);
	void AddFolderItem(const CFolderItem& item);
};

struct CIUUploadParams
{
		int apiVersion;
		std::wstring serverName;
		std::wstring data;
		std::wstring folderId;
		std::wstring DirectUrl;
	
		std::wstring ThumbUrl;
		std::wstring ViewUrl;
		std::wstring ServerFileName;

		const std::string getFolderID() { return WstringToUtf8(folderId); }
		void setDirectUrl(const std::string& url) { DirectUrl = Utf8ToWstring(url);}
		void setThumbUrl(const std::string& url) { ThumbUrl = Utf8ToWstring(url);}
		void setViewUrl(const std::string& url) { ViewUrl = Utf8ToWstring(url);}
		const std::string getServerFileName() { return WstringToUtf8(ServerFileName); }
		

		//LPWSTR szViewUrl;
		//int szViewUrlBufferSize;

};

struct ServerSettingsStruct;
class CUploadScript
{
	public:
		SquirrelObject m_Object; 
		CUploadScript(LPCTSTR pluginName);
		~CUploadScript();
		bool load(LPCTSTR fileName, ServerSettingsStruct& params);
		void bindNetworkManager(NetworkManager * nm);
		int uploadFile(LPCWSTR FileName, CIUUploadParams &params);
		int getFolderList(CFolderList &FolderList);
		int  createFolder(CFolderItem &parent, CFolderItem &folder);
		int  modifyFolder(CFolderItem &folder);
		int getAccessTypeList(std::vector<std::wstring> &list);
		int getServerParamList(std::map<std::wstring,std::wstring> &list);
		bool isLoaded();
		void setServerParams(ServerSettingsStruct& params);
		bool supportsSettings();
		CString name();
		DWORD getCreationTime();
	protected:
		CFolderList m_FolderList;
		CString m_sName;
		ServerSettingsStruct *m_pServerParams;
		SquirrelObject m_SquirrelScript;
		DWORD m_CreationTime;
		bool m_bIsPluginLoaded;
};

class CPluginManager
{
	public:
		CPluginManager();
		~CPluginManager();
		void UnloadPlugins();
		CUploadScript* getPlugin(LPCTSTR name, ServerSettingsStruct& params, bool UseExisting = false);
	protected:
		CUploadScript* m_plugin;
};