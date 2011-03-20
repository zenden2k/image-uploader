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

#include "FileQueueUploader.h"
#include <algorithm>
#include "../../LogWindow.h"

CFileQueueUploader::CFileQueueUploader()
{
	m_nThreadCount = 1;
	callback_ = 0;
	m_NeedStop = false;
	m_IsRunning = false;
	m_engine = 0;
}

void CFileQueueUploader::AddFile(const std::string& fileName, const std::string& displayName, void* user_data)
{
	FileListItem newFileListItem;
	newFileListItem.user_data = user_data;
	newFileListItem.displayName = displayName;
	newFileListItem.fileName = fileName;
	m_fileList.push_back(newFileListItem);
}

bool CFileQueueUploader::start()
{
	m_NeedStop = false;
	m_IsRunning = true;
	int numThreads = min(size_t(m_nThreadCount), m_fileList.size());
	m_nRunningThreads = numThreads;
	for(int i = 0; i < numThreads; i++)
	{
		ZThread::Thread t1(this); // starting new thread
	}
	return 0;
}

bool CFileQueueUploader::getNextJob(FileListItem* item)
{
	if(m_NeedStop) return false;
	std::string url;
	mutex_.acquire();
	bool result = false;
	if(!m_fileList.empty() && !m_NeedStop)
	{
		*item = *m_fileList.begin();
		m_fileList.erase(m_fileList.begin());
		result = true;
	}
	mutex_.release();
	return result;
}

void CFileQueueUploader::setCallback(CFileUploaderCallback* callback)
{
	callback_ = callback;
}

/*int CUploader::pluginProgressFunc (void* userData, double dltotal,double dlnow,double ultotal, double ulnow)
{
	if(m_NeedStop) return -1;
	return 0;
}*/

void CFileQueueUploader::stop()
{
	m_NeedStop = true;
}

bool CFileQueueUploader::IsRunning()
{
	return m_IsRunning;
}

void CFileQueueUploader::setUploadSettings(CAbstractUploadEngine * engine)
{
	m_engine = engine;
}

void CFileQueueUploader::run()
{
	CUploader uploader;
	uploader.onErrorMessage.bind(DefaultErrorHandling::ErrorMessage);

	for(;;)
	{
		FileListItem it;
		if(!getNextJob(&it)) break;
		uploader.setUploadEngine(m_engine);
		bool res = uploader.UploadFile(it.fileName, it.displayName.c_str());
		mutex_.acquire();
		//m_CS.Lock();

		if(res)
		{
			it.imageUrl = (uploader.getDirectUrl());
			it.downloadUrl = (uploader.getDownloadUrl());
			it.thumbUrl = (uploader.getThumbUrl());
			if(callback_)
				callback_->OnFileFinished(true,it);
		}
		else
		{
			if(callback_)
				callback_->OnFileFinished(false,it);
		}
		mutex_.release();
	}
	mutex_.acquire();
	m_nRunningThreads--;
	if(!m_nRunningThreads)
	{
		m_IsRunning = false;
		if(callback_)
			callback_->OnQueueFinished();
	}
	mutex_.release();
}