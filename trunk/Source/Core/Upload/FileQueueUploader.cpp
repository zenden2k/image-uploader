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
#include "../../Gui/Dialogs/LogWindow.h"
#include <zthread/thread.h>
#include <zthread/mutex.h>
#include "Uploader.h"

class CFileQueueUploader::Impl
{
	public:
		Impl();
		virtual ~Impl();
		void AddFile(const std::string& fileName, const std::string& displayName, void* user_data);
		void start();
		virtual void run();
		bool getNextJob(FileListItem* item);
		
		ZThread::Mutex mutex_;
		CFileUploaderCallback *callback_;
		volatile bool m_NeedStop;
		bool m_IsRunning;
		CAbstractUploadEngine *m_engine;
		ServerSettingsStruct m_serverSettings;
		std::vector<FileListItem> m_fileList;
		int m_nThreadCount;
		int m_nRunningThreads;
	protected:
		class Runnable;
		bool onNeedStopHandler();
};

class CFileQueueUploader::Impl::Runnable: public ZThread::Runnable
{
	public:
		CFileQueueUploader::Impl* uploader_;
		Runnable(CFileQueueUploader::Impl *uploader){uploader_ = uploader;}
		virtual void run(){uploader_->run();}
};

/* private CFileQueueUploader::Impl class */

CFileQueueUploader::Impl::Impl()
{
	m_nThreadCount = 1;
	callback_ = 0;
	m_NeedStop = false;
	m_IsRunning = false;
	m_engine = 0;
}

CFileQueueUploader::Impl::~Impl()
{

}

bool CFileQueueUploader::Impl::onNeedStopHandler()
{
	return m_NeedStop;
}

bool CFileQueueUploader::Impl::getNextJob(FileListItem* item)
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

void CFileQueueUploader::Impl::AddFile(const std::string& fileName, const std::string& displayName, void* user_data)
{
	FileListItem newFileListItem;
	newFileListItem.user_data = user_data;
	newFileListItem.displayName = displayName;
	newFileListItem.fileName = fileName;
	m_fileList.push_back(newFileListItem);
}

void CFileQueueUploader::Impl::start()
{
	m_NeedStop = false;
	m_IsRunning = true;
	int numThreads = min(size_t(m_nThreadCount), m_fileList.size());
	m_nRunningThreads = numThreads;
	for(int i = 0; i < numThreads; i++)
	{
		ZThread::Thread t1(new Runnable(this)); // starting new thread
	}
}

void CFileQueueUploader::Impl::run()
{
	CUploader uploader;
	uploader.onErrorMessage.bind(DefaultErrorHandling::ErrorMessage);

	for(;;)
	{
		FileListItem it;
		if(!getNextJob(&it)) break;
		uploader.setUploadEngine(m_engine);
		uploader.onNeedStop.bind(this, &Impl::onNeedStopHandler);
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

/* public CFileQueueUploader class */

CFileQueueUploader::CFileQueueUploader()
{
	_impl = new Impl();
}

void CFileQueueUploader::AddFile(const std::string& fileName, const std::string& displayName, void* user_data)
{
	_impl->AddFile(fileName, displayName, user_data);
}

bool CFileQueueUploader::start()
{
	_impl->start();
	return true;
}


void CFileQueueUploader::setCallback(CFileUploaderCallback* callback)
{
	_impl->callback_ = callback;
}



void CFileQueueUploader::stop()
{
	_impl->m_NeedStop = true;
}

bool CFileQueueUploader::IsRunning() const
{
	return _impl->m_IsRunning;
}

void CFileQueueUploader::setUploadSettings(CAbstractUploadEngine * engine)
{
	_impl->m_engine = engine;
}



CFileQueueUploader::~CFileQueueUploader()
{
	delete _impl;
}
