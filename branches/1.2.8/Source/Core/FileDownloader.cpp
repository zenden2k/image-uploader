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

#include "Core/FileDownloader.h"
#include <algorithm>
#include "Func/myutils.h"
#include "Func/Common.h"
#include <Func/WinUtils.h>

// TODO: 1. use ZThread classes instead  CThread with ,
// 2. remove dependency from non-core headers ( "myutils.h", "Common.h")

// 3. Use pimpl
CFileDownloader::CFileDownloader()
{
	m_nThreadCount = 3;
	m_NeedStop = false;
	m_IsRunning = false;
	m_nRunningThreads = 0;
}

CFileDownloader::~CFileDownloader()
{
}

void CFileDownloader::AddFile(const std::string& url, void* id)
{
	if (m_NeedStop)
		return;  // Cannot add file to queue while stopping process
	m_CS.Lock();

	DownloadFileListItem newItem;
	newItem.url = url;
	newItem.id = reinterpret_cast <void*>(id);
	m_fileList.push_back(newItem);
	m_CS.Unlock();
}

void CFileDownloader::setThreadCount(int n)
{
	m_nThreadCount = n;
}

bool CFileDownloader::start()
{
	// TODO: Rewrite this using ZThread features
	m_NeedStop = false;
	m_CS.Lock();

	size_t numThreads = min(m_nThreadCount - m_nRunningThreads, int(m_fileList.size()));
	for (size_t i = 0; i < numThreads; i++)
	{
		m_nRunningThreads++;
		m_IsRunning = true;
		m_hThreads.push_back((HANDLE) _beginthreadex(0, 0, thread_func, this, 0, 0));
	}

	m_CS.Unlock();
	return 0;
}

unsigned int __stdcall CFileDownloader::thread_func(void* param)
{
	CFileDownloader* p = reinterpret_cast<CFileDownloader*>(param);
	p->memberThreadFunc();
	return 0;
}

void CFileDownloader::memberThreadFunc()
{
	NetworkManager nm;

	// Providing callback function to stop downloading
	nm.setProgressCallback(CFileDownloader::ProgressFunc, this);
	m_CS.Lock();
	if (onConfigureNetworkManager)
		onConfigureNetworkManager(&nm);
	m_CS.Unlock();

	for (;; )
	{
		DownloadFileListItem curItem;
		if (!getNextJob(curItem))
			break;

		std::string url = curItem.url;
		if (url.empty())
			break;

		nm.setOutputFile(curItem.fileName);
		nm.doGet(url);
		if (m_NeedStop)
			break;

		m_CS.Lock();
		if (nm.responseCode() >= 200 && nm.responseCode() <= 299)
		{
			std::string name = IuCoreUtils::ExtractFileName(url);
			if (!onFileFinished.empty())
				onFileFinished(true, curItem);                                                                                                                                                                                                                                                  // delegate call
		}
		else
		{
			if (!onFileFinished.empty())
				onFileFinished(false, curItem);                                                                                                                                                                                                                                                 // delegate call
		}

		if (m_NeedStop)
			m_fileList.clear();
		m_CS.Unlock();
	}

	m_CS.Lock();

	HANDLE hThread = GetCurrentThread();
	for (size_t i = 0; i < m_hThreads.size(); i++)
	{
		if (m_hThreads[i] == hThread)
		{
			m_hThreads.erase(m_hThreads.begin() + i);
			break;
		}
	}

	m_nRunningThreads--;

	if (m_NeedStop)
		m_fileList.clear();
	m_CS.Unlock();  // We need to release  mutex before calling  onQueueFinished()

	// otherwise we may get a deadlock
	if (!m_nRunningThreads)
	{
		m_IsRunning = false;
		m_NeedStop = false;
		if (onQueueFinished)                                                                                                                         // it is a delegate
			onQueueFinished();
	}

	return;
}

bool CFileDownloader::getNextJob(DownloadFileListItem& item)
{
	bool result = false;
	m_CS.Lock();
	if (!m_fileList.empty() && !m_NeedStop)
	{
		item = *m_fileList.begin();

		std::string url;
		url = item.url;
		m_fileList.erase(m_fileList.begin());

		std::string ext = IuCoreUtils::ExtractFileExt(url);
		std::string fileName = IuCoreUtils::ExtractFileName(url);
		CString wFileName =
			WinUtils::GetUniqFileName(IUTempFolder + IuCoreUtils::Utf8ToWstring(fileName.c_str()).c_str());
		std::string filePath = WCstringToUtf8(wFileName);

		// Creating file
		FILE* f = _tfopen(wFileName, L"wb");
		if (f)
			fclose(f);
		item.fileName = filePath;
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

bool CFileDownloader::waitForFinished(unsigned int msec)
{
	if (!m_IsRunning)
		return true;

	int nCount = m_hThreads.size();
	if (!nCount)
		return true;
	m_CS.Lock();

	HANDLE* threads = new HANDLE[m_hThreads.size()];
	memcpy(threads, &m_hThreads[0], sizeof(HANDLE) * nCount);
	m_CS.Unlock();

	DWORD res = WaitForMultipleObjects(nCount, threads, TRUE, msec);
	if (res == WAIT_TIMEOUT || res == WAIT_FAILED)
		return false;
	else
		return true;
}

int CFileDownloader::ProgressFunc(void* userData, double dltotal, double dlnow,
                                  double ultotal,
                                  double ulnow)
{
	CFileDownloader* fd = reinterpret_cast<CFileDownloader*>(userData);
	if (fd->m_NeedStop)
		return -1;
	return 0;
}
