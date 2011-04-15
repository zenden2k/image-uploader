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

#ifndef IU_FUNC_HISTORY_MANAGER_H
#define IU_FUNC_HISTORY_MANAGER_H

#pragma once

#include <string>
#include "../Core/Utils/CoreTypes.h"
#include <vector>

struct HistoryItem
{
	std::string localFilePath;
	std::string directUrl;
	std::string viewUrl;
	std::string thumbUrl;
	std::string serverName;
	time_t timeStamp;
	zint64 uploadFileSize;
};

class ZSimpleXmlNode;
class CHistoryReader_impl;

class CHistorySession
{
	public:
		CHistorySession(const std::string& filename, const std::string& sessionId);
		bool AddItem(const HistoryItem &ht);
		
		int entriesCount() const;
		HistoryItem entry(const int index) const;
		std::string serverName() const;
		time_t timeStamp() const;
		void loadFromXml(ZSimpleXmlNode& sessionNode);
	private:
		std::string m_historyXmlFileName;
		std::string m_sessId;
		time_t m_timeStamp;
		std::string m_serverName;
		std::vector<HistoryItem> m_entries;
};

class CHistoryManager 
{
	public:
		CHistoryManager();
		virtual ~CHistoryManager();
		void setHistoryFileName(const std::string& filepath, const std::string& nameprefix);
		CHistorySession newSession();
		std::string makeFileName() const;

	private:
		std::string m_historyFilePath;
		std::string m_historyFileNamePrefix;	
};


class CHistoryReader
{
	public:
		CHistoryReader();
		virtual ~CHistoryReader();
		bool loadFromFile(const std::string& filename);
		int getSessionCount() const;
		CHistorySession* getSession(const int index);
	private:
		CHistoryReader(const CHistoryReader&);
		CHistoryReader_impl *_impl;
};
#endif