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

#include "UploadEngine.h"
#include <stdlib.h>
#include "Core/Utils/StringUtils.h"

CUploadEngineData::CUploadEngineData()
{

}

CUploadEngineList_Base::CUploadEngineList_Base()
{

}

CUploadEngineData* CUploadEngineList_Base::byIndex(size_t index)
{
        if(index<m_list.size())
            return &m_list[index];
	else return 0;
}

int CUploadEngineList_Base::count()
{
	return m_list.size();
}

CUploadEngineData* CUploadEngineList_Base::byName(const std::string &name)
{
	for(size_t i=0; i<m_list.size(); i++)
	{
		if(!IuStringUtils::stricmp(m_list[i].Name.c_str(), name.c_str())) return &m_list[i];
	}
	return 0;
}

int CUploadEngineList_Base::getRandomImageServer()
{
	std::vector<int> m_suitableServers;
	for(size_t i=0; i<m_list.size(); i++)
	{
		if(m_list[i].NeedAuthorization!=2 && m_list[i].ImageHost) m_suitableServers.push_back(i);
	}
	return m_suitableServers[rand()%(m_suitableServers.size())];
}

int CUploadEngineList_Base::getRandomFileServer()
{
	std::vector<int> m_suitableServers;
	for(size_t i=0; i<m_list.size(); i++)
	{
		if(m_list[i].NeedAuthorization!=2 && !m_list[i].ImageHost) m_suitableServers.push_back(i);
	}
	return m_suitableServers[rand() % m_suitableServers.size()];
}

int CUploadEngineList_Base::GetUploadEngineIndex(const std::string Name)
{
	for(size_t i=0; i<m_list.size(); i++)
	{
		if(m_list[i].Name == Name) return i;
	}
	return -1;
}


/* CAbstractUploadEngine */


CAbstractUploadEngine::~CAbstractUploadEngine()
{

}

bool CAbstractUploadEngine::DebugMessage(const std::string& message, bool isServerResponseBody)
{
	if(onDebugMessage)
		onDebugMessage(message, isServerResponseBody);
	return true;
}

bool CAbstractUploadEngine::ErrorMessage(ErrorInfo ei)
{
	if(onErrorMessage)
		onErrorMessage(ei);
	return true;
}

void CAbstractUploadEngine::setServerSettings(ServerSettingsStruct settings)
{
	m_ServersSettings = settings;
}

/*bool  CAbstractUploadEngine::uploadFile(Utf8String FileName,Utf8String DisplayName)
{
	CIUUploadParams params;
	m_NetworkManager->setProgressCallback(ProgressFunc, (void*)this);

	if(onConfigureNetworkManager)
		onConfigureNetworkManager(m_NetworkManager);

	bool EngineRes = false;
	int i=0;
	do
	{
		if(needStop()) return false;

		doUpload(FileName, DisplayName, params);

		i++;
		if(needStop()) return false;
		if(!EngineRes && i!=RetryLimit()) 
		{
			ErrorMessage(mtWarning, etRepeating, "", i);
		}
	}
	while(!EngineRes && i < RetryLimit());

	if(!EngineRes) 
	{
		ErrorMessage(mtError, etRetriesLimitReached, "", i);
		return false;
	}

}*/

/*int CAbstractUploadEngine::ProgressFunc (void* userData, double dltotal,double dlnow,double ultotal, double ulnow)
{
	CAbstractUploadEngine *uploader = reinterpret_cast<CAbstractUploadEngine*>(userData);

	if(!uploader) return 0;

	if(uploader->needStop())
		return -1;

	if(ultotal<0 || ulnow<0) return 0;

	if(ultotal == ulnow)
	{
		uploader->m_PrInfo.IsUploading = false;

		if(ultotal != 0)
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
}*/

bool CAbstractUploadEngine::needStop()
{
	if(m_bShouldStop)
		return m_bShouldStop;
	if(onNeedStop)
		m_bShouldStop = onNeedStop(); // delegate call
	return m_bShouldStop;
}

void CAbstractUploadEngine::SetStatus(StatusType status, std::string param)
{
	if(onStatusChanged)
		onStatusChanged(status, 0,  param);
}

void CAbstractUploadEngine::setNetworkManager(NetworkManager* nm)
{
	m_NetworkManager = nm;
}

void CAbstractUploadEngine::setUploadData(CUploadEngineData* data)
{
	m_UploadData = data;
}

CAbstractUploadEngine::CAbstractUploadEngine()
{
	m_bShouldStop = 0;
	m_NetworkManager = 0;
	m_UploadData = 0;
}


void CAbstractUploadEngine::setThumbnailWidth(int width)
{
	m_ThumbnailWidth = width;
}

CUploadEngineData* CAbstractUploadEngine::getUploadData() const
{
	return m_UploadData;
}
