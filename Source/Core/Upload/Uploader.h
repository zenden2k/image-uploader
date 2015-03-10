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

#ifndef _UPLOADER_H_
#define _UPLOADER_H_

#include <string>
#include "Core/Utils/CoreTypes.h"
#include "Core/Network/NetworkManager.h"
#include "Core/Upload/UploadEngine.h"
#include "Core/3rdpart/FastDelegate.h"

class CUploader
{
	public:
		CUploader(void);
		~CUploader(void);
		
		bool setUploadEngine(CAbstractUploadEngine* UploadEngine);
		CAbstractUploadEngine * getUploadEngine();
		
		void setThumbnailWidth(int width);
		bool UploadFile(const std::string & FileName, const std::string displayFileName);
		bool Upload(UploadTask* task);
		const std::string  getDownloadUrl();
		const std::string  getDirectUrl();
		const std::string  getThumbUrl();
		void stop();
		bool needStop();
		// events
		fastdelegate::FastDelegate0<bool> onNeedStop;
		fastdelegate::FastDelegate2<CUploader*,InfoProgress> onProgress;
		fastdelegate::FastDelegate3<StatusType, int, std::string> onStatusChanged;
		fastdelegate::FastDelegate2<const std::string&, bool> onDebugMessage;
		fastdelegate::FastDelegate1<ErrorInfo> onErrorMessage;
		fastdelegate::FastDelegate1<NetworkManager*> onConfigureNetworkManager;

		void DebugMessage(const std::string& message, bool isServerResponseBody = false);
		void SetStatus(StatusType status, int param1=0, std::string param="");
		StatusType GetStatus() const;
	protected:
		InfoProgress m_PrInfo;
		static int pluginProgressFunc (void* userData, double dltotal,double dlnow,double ultotal, double ulnow);

		bool m_bShouldStop;
		int m_nThumbWidth;
		StatusType m_CurrentStatus;
		std::string m_FileName;
		std::string m_displayFileName;
		
		std::string m_ThumbUrl;
		std::string m_ImageUrl;
		std::string m_DownloadUrl;
		std::string m_ErrorReason;
		
		void Error(bool error, std::string message, ErrorType type = etOther, int retryIndex = -1);
		void ErrorMessage(ErrorInfo);
		NetworkManager m_NetworkManager;
		CAbstractUploadEngine *m_CurrentEngine;
		void Cleanup();
	private:
		DISALLOW_COPY_AND_ASSIGN(CUploader);
};

#endif
