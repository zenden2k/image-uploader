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

#include <vector>
#include <map>
#include <deque>
#include <string>

#include "../Network/NetworkManager.h"
#include "UploadEngine.h"
//#include "DefaultUploadEngine.h"
//#include "ScriptUploadEngine.h"
#include "../../3rdpart/FastDelegate.h"

using namespace fastdelegate;

class CUploader
{
	public:
		CUploader(void);
		~CUploader(void);
		
		bool setUploadEngine(CAbstractUploadEngine* UploadEngine);
		CAbstractUploadEngine * getUploadEngine();
		
		//void setServerSettings(ServerSettingsStruct* serverSettings);
		void setThumbnailWidth(int width);
		bool UploadFile(const std::string & FileName, const std::string displayFileName);
		const std::string  getDownloadUrl();
		const std::string  getDirectUrl();
		const std::string  getThumbUrl();
		void stop();
		bool needStop();
		// events
		FastDelegate0<bool> onNeedStop;
		FastDelegate1<InfoProgress> onProgress;
		FastDelegate3<StatusType, int, std::string> onStatusChanged;
		FastDelegate2<const std::string&, bool> onDebugMessage;
		FastDelegate1<ErrorInfo> onErrorMessage;
		FastDelegate1<NetworkManager*> onConfigureNetworkManager;

		void DebugMessage(const std::string& message, bool isServerResponseBody = false);
		void SetStatus(StatusType status, int param1=0, std::string param="");
		
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
	
		
};

#endif
