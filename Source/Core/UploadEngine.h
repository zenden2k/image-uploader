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
#include <atlbase.h>
#include <atlapp.h>
#include <atlmisc.h>

#include <atlcoll.h>

struct ActionVariable
{
	CString Name;
	int nIndex;
};

struct UploadAction
{
	UINT Index;
	bool IgnoreErrors;
	bool OnlyOnce;
	CString Url;
	CString Description;
	CString Referer;
	CString PostParams;
	CString Type;
	CString RegExp;
	std::vector<ActionVariable> Variables;

	//----
	int RetryLimit;
	int NumOfTries;
};

class CUploadEngine
{
public:
		CString Name;
		CString PluginName;
		bool SupportsFolders;
		bool UsingPlugin;
		bool Debug;
		bool ImageHost;
		bool SupportThumbnails;
		int NeedAuthorization;
		DWORD MaxFileSize;
		CString RegistrationUrl;
		CString CodedLogin;
		CString CodedPassword;
		CString ThumbUrlTemplate, ImageUrlTemplate, DownloadUrlTemplate;
		std::vector<UploadAction> Actions;
		int RetryLimit;
		int NumOfTries;
	public:
		CUploadEngine();
};

class CUploadEngineList
{
	private:
		CString m_ErrorStr;
		std::vector<CUploadEngine> m_list;
		public:
			CUploadEngineList();
			bool LoadFromFile(const CString& filename);
			CUploadEngine* byIndex(int index);
			CUploadEngine* byName(const CString &name);
			const CString ErrorStr();
			int count();
			int getRandomImageServer();
			int getRandomFileServer();
			int GetUploadEngineIndex(const CString Name);
};
