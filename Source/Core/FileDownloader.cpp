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

#include "Core/FileDownloader.h"
#include <algorithm>
#include "Func/myutils.h"
#include "Func/Common.h"
#include "Func/IuCommonFunctions.h"
#include "Func/WinUtils.h"
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

void CFileDownloader::AddFile(const std::string& url, void* id, const std::string& referer) {
	if (m_NeedStop)
		return;  // Cannot add file to queue while stopping process
	m_CS.Lock();

	DownloadFileListItem newItem;
	newItem.url = url;
	newItem.id = reinterpret_cast <void*>(id);
	newItem.referer = referer;
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
	NetworkClient nm;

	// Providing callback function to stop downloading
	nm.setProgressCallback(CFileDownloader::ProgressFunc, this);
	m_CS.Lock();
	if (onConfigureNetworkClient)
		onConfigureNetworkClient(&nm);
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
		if ( !curItem.referer.empty() ) {
			nm.setReferer(curItem.referer);
		}
		nm.doGet(url);
		if (m_NeedStop)
			break;

		m_CS.Lock();
		if (nm.responseCode() >= 200 && nm.responseCode() <= 299)
		{
			std::string name = IuCoreUtils::ExtractFileName(url);
			if (!onFileFinished.empty())
				onFileFinished(true, nm.responseCode(), curItem);                                                                                                                                                                                                                                                  // delegate call
		}
		else
		{
			if (!onFileFinished.empty())
				onFileFinished(false, nm.responseCode(), curItem);                                                                                                                                                                                                                                                 // delegate call
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
			GetUniqFileName(IuCommonFunctions::IUTempFolder + Utf8ToWstring(fileName.c_str()).c_str());
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
