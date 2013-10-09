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

#ifndef IU_CORE_FILEDOWNLOADER_H
#define IU_CORE_FILEDOWNLOADER_H

#include "atlheaders.h"
#include "3rdpart/thread.h"
#include "Core/3rdpart/FastDelegate.h"
#include "Core/Network/NetworkManager.h"
#include "Core/Utils/CoreTypes.h"

class CFileDownloader
{
	public:
		struct DownloadFileListItem
		{
			std::string fileName;
			std::string displayName;
			std::string url;
			std::string referer;
			void* id;
		};

		CFileDownloader();
		virtual ~CFileDownloader();
		void AddFile(const std::string& url, void* id, const std::string& referer = std::string());
		bool start();
		bool waitForFinished(unsigned int msec = -1);
		void setThreadCount(int n);
		void stop();
		bool IsRunning();

		fastdelegate::FastDelegate0<> onQueueFinished;
		fastdelegate::FastDelegate1<NetworkManager*> onConfigureNetworkManager;
		fastdelegate::FastDelegate3<bool, int, DownloadFileListItem, bool> onFileFinished;
	protected:
		CString m_ErrorStr;
		CAutoCriticalSection m_CS;
		std::vector<DownloadFileListItem> m_fileList;
		int m_nThreadCount;
		int m_nRunningThreads;
		std::vector<HANDLE> m_hThreads;
		bool m_NeedStop;
		volatile bool m_IsRunning;
		static int ProgressFunc (void* userData, double dltotal, double dlnow, double ultotal, double ulnow);
		static unsigned int __stdcall thread_func(void* param);
		void memberThreadFunc();
		bool getNextJob(DownloadFileListItem& item);

	private:
		DISALLOW_COPY_AND_ASSIGN(CFileDownloader);
};

#endif
