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

#include <math.h>
#include <curl/curl.h>
#include <iostream>
#include <fstream>
#include "Core/Upload/Uploader.h"
#include "Core/Utils/CoreUtils.h"
#include "Core/Network/NetworkClient.h"
#include "Core/UploadEngineList.h"
#include "Core/Upload/DefaultUploadEngine.h"
#include "Core/OutputCodeGenerator.h"
#include "Core/Upload/ScriptUploadEngine.h"
#include "Core/Utils/StringUtils.h"
#include <Core/AppParams.h>
#include "Func/Settings.h"
#include <Core/Logging.h>
#ifdef _WIN32
#include "Func/UpdatePackage.h"
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <Func/IuCommonFunctions.h>
#else
#include <sys/stat.h>
#endif
#include "versioninfo.h"
#ifndef _WIN32
#include <sys/time.h>
#endif
#define IU_CLI_VER "0.2.5"

#ifdef _WIN32
CAppModule _Module;
std::string dataFolder = "Data/";
#else
std::string dataFolder = "/usr/share/imageuploader/";
#endif

std::vector<std::string> filesToUpload;
std::string serverName;
std::string login;
std::string password;
std::string folderId;
std::string proxy;
int  proxyPort;
int proxyType; /* CURLPROXY_HTTP, CURLPROXY_SOCKS4, CURLPROXY_SOCKS4A, 
			   CURLPROXY_SOCKS5, CURLPROXY_SOCKS5_HOSTNAME */
std::string proxyUser;
std::string proxyPassword;

CUploadEngineList list;


ZOutputCodeGenerator::CodeType codeType = ZOutputCodeGenerator::ctClickableThumbnails;
ZOutputCodeGenerator::CodeLang codeLang = ZOutputCodeGenerator::clPlain;
bool autoUpdate = true;



#ifdef _WIN32
void DoUpdates(bool force = false);
#endif
void DebugMessage2(const std::string& message, bool isServerResponseBody)
{   
#ifdef _WIN32
	std::wcerr<<IuCoreUtils::Utf8ToWstring(message);
#else
std::cerr<<IuCoreUtils::Utf8ToSystemLocale(message);
#endif
}

bool UploadFile(CUploader &uploader, std::string fileName, /*[out]*/ ZUploadObject &uo)
{
   std::cerr<<" Uploading file '";
	#ifdef _WIN32
   std::wcerr<<IuCoreUtils::Utf8ToWstring(fileName);
	#else
	std::cerr<<IuCoreUtils::Utf8ToSystemLocale(fileName);
#endif
   std::cerr<<"' to server "<< uploader.getUploadEngine()->getUploadData()->Name<<std::endl;
   uploader.onDebugMessage.bind(&DebugMessage2);
   //return false;
   if(!uploader.UploadFile(fileName, IuCoreUtils::ExtractFileName(fileName)))
   {
      std::cerr<<"Unable to upload file!"<<std::endl;
      return false;
   }
   uo.displayFileName = IuCoreUtils::ExtractFileName(fileName);
   uo.directUrl = uploader.getDirectUrl();
   uo.thumbUrl = uploader.getThumbUrl();
   uo.viewUrl = uploader.getDownloadUrl();
   return true;
}

void destr()
{

}


void IU_ConfigureProxy(NetworkClient& nm)
{
	if ( !proxy.empty())
	{
		
		nm.setProxy(proxy,proxyPort, proxyType);

		if( !proxyUser.empty()) {
			nm.setProxyUserPassword(proxyUser,proxyPassword);
		}
	}
	nm.setUploadBufferSize(Settings.UploadBufferSize);
}

void OnConfigureNM(NetworkClient* nm) {
	IU_ConfigureProxy(*nm);
}

#ifdef _WIN32
/* FILETIME of Jan 1 1970 00:00:00. */
static const unsigned __int64 epoch = ((unsigned __int64) 116444736000000000ULL);

/*
 * PostgreSQL's implementation of gettimeofday for windows:
 *
 * timezone information is stored outside the kernel so tzp isn't used anymore.
 *
 * Note: this function is not for Win32 high precision timing purpose. See
 * elapsed_time().
 */
int
gettimeofday(struct timeval * tp, struct timezone * tzp)
{
    FILETIME    file_time;
    SYSTEMTIME  system_time;
    ULARGE_INTEGER ularge;

    GetSystemTime(&system_time);
    SystemTimeToFileTime(&system_time, &file_time);
    ularge.LowPart = file_time.dwLowDateTime;
    ularge.HighPart = file_time.dwHighDateTime;

    tp->tv_sec = (long) ((ularge.QuadPart - epoch) / 10000000L);
    tp->tv_usec = (long) (system_time.wMilliseconds * 1000);

    return 0;
}
#endif

int64_t lastProgressTime = 0;
void OnProgress(CUploader* uploader, InfoProgress info)
{
    struct timeval tp;
    gettimeofday(&tp, NULL);
    int64_t ms = tp.tv_sec * 1000 + tp.tv_usec / 1000;

    if ( info.Total != info.Uploaded && ms - lastProgressTime < 500 ) { // Windows console output is too slow
        return;
    }
    lastProgressTime = ms;
    int totaldotz=40;
    if(info.Total == 0)
       return;
       double fractiondownloaded = static_cast<double>(info.Uploaded) / info.Total;
       if(fractiondownloaded > 100)
          fractiondownloaded = 0;
       // part of the progressmeter that's already "full"
       int dotz = floor(fractiondownloaded * totaldotz);

       // create the "meter"
       int ii=0;
       fprintf(stderr, "%3.0f%% [",fractiondownloaded*100);
       // part  that's full already
       for ( ; ii < dotz;ii++) {
           fprintf(stderr,"=");
       }
       // remaining part (spaces)
       for ( ; ii < totaldotz;ii++) {
           fprintf(stderr," ");
       }
       // and back to line begin - do not forget the fflush to avoid output buffering problems!
       fprintf(stderr,"]");
       fprintf(stderr," %s/%s            ", IuCoreUtils::fileSizeToString(info.Uploaded).c_str(), IuCoreUtils::fileSizeToString(info.Total).c_str());
       fprintf(stderr,"\r");
       fflush(stderr);
}

void OnError(ErrorInfo errorInfo)
{
   /*std::cerr<<"---------------------"<<std::endl;
   if(ei.errorType == etUserError)
	{
      std::cerr<<"Error: "<<ei.error<<std::endl;
	}
   else std::cerr<<"Unknown error!"<<std::endl;
   std::cerr<<"---------------------"<<std::endl;*/
	
	std::string errorMsg;

	std::string infoText;
	if(!errorInfo.FileName.empty())
		infoText += "File: " + errorInfo.FileName+ "\n";

	if(!errorInfo.ServerName.empty())
	{
		std::string serverName = errorInfo.ServerName;
		if(!errorInfo.sender.empty())
		serverName+= "("+errorInfo.sender+")";
		infoText += "Server: " +serverName +  "\n";
	}

	if(!errorInfo.Url.empty())
		infoText += "URL: " + errorInfo.Url+ "\n";


	if(errorInfo.ActionIndex != -1)
		infoText += "Action: #" + IuCoreUtils::toString(errorInfo.ActionIndex);


	if(!errorInfo.error.empty())
	{
		errorMsg += errorInfo.error;
	
	}else
	{
		if(errorInfo.errorType == etRepeating)

		{
			char buf[256];
			sprintf(buf, "Upload failed. Another retry (%d)", errorInfo.RetryIndex);
			errorMsg = buf;
		}
		else if (errorInfo.errorType == etRetriesLimitReached)
		{
			errorMsg = "Upload failed! (retry limit reached)";
		}
	}

	std::cerr<<infoText<<std::endl;
	std::cerr<<errorMsg<<std::endl;
	std::cerr<<"---------------------"<<std::endl;
		
}

void PrintUsage(bool help = false)
{
   std::cerr<<"USAGE:  "<<"imgupload [OPTIONS] filename1 filename2 ..."<<std::endl;
   if ( !help ) {
	std::cerr<<"Use '--help' option for more detailed information."<<std::endl;
   }
}

void PrintHelp()
{
   std::cerr<<"\r\nAvailable options:"<<std::endl;
   std::cerr<<" -l   Prints server list"<<std::endl;
   std::cerr<<" -s <server_name>"<<std::endl;
   std::cerr<<" -u <username>"<<std::endl;
   std::cerr<<" -p <password>"<<std::endl;
   std::cerr<<" -cl <bbcode|html|plain> (Default: plain)"<<std::endl;
   std::cerr<<" -ct <TableOfThumbnails|ClickableThumbnails|Images|Links>"<<std::endl;
   std::cerr<<" -fl <folder_id> ID of remote folder (supported by some servers)\r\n"
	   << "     Note that this is not the folder\'s name! \r\n"
	   << "     How to obtain it: open Image Uploader GUI version's\r\n"
	   << "     configuration file 'settings.xml' in text editor, \r\n"
	   << "     find your server under 'ServersParams' node,\r\n"
	   << "     and copy value of the '_FolderID' attribute"<<
	   std::endl;
   std::cerr<<" -pr <x.x.x.x:xxxx> Proxy address "<<std::endl;
   std::cerr<<" -pt <http|socks4|socks4a|socks5|socks5dns> Proxy type  (default http)"<<std::endl;
   std::cerr<<" -pu <username> Proxy username"<<std::endl;
   std::cerr<<" -pp <password> Proxy password"<<std::endl;
#ifdef _WIN32
    std::cerr<<" --disable-update Disable auto-updating servers.xml"<<std::endl;
	std::cerr<<std::endl<<" -up   Update servers.xml\r\n"
		<<"     the 'Data' directory must be writable, otherwise update will fail"
		<<std::endl;
#endif
}

void PrintServerList()
{
	for(int i=0; i<list.count(); i++) {
   
	   if ( list.byIndex(i)->Type == CUploadEngineData::TypeUrlShorteningServer ) {
		   continue;
	   }
      std::cout<<list.byIndex(i)->Name<<std::endl;
   }
}


bool parseCommandLine(int argc, char *argv[])
{
   if(argc == 1)
   {
      PrintUsage();
      return false;
   }
   int i = 1;
   while(i < argc)
   {
      char *opt = argv[i];
      if(!IuStringUtils::stricmp(opt, "--help"))
      {
         PrintUsage(true);
         PrintHelp();
         i++;
         continue;
      }
      else if(!IuStringUtils::stricmp(opt, "-s"))
      {
         if(i+1 == argc)
            return false;
         serverName = argv[++i];
         i++;
         continue;
      }
      else if(!IuStringUtils::stricmp(opt, "-l"))
      {
        PrintServerList();
         i++;
         continue;
      }
      else if(!IuStringUtils::stricmp(opt, "-cl"))
      {
         if(i+1 == argc)
            return false;
         char * codelang = argv[++i];
         if(!IuStringUtils::stricmp(codelang, "plain"))
            codeLang =  ZOutputCodeGenerator::clPlain;
         else if(!IuStringUtils::stricmp(codelang, "html"))
           codeLang =  ZOutputCodeGenerator::clHTML;
         else if(!IuStringUtils::stricmp(codelang, "bbcode"))
            codeLang =  ZOutputCodeGenerator::clBBCode;
         i++;
         continue;
      }
      else if(!IuStringUtils::stricmp(opt, "-ct"))
      {
         if(i+1 == argc)
            return false;
         char * codetype = argv[++i];
         if(!IuStringUtils::stricmp(codetype, "TableOfThumbnails"))
            codeType =  ZOutputCodeGenerator::ctTableOfThumbnails;
         else if(!IuStringUtils::stricmp(codetype, "ClickableThumbnails"))
           codeType =  ZOutputCodeGenerator::ctClickableThumbnails;
         else if(!IuStringUtils::stricmp(codetype, "Images"))
            codeType =  ZOutputCodeGenerator::ctImages;
         else if(!IuStringUtils::stricmp(codetype, "Links"))
            codeType =  ZOutputCodeGenerator::ctLinks;
         i++;
         continue;
      }
      else if(!IuStringUtils::stricmp(opt, "-u"))
      {
         if(i+1 == argc)
            return false;
         login = argv[++i];
         i++;
         continue;
      }
      else if(!IuStringUtils::stricmp(opt, "-p"))
      {
         if(i+1 == argc)
            return false;
         password = argv[++i];
         i++;
         continue;
	  }
	  else if(!IuStringUtils::stricmp(opt, "-fl"))
	  {
		  if(i+1 == argc)
			  return false;
		  folderId = argv[++i];
		  i++;
		  continue;
	  }
	  else if(!IuStringUtils::stricmp(opt, "-pr"))
	  {
		  if(i+1 == argc)
			  return false;
		  proxy = argv[++i];
		  std::vector<std::string> tokens;
		  IuStringUtils::Split(proxy,":",tokens,2);
		  if ( tokens.size() > 1) {
			proxy = tokens[0];
			proxyPort = IuCoreUtils::stringToint64_t(tokens[1]);
		  }
		  i++;
		  continue;
	  }
	  else if(!IuStringUtils::stricmp(opt, "-pu"))
	  {
		  if(i+1 == argc)
			  return false;
		  proxyUser = argv[++i];
		  
		  i++;
		  continue;
	  }
	  else if(!IuStringUtils::stricmp(opt, "-pp"))
	  {
		  if(i+1 == argc)
			  return false;
		  proxyPassword = argv[++i];

		  i++;
		  continue;
	  }
	  else if(!IuStringUtils::stricmp(opt, "-pt"))
	  {
		  if(i+1 == argc)
			  return false;
		  std::map<std::string, int> types;
		  std::string type = argv[++i];
		  types["http"] = CURLPROXY_HTTP;
		  types["socks4"] = CURLPROXY_SOCKS4;
		  types["socks4a"] = CURLPROXY_SOCKS4A;
		  types["socks5"] = CURLPROXY_SOCKS5;
		  types["socks5dns"] = CURLPROXY_SOCKS5_HOSTNAME;
		  std::map<std::string, int>::const_iterator it = types.find(type);
		  if ( it != types.end() ) {
			proxyType = it->second; 
		  } else {
			  std::cerr<<"Invalid proxy type"<<std::endl;
		  }

		  i++;
		  continue;
	  }
	  #ifdef _WIN32
	  else if(!IuStringUtils::stricmp(opt, "-up"))
	  {
		 DoUpdates(true);
		  i++;
		  return 0;
	  }
	  else if(!IuStringUtils::stricmp(opt, "--disable-update"))
	  {
		  autoUpdate = false;
			  i++;
		           continue;
	  }

#endif
#ifndef _WIN32	
		filesToUpload.push_back(IuCoreUtils::SystemLocaleToUtf8(argv[i]));
#else
	filesToUpload.push_back(argv[i]);
#endif
      //else if()
      i++;
   }

   return true;
}

CUploadEngineData* getServerByName(std::string name)
{
   CUploadEngineData*   uploadEngineData = list.byName(serverName);
   if(!uploadEngineData)
   {
      for(int i=0; i<list.count(); i++)
      {
         if((IuStringUtils::toLower(list.byIndex(i)->Name).find(IuStringUtils::toLower((name)))) != -1)
            return list.byIndex(i);
      }
   }
   return uploadEngineData;
}

CAbstractUploadEngine *lastEngine = 0;
//ServerSettingsStruct s;
CAbstractUploadEngine* getUploadEngineByData(CUploadEngineData * data,std::string login)
{
	ServerSettingsStruct& s = Settings.ServersSettings[data->Name][login];
	
	//printf("Login: %s", login.c_str());
	s.authData.Password = password;
	s.authData.Login = login;
	if(!login.empty())
	s.authData.DoAuth = true;

	s.setParam("FolderID", folderId);

      if(lastEngine && lastEngine->getUploadData() == data)
      {
         return lastEngine;
      }
      delete lastEngine;
   if(data->PluginName.empty())
   {
      lastEngine = new CDefaultUploadEngine();
	lastEngine->setServerSettings(s);
   }
   else
   {
      CScriptUploadEngine * eng = new CScriptUploadEngine(data->Name);

      
      if(!eng->load(dataFolder + "Scripts/"+data->PluginName+".nut", s))
      {
         std::cout<<"Cannot load script!"<<std::endl;
         return 0;
      }
      lastEngine = eng;
   }
   return lastEngine;
}

int func()
{
	int res = 0;
   CUploader uploader;

   uploader.onProgress.bind(OnProgress);
   uploader.onConfigureNetworkClient.bind(OnConfigureNM);
   uploader.onErrorMessage.bind(OnError);
  CUploadEngineData* uploadEngineData = 0;
  if(!serverName.empty())
   {
      uploadEngineData = getServerByName(serverName);
     if(!uploadEngineData)
      {
        std::cerr<<"No such server '"<<serverName<<"'!"<<std::endl;
        return 0;
     }
  }
  else
  {
     int index = list.getRandomImageServer();
     uploadEngineData = list.byIndex(index);
  }

	if(uploadEngineData->NeedAuthorization == 2 && login.empty())
	{
		std::cerr<<"Server '"<<uploadEngineData->Name<<"' requires authentication! Use -u and -p options."<<std::endl;
		return -1;
	}

  std::vector<ZUploadObject> uploadedList;
  for(size_t i=0; i<filesToUpload.size(); i++)
  {
		if(!IuCoreUtils::FileExists(filesToUpload[i]))
		{
				std::cerr<<"File '"+ filesToUpload[i] + "' doesn't exist!"<<std::endl;	
				res ++;
				continue;				
		}
     CAbstractUploadEngine* engine = getUploadEngineByData(uploadEngineData,login);
     if(!engine) return false;
     engine->setUploadData(uploadEngineData);
     uploader.setUploadEngine(engine);

     ZUploadObject uo;
     UploadFile(uploader, filesToUpload[i], uo);
     uploadedList.push_back(uo);
  }

  ZOutputCodeGenerator generator;
  generator.setLang(codeLang);
  generator.setType(codeType);
  if ( !uploadedList.empty() ) {
	  std::cerr<<std::endl<<"Result:"<<std::endl;
	  std::cout<<generator.generate(uploadedList);
	  std::cerr<<std::endl;
  }
	return res;	
}

#ifdef _WIN32


class Updater: public CUpdateStatusCallback {
	public:

		Updater() {
			m_UpdateManager.setUpdateStatusCallback(this);
		}
	void updateServers() {
		
		
		std::cout<<"Checking for updates..."<<std::endl;
		if (!m_UpdateManager.CheckUpdates()) {
			std::cout<<"Error while updating"<<std::endl;
			return;
		}

		Settings.LastUpdateTime = static_cast<int>(time(0));
		if (m_UpdateManager.AreUpdatesAvailable())
		{
			for (size_t i = 0; i < m_UpdateManager.m_updateList.size(); i++)
			{
				std::cerr<<"Beginning to update: "<< IuCoreUtils::WstringToUtf8((LPCTSTR)m_UpdateManager.m_updateList[i].displayName())<<std::endl;
			}

			m_UpdateManager.DoUpdates();
			if (m_UpdateManager.successPackageUpdatesCount())
			{
				std::cerr<<"Succesfully updated!";
			}
		} 
		else
		{
			
			std::cerr<<"All is up-to-date"<<std::endl;
		}

	}
	virtual void updateStatus(int packageIndex, const CString& status) {	
		//std::wcout << (LPCTSTR)m_UpdateManager.m_updateList[packageIndex].displayName() <<" : "<< (LPCTSTR)status<<std::endl;
		fprintf(stderr, "%s : %s", IuCoreUtils::WstringToUtf8((LPCTSTR)m_UpdateManager.m_updateList[packageIndex].displayName()).c_str(), IuCoreUtils::WstringToUtf8((LPCTSTR)status).c_str());
		fprintf(stderr, "\r");
		fflush(stderr);
		//std::cout<< "\r"<<IuCoreUtils::WstringToUtf8((LPCTSTR)m_UpdateManager.m_updateList[packageIndex].displayName()) <<" : "<< IuCoreUtils::WstringToUtf8((LPCTSTR)status);
	}
protected:
	CUpdateManager m_UpdateManager;
};


void DoUpdates(bool force) {
	if(force || time(0) - Settings.LastUpdateTime > 3600*24*7 /* 7 days */) {
		IuCommonFunctions::CreateTempFolder();
		Updater upd;
		upd.updateServers();
		IuCommonFunctions::ClearTempFolder(IuCommonFunctions::IUTempFolder);
		Settings.LastUpdateTime = time(0);
	}

}


char ** convertArgv(int argc, _TCHAR* argvW[]) {
	char ** result = new char *[argc];
	for ( int i = 0; i < argc; i++) {
		std::string unicodeString = IuCoreUtils::WstringToUtf8(argvW[i]).c_str();
		char *buffer = new char[unicodeString.length()+1];
		strcpy(buffer, unicodeString.c_str());
		result[i] = buffer;
	}
	return result;
}

int _tmain(int argc, _TCHAR* argvW[]) {	
	char **argv = convertArgv(argc, argvW);
	FLAGS_logtostderr = true;
	//google::SetLogDestination(google::GLOG_INFO,"d:/" );

	google::InitGoogleLogging(argv[0]);

	/*UINT oldcp = GetConsoleOutputCP();
	SetConsoleOutputCP(CP_UTF8);*/
	//_setmode(_fileno(stdout), _O_U16TEXT);
#else
int main(int argc, char *argv[]){
#endif

	int res  = 0;
	std::string appDirectory = IuCoreUtils::ExtractFilePath(argv[0]);
    std::string settingsFolder;
    setlocale(LC_ALL, "");

   if(IuCoreUtils::FileExists(appDirectory + "/Data/servers.xml"))
	{
		dataFolder = appDirectory+"/Data/";
        settingsFolder = dataFolder;
    }
#ifndef _WIN32
   else {
dataFolder = "/usr/share/imgupload/";
   }
#ifndef __APPLE__
settingsFolder = getenv("HOME")+std::string("/.config/imgupload/");
mkdir(settingsFolder.c_str(), 0700);
#endif

#endif
	AppParams* params = AppParams::instance();
	params->setDataDirectory(dataFolder);
	params->setSettingsDirectory(settingsFolder);

   std::cerr<<"Zenden Image Uploader console utility v"<< IU_CLI_VER <<" (based on IU v"<<_APP_VER<<" build "<<BUILD<<")"<<std::endl;
   if(! list.LoadFromFile(dataFolder + "servers.xml", Settings.ServersSettings))
   {
	   std::cerr<<"Cannot load server list!"<<std::endl;
   }

   if( IuCoreUtils::FileExists(dataFolder + "userservers.xml") && !list.LoadFromFile(dataFolder + "userservers.xml", Settings.ServersSettings))
   {
	   std::cerr<<"Cannot load server list userservers.xml!"<<std::endl;
   }
    Settings.LoadSettings(settingsFolder,"settings_cli.xml");

   
   if(!parseCommandLine(argc, argv))
   {
      return 0;
   }
   if(!filesToUpload.size())
         return 0;

#ifdef _WIN32
   if (autoUpdate) {
	   DoUpdates();
   }
#endif
   CScriptUploadEngine::InitScriptEngine();

  res = func();

  if ( !Settings.SaveSettings() ) {
	std::cerr<<"Cannot save settings!"<<std::endl;
  }

   delete lastEngine;
	CScriptUploadEngine::DestroyScriptEngine();
#ifdef _WIN32
//SetConsoleOutputCP(oldcp);
#endif
  return res;
}
