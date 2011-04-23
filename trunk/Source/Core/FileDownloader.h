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

#pragma once
#include <atlbase.h>
#include <atlapp.h>
#include <atlmisc.h>
#include "../3rdpart/thread.h"
#include <atlcoll.h>
#include "Network/NetworkManager.h"
#include "../3rdpart/FastDelegate.h"

struct DownloadFileListItem
{
	std::string fileName;
	std::string displayName;
	std::string url;
	void* id;
};

using namespace fastdelegate;

class CFileDownloader
{
	private:
		CString m_ErrorStr;
		std::vector<DownloadFileListItem> m_fileList;
		int m_nThreadCount;
		int m_nRunningThreads;
		std::vector<HANDLE> m_hThreads;
		public:
			CFileDownloader();
			void AddFile(const std::string& url, void* id);
			bool start();
			static unsigned int __stdcall thread_func(void * param);
			void memberThreadFunc();
			bool waitForFinished(unsigned int msec = -1);
			bool getNextJob(DownloadFileListItem& item);
			void setThreadCount(int n);
			CAutoCriticalSection m_CS;
			bool m_NeedStop;
			volatile bool m_IsRunning;
			void stop();
			//void kill();
			bool IsRunning();
			FastDelegate0<> onQueueFinished;
			FastDelegate1<NetworkManager*> onConfigureNetworkManager;
			FastDelegate2<bool, DownloadFileListItem, bool> onFileFinished;
			static int ProgressFunc (void* userData, double dltotal,double dlnow,double ultotal, double ulnow);
};
