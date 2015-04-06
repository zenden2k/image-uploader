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

#ifndef IU_CORE_UPLOAD_FILEQUEUEUPLOADER_H
#define IU_CORE_UPLOAD_FILEQUEUEUPLOADER_H

#include <string>
#include "Core/Utils/CoreTypes.h"
#include "Core/Upload/UploadEngine.h"

class NetworkClient;

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
			FileListItem() {
				uploadTask = 0;
				fileSize = 0;
			}
			//void * user_data;
		};

		

		class Callback
		{
		public:
			virtual bool OnFileFinished(bool ok, FileListItem& result){return true;}
			virtual bool OnQueueFinished(CFileQueueUploader* queueUploader) { return true;}
			virtual bool OnConfigureNetworkClient(CFileQueueUploader*, NetworkClient* nm){return true;}
			virtual bool OnUploadProgress(UploadProgress progress, Task* task, NetworkClient* nm){return true;}
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
