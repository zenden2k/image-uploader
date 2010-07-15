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
#include "FileDownloader.h"
#include "../myutils.h"
#include <algorithm>
#include "../Common.h"
CFileDownloader::CFileDownloader()
{
	m_nThreadCount = 4;
	m_CallBack = 0;
	m_NeedStop = false;
	m_IsRunning = false;
}

void CFileDownloader::AddFile(const std::string& url)
{
	m_fileList.push_back(url);
}

bool CFileDownloader::start()
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
void CFileDownloader::thread_func(void * param)
{
	CFileDownloader * p = reinterpret_cast<CFileDownloader*>(param);
	NetworkManager nm;
	p->m_CS.Lock();
	if(p->m_CallBack)
		p->m_CallBack->OnConfigureNetworkManager(&nm);
	p->m_CS.Unlock();
	for(;;)
	{
		std::string filePath;
		std::string url = p->getNextJob(filePath);
		if(url.empty()) break; ;
		
		nm.setOutputFile( filePath);
		nm.doGet(url);

		p->m_CS.Lock();
		if(nm.responseCode()>=200 && nm.responseCode()<=299)
		{
			//ShowVar((int)p->m_CallBack);
			std::string name = ExtractFileNameA(url.c_str());
			if(p->m_CallBack)
				p->m_CallBack->OnFileFinished(true,url, filePath, name);
		}
		else
		{

			if(p->m_CallBack)
				p->m_CallBack->OnFileFinished(false,url,"", "");
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

std::string CFileDownloader::getNextJob(std::string& saveAs)
{
	std::string url;
	m_CS.Lock();
	if(!m_fileList.empty() && !m_NeedStop)
	{
		url = *m_fileList.begin();
		m_fileList.erase(m_fileList.begin());
		std::string ext = GetFileExtA(url.c_str());
		std::string fileName =  ExtractFileNameA(url.c_str());
		CString wFileName = GetUniqFileName(IUTempFolder+Utf8ToWstring(fileName.c_str()).c_str());
		std::string filePath= WCstringToUtf8(wFileName);
		FILE *f = _tfopen(wFileName,L"wb");
		if(f) fclose(f);
			//WCstringToUtf8(GenerateFileName(_T("%md5"),0,CPoint()));

		 
		saveAs = filePath;
	}
	m_CS.Unlock();
	return url;
}

void CFileDownloader::setCallback(CFileDownloaderCallback* callback)
{
	m_CallBack = callback;
}

void CFileDownloader::stop()
{
	m_NeedStop = true;
}

bool CFileDownloader::IsRunning()
{
	return m_IsRunning;
}