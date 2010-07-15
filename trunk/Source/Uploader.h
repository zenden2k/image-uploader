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

#ifndef _UPLOADER_H_
#define _UPLOADER_H_


#include <deque>
#include "thread.h"
#include "common.h"
#include "Common/NetworkManager.h"
#include "Core/UploadEngine.h"
class CUploader
{
	public:
		CUploader(void);
		~CUploader(void);
		CString GetStatusText();
		LPTSTR ProgressBuffer;
		bool *ShouldStop;
		InfoProgress *PrInfo;
		bool SetUploadEngine(CUploadEngine *UploadEngine);
		void setServerSettings(ServerSettingsStruct* serverSettings);
		CUploadEngine * getUploadEngine();
		bool ServerSupportThumbnails(int ServerID);
		void setThumbnailWidth(int width);
		bool UploadFile(const CString & FileName, const CString &displayFileName);	
		const CString getDownloadUrl();
		const CString getDirectUrl();
		const CString getThumbUrl();
	protected:
		static int pluginProgressFunc (void* userData, double dltotal,double dlnow,double ultotal, double ulnow);
		int m_nThumbWidth;
		CString m_FileName;
		CString m_displayFileName;
		ServerSettingsStruct* m_ServerSettings;
		
		NetworkManager m_NetworkManager;
		bool DoAction(UploadAction &Action);
		bool DoUploadAction(UploadAction &Action, bool bUpload);
		bool DoGetAction(UploadAction &Action);
		bool SendHttpRequest(LPTSTR szMethod, LPTSTR szBody, LPTSTR szAddress, int nPort);
		bool ParseAnswer(UploadAction &Action, std::string& Body);
		void UploadError(WORD EventType, LPCTSTR Url, LPCTSTR  Error, int ErrorCode=0, LPCTSTR ErrorExplication=NULL);
		CString ReplaceVars(const CString Text);
		void ConfigureProxy();
		void AddQueryPostParams(UploadAction &Action);
		bool ReadServerResponse(UploadAction &Action);
		void SetStatusText(CString Text);
		bool DoTry();

		std::map<CString, CString> m_Vars;
		std::map<CString, CString> m_Consts;
		std::map<UINT, boolean> m_PerformedActions;
		LoginInfo li;
		CString m_StatusText;
		

		CUploadEngine *m_CurrentEngine;
		
		CString m_ThumbUrl;
		CString m_ImageUrl;
		CString m_DownloadUrl;
		CString m_ErrorReason;
		CAutoCriticalSection m_CS;
};

#endif