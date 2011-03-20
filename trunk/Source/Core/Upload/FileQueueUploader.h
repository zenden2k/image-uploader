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

#ifndef IU_CORE_UPLOAD_FILEQUEUEUPLOADER_H
#define IU_CORE_UPLOAD_FILEQUEUEUPLOADER_H

#pragma once

#include "../Network/NetworkManager.h"
#include "UploadEngine.h"
#include "Uploader.h"
#include <zthread/thread.h>
#include <zthread/mutex.h>

struct FileListItem
{
	std::string fileName;
	std::string displayName;
	std::string imageUrl;
	std::string thumbUrl;
	std::string downloadUrl;
	void * user_data;
};

class CFileUploaderCallback
{
	public:
		virtual bool OnFileFinished(bool ok, FileListItem& result){return true;};
		virtual bool OnQueueFinished() { return true;}
		virtual bool OnConfigureNetworkManager(NetworkManager* nm){return true;}
};

class CFileQueueUploader: public ZThread::Runnable
{
	public:
		CFileQueueUploader();
		void AddFile(const std::string& fileName, const std::string& displayName, void* user_data);
		void setUploadSettings(CAbstractUploadEngine * engine);
		void setCallback(CFileUploaderCallback* callback);

		bool start();
		void stop();
		bool IsRunning();
	private:
		virtual void run();
		bool getNextJob(FileListItem* item);
		ZThread::Mutex mutex_;
		CFileUploaderCallback *callback_;
		bool m_NeedStop;
		bool m_IsRunning;
		CAbstractUploadEngine *m_engine;
		ServerSettingsStruct m_serverSettings;
		std::vector<FileListItem> m_fileList;
		int m_nThreadCount;
		int m_nRunningThreads;
};

#endif