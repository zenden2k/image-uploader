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

#include "HistoryManager.h"

#include <time.h>
#include <algorithm>
#include <sstream>

#include <boost/filesystem.hpp>
#include "Core/Utils/SimpleXml.h"
#include "Core/Utils/CoreUtils.h"
#include "Core/Utils/CryptoUtils.h"
#include "Utils/GlobalMutex.h"
#include "Core/3rdpart/pcreplusplus.h"

class CHistoryReader_impl
{
    public:
        SimpleXml m_xml;
        std::vector<CHistorySession> m_sessions;
};

CHistoryManager::CHistoryManager()
{
    m_historyFileNamePrefix = "history";
}

CHistoryManager::~CHistoryManager()
{
    
}

void CHistoryManager::setHistoryDirectory(const std::string& directory) {
    m_historyFilePath = directory;
    IuCoreUtils::createDirectory(m_historyFilePath);
}

void CHistoryManager::setHistoryFileName(const std::string& filepath, const std::string& nameprefix)
{
    m_historyFilePath = filepath;
    IuCoreUtils::createDirectory(m_historyFilePath);
    m_historyFileNamePrefix = nameprefix;
}

std::shared_ptr<CHistorySession> CHistoryManager::newSession()
{
    time_t t = time(0);
    tm * timeinfo = localtime ( &t );
    std::string fileName = m_historyFilePath + m_historyFileNamePrefix +"_" + IuCoreUtils::toString(1900+timeinfo->tm_year)+"_" + IuCoreUtils::toString(timeinfo->tm_mon+1) + ".xml";
    std::string str = IuCoreUtils::toString(rand()%(256 * 256)) + IuCoreUtils::toString(int(t));
    std::string id = IuCoreUtils::CryptoUtils::CalcMD5HashFromString(str + IuCoreUtils::toString(rand()%(256))).substr(0, 16);
    return std::make_shared<CHistorySession>(fileName, id);
}

CHistorySession::CHistorySession(const std::string& filename, const std::string&  sessionId)
{
    m_historyXmlFileName = filename;
    m_sessId = sessionId;
    m_timeStamp = 0;
}

void CHistorySession::loadFromXml(SimpleXmlNode& sessionNode)
{
    m_timeStamp = sessionNode.AttributeInt("TimeStamp" );
    m_serverName = sessionNode.Attribute("ServerName" );
    std::vector<SimpleXmlNode> allEntries;
    sessionNode.GetChilds("Entry", allEntries);

    for(size_t i=0; i<allEntries.size(); i++)
    {
        HistoryItem ht;
        ht.localFilePath = allEntries[i].Attribute("LocalFilePath");
        ht.serverName = allEntries[i].Attribute("ServerName");
        ht.timeStamp = allEntries[i].AttributeInt("TimeStamp" );
        ht.directUrl = allEntries[i].Attribute("DirectUrl" );
        ht.thumbUrl = allEntries[i].Attribute("ThumbUrl");
        ht.viewUrl = allEntries[i].Attribute("ViewUrl");
        ht.editUrl = allEntries[i].Attribute("EditUrl");
        ht.deleteUrl = allEntries[i].Attribute("DeleteUrl");
        ht.displayName = allEntries[i].Attribute("DisplayName");
        ht.uploadFileSize = allEntries[i].AttributeInt64("UploadFileSize");
        std::string sortIndex = allEntries[i].Attribute("Index");
        ht.sortIndex = sortIndex.empty() ? i : std::stoi(sortIndex);
        // Fix invalid file size
        if ( ht.uploadFileSize > 1000000000000 || ht.uploadFileSize < 0 ){
             ht.uploadFileSize  = 0;
        }
        m_entries.push_back(ht);
    }

    std::sort(m_entries.begin(), m_entries.end(), [](const HistoryItem& lhs, const HistoryItem& rhs) {
        return lhs.sortIndex < rhs.sortIndex;
    });
}

int CHistorySession::entriesCount() const
{
    return m_entries.size();
}

HistoryItem CHistorySession::entry(const int index) const
{
    return m_entries[index];
}

bool CHistorySession::addItem(const HistoryItem &ht)
{
    IuCoreUtils::ZGlobalMutex mutex("IuHistoryFileSessionMutex");
    SimpleXml xml;
    
    std::string fileName = m_historyXmlFileName;
    
    if(IuCoreUtils::FileExists(fileName))
    {
        xml.LoadFromFile(fileName);
    }

    time_t t;
    time(&t);
   /* Converting time_t variable to string */
    char dateStr[50];
    memset(dateStr, 0, sizeof(dateStr));
#ifdef _MSC_VER
    ctime_s(dateStr, sizeof(dateStr), &t);
#else
    char *str = ctime(&t);
    strcpy(dateStr, str);
#endif
    //dateStr[strlen(dateStr)-1] = 0;

    SimpleXmlNode root = xml.getRoot("History");
    std::vector<SimpleXmlNode> allSessions;
    root.GetChilds("Session", allSessions);
    SimpleXmlNode session;
    int xmlSessionNodeIndex = -1;

    for(size_t i=0; i<allSessions.size(); i++)
    {
        if( allSessions[i].Attribute("ID") == m_sessId)
        {
            xmlSessionNodeIndex = i;
            session = allSessions[i];
            break;
        }
    }

    if(xmlSessionNodeIndex == -1)
    {
        session = root.CreateChild("Session");
        session.SetAttribute("Date", dateStr);
        session.SetAttribute("TimeStamp", int(t));
        session.SetAttribute("ID", m_sessId);
        session.SetAttribute("ServerName", ht.serverName);
    }

    SimpleXmlNode entry = session.CreateChild("Entry");
    entry.SetAttribute("TimeStamp", int(t));
    entry.SetAttribute("LocalFilePath", ht.localFilePath);
    entry.SetAttribute("ServerName", ht.serverName);
    if (!ht.directUrlShortened.empty())
        entry.SetAttribute("DirectUrlShortened", ht.directUrlShortened);
    if (!ht.directUrl.empty())
        entry.SetAttribute("DirectUrl", ht.directUrl);

    if(! ht.thumbUrl.empty())
        entry.SetAttribute("ThumbUrl", ht.thumbUrl);
    if(! ht.viewUrl.empty())
        entry.SetAttribute("ViewUrl", ht.viewUrl);
    if (!ht.viewUrlShortened.empty())
        entry.SetAttribute("ViewUrlShortened", ht.viewUrlShortened);
    entry.SetAttribute("UploadFileSize", ht.uploadFileSize);

    if (!ht.editUrl.empty())
        entry.SetAttribute("EditUrl", ht.editUrl);

    if (!ht.deleteUrl.empty())
        entry.SetAttribute("DeleteUrl", ht.deleteUrl);
    if (!ht.displayName.empty())
        entry.SetAttribute("DisplayName", ht.displayName);
    entry.SetAttributeInt("Index", ht.sortIndex);

    std::string dir = IuCoreUtils::ExtractFilePath(fileName);
    if(!IuCoreUtils::DirectoryExists(dir))
    {
        IuCoreUtils::createDirectory(dir);
    }
    xml.SaveToFile(fileName);
    return true;
}

std::string CHistorySession::serverName() const
{
    return m_serverName;
}

time_t CHistorySession::timeStamp() const
{
    return m_timeStamp;
}

/*std::string CHistoryManager::makeFileName() const
{
    time_t t = time(0);
    tm * timeinfo = localtime(&t);
    std::string fileName = m_historyFilePath + m_historyFileNamePrefix +"_" + IuCoreUtils::toString(1900+timeinfo->tm_year)+"_" + IuCoreUtils::toString(timeinfo->tm_mon+1) + ".xml";
    return fileName;
}*/

bool CHistoryManager::clearHistory(HistoryClearPeriod period) {
    IuCoreUtils::ZGlobalMutex mutex("IuHistoryFileSessionMutex");
    std::string historyFolder = m_historyFilePath;
    boost::filesystem::directory_iterator end_itr; // Default ctor yields past-the-end

    pcrepp::Pcre regexp("^history_(\\d+)_(\\d+)\\.xml$");
    time_t t = time(nullptr);
    tm * timeinfo = localtime(&t);
    int currentMonth = timeinfo->tm_mon + 1;
    int currentYear = 1900 + timeinfo->tm_year;

    for (boost::filesystem::directory_iterator i(historyFolder); i != end_itr; ++i) {
        // Skip if not a file
        if (!boost::filesystem::is_regular_file(i->status())) {
            continue;
        }

        // Skip if no match
        if (!regexp.search(i->path().filename().string())) {
            continue;
        }
        int year = atoi(regexp[1].c_str());
        int month = atoi(regexp[2].c_str());
        if (period == HistoryClearPeriod::ClearAll || (period == HistoryClearPeriod::CurrentMonth && month == currentMonth && year == currentYear)) {
            std::string fileName = i->path().string();
            bool res = IuCoreUtils::RemoveFile(fileName);
            if (!res) {
                LOG(ERROR) << "Unable to delete file " << fileName;
            }
        }
    }
    return true;
}

// class CHistoryReader
//
CHistoryReader::CHistoryReader()
{
    d_ptr = new CHistoryReader_impl();
}

int CHistoryReader::getSessionCount() const
{
    return d_ptr->m_sessions.size();
}

CHistorySession* CHistoryReader::getSession(size_t index)
{
    return &d_ptr->m_sessions[index];
}

bool CHistoryReader::loadFromFile(const std::string& filename)
{
    if(IuCoreUtils::FileExists(filename))
    {
        if (!d_ptr->m_xml.LoadFromFile(filename)) {
            return false;
        }
    }
    else {
        return false;
    }
    SimpleXmlNode root = d_ptr->m_xml.getRoot("History");
    std::vector<SimpleXmlNode> allSessions;
    root.GetChilds("Session", allSessions);

    for(size_t i = 0; i<allSessions.size(); i++)
    {
        std::string fName = filename;
        CHistorySession session(fName, "0");
        session.loadFromXml(allSessions[i]);
        d_ptr->m_sessions.push_back(session);
    }
    return true;
}

CHistoryReader::~CHistoryReader()
{
    delete d_ptr;
}