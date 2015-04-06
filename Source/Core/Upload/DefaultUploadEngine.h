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

#ifndef IU_DEFAULT_UPLOAD_ENGINE_H
#define IU_DEFAULT_UPLOAD_ENGINE_H

#include <vector>
#include <string>
#include "CommonTypes.h"
#include "UploadEngine.h"

class FileUploadTask;
class UrlShorteningTask;

class CDefaultUploadEngine: public CAbstractUploadEngine
{
	public:
		CDefaultUploadEngine();
		virtual int doUpload(UploadTask* task, CIUUploadParams &params);
	
	protected:
		bool DoAction(UploadAction &Action);
		bool DoUploadAction(UploadAction &Action, bool bUpload);
		bool DoGetAction(UploadAction &Action);
		bool ParseAnswer(UploadAction &Action, std::string& Body);
		std::string ReplaceVars(const std::string& Text);
		int RetryLimit();
		void AddQueryPostParams(UploadAction& Action);
		bool ReadServerResponse(UploadAction& Action);
		void SetStatus(StatusType status, std::string param = "");
		bool needStop();
		void UploadError(bool error, const std::string errorStr, UploadAction* m_CurrentAction, bool writeToBuffer = true);
		bool doUploadFile(FileUploadTask* task, CIUUploadParams &params);
		bool doUploadUrl(UrlShorteningTask* task, CIUUploadParams &params);
		void prepareUpload();
		bool executeActions();

		Utf8String m_ErrorReason;
		Utf8String m_FileName;
		Utf8String m_displayFileName;
		LoginInfo li;
		ErrorInfo m_LastError;
		std::string m_ErrorBuffer;
		int m_CurrentActionIndex;
		std::map<std::string, std::string> m_Vars;
		std::map<std::string, std::string> m_Consts;
		std::map<size_t, bool> m_PerformedActions;
	private:
		DISALLOW_COPY_AND_ASSIGN(CDefaultUploadEngine);
};

#endif