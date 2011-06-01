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

#include "HistoryManager.h"

#include "Core/Utils/SimpleXml.h"
#include "Core/Utils/CoreUtils.h"
#include "Core/Utils/GlobalMutex.h"
#include <time.h>
#include <sstream>

class CHistoryReader_impl
{
	public:
		ZSimpleXml m_xml;
		std::vector<CHistorySession> m_sessions;
};

CHistoryManager::CHistoryManager()
{
	
} 

CHistoryManager::~CHistoryManager()
{
	
}

void CHistoryManager::setHistoryFileName(const std::string& filepath, const std::string& nameprefix)
{
	m_historyFilePath = filepath;
	IuCoreUtils::createDirectory(m_historyFilePath);
	m_historyFileNamePrefix = nameprefix;
}

CHistorySession CHistoryManager::newSession()
{
	time_t t = time(0);
	tm * timeinfo = localtime ( &t );
	std::string fileName = m_historyFilePath + m_historyFileNamePrefix +"_" + IuCoreUtils::toString(1900+timeinfo->tm_year)+"_" + IuCoreUtils::toString(timeinfo->tm_mon+1) + ".xml";
	std::string str = IuCoreUtils::toString(rand()%(256 * 256)) + IuCoreUtils::toString(int(t));
	std::string id = IuCoreUtils::CalcMD5Hash(str + IuCoreUtils::toString(rand()%(256))).substr(0, 16);
	return CHistorySession(fileName, id);
}

CHistorySession::CHistorySession(const std::string& filename, const std::string&  sessionId)
{
	m_historyXmlFileName = filename;
	m_sessId = sessionId;
}

void CHistorySession::loadFromXml(ZSimpleXmlNode& sessionNode)
{
	m_timeStamp = sessionNode.AttributeInt("TimeStamp" );
	m_serverName = sessionNode.Attribute("ServerName" );
	std::vector<ZSimpleXmlNode> allEntries;
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
		ht.uploadFileSize = allEntries[i].AttributeInt64("UploadFileSize");
		m_entries.push_back(ht);
	}
}

int CHistorySession::entriesCount() const
{
	return m_entries.size();
}

HistoryItem CHistorySession::entry(const int index) const
{
	return m_entries[index];
}

bool CHistorySession::AddItem(const HistoryItem &ht)
{
	IuCoreUtils::ZGlobalMutex mutex("IuHistoryFileSessionMutex");
	ZSimpleXml xml;
	
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
	ctime_s(dateStr, sizeof(dateStr), &t);
	dateStr[strlen(dateStr)-1] = 0;

	ZSimpleXmlNode root = xml.getRoot("History");
	std::vector<ZSimpleXmlNode> allSessions;
	root.GetChilds("Session", allSessions);
	ZSimpleXmlNode session;
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

	ZSimpleXmlNode entry = session.CreateChild("Entry");
	entry.SetAttribute("TimeStamp", int(t));
	entry.SetAttribute("LocalFilePath", ht.localFilePath);
	entry.SetAttribute("ServerName", ht.serverName);
	if (!ht.directUrl.empty())
		entry.SetAttribute("DirectUrl", ht.directUrl);
	if(! ht.thumbUrl.empty())
		entry.SetAttribute("ThumbUrl", ht.thumbUrl);
	if(! ht.viewUrl.empty())
		entry.SetAttribute("ViewUrl", ht.viewUrl);
	entry.SetAttribute("UploadFileSize", ht.uploadFileSize);

	Utf8String dir = IuCoreUtils::ExtractFilePath(fileName);
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

std::string CHistoryManager::makeFileName() const
{
	time_t t = time(0);
	tm * timeinfo = localtime(&t);
	std::string fileName = m_historyFilePath + m_historyFileNamePrefix +"_" + IuCoreUtils::toString(1900+timeinfo->tm_year)+"_" + IuCoreUtils::toString(timeinfo->tm_mon+1) + ".xml";
	return fileName;
}


// class CHistoryReader
//
CHistoryReader::CHistoryReader()
{
	_impl = new CHistoryReader_impl();
}

int CHistoryReader::getSessionCount() const
{
	return _impl->m_sessions.size();
}

// The pointer returned by this function is only valid
//  during lifetime of CHistoryReader object
CHistorySession* CHistoryReader::getSession(const int index)
{
	return &_impl->m_sessions[index];
}

// filename must be utf-8 encoded
bool CHistoryReader::loadFromFile(const std::string& filename)
{
	if(IuCoreUtils::FileExists(filename))
	{
		_impl->m_xml.LoadFromFile(filename);
	}
	else 
		return false;
	ZSimpleXmlNode root = _impl->m_xml.getRoot("History");
	std::vector<ZSimpleXmlNode> allSessions;
	root.GetChilds("Session", allSessions);

	for(size_t i = 0; i<allSessions.size(); i++)
	{
		std::string fName = filename;
		CHistorySession session(fName, "0");
		session.loadFromXml(allSessions[i]);
		_impl->m_sessions.push_back(session);
	}
	return true;
}

CHistoryReader::~CHistoryReader()
{
	delete _impl;
}