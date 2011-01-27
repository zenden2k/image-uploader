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

#include "Uploader.h"
#include <cstdlib>
#include <ctime>

CUploader::CUploader(void)
{
	srand(time(0));
	m_bShouldStop = false;
	m_nThumbWidth = 160;
	m_CurrentStatus = stNone;
	m_CurrentEngine = NULL;
}

CUploader::~CUploader(void)
{
}

void CUploader::Cleanup()
{
	m_CurrentEngine->onDebugMessage.clear();
	m_CurrentEngine->onNeedStop.clear();
	m_CurrentEngine->onStatusChanged.clear();
	m_CurrentEngine->onErrorMessage.clear();
}

int CUploader::pluginProgressFunc (void* userData, double dltotal,double dlnow,double ultotal, double ulnow)
{
	CUploader *uploader = reinterpret_cast<CUploader*>(userData);
	
	if(!uploader) return 0;

	if(uploader->needStop())
		return -1;

	if(ultotal<0 || ulnow<0) return 0;

	if(ultotal == ulnow)
	{
		uploader->m_PrInfo.IsUploading = false;

		if(ultotal != 0 && uploader->m_CurrentStatus == stUploading)
			uploader->SetStatus(stWaitingAnswer);
	}
	else
	{
		uploader->m_PrInfo.IsUploading = true;		
		uploader->m_PrInfo.Total = ultotal;
		uploader->m_PrInfo.Uploaded = ulnow;	
	}

	if(!uploader->onProgress.empty())
		uploader->onProgress(uploader->m_PrInfo);
	return 0;
}

bool CUploader::UploadFile(const std::string & FileName, const std::string displayFileName)
{
	if(!m_CurrentEngine)
	{
		Error(true, "Cannnot proceed: m_CurrentEngine is NULL!");
		return false;
	}
	m_FileName = FileName;
	m_bShouldStop = false;
	if(onConfigureNetworkManager)
		onConfigureNetworkManager(&m_NetworkManager);
	m_CurrentEngine->setNetworkManager(&m_NetworkManager);
	m_CurrentEngine->onDebugMessage.bind(this, &CUploader::DebugMessage);
	m_CurrentEngine->onNeedStop.bind(this, &CUploader::needStop);
	m_CurrentEngine->onStatusChanged.bind(this, &CUploader::SetStatus);
	m_CurrentEngine->onErrorMessage.bind(this, &CUploader::ErrorMessage);

	m_CurrentEngine->setThumbnailWidth(m_nThumbWidth);
	std::string displayName = displayFileName;
	if(displayName.empty())
		displayName = IuCoreUtils::ExtractFileName(FileName);
	CIUUploadParams uparams;
	m_NetworkManager.setProgressCallback(pluginProgressFunc, (void*)this);
	bool EngineRes = false;
	int i=0;
	do
	{
		if(needStop()) { Cleanup(); return false;}
		EngineRes = m_CurrentEngine->doUpload(FileName,displayName, uparams);
		i++;
		if(needStop()) { Cleanup(); return false;}
		if(!EngineRes && i!=m_CurrentEngine->RetryLimit()) 
		{
			Error(false, "", etRepeating, i);
		}
	}
	while(!EngineRes && i < m_CurrentEngine->RetryLimit());

	
	if(!EngineRes) 
	{
		Error(true, "", etRetriesLimitReached);
		Cleanup();
		return false;
	}

	m_ImageUrl= (uparams.DirectUrl); 

	m_ThumbUrl = (uparams.ThumbUrl); 

	m_DownloadUrl =  (uparams.ViewUrl);


	return true;
}

bool CUploader::setUploadEngine(CAbstractUploadEngine* UploadEngine)
{
 	if(m_CurrentEngine == UploadEngine) return true;
	m_CurrentEngine = UploadEngine;
	return true;
}

void CUploader::SetStatus(StatusType status, int param1, std::string param)
{
	m_CurrentStatus = status;
	if(onStatusChanged)
		onStatusChanged(status, param1,  param);
}

const std::string CUploader::getDownloadUrl()
{
	return m_DownloadUrl;
}

CAbstractUploadEngine * CUploader::getUploadEngine()
{
	return m_CurrentEngine;
}

void CUploader::setThumbnailWidth(int width)
{
	m_nThumbWidth = width;
}

const std::string CUploader::getDirectUrl()
{
	return m_ImageUrl;
}
const std::string CUploader::getThumbUrl()
{
	return m_ThumbUrl;
}

void CUploader::stop()
{
	m_bShouldStop = true;
}

bool CUploader::needStop()
{
	if(m_bShouldStop)
		return m_bShouldStop;
	if(onNeedStop)
		m_bShouldStop = onNeedStop(); // delegate call
	return m_bShouldStop;
}

void CUploader::DebugMessage(const std::string& message, bool isServerResponseBody)
{
	if(onDebugMessage)
		onDebugMessage(message, isServerResponseBody);
}

void CUploader::ErrorMessage(ErrorInfo error)
{
	if(onErrorMessage)
		onErrorMessage(error);

}


void CUploader::Error(bool error, std::string message, ErrorType type, int retryIndex)
{
	ErrorInfo err;
	err.ActionIndex  = -1;
	err.messageType = error? mtError: mtWarning;
	err.error = message;
	err.FileName = m_FileName;
	err.errorType = type;
	err.sender = "CUploader";
	err.RetryIndex = retryIndex;
	ErrorMessage(err);
}