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

#include "FileDownloader.h"
#include "../myutils.h"
#include <algorithm>
#include "../Common.h"
CFileDownloader::CFileDownloader()
{
	m_nThreadCount = 4;
	m_NeedStop = false;
	m_IsRunning = false;
	m_nRunningThreads = 0;
}

void CFileDownloader::AddFile(const std::string& url, int id)
{
	m_CS.Lock();
	DownloadFileListItem newItem;
	newItem.url = url;
	newItem.id = id;
	m_fileList.push_back(newItem);
	m_CS.Unlock();
}

bool CFileDownloader::start()
{
	m_NeedStop = false;
	
	m_CS.Lock();
	int numThreads = min(m_nThreadCount - m_nRunningThreads, (int)m_fileList.size());
	//m_nRunningThreads = numThreads;

	for(int i=0; i<numThreads; i++)
	{
		m_nRunningThreads++;
		m_IsRunning = true;
		_beginthread(thread_func, 0, this);
	}
	m_CS.Unlock();
	return 0;
}
void CFileDownloader::thread_func(void * param)
{
	CFileDownloader * p = reinterpret_cast<CFileDownloader*>(param);
	p->memberThreadFunc();
}

void CFileDownloader::memberThreadFunc()
{
	NetworkManager nm;
	m_CS.Lock();
	if(onConfigureNetworkManager)
	onConfigureNetworkManager(&nm);
	m_CS.Unlock();
	for(;;)
	{
		DownloadFileListItem curItem;
		if(!getNextJob(curItem)) break;
		std::string url = curItem.url;
		if(url.empty()) break; 

		nm.setOutputFile( curItem.fileName);
		nm.doGet(url);

		m_CS.Lock();
		if(nm.responseCode()>=200 && nm.responseCode()<=299)
		{
			
			std::string name = ExtractFileNameA(url.c_str());
			if(!onFileFinished.empty())
				onFileFinished(true, curItem); // delegate call
		}
		else
		{
			if(!onFileFinished.empty())
				onFileFinished(false, curItem); // delegate call
		}
		m_CS.Unlock();

	}
	m_CS.Lock();
	m_nRunningThreads--;
	if(!m_nRunningThreads)
	{
		m_IsRunning = false;
		if(onQueueFinished)
			onQueueFinished(); // delegate call
	}
	m_CS.Unlock();
	return ;

}
bool CFileDownloader::getNextJob(DownloadFileListItem& item)
{
	bool result=false;
	
	m_CS.Lock();
	if(!m_fileList.empty() && !m_NeedStop)
	{
		item = *m_fileList.begin();
		std::string url;
		url = item.url;
		m_fileList.erase(m_fileList.begin());
		std::string ext = GetFileExtA(url.c_str());
		std::string fileName =  ExtractFileNameA(url.c_str());
		CString wFileName = GetUniqFileName(IUTempFolder+Utf8ToWstring(fileName.c_str()).c_str());
		std::string filePath= WCstringToUtf8(wFileName);
		FILE *f = _tfopen(wFileName,L"wb");
		if(f) fclose(f);
			//WCstringToUtf8(GenerateFileName(_T("%md5"),0,CPoint()));

		 item.fileName = filePath;
		//saveAs = filePath;
		result = true;
	}
	
	m_CS.Unlock();
	return result;
}

void CFileDownloader::stop()
{
	m_NeedStop = true;
}

bool CFileDownloader::IsRunning()
{
	return m_IsRunning;
}