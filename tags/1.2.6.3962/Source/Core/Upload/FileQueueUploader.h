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
#include "../../thread.h"
#include <atlcoll.h>
#include "../Network/NetworkManager.h"
#include "UploadEngine.h"
#include "Uploader.h"
//#include "../Settings.h"
struct FileListItem
{
	std::string fileName;
	std::string displayName;
	std::string imageUrl;
	std::string thumbUrl;
	std::string downloadUrl;
	int id;
};

class CFileUploaderCallback
{
public:
	virtual bool OnFileFinished(bool ok, FileListItem& result)
	{
		return true;
	};
	virtual bool OnQueueFinished() { return true;}
	virtual bool OnConfigureNetworkManager(NetworkManager* nm){return true;}
};



class CFileQueueUploader
{
	
	private:

		CString m_ErrorStr;
		CAbstractUploadEngine *m_engine;
		ServerSettingsStruct m_serverSettings;
		std::vector<FileListItem> m_fileList;
		int m_nThreadCount;
		int m_nRunningThreads;
		public:
			CFileQueueUploader();
			void AddFile(std::string fileName, std::string displayName, int id);
			bool start();
			void setUploadSettings(CAbstractUploadEngine * engine);
			static void thread_func(void * param);
			bool getNextJob(FileListItem* item);
			void setThreadCount();
			CAutoCriticalSection m_CS;
			CFileUploaderCallback *m_CallBack;
			void setCallback(CFileUploaderCallback* callback);
			bool m_NeedStop;
			bool m_IsRunning;
			void stop();
			bool IsRunning();
};
