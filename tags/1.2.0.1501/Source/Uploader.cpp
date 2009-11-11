/*
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2009 ZendeN <zenden2k@gmail.com>
	 
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

#include <wininet.h>

#include <cstdlib>
#include <ctime>
#include "Common/regexp.h"

#define randomize() (srand((unsigned)time(NULL)))
#define random(x) (rand() % x)

CAtlArray<UploadEngine> EnginesList;

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

bool CUploader::UploadFile(LPTSTR FileName,LPTSTR szUrlBuffer,LPTSTR szThumbUrlBuffer,int ThumbWidth)
{
	if(CurrentServer<0) return false;

	CurrentEngine = EnginesList[CurrentServer];
	UploadEngine& Engine = CurrentEngine;

	m_Vars.clear();
	if(Engine.NeedAuthorization)
	{
		li = LoadLogin(CurrentServer);
		if(!li.UseIeCookies)
		{
			m_Consts[_T("_LOGIN")] = li.Login;
			m_Consts[_T("_PASSWORD")] = li.Password;
		}
	}
	CString FileExt = GetFileExt(FileName);
	FileExt.MakeLower();
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
using namespace Ryeol;
const DWORD     cbBuff = 1024 * 10 ;

bool CUploader::DoUploadAction(UploadAction &Action, bool bUpload)
{
	CHttpClient         objHttpReq ;
	CHttpResponse *     pobjHttpRes = NULL ;

	try
	{	
		ConfigureProxy(objHttpReq);
		AddQueryPostParams(Action, objHttpReq);

		if(bUpload)  // If we do file upload
		{
			const DWORD     cbProceed = Settings.UploadBufferSize;
			objHttpReq.BeginUpload (Action.Url);

			CHttpPostStat       objPostStat ;
			if(PrInfo)
			{
				PrInfo->CS.Lock();
				PrInfo->Uploaded = 0;
				PrInfo->Total = 0;
				PrInfo->Bytes.clear();
				PrInfo->CS.Unlock();
			}
			do 
			{
				if(*ShouldStop) return 0;
				// Displays progress information
				objHttpReq.Query (objPostStat) ;

				if(PrInfo)
				{
					PrInfo->CS.Lock();
					PrInfo->IsUploading = true;
					PrInfo->Uploaded =objPostStat.PostedByte();//objPostStat.CurrParamPostedByte ();
					PrInfo->Total = objPostStat.TotalByte();//objPostStat.CurrParamTotalByte();
					PrInfo->CS.Unlock();
				}
				if(objPostStat.PostedByte() == objPostStat.TotalByte())
					SetStatusText(TR("Ожидание ответа от сервера..."));
			} 
			while ( !(pobjHttpRes = objHttpReq.Proceed (cbProceed))) ;
			SetStatusText(TR("Ожидание ответа от сервера..."));
			PrInfo->CS.Lock();
			PrInfo->IsUploading = false;		
			PrInfo->Total = objPostStat.TotalByte();
			PrInfo->Uploaded = PrInfo->Total;
			PrInfo->CS.Unlock();
		} 
		else // If we do simple post request
		{
			pobjHttpRes=objHttpReq.RequestPost(Action.Url);
		}	
		bool Res = ReadServerResponse(Action, pobjHttpRes);
		SetStatusText(_T(""));
		return Res;   
	} 
	catch (httpclientexception & e) 
	{
		if(e.LastError()>400 && e.LastError()<600)
		{
			DWORD ErrorCode = e.Win32LastError();
			UploadError(false, 0, 0, ErrorCode, 0);
		}
		return FALSE;
	}

	return true;
}

bool CUploader::DoGetAction(UploadAction &Action)
{
	bool Result = false;

	CHttpClient         objHttpReq ;
	CHttpResponse *     pobjHttpRes = NULL ;

	try
	{	
		ConfigureProxy(objHttpReq);
		pobjHttpRes = objHttpReq.RequestGet(Action.Url);
		SetStatusText(TR("Ожидание ответа от сервера..."));
		if(*ShouldStop) return 0;
		Result = ReadServerResponse(Action, pobjHttpRes);
		SetStatusText(_T(""));
	} 
	catch (httpclientexception & e) 
	{
		if(e.LastError()>400 && e.LastError()<600)
		{
			DWORD ErrorCode = e.Win32LastError();
			CHttpResponse p(objHttpReq.m_hLastReq);
			UploadError(false, 0, 0, ErrorCode, 0);
		}
		return FALSE;
	}
	return Result;
}

bool CUploader::ParseAnswer(UploadAction &Action, LPTSTR Body)
{
	if((!ThumbUrl && !ImgUrl) || !Body) return false;

	if(!Action.RegExp.IsEmpty())
	{
		if(EnginesList[CurrentServer].Debug)
		{
			TCHAR buf[512];
			lstrcpyn(buf, Body, 511);

			if(MessageBox(GetActiveWindow(), CString(_T("Server reponse:\r\n"))+buf+_T("\r\n\r\nSave to file?"),APPNAME,MB_YESNO)==IDYES)
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

	if(!*ImgUrl) return false;
	return true;
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
		if(ErrorCode == ERROR_INTERNET_TIMEOUT)
		{
			Explication = TR("Превышен интервал ожидания запроса");
		}
		else if (ErrorCode ==ERROR_INTERNET_NAME_NOT_RESOLVED  )
			Explication= _T("Couldn't resolve host name. ");
		
		else if (ErrorCode ==ERROR_INTERNET_CONNECTION_ABORTED  )
			Explication= _T("The connection with the server has been terminated. ");
		
		else if(ErrorCode == ERROR_INTERNET_CONNECTION_RESET )
			Explication = _T("The connection with the server has been reset.");

		else if(ErrorCode == ERROR_INTERNET_CANNOT_CONNECT  )
			Explication = _T("The attempt to connect to the server failed.");
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
		else if(Action.Type == _T("login") && (EnginesList[CurrentServer].NeedAuthorization && !li.UseIeCookies))
			Status = TR("Авторизация на сервере...");
		else
		Status.Format(TR("Выполняю действие #%d..."), Action.Index+1);
	}
	SetStatusText(Status);

	UploadAction Current = Action;
	CString temp = Current.Url;
	 Current.Url = ReplaceVars(temp);
	
	if(EnginesList[CurrentServer].Debug && Action.Type != _T("post") && Action.Type != _T("upload"))
		MessageBox(GetActiveWindow(),  Status+_T("\r\nType:")+Action.Type+_T("\r\nURL: ")+Current.Url,_T("Debug"), MB_ICONINFORMATION);

	if(Action.Type == _T("upload"))
		Result = DoUploadAction(Current,true);
	else if(Action.Type == _T("post"))
		Result = DoUploadAction(Current, false);
	else if(Action.Type == _T("login"))
	{
		if(EnginesList[CurrentServer].NeedAuthorization && !li.UseIeCookies)
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

void CUploader::ConfigureProxy(CHttpClient &objHttpReq)
{
	 // Initialize the User Agent
	if(Settings.ConnectionSettings.UseProxy)
	{
		TCHAR Proxy[256];
		wsprintf(Proxy, _T("%s=%s:%d"), /*( Settings.ConnectionSettings.ProxyType?_T("socks"):_T("http")),*/(Settings.ConnectionSettings.ProxyType?_T("socks"):_T("http")),
		Settings.ConnectionSettings.ServerAddress, Settings.ConnectionSettings.ProxyPort);
		objHttpReq.SetInternet (_T ("Mozilla 4.0"),INTERNET_OPEN_TYPE_PROXY , Proxy, _T("localhost")) ;
		
	}
	else objHttpReq.SetInternet (_T ("Mozilla 4.0")) ;
	

	objHttpReq.SetUseUtf8 (FALSE) ;
	objHttpReq.SetAnsiCodePage (1251) ;
	
	/*objHttpReq.RemoveHeader(_T("Cache-Control") ,0);
	objHttpReq.RemoveHeader(_T("Accept") ,0);
	objHttpReq.AddHeader (_T ("Accept-Encoding"), _T("identity")) ;
	objHttpReq.AddHeader (_T ("Accept"), _T("text/html, *")) ;*/

	objHttpReq.AddHeader (_T ("Referer"), EnginesList[CurrentServer].Actions[0].Url) ;
	
	if(Settings.ConnectionSettings.UseProxy && Settings.ConnectionSettings.NeedsAuth && Settings.ConnectionSettings.ProxyType==0)
	{
		// Authorization on http(s) proxy (only basic)
		CHAR    szUser[128];
		TCHAR    szHeader[512];
		int    nLen = 128;
		TCHAR str[256];
		CHAR a_str[256];
		wsprintf(str, _T("%s:%s"),Settings.ConnectionSettings.ProxyUser, Settings.ConnectionSettings.ProxyPassword);
		::WideCharToMultiByte (CP_ACP	, 0, str, -1, a_str, 256, NULL, NULL);

		BOOL bRes = Base64Encode ((BYTE*)a_str,lstrlen(str), szUser, &nLen); 
		szUser[nLen]=0;
		wsprintf (szHeader, _T("Basic %S"), szUser);
		objHttpReq.AddHeader(_T("Proxy-Authorization"), szHeader);
	}

	HINTERNET hInternet = objHttpReq.OpenInternet();

	unsigned long SendTimeout = 1000000, RecTimeout = 1000000;
		BOOL bT = ::InternetSetOption(hInternet, INTERNET_OPTION_SEND_TIMEOUT,
	&SendTimeout, sizeof(unsigned long));
	bT = ::InternetSetOption(hInternet, INTERNET_OPTION_RECEIVE_TIMEOUT,
	&RecTimeout, sizeof(unsigned long));

	bT = ::InternetSetOption(hInternet, INTERNET_OPTION_CONNECT_TIMEOUT,
	&SendTimeout, sizeof(unsigned long));
	bT = ::InternetSetOption(hInternet, INTERNET_OPTION_DATA_RECEIVE_TIMEOUT,
	&RecTimeout, sizeof(unsigned long));
	bT = ::InternetSetOption(hInternet, INTERNET_OPTION_DATA_SEND_TIMEOUT,
	&SendTimeout, sizeof(unsigned long));
}

void CUploader::AddQueryPostParams(UploadAction &Action, CHttpClient &objHttpReq)
{

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
			objHttpReq.AddParam (NewName, m_FileName, CHttpClient::ParamFile) ;
			
		}
		else 
		{
			
			NewValue = ReplaceVars(NewValue);
			_Post+=NewName+_T(" = ")+NewValue+_T("\r\n");
			objHttpReq.AddParam (NewName, NewValue) ;
		}
	}

	if(EnginesList[CurrentServer].Debug)
		MessageBox(GetActiveWindow(), _Post, _T("Debug"),MB_ICONINFORMATION);

}

bool CUploader::ReadServerResponse(UploadAction &Action, CHttpResponse *pobjHttpRes)
{
	BYTE byBuff[cbBuff*40];
	bool Result = false;
	bool Exit = false;
	
	PBYTE buf = byBuff;
	if(pobjHttpRes)
	{
		int StatusCode = pobjHttpRes->GetStatus();
		if(StatusCode!=200)
		{
			UploadError(false, 0, 0, StatusCode, pobjHttpRes->GetStatusText());
			::InternetCloseHandle (pobjHttpRes->GetRequestHandle()) ;
			return false;
		}
	}

	*byBuff=0;
	DWORD           dwRead ;
	size_t          cbTotal = 0 ;

	while ( dwRead = pobjHttpRes->ReadContent (buf, cbBuff - 1) ) {
		cbTotal += dwRead ;
		if(*ShouldStop) return 0;
		buf[dwRead] = '\0' ;
		buf += dwRead;
	}

	// Если нас сервер куда-то перенаправляет
	LPTSTR Refresh;
	Refresh = (LPTSTR) pobjHttpRes->GetHeader(_T("Refresh"));
	if(lstrlen(Refresh))
	{			
		Refresh = MoveToEndOfW(Refresh,_T("url="));
		UploadAction Redirect = Action;
		Redirect.Url = Refresh;
		Redirect.Referer = Action.Url;
		Redirect.Type = _T("get");
		Result = DoGetAction(Redirect);
		Exit = true;
	}
	if(pobjHttpRes)
	{
		::InternetCloseHandle (pobjHttpRes->GetRequestHandle()) ;

	}

	if(!Exit)
	{
		LPSTR szData = (LPSTR)byBuff;
		if(!szData)
		{
			UploadError(false, 0, TR("Сервер вернул пустой ответ."));
			return FALSE;
		}
		LPTSTR  ReBuf=(LPTSTR)::VirtualAlloc(NULL, strlen(szData)*2+2, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

		MultiByteToWideChar(CP_ACP, 0,szData,strlen(szData),ReBuf,strlen(szData)*2+2);

		Result =  ParseAnswer(Action,ReBuf);
		if(!Result) 
			UploadError(false, 0, TR("Ответ сервера не содержит нужных данных."));

		VirtualFree(ReBuf, strlen(szData)*2+2, MEM_DECOMMIT);
	}
	return Result;
}

CString CUploader::GetStatusText()
{
	return m_StatusText;
}

void CUploader::SetStatusText(CString Text)
{
	 m_StatusText  = Text;
}

CString CUploader::getDownloadUrl()
{
	return m_DownloadUrl;
}