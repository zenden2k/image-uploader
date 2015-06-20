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
#include <signal.h>
#include <condition_variable>
#include "Core/Upload/Uploader.h"
#include "Core/Utils/CoreUtils.h"
#include "Core/Network/NetworkClient.h"
#include "Core/UploadEngineList.h"
#include "Core/Upload/UploadManager.h"
#include "Core/Upload/DefaultUploadEngine.h"
#include "Core/Upload/FileUploadTask.h"
#include "Core/Upload/UploadSession.h"
#include "Core/Upload/ConsoleUploadErrorHandler.h"
#include "Core/Upload/UploadEngineManager.h"
#include "Core/OutputCodeGenerator.h"
#include "Core/Upload/ScriptUploadEngine.h"
#include "Core/ServiceLocator.h"
#include "Core/Utils/StringUtils.h"
#include "Core/AppParams.h"
#include "Core/Settings.h"
#include "Core/Logging.h"
#include "Core/Logging/MyLogSink.h"
#include "Core/Logging/ConsoleLogger.h"
#include "ConsoleScriptDialogProvider.h"
#include "ConsoleUtils.h"
#ifdef _WIN32
#include <windows.h>
#include "Func/UpdatePackage.h"
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include "Func/IuCommonFunctions.h"
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
int proxyPort;
int proxyType; /* CURLPROXY_HTTP, CURLPROXY_SOCKS4, CURLPROXY_SOCKS4A, 
			   CURLPROXY_SOCKS5, CURLPROXY_SOCKS5_HOSTNAME */
std::string proxyUser;
std::string proxyPassword;

CUploadEngineList list;
std::unique_ptr<UploadEngineManager> uploadEngineManager;
std::unique_ptr<UploadManager> uploadManager;

ZOutputCodeGenerator::CodeType codeType = ZOutputCodeGenerator::ctClickableThumbnails;
ZOutputCodeGenerator::CodeLang codeLang = ZOutputCodeGenerator::clPlain;
bool autoUpdate = true;
std::shared_ptr<UploadSession> session;

std::mutex finishSignalMutex;
std::condition_variable finishSignal;
bool finished = false;

std::mutex progressMutex;

struct TaskUserData {
    int index;
};

void SignalHandler (int param)
{
    if ( session ) {
        session->stop();
    }
}

void PrintWelcomeMessage() {
    std::cerr<<"imgupload v"<< IU_CLI_VER <<" (based on IU v"<<_APP_VER<<" build "<<BUILD<<")"<<std::endl;
}


class Translator: public ITranslator{
public:
    virtual std::string getCurrentLanguage() override {
        return "English";
    }
    virtual std::string getCurrentLocale() override {
        return "en_US";
    }

};
Translator translator; // dummy translator

#ifdef _WIN32
void DoUpdates(bool force = false);
#endif

int64_t lastProgressTime = 0;

void PrintUsage(bool help = false) {
   std::cerr<<"USAGE:  "<<"imgupload [OPTIONS] filename1 filename2 ..."<<std::endl;
   if ( !help ) {
	std::cerr<<"Use '--help' option for more detailed information."<<std::endl;
   }
}

void PrintHelp() {
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
       if ( !list.byIndex(i)->hasType(CUploadEngineData::TypeImageServer) && !list.byIndex(i)->hasType(CUploadEngineData::TypeFileServer) ) {
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
            proxyPort = IuCoreUtils::stringToInt64(tokens[1]);
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

    std::string fileName =
#ifndef _WIN32
    IuCoreUtils::SystemLocaleToUtf8(argv[i]);
#else
    argv[i];
#endif
    if ( !IuCoreUtils::FileExists(fileName) ) {
        std::cerr << "File '" + fileName + "' doesn't exist!" << std::endl;
    }
    filesToUpload.push_back(fileName);
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

void UploadSessionFinished(UploadSession* session) {
    int taskCount = session->taskCount();
    std::vector<ZUploadObject> uploadedList;
    for ( int i = 0; i < taskCount; i++ ) {
        auto task = session->getTask(i);
        UploadResult* res = task->uploadResult();
        if ( task->uploadSuccess() ) {
            ZUploadObject uo;
            uo.directUrl = res->directUrl;
            uo.thumbUrl = res->thumbUrl;
            uo.viewUrl = res->downloadUrl;
            uo.serverName = task->serverName();
            //uo.localFilePath = task->
            uploadedList.push_back(uo);
        }
    }
    ZOutputCodeGenerator generator;
    generator.setLang(codeLang);
    generator.setType(codeType);
    ConsoleUtils::SetCursorPos(0, taskCount + 2);
    if ( !uploadedList.empty() ) {
        std::cerr<<std::endl<<"Result:"<<std::endl;
        std::cout<<generator.generate(uploadedList);
        std::cerr<<std::endl;
    }
    {
        std::lock_guard<std::mutex> lk(finishSignalMutex);
        finished = true;
    }

    finishSignal.notify_one();
}

void UploadTaskProgress(UploadTask* task) {
    std::lock_guard<std::mutex> guard(progressMutex);
    UploadProgress* progress = task->progress();
    TaskUserData *userData = reinterpret_cast<TaskUserData*>(task->userData());

    struct timeval tp;
    gettimeofday(&tp, NULL);
    int64_t ms = tp.tv_sec * 1000 + tp.tv_usec / 1000;

    if ( progress->totalUpload != progress->uploaded && ms - lastProgressTime < 500 ) { // Windows console output is too slow
        return;
    }
    lastProgressTime = ms;
    int totaldotz=25;
    if(progress->totalUpload == 0)
       return;

    ConsoleUtils::SetCursorPos(0, 2 + userData->index);
    double fractiondownloaded = static_cast<double>(progress->uploaded) / progress->totalUpload;
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
       fprintf(stderr," %s/%s", IuCoreUtils::fileSizeToString(progress->uploaded).c_str(),
               IuCoreUtils::fileSizeToString(progress->totalUpload).c_str());
       //fprintf(stderr,"\r");
       fflush(stderr);
}

void OnUploadTaskStatusChanged(UploadTask* task) {
    std::lock_guard<std::mutex> guard(progressMutex);
    UploadProgress* progress = task->progress();
    TaskUserData *userData = reinterpret_cast<TaskUserData*>(task->userData());
    ConsoleUtils::SetCursorPos(55, 2 + userData->index);
    fprintf(stderr, progress->statusText.c_str());
}

int func() {
	int res = 0;
    ConsoleLogger defaultLogger;
    ConsoleUploadErrorHandler uploadErrorHandler;

    ServiceLocator* serviceLocator = ServiceLocator::instance();
    serviceLocator->setUploadErrorHandler(&uploadErrorHandler);
    serviceLocator->setLogger(&defaultLogger);
    MyLogSink logSink(&defaultLogger);
    google::AddLogSink(&logSink);
    ConsoleScriptDialogProvider dialogProvider;
    serviceLocator->setDialogProvider(&dialogProvider);
    serviceLocator->setTranslator(&translator);

    Settings.setEngineList(&list);
    ServiceLocator::instance()->setEngineList(&list);
    ScriptsManager scriptsManager;
    uploadEngineManager.reset( new UploadEngineManager(&list, &uploadErrorHandler));
    uploadEngineManager->setScriptsDirectory(IuCoreUtils::WstringToUtf8((LPCTSTR)(IuCommonFunctions::GetDataFolder() + _T("\\Scripts\\"))));
    uploadManager.reset( new UploadManager(uploadEngineManager.get(), &list, &scriptsManager, &uploadErrorHandler));

    if ( !proxy.empty()) {
        Settings.ConnectionSettings.UseProxy = true;
        Settings.ConnectionSettings.ServerAddress= proxy;
        Settings.ConnectionSettings.ProxyPort = proxyPort;

        if( !proxyUser.empty()) {
            Settings.ConnectionSettings.NeedsAuth = true;
            Settings.ConnectionSettings.ProxyUser = proxyUser;
            Settings.ConnectionSettings.ProxyPassword = proxyPassword;
        }
    }

    CUploadEngineData* uploadEngineData = 0;
    if(!serverName.empty()) {
        uploadEngineData = getServerByName(serverName);
        if(!uploadEngineData) {
            std::cerr<<"No such server '"<<serverName<<"'!"<<std::endl;
            return 0;
        }
    } else {
        int index = list.getRandomImageServer();
        uploadEngineData = list.byIndex(index);
    }

	if(uploadEngineData->NeedAuthorization == 2 && login.empty())
	{
		std::cerr<<"Server '"<<uploadEngineData->Name<<"' requires authentication! Use -u and -p options."<<std::endl;
		return -1;
	}

    ServerProfile serverProfile(uploadEngineData->Name);
    serverProfile.setProfileName(login);
    serverProfile.setShortenLinks(false);

    ServerSettingsStruct& s = Settings.ServersSettings[uploadEngineData->Name][login];

    //printf("Login: %s", login.c_str());
    s.authData.Password = password;
    s.authData.Login = login;
    if(!login.empty())
    s.authData.DoAuth = true;

    s.setParam("FolderID", folderId);
    serverProfile.setFolderId(folderId);

    //std::vector<ZUploadObject> uploadedList;
    session.reset(new UploadSession);
    for(size_t i=0; i<filesToUpload.size(); i++) {
        if(!IuCoreUtils::FileExists(filesToUpload[i]))
        {
            std::cerr<<"File '"+ filesToUpload[i] + "' doesn't exist!"<<std::endl;
            res++;
            continue;
        }

        std::shared_ptr<FileUploadTask> task(new FileUploadTask(filesToUpload[i], IuCoreUtils::ExtractFileName(filesToUpload[i])));
        task->setServerProfile(serverProfile);
        task->OnUploadProgress.bind(UploadTaskProgress);
        task->OnStatusChanged.bind(OnUploadTaskStatusChanged);
        TaskUserData *userData = new TaskUserData;
        userData->index = i;
        task->setUserData(userData);
        session->addTask(task);
    }
    session->addSessionFinishedCallback(UploadSession::SessionFinishedCallback(UploadSessionFinished));

    ConsoleUtils::Clear();
    PrintWelcomeMessage();
    uploadManager->addSession(session);
    uploadManager->start();

    // Wait until upload session is finished
    std::unique_lock<std::mutex> lk(finishSignalMutex);
    finishSignal.wait(lk, [] {return finished;});
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
    virtual void updateStatus(int packageIndex, const CString& status) override {
        //std::wcout << (LPCTSTR)m_UpdateManager.m_updateList[packageIndex].displayName() <<" : "<< (LPCTSTR)status<<std::endl;
        if ( m_UpdateManager.m_updateList.size() > packageIndex+1) {
            fprintf(stderr, "%s : %s", IuCoreUtils::WstringToUtf8((LPCTSTR)m_UpdateManager.m_updateList[packageIndex].displayName()).c_str(), IuCoreUtils::WstringToUtf8((LPCTSTR)status).c_str());
            fprintf(stderr, "\r");
            fflush(stderr);
        }
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

// Convert UNICODE (UCS-2) command line arguments to utf-8
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
    signal(SIGINT, SignalHandler);
#ifdef _WIN32
    //SetConsoleCtrlHandler ();
    //SetConsoleTitle(_T("imgupload");
#endif

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
	
#ifdef _WIN32
    TCHAR ShortPath[1024];
    GetTempPath(ARRAY_SIZE(ShortPath), ShortPath);
    TCHAR TempPath[1024];
    if (!GetLongPathName(ShortPath,TempPath, ARRAY_SIZE(TempPath)) ) {
        lstrcpy(TempPath, ShortPath);
    }
    params->setTempDirectory(IuCoreUtils::WstringToUtf8(TempPath));
#else
    return  params->setTempDirectory("/var/tmp/");
#endif
    PrintWelcomeMessage();
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

   res = func();

   if ( !Settings.SaveSettings() ) {
       std::cerr<<"Cannot save settings!"<<std::endl;
   }

	CScriptUploadEngine::DestroyScriptEngine();
#ifdef _WIN32
//SetConsoleOutputCP(oldcp);
#endif
  return res;
}
