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
#include "UploadSession.h"

class UploadEngineManager;
class UploadFilter;
class NetworkClient;


class FileQueueUploaderPrivate;

class CFileQueueUploader
{
	public:
		CFileQueueUploader(UploadEngineManager* uploadEngineManager);
		void addSession(std::shared_ptr<UploadSession> uploadSession);
		void addTask(std_tr::shared_ptr<UploadTask> task);
		void removeSession(std::shared_ptr<UploadSession> uploadSession);
		virtual ~CFileQueueUploader();
		bool start();
		void stop();
		bool IsRunning() const;
		void setMaxThreadCount(int threadCount);
		bool isSlotAvailableForServer(std::string serverName, int maxThreads);
		void addUploadFilter(UploadFilter* filter);
		void removeUploadFilter(UploadFilter* filter);
		int sessionCount();
		std_tr::shared_ptr<UploadSession> session(int index);
		fastdelegate::FastDelegate1<CFileQueueUploader*> OnQueueFinished;
		fastdelegate::FastDelegate1<UploadSession*> OnSessionAdded;
		fastdelegate::FastDelegate1<UploadTask*> OnTaskAdded;

		fastdelegate::FastDelegate2<CFileQueueUploader*, NetworkClient*> OnConfigureNetworkClient;
		friend class FileQueueUploaderPrivate;
	private:
		DISALLOW_COPY_AND_ASSIGN(CFileQueueUploader);
		FileQueueUploaderPrivate* _impl;
	protected:
		virtual void sessionAdded(UploadSession* session);
		virtual void taskAdded(UploadTask* task);
};

#endif
