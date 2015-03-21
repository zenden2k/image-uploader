/*
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2015 ZendeN <zenden2k@gmail.com>
	 
    HomePage:    http://zenden.ws/imageuploader

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef IU_CORE_UPLOAD_FILEQUEUEUPLOADER_H
#define IU_CORE_UPLOAD_FILEQUEUEUPLOADER_H

#include <string>
#include "Core/Utils/CoreTypes.h"
#include "Core/Upload/UploadEngine.h"

class NetworkManager;

class UploadProgress {
public:
	std::string statusText;
	int stage;
	int64_t uploaded;
	int64_t totalUpload;
};

class CFileQueueUploader
{
	public:
		class Task {
		public:
			/*std::string fileName;
			std::string displayFileName; // without path*/
			void *userData;
			//int64_t fileSize;
			CAbstractUploadEngine *uploadEngine;
			std::string serverName;
			std_tr::shared_ptr<UploadTask> uploadTask;
		};

		struct FileListItem
		{
			std::string fileName;
			std::string displayName;
			std::string imageUrl;
			std::string thumbUrl;
			std::string downloadUrl;
			std::string serverName;
			int64_t fileSize;
			Task * uploadTask;
			//void * user_data;
		};

		

		class Callback
		{
		public:
			virtual bool OnFileFinished(bool ok, FileListItem& result){return true;}
			virtual bool OnQueueFinished(CFileQueueUploader* queueUploader) { return true;}
			virtual bool OnConfigureNetworkManager(CFileQueueUploader*, NetworkManager* nm){return true;}
			virtual bool OnUploadProgress(UploadProgress progress, Task* task, NetworkManager* nm){return true;}
		};

		CFileQueueUploader();
		void AddFile(const std::string& fileName, const std::string& displayName, void* user_data, CAbstractUploadEngine *uploadEngine);
		void AddUploadTask(std_tr::shared_ptr<UploadTask> task, void* user_data, CAbstractUploadEngine *uploadEngine);
		void AddFile(Task task);
		void setUploadSettings(CAbstractUploadEngine * engine);
		void setCallback(Callback* callback);
		~CFileQueueUploader();
		bool start();
		void stop();
		bool IsRunning() const;
		void setMaxThreadCount(int threadCount);
		bool isSlotAvailableForServer(std::string serverName, int maxThreads);
	private:
		CFileQueueUploader(const CFileQueueUploader&);
		class Impl;
		Impl* _impl;
};

#endif
