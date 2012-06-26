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

#include <math.h>
#include <curl/curl.h>
#include <iostream>
#include "Core/Upload/Uploader.h"
#include "Core/Utils/CoreUtils.h"
#include "Core/Network/NetworkManager.h"
#include "Core/UploadEngineList.h"
#include "Core/Upload/DefaultUploadEngine.h"
#include "Core/OutputCodeGenerator.h"
#include "Core/Upload/ScriptUploadEngine.h"
#include "Core/Utils/StringUtils.h"

#define IU_CLI_VER "0.1"
#ifdef _WIN32
std::string dataFolder = "Data/";
#else
std::string dataFolder = "/opt/imageuploader/";
#endif

std::vector<std::string> filesToUpload;
std::string serverName;
std::string login;
std::string password;
CUploadEngineList list;

ZOutputCodeGenerator::CodeType codeType = ZOutputCodeGenerator::ctClickableThumbnails;
ZOutputCodeGenerator::CodeLang codeLang = ZOutputCodeGenerator::clPlain;

bool UploadFile(CUploader &uploader, std::string fileName, /*[out]*/ ZUploadObject &uo)
{
   std::cerr<<" Uploading file '";
	#ifdef _WIN32
   std::wcerr<<IuCoreUtils::Utf8ToWstring(fileName);
	#else
	std::cerr<<IuCoreUtils::Utf8ToSystemLocale(fileName);
#endif
   std::cerr<<"' to server "<< uploader.getUploadEngine()->getUploadData()->Name<<std::endl;
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

void OnProgress(InfoProgress info)
{
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

void PrintUsage()
{
   std::cerr<<"USAGE:  "<<"imgupload [OPTIONS] filename1 filename2 ..."<<std::endl;
   std::cerr<<"Use '--help' option for more detailed information."<<std::endl;
}

void PrintHelp()
{
   std::cerr<<"Available options:"<<std::endl;
   std::cerr<<" --list   (print servers list)"<<std::endl;
   std::cerr<<" --server <server_name>"<<std::endl;
   std::cerr<<" --login <login>"<<std::endl;
   std::cerr<<" --password <pass>"<<std::endl;
   std::cerr<<" --codelang <bbcode|html|plain> (Default: plain)"<<std::endl;
   std::cerr<<" --codetype <TableOfThumbnails|ClickableThumbnails|Images|Links>"<<std::endl;
}

void PrintServerList()
{
   for(int i=0; i<list.count(); i++)
   {
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
         PrintUsage();
         PrintHelp();
         i++;
         continue;
      }
      else if(!IuStringUtils::stricmp(opt, "--server"))
      {
         if(i+1 == argc)
            return false;
         serverName = argv[++i];
         i++;
         continue;
      }
      else if(!IuStringUtils::stricmp(opt, "--list"))
      {
        PrintServerList();
         i++;
         continue;
      }
      else if(!IuStringUtils::stricmp(opt, "--codelang"))
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
      else if(!IuStringUtils::stricmp(opt, "--codetype"))
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
      else if(!IuStringUtils::stricmp(opt, "--login"))
      {
         if(i+1 == argc)
            return false;
         login = argv[++i];
         i++;
         continue;
      }
      else if(!IuStringUtils::stricmp(opt, "--password"))
      {
         if(i+1 == argc)
            return false;
         password = argv[++i];
         i++;
         continue;
      }
		
		 filesToUpload.push_back(IuCoreUtils::SystemLocaleToUtf8(argv[i]));
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
ServerSettingsStruct s;
CAbstractUploadEngine* getUploadEngineByData(CUploadEngineData * data)
{
	s.authData.Password = password;
	s.authData.Login = login;
	if(!login.empty())
	s.authData.DoAuth = true;

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
		std::cerr<<"Server '"<<uploadEngineData->Name<<"' requires authentication! Use --login and --password options."<<std::endl;
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
     CAbstractUploadEngine* engine = getUploadEngineByData(uploadEngineData);
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
  std::cerr<<std::endl<<"Result:"<<std::endl;
  std::cout<<generator.generate(uploadedList);
  std::cerr<<std::endl;
	return res;	
}

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "");
  
   if(IuCoreUtils::FileExists("Data/servers.xml"))
	{
		dataFolder = "Data/";
	}
   std::cerr<<"Zenden Image Uploader console utility v"<< IU_CLI_VER <<" (based on IU v1.2.7)"<<std::endl;
   if(! list.LoadFromFile(dataFolder + "servers.xml"))
   {
      std::cerr<<"Cannot load server list!";
   }

   if(!parseCommandLine(argc, argv))
   {
      return 0;
   }
   if(!filesToUpload.size())
         return 0;
   CScriptUploadEngine::InitScriptEngine();

  int res = func();
   delete lastEngine;
	CScriptUploadEngine::DestroyScriptEngine();

  return res;
}
