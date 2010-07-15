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

#include "stdafx.h"
#include "UploadEngine.h"
#include "../myutils.h"
#include "../settings.h"
#include "../3rdpart/markup.h"
#include <pcre++.h>
#include <algorithm>

bool compareEngines(const CUploadEngine& elem1, const CUploadEngine& elem2)
{
	return elem1.Name.CompareNoCase(elem2.Name)<0;
	//elem1.< elem2.key1;
}

CUploadEngine::CUploadEngine()
{
}

CUploadEngineList::CUploadEngineList()
{

}

bool CUploadEngineList::LoadFromFile(const CString& filename)
{
	int i = 0;

	CMarkup XML;
	CString XmlFileName = IU_GetDataFolder() + /*_T("servers.xml")*/filename;

	if(!FileExists(XmlFileName))
	{
		m_ErrorStr = TR("Файл не найден.");
		return false;
	}

	if(!XML.Load(XmlFileName))
	{
		m_ErrorStr = XML.GetError();
		return false;
	}

	if(!XML.FindElem(_T("Servers")))
	{
		m_ErrorStr = _T("Unable to find Servers node");
		return false;	
	}

	XML.IntoElem();

	while(XML.FindElem(_T("Server")))
	{
		CUploadEngine UE;
		UE.NumOfTries = 0;
		UE.NeedAuthorization = _ttoi(XML.GetAttrib(_T("Authorize")));
		//*UE.Name =0;
		CString RetryLimit = XML.GetAttrib(_T("RetryLimit"));
		if(RetryLimit.IsEmpty())
		{
			UE.RetryLimit = Settings.FileRetryLimit;
		}
		else UE.RetryLimit = _ttoi(RetryLimit);

		CString ServerName = XML.GetAttrib(_T("Name"));
		UE.SupportsFolders = (bool) _ttoi(XML.GetAttrib(_T("SupportsFolders")));
		UE.RegistrationUrl = XML.GetAttrib(_T("RegistrationUrl"));
		UE.PluginName = XML.GetAttrib(_T("Plugin"));
		UE.UsingPlugin = !UE.PluginName.IsEmpty();
		UE.Debug =  (bool) _ttoi(XML.GetAttrib(_T("Debug")));
		UE.ImageHost =  !(bool) _ttoi(XML.GetAttrib(_T("FileHost")));
		UE.MaxFileSize =   _ttoi(XML.GetAttrib(_T("MaxFileSize")));
		XML.IntoElem();
		UE.Name =  ServerName;

		if(XML.FindElem(_T("Actions")))
		{
			XML.IntoElem();
			int ActionIndex = 0;
			while(XML.FindElem())
			{
				UploadAction UA;
				UA.NumOfTries = 0;
				UA.Index = ActionIndex;

				CString RetryLimit = XML.GetAttrib(_T("RetryLimit"));
				if(RetryLimit.IsEmpty())
				{
					UA.RetryLimit =Settings.ActionRetryLimit;
				}
				else UA.RetryLimit = _ttoi(RetryLimit);
				UA.IgnoreErrors = _ttoi(XML.GetAttrib(_T("IgnoreErrors")));
				UA.Description= XML.GetAttrib(_T("Description"));
				ActionIndex++;
				UA.Type = XML.GetAttrib(_T("Type"));
				UA.Url = XML.GetAttrib(_T("Url"));
				UA.PostParams = XML.GetAttrib(_T("PostParams"));
				UA.RegExp = XML.GetAttrib(_T("RegExp"));
				UA.OnlyOnce = _ttoi(XML.GetAttrib(_T("OnlyOnce")));

				CString AssignVars = XML.GetAttrib(_T("AssignVars"));
				
				pcrepp::Pcre reg("([A-z0-9_]*?):([0-9]{1,3})", "imc");
				std::string str = WCstringToUtf8(AssignVars);
				size_t pos = 0;
				while (pos <= str.length()) 
				{
					if( reg.search(str, pos)) 
					{
						pos = reg.get_match_end()+1;
						CString VariableName, VariableIndex;
						ActionVariable AV;
						AV.Name = Utf8ToWstring(reg[0]).c_str();
						AV.nIndex = atoi(reg[1].c_str());
						UA.Variables.push_back(AV);
					}
					else
						break;
				}

				/*CComBSTR BstrAssignVars = AssignVars;	
				RegExp exp;
				exp.SetPattern(_T("([A-z0-9_]*?):([0-9]{1,3})"));
				exp.Execute(BstrAssignVars);

				int n = exp.MatchCount();

				for(int i=0; i<n; i++) // count of variables
				{
					int nSub =	exp.SubMatchCount(i);
					CString VariableName, VariableIndex;
					ActionVariable AV;
					AV.Name = exp.GetSubMatch(i,0);
					AV.nIndex = _ttoi(exp.GetSubMatch(i,1));
					UA.Variables.push_back(AV); //Adding variable name and it's index to the map
				}*/
				UE.Actions.push_back(UA);
			}

			XML.OutOfElem();
		}

		if(XML.FindElem(_T("Result")))
		{
			UE.DownloadUrlTemplate = XML.GetAttrib(_T("DownloadUrlTemplate"));
			UE.ImageUrlTemplate = XML.GetAttrib(_T("ImageUrlTemplate"));
			UE.ThumbUrlTemplate = XML.GetAttrib(_T("ThumbUrlTemplate"));
		}

		UE.SupportThumbnails = !UE.ThumbUrlTemplate.IsEmpty();
		XML.OutOfElem();

		m_list.push_back(UE);
	}
	std::sort(m_list.begin(), m_list.end(), compareEngines);
	return true;
}

CUploadEngine* CUploadEngineList::byIndex(int index)
{
	if(index>=0 && index<m_list.size())
	return &m_list[index];
	else return 0;
}

int CUploadEngineList::count()
{
	return m_list.size();
}

CUploadEngine* CUploadEngineList::byName(const CString &name)
{
	for(int i=0; i<m_list.size(); i++)
	{
		if(!lstrcmp(m_list[i].Name, name)) return &m_list[i];
	}
	return 0;
	
}

const CString CUploadEngineList::ErrorStr()
{
	return m_ErrorStr;
}

int CUploadEngineList::getRandomImageServer()
{
	std::vector<int> m_suitableServers;
	for(int i=0; i<m_list.size(); i++)
	{
		if(m_list[i].NeedAuthorization!=2 && m_list[i].ImageHost) m_suitableServers.push_back(i);
	}
	return m_suitableServers[rand()%(m_suitableServers.size())];
}

int CUploadEngineList::getRandomFileServer()
{
	std::vector<int> m_suitableServers;
	for(int i=0; i<m_list.size(); i++)
	{
		if(m_list[i].NeedAuthorization!=2 && !m_list[i].ImageHost) m_suitableServers.push_back(i);
	}
	return m_suitableServers[rand() % m_suitableServers.size()];
}

int CUploadEngineList::GetUploadEngineIndex(const CString Name)
{
	for(int i=0; i<m_list.size(); i++)
	{
		if(m_list[i].Name == Name) return i;
	}
	return -1;
}