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

#include "StdAfx.h"
#include "Uploader.h"
#include "atlenc.h"
#include <atlconv.h>

//#include <wininet.h>

#include <cstdlib>
#include <ctime>
#include "Common/regexp.h"
#include "TextViewDlg.h"


//CAtlArray<UploadEngine> EnginesList;
//std::vector<UploadEngine> EnginesList;
#ifdef DEBUG
void TestExpr(LPCTSTR Expr, LPCTSTR Text)
{
	CComBSTR t1 = Expr;
	RegExp exp;
	MessageBox(0, Text, Expr, 0);
	 exp.SetPattern(t1);
	 if(exp.Execute(Text))
	 {
		 MessageBox(0, _T("Error while executing RegExp"), 0,0);
	 }
	 int n = exp.MatchCount();
	 
	 ShowVar(n);
	 for(int i=0; i<n; i++)
	 {
		int nSub =	exp.SubMatchCount(i);
		MessageBox( 0,exp.GetMatch(i),0,0);
		ShowVar(nSub);
		for(int j=0; j<nSub; j++)
		{
			MessageBox(0,exp.GetSubMatch(i,j),0,0);
		}
	 }
}
#endif 

CUploader::CUploader(void)
{
	CurrentServer = -1;
	randomize();
}

CUploader::~CUploader(void)
{
}

bool CUploader::ServerSupportThumbnails(int ServerID)
{
	return EnginesList[ServerID].SupportThumbnails;

}

CString CUploader::ReplaceVars(const CString Text)
{
	CString Result;
	Result =  Text;

	RegExp exp;
	CComBSTR Pat = _T("\\$\\(([A-z0-9_]*?)\\)");
	CComBSTR Txt = Text;
	exp.SetPattern(Pat);
	exp.Execute(Txt);
	
	int n = exp.MatchCount();

	for(int i=0; i<n; i++) // count of variables
	{
		CComBSTR VarName = exp.GetSubMatch(i, 0);
		CString vv = VarName;
		if(!vv.IsEmpty() && vv[0] == _T('_'))
		Result.Replace(CString(_T("$(")) + vv + _T(")"),m_Consts[vv]);
		else
		Result.Replace(CString(_T("$(")) + vv + _T(")"),m_Vars[vv]);
	}

	return Result;
}
LoginInfo LoadLogin(int ServerId);

int CUploader::pluginProgressFunc (void* userData, double dltotal,double dlnow,double ultotal, double ulnow)
{
	CUploader *uploader = reinterpret_cast<CUploader*>(userData);
	
	if(!uploader) return 0;

	if(*uploader->ShouldStop)
		return -1;

	if(ultotal<0 || ulnow<0) return 0;

	uploader->PrInfo->CS.Lock();

	if(ultotal == ulnow)
	{
		uploader->PrInfo->IsUploading = false;
		if(uploader->PrInfo->Bytes.size() || (bool)ultotal /*|| uploader->PrInfo->Uploaded*/)
		uploader->SetStatusText(TR("Ожидание ответа от сервера..."));
	
		uploader->PrInfo->Bytes.clear();
		}
	else
	{
		uploader->PrInfo->IsUploading = true;		
		uploader->PrInfo->Total = ultotal;
		uploader->PrInfo->Uploaded = ulnow;
		
	}
	uploader->PrInfo->CS.Unlock();

	return 0;
}
	
bool CUploader::UploadFile(LPTSTR FileName,LPTSTR szUrlBuffer,LPTSTR szThumbUrlBuffer,int ThumbWidth)
{
	if(CurrentServer<0) return false;
	CIUUploadParams uparams;
	CurrentEngine = EnginesList[CurrentServer];
	UploadEngine& Engine = CurrentEngine;


m_NetworkManager.setProgressCallback(pluginProgressFunc, (void*)this);
	if(Engine.UsingPlugin)
	{
		CUploadScript *plugin = iuPluginManager.getPlugin(Engine.PluginName,Settings.ServersSettings[Engine.Name]);
		if(!plugin)
		{
			return false;
		}
		plugin->bindNetworkManager(&m_NetworkManager);
		ConfigureProxy();

		
		if(!plugin) return false;
		PrInfo->CS.Lock();
				PrInfo->Uploaded = 0;
				PrInfo->Total = 0;
				PrInfo->Bytes.clear();
				PrInfo->CS.Unlock();


		CFolderItem parent, newFolder = Settings.ServersSettings[Engine.Name].newFolder;
		CString folderID = Settings.ServersSettings[Engine.Name].params["FolderID"];
		
		if(folderID == IU_NEWFOLDERMARK)
		{
			SetStatusText((TR("Создание папки \"") + newFolder.title + _T("\"...")).c_str());
			if( plugin->createFolder(parent,newFolder))
			{
				folderID = newFolder.id.c_str();
				Settings.ServersSettings[Engine.Name].params["FolderID"] = folderID; 
				Settings.ServersSettings[Engine.Name].params["FolderUrl"] = newFolder.viewUrl.c_str(); 
			}
			else folderID.Empty();
		}
		
		uparams.folderId = folderID; 
			
		int result = 0;
		do
		{
			if(*ShouldStop) return false;
			result = plugin->uploadFile(FileName,uparams); 
			CurrentEngine.NumOfTries++;
			if(*ShouldStop) return false;
			if(!result && CurrentEngine.NumOfTries!=CurrentEngine.RetryLimit) 
			{
				CString Err;
				Err.Format(TR("Загрузка на сервер не удалась. Повторяю (%d)..."),CurrentEngine.NumOfTries );
				UploadError(2, 0,Err); 
			}
		}
		while(!result && CurrentEngine.NumOfTries<CurrentEngine.RetryLimit);

		if(szUrlBuffer)
			lstrcpy(szUrlBuffer, uparams.DirectUrl.c_str()); 

		if( szThumbUrlBuffer)
			lstrcpy(szThumbUrlBuffer, uparams.ThumbUrl.c_str()); 

		m_DownloadUrl =  uparams.ViewUrl.c_str();
		return result;
	}

	m_Vars.clear();
	if(Engine.NeedAuthorization)
	{
		li = LoadLogin(CurrentServer);
		if(li.DoAuth)
		{
			m_Consts[_T("_LOGIN")] = li.Login;
			m_Consts[_T("_PASSWORD")] = li.Password;
		}
	}
	CString FileExt = GetFileExt(FileName);
	FileExt.MakeLower();
	m_Consts[_T("_FILENAME")] = myExtractFileName(FileName);

	CString OnlyFname;
	GetOnlyFileName(FileName, OnlyFname.GetBuffer(lstrlen(FileName)*2));
	m_Consts[_T("_FILENAMEWITHOUTEXT")] = OnlyFname;
	m_Consts[_T("_FILEEXT")] = FileExt ;
	m_Consts[_T("_THUMBWIDTH")] = IntToStr(ThumbWidth);

	int n = random(256 * 256);
	m_Consts[_T("_RAND16BITS")] = IntToStr(n);
	this->ThumbWidth = ThumbWidth;
	m_FileName =  FileName;
	ImgUrl = szUrlBuffer;
	ThumbUrl = szThumbUrlBuffer;

	bool EngineRes = false;
	do
	{
		if(*ShouldStop) return false;
		EngineRes = DoTry();
		CurrentEngine.NumOfTries++;
		if(*ShouldStop) return false;
		if(!EngineRes && CurrentEngine.NumOfTries!=CurrentEngine.RetryLimit) 
		{
			CString Err;
			Err.Format(TR("Загрузка на сервер не удалась. Повторяю (%d)..."),CurrentEngine.NumOfTries );
			UploadError(2, 0,Err); 
		}
	}
	while(!EngineRes && CurrentEngine.NumOfTries<CurrentEngine.RetryLimit);

	if(!EngineRes) 
	{
		UploadError(true, 0,TR("Загрузка не сервер удалась! (лимит попыток исчерпан)"));
		return false;
	}
	if(szUrlBuffer)
		lstrcpy(szUrlBuffer, m_ImageUrl); 

	if( szThumbUrlBuffer)
		lstrcpy(szThumbUrlBuffer, m_ThumbUrl); 

	return true;
}

bool CUploader::DoTry()
{
	for(int i = 0; i<CurrentEngine.Actions.size(); i++)
	{
		PrInfo->CS.Lock();
		PrInfo->Bytes.clear();
		PrInfo->CS.Unlock();
		CurrentEngine.Actions[i].NumOfTries = 0;
		bool ActionRes = false;
		do
		{
			if(*ShouldStop) return false;
			ActionRes = DoAction(CurrentEngine.Actions[i]);
			CurrentEngine.Actions[i].NumOfTries++;
			if(*ShouldStop) return false;
			if(!ActionRes )
			{
				CString ErrorStr ; 
				ErrorStr.Format(TR("Ошибка при выполнении действия #%d."), i+1);
				if(!m_ErrorReason.IsEmpty())
				{
					ErrorStr+=CString(_T("\r\n")) + TR("Причина: ") + m_ErrorReason;
				}

				if(CurrentEngine.Actions[i].NumOfTries == CurrentEngine.Actions[i].RetryLimit)
					ErrorStr+=CString(_T("\r\n"))+TR("Исчерпан лимит попыток.");
				else
				{
					CString TryIndex ;
					TryIndex.Format(CString(_T("\r\n"))+TR("Ещё одна попытка (%d)..."),CurrentEngine.Actions[i].NumOfTries);
					ErrorStr+=TryIndex ;
				}

				UploadError(2, ReplaceVars(CurrentEngine.Actions[i].Url), ErrorStr);
			}
		}
		while(CurrentEngine.Actions[i].NumOfTries<CurrentEngine.Actions[i].RetryLimit && !ActionRes);
		if(!ActionRes) return false;
	}
	m_ThumbUrl = ReplaceVars(CurrentEngine.ThumbUrlTemplate); // We are getting results :)
	m_ImageUrl = ReplaceVars(CurrentEngine.ImageUrlTemplate); // We are getting results :)
	m_DownloadUrl = ReplaceVars(CurrentEngine.DownloadUrlTemplate); // We are getting results :)
	return true;
}

bool CUploader::SelectServer(DWORD ServerID)
{
 	if(CurrentServer == ServerID) return true;
	m_PerformedActions.clear();
	CurrentServer = ServerID;
	return true;
}

#define HTTP_UPLOAD_BUFFER_SIZE 1024 * 10



bool CUploader::DoUploadAction(UploadAction &Action, bool bUpload)
{
	try
	{	
		ConfigureProxy();
		AddQueryPostParams(Action);

			const DWORD     cbProceed = Settings.UploadBufferSize;
			m_NetworkManager.setUrl(WCstringToUtf8(Action.Url));

			if(bUpload)
			{
				if (Action.Type == _T("put"))
				{
					m_NetworkManager.setMethod("PUT");
					m_NetworkManager.doUpload(WCstringToUtf8(m_FileName), "");

				}
				else
			m_NetworkManager.doUploadMultipartData();
			}
			else
				m_NetworkManager.doPost();
			
		bool Res = ReadServerResponse(Action);
		SetStatusText(_T(""));
		return Res;   
	} 
	catch (...) 
	{
		
		return FALSE;
	}
	return true;
}

bool CUploader::DoGetAction(UploadAction &Action)
{
	bool Result = false;

	try
	{	
		ConfigureProxy();
		m_NetworkManager.doGet(WCstringToUtf8( Action.Url));

		if(*ShouldStop) return 0;
		Result = ReadServerResponse(Action);
		SetStatusText(_T(""));
	} 
	catch (...) 
	{
		
		return FALSE;
	}
	return Result;
}

bool CUploader::ParseAnswer(UploadAction &Action, LPCTSTR Body)
{

	if((!ThumbUrl && !ImgUrl) || !Body) return false;

	if(!Action.RegExp.IsEmpty())
	{
		if(EnginesList[CurrentServer].Debug)
		{
			CTextViewDlg TextViewDlg(Body, CString(_T("Server reponse")), CString(_T("Server reponse:")), _T("Save to file?"));
	
			if(TextViewDlg.DoModal(GetActiveWindow())==IDOK)
			{
				CFileDialog fd(false, 0, 0, 4|2, _T("*.*\0*.*\0\0") ,GetActiveWindow());
				lstrcpy(fd.m_szFileName,_T("file.html"));
				if(fd.DoModal() == IDOK) 
				{
					FILE * f = _tfopen(fd.m_szFileName,_T("wb"));
					if(f)
					{
						WORD BOM = 0xFEFF;
						fwrite(&BOM, sizeof(BOM),1,f);
						fwrite(Body, lstrlen(Body), sizeof(TCHAR), f);
						fclose(f);
					}
				}
			}
		}
		RegExp exp;
		CComBSTR Pat = Action.RegExp;

		exp.SetPattern(Pat);
		exp.Execute(Body);

		int n = exp.MatchCount();

		CString DebugVars = _T("Regex: ") + Action.RegExp + _T("\r\n\r\n");

		if(!n) 
		{
			if(EnginesList[CurrentServer].Debug)
			{
				DebugVars += _T("NO MATCHES FOUND!");
				MessageBox(GetActiveWindow(), DebugVars, _T("Debug"), MB_ICONWARNING);
			}
			return false; //ERROR! Current action failed!
		}

		DebugVars += _T("\r\nVars assigned:\r\n");
		for(int i=0; i < Action.Variables.size(); i++)
		{
			ActionVariable &v = Action.Variables[i];
			CString temp;
			temp = exp.GetSubMatch(0, v.nIndex);
			if(!v.Name.IsEmpty() && v.Name[0] == _T('_'))
				m_Consts[v.Name] = temp;
			m_Vars[v.Name] = temp;
			DebugVars += v.Name +_T(" = ") + temp + _T("\r\n");
		}
		if(EnginesList[CurrentServer].Debug)
		{
			MessageBox(GetActiveWindow(), DebugVars, _T("Debug"), MB_ICONINFORMATION);
		}
	}

	return true; //ALL OK!

}

// WORD EventType { 0 - stores error string into m_ErrorReason, 1 - displays error into error console }
void CUploader::UploadError(WORD EventType, LPCTSTR Url, LPCTSTR  Error, int ErrorCode, LPCTSTR ErrorExplication)
{
	CString FullErrorString;
	CString BoldError;
	if(!m_FileName.IsEmpty())
	FullErrorString += CString(TR("Файл:"))+_T("   ")+ m_FileName+_T("\n");
	FullErrorString += CString(TR("Сервер:"))+_T("   ")+EnginesList[CurrentServer].Name;

	if(Url && *Url)
	{
		FullErrorString+=_T("\n");
		FullErrorString+=CString(_T("URL:"))+_T("   ");
		FullErrorString+=Url;
		
	}
	
	if(Error && *Error && ErrorCode)
	{
		FullErrorString+=_T("\n");
		FullErrorString+=Error;
	}
	else if(Error && *Error && !ErrorCode)
	{
		BoldError+=Error;
	}

	if(ErrorCode && ErrorExplication)
	{
		
		BoldError+= IntToStr(ErrorCode) +_T(" ")+ ErrorExplication;
	}

	else if (ErrorCode && !ErrorExplication)
	{
		CString Explication =  DisplayError(ErrorCode);

		if(Explication.IsEmpty())
		{

		}
		BoldError += CString(TR("Код ошибки WinInet: ")) + IntToStr(ErrorCode);
		
		if(!Explication.IsEmpty())
		{
			BoldError+=_T("\n");
			BoldError += Explication;
		}
	}

	if(!EventType)
		m_ErrorReason = BoldError;
	else if (EventType == 2)
		WriteLog(logWarning,TR("Модуль загрузки"), BoldError, FullErrorString);
	else
	WriteLog(logError,TR("Модуль загрузки"), BoldError, FullErrorString);
}

bool CUploader::DoAction(UploadAction &Action)
{
	bool Result = true;

	if(Action.OnlyOnce)
	{
		if(m_PerformedActions[Action.Index]==true)
			return true;
	}
	CString Status;
	if(!Action.Description.IsEmpty())
		 Status = Action.Description;
	else 
	{
		if(Action.Type == _T("upload"))
			Status = TR("Отправка файла на сервер...");
		else if(Action.Type == _T("login") && (EnginesList[CurrentServer].NeedAuthorization && li.DoAuth))
			Status = TR("Авторизация на сервере...");
		else
		Status.Format(TR("Выполняю действие #%d..."), Action.Index+1);
	}
	SetStatusText(Status);

	UploadAction Current = Action;
	CString temp = Current.Url;
	 Current.Url = ReplaceVars(temp);
	
	if(EnginesList[CurrentServer].Debug /*&& Action.Type != _T("post") && Action.Type != _T("upload")*/)
		MessageBox(GetActiveWindow(),  Status+_T("\r\nType:")+Action.Type+_T("\r\nURL: ")+Current.Url,_T("Debug"), MB_ICONINFORMATION);

	if(Action.Type == _T("upload"))
		Result = DoUploadAction(Current,true);
	else if(Action.Type == _T("put"))
		Result = DoUploadAction(Current,true);
	else if(Action.Type == _T("post"))
		Result = DoUploadAction(Current, false);
	else if(Action.Type == _T("login"))
	{
		if(EnginesList[CurrentServer].NeedAuthorization && li.DoAuth)
		Result = DoUploadAction(Current, false);
	}
	else if(Action.Type == _T("get"))
		Result = DoGetAction(Current);


	if(Action.OnlyOnce)
	{
		if(Result)
		 m_PerformedActions[Action.Index]=true;
	}

	if(Action.IgnoreErrors) return true;
	else return Result;
}

void CUploader::ConfigureProxy()
{
	IU_ConfigureProxy(m_NetworkManager);	
}


void CUploader::AddQueryPostParams(UploadAction &Action)
{
	m_NetworkManager.setReferer(WCstringToUtf8( Action.Referer.IsEmpty()?Action.Url:Action.Referer));
	RegExp exp;
	CComBSTR Pat = _T("(.*?)=(.*?[^\\x5c]{0,1});");
	CComBSTR Txt = Action.PostParams;
	int len = Txt.Length();
	if(len)
	{
		if(Txt[len-1]!=_T(';'))
		{
			Txt+=_T(";");
		}
	}
		
	exp.SetPattern(Pat);
	exp.Execute(Txt);
	CString _Post = CString(*_T("Post Request to URL: "))+Action.Url+_T("\r\n");
	
	int n = exp.MatchCount();

	for(int i=0; i<n; i++) // count of variables
	{
		CComBSTR VarName = exp.GetSubMatch(i, 0);
		CComBSTR VarValue= exp.GetSubMatch(i, 1);
		
		if(!VarName.Length()) continue;
		
		CString NewValue = VarValue;
		NewValue.Replace(_T("\\;"),_T(";"));
		CString NewName = VarName;
		NewName = ReplaceVars(NewName);
		CString vv =NewName;
		if(NewValue == _T("%filename%"))
		{
			_Post+=NewName+_T(" = ** THE FILE IS HERE ** \r\n");
			m_NetworkManager.addQueryParamFile( WCstringToUtf8(NewName), WCstringToUtf8(m_FileName),
			WCstringToUtf8(myExtractFileName(m_FileName)), WCstringToUtf8(IU_GetFileMimeType(m_FileName)));
		}
		else 
		{
			
			NewValue = ReplaceVars(NewValue);
			_Post+=NewName+_T(" = ")+NewValue+_T("\r\n");
			m_NetworkManager.addQueryParam(WCstringToUtf8(NewName),WCstringToUtf8(NewValue));
		}
	}

	if(EnginesList[CurrentServer].Debug)
		MessageBox(GetActiveWindow(), _Post, _T("Debug"),MB_ICONINFORMATION);
}

bool CUploader::ReadServerResponse(UploadAction &Action)
{
	
	bool Result = false;
	bool Exit = false;

	std::map<int,CString> errors;
	errors[ CURLE_UNSUPPORTED_PROTOCOL]="CURLE_UNSUPPORTED_PROTOCOL";
	errors[ CURLE_FAILED_INIT]="CURLE_FAILED_INIT";
	errors[ CURLE_URL_MALFORMAT]="CURLE_URL_MALFORMAT";
	errors[ CURLE_OBSOLETE4]="CURLE_OBSOLETE4";
	errors[ CURLE_COULDNT_RESOLVE_PROXY]="CURLE_COULDNT_RESOLVE_PROXY";
	errors[ CURLE_COULDNT_RESOLVE_HOST]="CURLE_COULDNT_RESOLVE_HOST";
	errors[ CURLE_COULDNT_CONNECT]="CURLE_COULDNT_CONNECT";
	errors[ CURLE_FTP_WEIRD_SERVER_REPLY]="CURLE_FTP_WEIRD_SERVER_REPLY";
	errors[ CURLE_REMOTE_ACCESS_DENIED]="CURLE_REMOTE_ACCESS_DENIED";
			
		int StatusCode = m_NetworkManager.responseCode();
		if(!(StatusCode>=200 && StatusCode<=299) && !(StatusCode>=300 && StatusCode<=304))
		{
			CString error;
			if(m_NetworkManager.getCurlResult()!=CURLE_OK)
		{
			error = _T("Curl error: ")+ errors[m_NetworkManager.getCurlResult()] +_T("\r\n");
		}
			if(!StatusCode)StatusCode= m_NetworkManager.getCurlResult();
			UploadError(false, 0, 0, StatusCode, /*pobjHttpRes->GetStatusText()*//*_T("Some error")*/error + Utf8ToWstring( m_NetworkManager.errorString()).c_str());
			return false;
		}
		
	DWORD           dwRead ;
	size_t          cbTotal = 0 ;

	// Если нас сервер куда-то перенаправляет
	CString Refresh;
	
	Refresh = Utf8ToWstring( m_NetworkManager.responseHeaderByName("Refresh")).c_str();
	if(lstrlen(Refresh))
	{			
		Refresh = MoveToEndOfW((LPTSTR)(LPCTSTR)Refresh,_T("url="));
		UploadAction Redirect = Action;
		Redirect.Url = Refresh;
		Redirect.Referer = Action.Url;
		Redirect.Type = _T("get");
		Result = DoGetAction(Redirect);
		Exit = true;
	}

	if(!Exit)
	{
		CString answer = Utf8ToWstring( m_NetworkManager.responseBody()).c_str();
		Result =  ParseAnswer(Action,answer);
		
		if(!Result) 
			UploadError(false, 0, TR("Ответ сервера не содержит нужных данных."));
	}
	return Result;
}

CString CUploader::GetStatusText()
{
	return m_StatusText;
}

void CUploader::SetStatusText(CString Text)
{
	m_CS.Lock();
	 m_StatusText  = Text;
	 m_CS.Unlock();
}

CString CUploader::getDownloadUrl()
{
	m_CS.Lock();
	return m_DownloadUrl;
	m_CS.Unlock();
}