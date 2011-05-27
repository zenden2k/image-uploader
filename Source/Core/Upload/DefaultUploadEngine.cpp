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

#include "DefaultUploadEngine.h"
#include "../3rdpart/codepages.h"
#include "../3rdpart/pcreplusplus.h"

CDefaultUploadEngine::CDefaultUploadEngine():CAbstractUploadEngine()
{
	m_CurrentActionIndex = -1;
}

bool CDefaultUploadEngine::doUpload(Utf8String FileName, Utf8String DisplayName, CIUUploadParams &params)
{
	if(FileName.empty())
	{
		UploadError(true, "Filename should not be empty!", 0);
		return false;
	}

	if(!m_UploadData)
	{
		UploadError(true, "m_UploadData should not be NULL!", 0);
		return false;
	}
	m_FileName = FileName;
	m_displayFileName = DisplayName;

	m_Vars.clear();
	if(m_UploadData->NeedAuthorization)
	{
		li = m_ServersSettings.authData;
		if(li.DoAuth)
		{
			m_Consts["_LOGIN"] = (li.Login);
			m_Consts["_PASSWORD"] = (li.Password);
		}	
	}
	std::string FileExt = IuCoreUtils::ExtractFileExt(DisplayName);
	m_Consts["_FILENAME"] = IuCoreUtils::ExtractFileName(FileName);
	std::string OnlyFname;
	OnlyFname = IuCoreUtils::ExtractFileNameNoExt(FileName);
	m_Consts["_FILENAMEWITHOUTEXT"] = OnlyFname;
	m_Consts["_FILEEXT"] = FileExt ;
	m_Consts["_THUMBWIDTH"] = IuCoreUtils::toString(m_ThumbnailWidth);

	int n = rand()%(256 * 256);
	m_Consts["_RAND16BITS"] = IuCoreUtils::toString(n);

	for(size_t i = 0; i<m_UploadData->Actions.size(); i++)
	{
		m_UploadData->Actions[i].NumOfTries = 0;
		bool ActionRes = false;
		do
		{
			if(needStop()) return false;
			ActionRes = DoAction(m_UploadData->Actions[i]);
			m_UploadData->Actions[i].NumOfTries++;
			if(needStop()) return false;
			if(!ActionRes )
			{
				std::string ErrorStr = m_ErrorReason;
				ErrorType errorType;
				MessageType mt = mtWarning;
				if(!m_ErrorReason.empty())
				{
					ErrorStr+=m_ErrorReason;
				}

				if(m_UploadData->Actions[i].NumOfTries == m_UploadData->Actions[i].RetryLimit)
				{
					errorType = etActionRetriesLimitReached;
				}
				else
				{
					errorType = etActionRepeating;
					ErrorStr += "retrying... ";
					ErrorStr += "("+IuCoreUtils::toString(m_UploadData->Actions[i].NumOfTries+1)
					+ " of "+ IuCoreUtils::toString(m_UploadData->Actions[i].RetryLimit)+")" ;
				}
				UploadError(mt == mtError, ErrorStr, /*&m_UploadData->Actions[i]*/0, false);
			}
		}
		while(m_UploadData->Actions[i].NumOfTries<m_UploadData->Actions[i].RetryLimit && !ActionRes);
		if(!ActionRes) return false;
	}
	Utf8String m_ThumbUrl = ReplaceVars(m_UploadData->ThumbUrlTemplate); // We are getting results :)
	Utf8String m_ImageUrl = ReplaceVars(m_UploadData->ImageUrlTemplate); // We are getting results :)
	Utf8String m_DownloadUrl = ReplaceVars(m_UploadData->DownloadUrlTemplate); // We are getting results :)

	params.ThumbUrl = m_ThumbUrl;
	params.DirectUrl = m_ImageUrl;
	params.ViewUrl = m_DownloadUrl;
	return true;
}

bool CDefaultUploadEngine::DoUploadAction(UploadAction &Action, bool bUpload)
{
	try
	{	
		AddQueryPostParams(Action);
		m_NetworkManager->setUrl(Action.Url);

		if(bUpload)
		{
			if (Action.Type == "put")
			{
				m_NetworkManager->setMethod("PUT");
				m_NetworkManager->doUpload(m_FileName, "");
			}
			else
				m_NetworkManager->doUploadMultipartData();
		}
		else
			m_NetworkManager->doPost();

		bool Res = ReadServerResponse(Action);
		return Res;   
	} 
	catch (...) 
	{

		return FALSE;
	}
	return true;
}

bool CDefaultUploadEngine::DoGetAction(UploadAction &Action)
{
	bool Result = false;

	try
	{	
		m_NetworkManager->doGet(Action.Url);
		if(needStop()) return 0;
		Result = ReadServerResponse(Action);
	} 
	catch (...) 
	{

		return FALSE;
	}
	return Result;
}

bool reg_single_match(const std::string pattern, const std::string &text, std::string &res)
{
	pcrepp::Pcre reg(pattern, "imc");
	if(reg.search(text) == true)
	{
		if(reg.matches()> 0)
		{
			res = reg.get_match(0);	
		}
		return true;
	}
	else return false;	
}

bool CDefaultUploadEngine::ParseAnswer(UploadAction &Action, std::string& Body)
{
	if(!Action.RegExp.empty())
	{
		std::string codePage;
		if(reg_single_match("text/html;\\s+charset=([\\w-]+)", Body,codePage))
		{
			IuCoreUtils::ConvertToUtf8(Body, codePage);
		}
		if(m_UploadData->Debug)
		{
			DebugMessage(Body, true);
		}
		try
		{
			pcrepp::Pcre reg(Action.RegExp, "imc");

			std::string DebugVars = "Regex: " + Action.RegExp + "\r\n\r\n";
			if(reg.search(Body) == true)
			{

				reg.matches();
				if(Action.Variables.empty())
				{
					DebugVars += "Variables list is empty!\r\n";
				}

				for(size_t i=0; i < Action.Variables.size(); i++)
				{
					ActionVariable &v = Action.Variables[i];
					std::string temp;
					temp = reg.get_match(v.nIndex);
					if(!v.Name.empty() && v.Name[0] == '_')
						m_Consts[v.Name] = temp;
					m_Vars[v.Name] = temp;
					DebugVars += v.Name + " = " + temp + "\r\n";
				}

			}

			else
			{
				if(m_UploadData->Debug)
				{
					DebugVars += "NO MATCHES FOUND!";
					DebugMessage(DebugVars);
				}
				UploadError(false, "Cannot obtain the necessary information from server response.", &Action);
				return false; //ERROR! Current action failed!

			}
			if(m_UploadData->Debug)
			{
				DebugMessage(DebugVars);
			}
		}
		catch(const std::exception& e)
		{
			UploadError(true, std::string("Regular expression error:")+e.what(), &Action, false);
		}

	}
	return true; 
}

bool CDefaultUploadEngine::DoAction(UploadAction &Action)
{
	bool Result = true;
	
	m_CurrentActionIndex = Action.Index;
	if(Action.OnlyOnce)
	{
		if(m_PerformedActions[Action.Index] == true)
			return true;
	}

	if(!Action.Description.empty())
		SetStatus(stUserDescription, Action.Description);
	else 
	{
		if(Action.Type == "upload")
			SetStatus(stUploading);
		else if(Action.Type == "login" && (m_UploadData->NeedAuthorization && li.DoAuth))
			SetStatus(stAuthorization);
		else
		{
			SetStatus(stPerformingAction);
		}
	}

	UploadAction Current = Action;
	std::string temp = Current.Url;
	Current.Url = ReplaceVars(temp);

	if(m_UploadData->Debug)
		DebugMessage("\r\nType:" + Action.Type + "\r\nURL: " + Current.Url);

	if(Action.Type == "upload")
		Result = DoUploadAction(Current,true);
	else if(Action.Type == "put")
		Result = DoUploadAction(Current,true);
	else if(Action.Type == "post")
		Result = DoUploadAction(Current, false);
	else if(Action.Type == "login")
	{
		if(m_UploadData->NeedAuthorization && li.DoAuth)
			Result = DoUploadAction(Current, false);
	}
	else if(Action.Type == "get")
		Result = DoGetAction(Current);

	if(Action.OnlyOnce)
	{
		if(Result)
			m_PerformedActions[Action.Index]=true;
	}

	if(Action.IgnoreErrors) return true;
	else return Result;
}

bool CDefaultUploadEngine::ReadServerResponse(UploadAction &Action)
{
	bool Result = false;
	bool Exit = false;

	int StatusCode = m_NetworkManager->responseCode();
	if(!(StatusCode>=200 && StatusCode<=299) && !(StatusCode>=300 && StatusCode<=304))
	{
		std::string error;
		if(m_NetworkManager->getCurlResult()!=CURLE_OK)
		{
			error = "Curl error: "+ m_NetworkManager->getCurlResultString();
		}
		else
		{
			error += "Server response code: "+IuCoreUtils::toString(StatusCode)+"\r\n";
			error += m_NetworkManager->errorString();
		}
		if(!StatusCode) StatusCode= m_NetworkManager->getCurlResult();
		UploadError(false, error, &Action);
		return false;
	}

	std::string Refresh = m_NetworkManager->responseHeaderByName("Refresh");

	if(!Refresh.empty()) // Redirecting to URL
	{			
		std::string redirectUrl;
		if(reg_single_match("url=(\\S+)", Refresh, redirectUrl))
		{
			UploadAction Redirect = Action;
			Redirect.Url = redirectUrl;
			Redirect.Referer = Action.Url;
			Redirect.Type = "get";
			Result = DoGetAction(Redirect);
			Exit = true;
		}
	}

	if(!Exit)
	{
		std::string answer = m_NetworkManager->responseBody();
		Result =  ParseAnswer(Action,answer);

		if(!Result) 
			UploadError(false, "Empty response!", &Action);
	}
	return Result;
}

void CDefaultUploadEngine::AddQueryPostParams(UploadAction &Action)
{
	m_NetworkManager->setReferer(Action.Referer.empty() ? Action.Url : ReplaceVars(Action.Referer));

	std::string Txt = Action.PostParams;
	int len = Txt.length();
	if(len)
	{
		if(Txt[len-1] != ';')
		{
			Txt+= ";";
		}
	}

	std::string _Post = "Post Request to URL: " + Action.Url + "\r\n";

	pcrepp::Pcre reg("(.*?)=(.*?[^\\x5c]{0,1});", "imc");
	std::string str = Txt;


	size_t pos = 0;
	while (pos <= str.length()) {
		if( reg.search(str, pos)) {

			std::string VarName = reg[0];
			std::string VarValue= reg[1];
			pos = reg.get_match_end() + 1;

			if(!VarName.length()) continue;

			std::string NewValue = VarValue;
			NewValue = IuCoreUtils::StrReplace(NewValue, "\\;",";");

			std::string NewName = VarName;
			NewName = ReplaceVars(NewName);
			std::string vv =NewName;
			if(NewValue == "%filename%")
			{
				_Post+= NewName + " = ** FILE CONTENTS ** \r\n";
				m_NetworkManager->addQueryParamFile(NewName, m_FileName,
					IuCoreUtils::ExtractFileName(m_displayFileName), IuCoreUtils::GetFileMimeType(m_FileName));
			}
			else 
			{

				NewValue = ReplaceVars(NewValue);
				_Post += NewName + " = " + NewValue + "\r\n";
				m_NetworkManager->addQueryParam(NewName, NewValue);
			}

		}
		else
			break;
	}

	if(m_UploadData->Debug)
		DebugMessage(_Post);
}

std::string  CDefaultUploadEngine::ReplaceVars(const std::string& Text)
{
	std::string  Result =  Text;

	pcrepp::Pcre reg("\\$\\(([A-z0-9_]*?)\\)", "imc");
	std::string str = (Text);
	size_t pos = 0;
	while (pos <= str.length()) 
	{
		if( reg.search(str, pos)) 
		{
			pos = reg.get_match_end()+1;
			std::string vv = reg[0];
			if(!vv.empty() && vv[0] == '_')
				Result = IuCoreUtils::StrReplace(Result,std::string("$(") + vv + std::string(")"),m_Consts[vv]);
			else
				Result = IuCoreUtils::StrReplace(Result,std::string("$(") + vv + std::string(")"),m_Vars[vv]);
		}
		else
			break;
	}
	return Result;
}

bool CDefaultUploadEngine::needStop()
{
	bool m_bShouldStop = false;
	if(onNeedStop)
		m_bShouldStop = onNeedStop(); // delegate call
	return m_bShouldStop;
}

void CDefaultUploadEngine::SetStatus(StatusType status, std::string param)
{
	if(onStatusChanged)
		onStatusChanged(status, m_CurrentActionIndex, param);
}

int CDefaultUploadEngine::RetryLimit()
{
	return m_UploadData->RetryLimit;
}

void CDefaultUploadEngine::UploadError(bool error, const std::string errorStr, UploadAction* m_CurrentAction,bool writeToBuffer )
{
	m_LastError.ServerName = m_UploadData->Name;

	if(m_CurrentAction)
	{
		m_LastError.ActionIndex = m_CurrentAction->Index+1;
		m_LastError.Url = m_CurrentAction->Url;
		m_LastError.RetryIndex = m_CurrentAction->NumOfTries+1;
	}
	
	if(!m_LastError.error.empty()) 
		m_LastError.error+="\n";
	
	m_LastError.error += errorStr;

	if(!writeToBuffer)
	{ 
		m_LastError.messageType = error ? mtError:mtWarning;
		m_LastError.errorType = etUserError;
		m_LastError.sender = "CDefaultUploadEngine";
		ErrorMessage(m_LastError);
		m_LastError.Clear();
	}
}	
