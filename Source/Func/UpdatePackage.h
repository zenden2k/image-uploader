/*
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2011 ZendeN <zenden2k@gmail.com>
	 
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

#ifndef IU_FUNC_UPDATEPACKAGE_H
#define IU_FUNC_UPDATEPACKAGE_H

#pragma once

#include <string>
#include "atlheaders.h"
#include "Core/Utils/CoreTypes.h"
#include "Core/Network/NetworkManager.h"
#include "Core/Utils/SimpleXml.h"

struct CUpdateItem
{
		CString name;
		CString hash;
		CString saveTo;
		CString action;
		std::string flags;
};

class CUpdateInfo
{
	public:
		CUpdateInfo();
		bool LoadUpdateFromFile(const CString& filename);
		bool LoadUpdateFromBuffer(const CString& buffer);
		bool DoUpdate(const CUpdateInfo& newPackage);
		bool SaveToFile(const CString& filename);
		bool Parse(SimpleXml& xml);	
		bool CheckUpdates();
		const CString getHash();
		bool CanUpdate(const CUpdateInfo& newInfo);
		bool	operator<(const CUpdateInfo& p);

		bool isCoreUpdate() const;
		CString displayName() const;
		CString downloadUrl() const;
		CString fileName() const;
		CString packageName() const;
		CString readableText() const;
		CString updateUrl() const;
		int timeStamp() const;

		void setFileName(const CString & name);
	protected:
		CString m_ReadableText;
		CString m_PackageName, m_DownloadUrl, m_UpdateUrl, m_Hash;
		CString m_FileName;
		bool m_CoreUpdate;
		int m_TimeStamp;
		CString m_DisplayName;
		CString m_Buffer;
};

class CUpdateStatusCallback
{
	public:
		virtual void updateStatus(int packageIndex, const CString& status)=0;
};

class CUpdatePackage
{
	public:
		CUpdatePackage();
		bool LoadUpdateFromFile(const CString& filename);
		bool doUpdate();
		void setUpdateStatusCallback(CUpdateStatusCallback * callback);
		int updatedFileCount() const;
		int totalFileCount() const;
	protected:
		void setStatusText(const CString& text);

		SimpleXml m_xml;
		CString m_PackageName;
		CString m_PackageFolder;
		bool m_CoreUpdate;
		int m_TimeStamp;
		std::vector<CUpdateItem> m_entries;
		int m_nUpdatedFiles;
		int m_nTotalFiles;
		CUpdateStatusCallback* m_statusCallback;
	private:
		DISALLOW_COPY_AND_ASSIGN(CUpdatePackage);
};


class CUpdateManager: public CUpdateStatusCallback
{
	public:
		CUpdateManager();
		bool CheckUpdates();
		bool DoUpdates();
		const CString ErrorString();
		CString generateReport();
		void Clear();
		bool AreUpdatesAvailable();
		bool AreCoreUpdates();
		void setUpdateStatusCallback(CUpdateStatusCallback * callback);
		int successPackageUpdatesCount() const;
		void stop();
		std::vector<CUpdateInfo> m_updateList;
	protected:
		static int progressCallback(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow);
		void updateStatus(int packageIndex, const CString& status);
		bool internal_load_update(CString name);
		bool internal_do_update(CUpdateInfo& ui);

		CString m_ErrorStr;
		int nCurrentIndex;
		CUpdateStatusCallback *m_statusCallback;
		
		NetworkManager nm;
		int m_nSuccessPackageUpdates;
		bool m_stop;
		int m_nCoreUpdates;
	private:
		DISALLOW_COPY_AND_ASSIGN(CUpdateManager);
};


#endif