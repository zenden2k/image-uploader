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

#include "stdafx.h"
#include "FileQueueUploader.h"
#include "../myutils.h"
#include <algorithm>
#include "../Common.h"
CFileQueueUploader::CFileQueueUploader()
{
	m_nThreadCount = 1;
	m_CallBack = 0;
	m_NeedStop = false;
	m_IsRunning = false;
	m_engine = 0;
}

void CFileQueueUploader::AddFile(std::string fileName, std::string displayName, int id)
{
	FileListItem newFileListItem;
	newFileListItem.id = id;
	newFileListItem.displayName = displayName;
	newFileListItem.fileName = fileName;
	m_fileList.push_back(newFileListItem);
}

bool CFileQueueUploader::start()
{
	m_NeedStop = false;
	m_IsRunning = true;
	int numThreads = min(m_nThreadCount, m_fileList.size());
	m_nRunningThreads = numThreads;
	for(int i=0; i<numThreads; i++)
	{
		_beginthread(thread_func, 0, this);
	}
	return 0;
}
void CFileQueueUploader::thread_func(void * param)
{
	CFileQueueUploader * p = reinterpret_cast<CFileQueueUploader*>(param);
	CUploader uploader;
	p->m_CS.Lock();

	p->m_CS.Unlock();
	for(;;)
	{
		FileListItem it;
		if(!p->getNextJob(&it)) break;
		TCHAR buf[256];
		//bool stopu=false;
		uploader.ProgressBuffer=buf;
		uploader.ShouldStop = &p->m_NeedStop;
		uploader.setServerSettings(&p->m_serverSettings);
		uploader.SetUploadEngine(p->m_engine);
		bool res = uploader.UploadFile(Utf8ToWstring(it.fileName).c_str(), Utf8ToWstring(it.displayName).c_str());
		
		p->m_CS.Lock();

		if(res)
		{
			it.imageUrl = WCstringToUtf8(uploader.getDirectUrl());
			it.downloadUrl = WCstringToUtf8(uploader.getDownloadUrl());
			it.thumbUrl = WCstringToUtf8(uploader.getThumbUrl());
			if(p->m_CallBack)
				p->m_CallBack->OnFileFinished(true,it);
		}
		else
		{

			if(p->m_CallBack)
				p->m_CallBack->OnFileFinished(false,it);
		}
		p->m_CS.Unlock();
		
	}
	p->m_CS.Lock();
	p->m_nRunningThreads--;
	if(!p->m_nRunningThreads)
	{
		p->m_IsRunning = false;
		if(p->m_CallBack)
			p->m_CallBack->OnQueueFinished();
	}
		p->m_CS.Unlock();
	return ;
}
bool CFileQueueUploader::getNextJob(FileListItem* item)
{
	if(m_NeedStop) return false;
	std::string url;
	m_CS.Lock();
	bool result = false;
	if(!m_fileList.empty() && !m_NeedStop)
	{
		*item = *m_fileList.begin();
		m_fileList.erase(m_fileList.begin());
		result = true;
	}
	m_CS.Unlock();
	return result;
}

void CFileQueueUploader::setCallback(CFileUploaderCallback* callback)
{
	m_CallBack = callback;
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

void CFileQueueUploader::setUploadSettings(CUploadEngine * engine, ServerSettingsStruct serverSettings)
{
	m_engine = engine;
	m_serverSettings = serverSettings;
}