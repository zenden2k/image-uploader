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

#ifndef _IU_DEFAULT_UPLOAD_ENGINE_H
#define _IU_DEFAULT_UPLOAD_ENGINE_H

#include <vector>
#include <string>
#include "CommonTypes.h"
#include "UploadEngine.h"
class CDefaultUploadEngine: public CAbstractUploadEngine
{
	public:
		CDefaultUploadEngine();
		virtual bool doUpload(Utf8String FileName,Utf8String DisplayName, CIUUploadParams &params);
protected:

	bool DoAction(UploadAction &Action);
	bool DoUploadAction(UploadAction &Action, bool bUpload);
	bool DoGetAction(UploadAction &Action);
	bool ParseAnswer(UploadAction &Action, std::string& Body);
	std::string ReplaceVars(const std::string& Text);
	int RetryLimit();
	//void ConfigureProxy();
	void AddQueryPostParams(UploadAction &Action);
	bool ReadServerResponse(UploadAction &Action);
	void SetStatus(StatusType status, std::string param = "");
		bool needStop();
		Utf8String m_ErrorReason;

		Utf8String m_FileName;
		Utf8String m_displayFileName;
		LoginInfo li;
		ErrorInfo m_LastError;
		std::string m_ErrorBuffer;
		void UploadError(bool error, const std::string errorStr, UploadAction* m_CurrentAction, bool writeToBuffer = true);
	//bool DoTry();
	int m_CurrentActionIndex;
	
	std::map<std::string, std::string> m_Vars;
	std::map<std::string, std::string> m_Consts;
	std::map<size_t, boolean> m_PerformedActions;
};

#endif