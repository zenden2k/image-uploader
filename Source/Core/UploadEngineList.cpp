/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2018 Sergey Svistunov (zenden2k@yandex.ru)

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

#include "UploadEngineList.h"

#include <algorithm>
#include <vector>
#include "Core/Utils/SimpleXml.h"
#include "Core/Utils/StringUtils.h"
#include "Core/Logging.h"
#include "AppParams.h"

CUploadEngineList::CUploadEngineList()
{
    m_EngineNumOfRetries = 3;
    m_ActionNumOfRetries = 2;
}

bool CUploadEngineList::LoadFromFile(const std::string& filename, ServerSettingsMap& serversSettings)
{
    SimpleXml xml;
    if(!xml.LoadFromFile(filename))
        return false;
        
    SimpleXmlNode root = xml.getRoot("ServerListTool");

    std::vector<SimpleXmlNode> childs;
    root.GetChilds("Server", childs);
    root.GetChilds("Server2", childs);
    root.GetChilds("Server3", childs);

    auto versionInfo = AppParams::instance()->GetAppVersion();

    std::string ver = versionInfo->FullVersion;
    std::vector<std::string> tokens;
    IuStringUtils::Split(ver,".", tokens, 3);
    int majorVersion = std::stoi(tokens[0]);
    int minorVersion = std::stoi(tokens[1]+tokens[2]);
    int build = versionInfo->Build;

    for(size_t i=0; i<childs.size(); i++)
    {
        SimpleXmlNode &cur = childs[i];
        CUploadEngineData UE;
        UE.NumOfTries = 0;
        UE.NeedAuthorization = cur.AttributeInt("Authorize");
        std::string needPassword = cur.Attribute("NeedPassword");
        UE.NeedPassword = needPassword.empty() ? true : (IuCoreUtils::stringToInt64(needPassword)!=0);
        UE.LoginLabel = cur.Attribute("LoginLabel");
        UE.PasswordLabel = cur.Attribute("PasswordLabel");
        std::string RetryLimit = cur.Attribute("RetryLimit");
        if(RetryLimit.empty())
        {
            // load from settings ;;;
            UE.RetryLimit = m_EngineNumOfRetries;//Settings.FileRetryLimit;
        }
            else
                UE.RetryLimit = atoi(RetryLimit.c_str());

            UE.Name =  cur.Attribute("Name");

            std::string serverMinVersion = cur.Attribute("MinVersion");
            if ( !serverMinVersion.empty() ) {
                std::vector<std::string> tokens;
                IuStringUtils::Split(serverMinVersion,".", tokens, 4);
                if ( tokens.size() >= 3 ) {
                    int serverMajorVersion = (int)IuCoreUtils::stringToInt64(tokens[0]);
                    int serverMinorVersion = (int)IuCoreUtils::stringToInt64(tokens[1]+tokens[2]);
                    int serverBuild = static_cast<int>(tokens.size() > 3 ? IuCoreUtils::stringToInt64(tokens[3]) : 0);
                    if ( !( majorVersion > serverMajorVersion || ( majorVersion == serverMajorVersion && minorVersion > serverMinorVersion) 
                        || ( majorVersion == serverMajorVersion && minorVersion ==  serverMinorVersion && ( !serverBuild || build >= serverBuild ))
                        ) ) {
                            continue;
                    }
                }
            }

            UE.SupportsFolders = cur.AttributeBool("SupportsFolders");
            UE.RegistrationUrl = cur.Attribute("RegistrationUrl");
            UE.UserAgent = cur.Attribute("UserAgent");
            UE.PluginName = cur.Attribute("Plugin");
            UE.Engine = cur.Attribute("Engine");
            std::string MaxThreadsStr = cur.Attribute("MaxThreads");
            UE.MaxThreads = atoi(MaxThreadsStr.c_str());
            
            if ( UE.PluginName == "ftp" ) {
                if ( serversSettings[UE.Name].size() ) {
                    std::string hostname = serversSettings[UE.Name].begin()->second.getParam("hostname");
                    if ( hostname.empty() || hostname == "ftp.example.com" ) {
                        //LOG(WARNING) << "Skipping server  "<< UE.Name;
                        continue;
                    }
                } else {
                    continue;
                }
                if (MaxThreadsStr.empty())
                {
                    UE.MaxThreads = 1;
                }
            }
            if (UE.PluginName == "directory" && MaxThreadsStr.empty() ) {
                UE.MaxThreads = 1;
            }
        
            UE.UsingPlugin = !UE.PluginName.empty();
            UE.Debug =   cur.AttributeBool("Debug");
            if (UE.Debug)
            {
                UE.MaxThreads = 1;
            }
            bool fileHost =  cur.AttributeBool("FileHost");
            UE.MaxFileSize =   cur.AttributeInt("MaxFileSize");

            std::string typeString =  cur.Attribute("Type");

            UE.TypeMask = 0;
            
            std::vector<std::string> types;
            
            std::string typesListString = cur.Attribute("Types");
            if (!typesListString.empty())
            {
                IuStringUtils::Split(typesListString, " ", types, 10);
            }
            if (!typeString.empty()) {
                types.push_back(typeString);
            }
            if (types.empty())
            {
                types.push_back(fileHost ? "file" : "image");
            }
            for (auto& it : types)
            {
                if (it == "image")
                {
                    UE.TypeMask |= CUploadEngineData::TypeImageServer;
                }
                else if (it == "file")
                {
                    UE.TypeMask |= CUploadEngineData::TypeFileServer;
                }
                else if (it == "text")
                {
                    UE.TypeMask |= CUploadEngineData::TypeTextServer;
                }
                else if (it == "urlshortening")
                {
                    UE.TypeMask |= CUploadEngineData::TypeUrlShorteningServer;
                }
            }

//            UE.ImageHost = (UE.TypeMask & CUploadEngineData::TypeImageServer);

            std::vector<SimpleXmlNode> actions;
            cur["Actions"].GetChilds("Action", actions);

            for(size_t j=0; j<actions.size(); j++)
            {
                SimpleXmlNode &actionNode = actions[j];
                UploadAction UA;
                UA.NumOfTries = 0;
                UA.Index = j;

                std::string RetryLimit = actionNode.Attribute("RetryLimit");
                if(RetryLimit.empty())
                {
                    UA.RetryLimit = m_ActionNumOfRetries;//Settings.ActionRetryLimit;
                }
                else UA.RetryLimit = atoi(RetryLimit.c_str());
                 
                UA.IgnoreErrors = actionNode.AttributeBool("IgnoreErrors");
                UA.Description= actionNode.Attribute("Description");

                UA.Type = actionNode.Attribute("Type");
                UA.Url = actionNode.Attribute("Url");
                UA.Referer = actionNode.Attribute("Referer");

                UA.PostParams = actionNode.Attribute("PostParams");
                UA.CustomHeaders = actionNode.Attribute("CustomHeaders");
                UA.OnlyOnce = actionNode.AttributeBool("OnlyOnce");

                ActionRegExp regexp;
                regexp.Pattern = actionNode.Attribute("RegExp");
                regexp.AssignVars = actionNode.Attribute("AssignVars");
                regexp.Required = true;
                UA.Regexes.push_back(regexp);

                std::vector<SimpleXmlNode> regexpNodes;
                actionNode.GetChilds("RegExp", regexpNodes);

                for (auto& regexpNode : regexpNodes)
                {
                    ActionRegExp newRegexp;
                    newRegexp.Pattern = regexpNode.Attribute("Pattern");
                    newRegexp.AssignVars = regexpNode.Attribute("AssignVars");
                    newRegexp.Required = regexpNode.AttributeBool("Required");
                    newRegexp.Data = regexpNode.Attribute("Data");
                    UA.Regexes.push_back(newRegexp);
                }

                for (auto& reg : UA.Regexes)
                {
                    std::vector<std::string> Vars;
                    nm_splitString(reg.AssignVars, ";", Vars);

                    for (std::vector<std::string>::iterator it = Vars.begin(); it != Vars.end(); ++it)
                    {
                        std::vector<std::string> NameAndValue;
                        //    std::cout<<"*************"<<*it<<std::endl;
                        nm_splitString(*it, ":", NameAndValue);
                        if (NameAndValue.size() == 2)
                        {
                            ActionVariable AV;
                            AV.Name = NameAndValue[0];
                            AV.nIndex = atoi(NameAndValue[1].c_str());
                            reg.Variables.push_back(AV);
                        }
                    }
                }
                
                UE.Actions.push_back(UA);
            }

            SimpleXmlNode resultNode = cur["Result"];
            {

                UE.DownloadUrlTemplate = resultNode.Attribute("DownloadUrlTemplate");
                if (UE.DownloadUrlTemplate.empty())
                {
                    UE.DownloadUrlTemplate = resultNode.Attribute("DownloadUrl");
                }
                UE.ImageUrlTemplate = resultNode.Attribute("ImageUrlTemplate");
                if (UE.ImageUrlTemplate.empty())
                {
                    UE.ImageUrlTemplate = resultNode.Attribute("ImageUrl");
                }
                UE.ThumbUrlTemplate = resultNode.Attribute("ThumbUrlTemplate");
                if (UE.ThumbUrlTemplate.empty())
                {
                    UE.ThumbUrlTemplate = resultNode.Attribute("ThumbUrl");
                }
                UE.EditUrlTemplate = resultNode.Attribute("EditUrl");
                UE.DeleteUrlTemplate = resultNode.Attribute("DeleteUrl");
                std::string directUrlTemplate = resultNode.Attribute("DirectUrlTemplate"); 
                if (directUrlTemplate.empty())
                {
                    directUrlTemplate = resultNode.Attribute("DirectUrl");
                }
                if ( !directUrlTemplate.empty() ) {

                    UE.ImageUrlTemplate = directUrlTemplate;
                }

            }
            UE.SupportThumbnails = !UE.ThumbUrlTemplate.empty();
            m_list.push_back(UE);
        }

    std::sort(m_list.begin(), m_list.end(), compareEngines );
    return true;
}

bool CUploadEngineList::compareEngines(const CUploadEngineData& elem1, const CUploadEngineData& elem2)
{
    return IuStringUtils::stricmp(elem1.Name.c_str(), elem2.Name.c_str()) < 0;
}

void CUploadEngineList::setNumOfRetries(int Engine, int Action)
{
    m_EngineNumOfRetries = Engine;
    m_ActionNumOfRetries = Action;
}

bool CUploadEngineList::addServer(const CUploadEngineData& data)
{
    m_list.push_back(data);
    std::sort(m_list.begin(), m_list.end(), compareEngines );
    return true;
}
